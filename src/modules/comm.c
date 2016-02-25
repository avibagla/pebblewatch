#include "comm.h"

// ADD A TRANSMIT WINDOW
static int16_t retry_count = 0;

static Window *s_transmit_phone_window;
static Layer *window_layer;

static TextLayer *s_transmit_text_layer;
static TextLayer *s_transmit_countdown_text_layer;

static AppTimer *app_timer_push_to_server;

static uint16_t cur_countdown;
static int16_t MAX_RETRY = 5;
static bool countdown_active = false;
static const int16_t NUM_SEC_TRANSMIT_SERVER = 15;

static void countdown_timer_handler(void *data);

time_t prev_acti_upload_time;
time_t attempt_acti_upload_time;

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//    TRANMISSION WINDOW CODE
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


// NOTE ! We don't want to give them any options to get out of this.
// WE have to assume that this is going to be alright, and that
// the tick handlers will kill the window when it is time.

static void close_transmit_window(){
  window_stack_remove(s_transmit_phone_window,false);
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context){}
static void back_click_handler(ClickRecognizerRef recognizer, void *context){}
static void up_click_handler(ClickRecognizerRef recognizer, void *context){}
static void down_click_handler(ClickRecognizerRef recognizer, void *context){}

static void click_config_provider(void *context){
  // single clicks, keep it simple
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_single_click_subscribe(BUTTON_ID_BACK, back_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
}


static void countdown_timer_handler(void *data){
  // Use a long-lived buffer, so declare static
  static char s_tr_buf[]="100"; // time remaining buffer

  cur_countdown -= 1; // decrement by one
  countdown_active = true;
  if(cur_countdown > 0){
    // write the current countdown to the string buffer
    snprintf(s_tr_buf, sizeof(s_tr_buf), "%d",cur_countdown);
    text_layer_set_text(s_transmit_countdown_text_layer, s_tr_buf);
    app_timer_push_to_server = app_timer_register(1000, countdown_timer_handler,NULL);
  }else{
    countdown_active = false;
    // once reach the end of the countdown, remove the window no matter what
    close_transmit_window();
  }
}

static void transmit_phone_window_load(Window *window) {

  window_layer = window_get_root_layer(window);
  GRect window_bounds = layer_get_bounds(window_layer);

  window_set_background_color(window,GColorBlack);
  // push explanantion text
  s_transmit_text_layer = text_layer_create(GRect(5,10,window_bounds.size.w-10,60));
  text_layer_set_text_alignment(s_transmit_text_layer, GTextAlignmentCenter);
  text_layer_set_font(s_transmit_text_layer,fonts_get_system_font(FONT_KEY_GOTHIC_28));
  // change color of text and background
  text_layer_set_background_color(s_transmit_text_layer, GColorBlack);
  text_layer_set_text_color(s_transmit_text_layer, GColorWhite);
  text_layer_set_text(s_transmit_text_layer, "Transmitting\nPlease wait");
  layer_add_child(window_layer, text_layer_get_layer(s_transmit_text_layer));

  // push the countdown text
  s_transmit_countdown_text_layer = text_layer_create(GRect(50,90,45,45));
  text_layer_set_text_alignment(s_transmit_countdown_text_layer, GTextAlignmentCenter);
  text_layer_set_font(s_transmit_countdown_text_layer,fonts_get_system_font(FONT_KEY_BITHAM_42_LIGHT));
  // change color of text and background
  text_layer_set_background_color(s_transmit_countdown_text_layer, GColorBlack);
  text_layer_set_text_color(s_transmit_countdown_text_layer, GColorWhite);
  layer_add_child(window_layer, text_layer_get_layer(s_transmit_countdown_text_layer));

  // since updating time, subscribe to tick timer service
  cur_countdown = NUM_SEC_TRANSMIT_SERVER;
  countdown_timer_handler(NULL);
}

static void transmit_phone_window_unload(Window *window) {
  retry_count = 0;
  // get rid of text layer
  text_layer_destroy(s_transmit_text_layer);
  // IF we get to the transmit count down timer, then we destroy those elements
  if(countdown_active){
    app_timer_cancel(app_timer_push_to_server);
  }
  text_layer_destroy(s_transmit_countdown_text_layer);
  // Destroy the banner layer

  // desregister the callbacks LAST
  app_message_deregister_callbacks();
}


// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//    TRANMISSION CODE
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void send_data_item(AppKey app_key){
  DictionaryIterator *out;


  // if app message begin is OK
  if(app_message_outbox_begin(&out) == APP_MSG_OK){
    // if AppKeyActiData
    if(app_key == AppKeyActiData){
      attempt_acti_upload_time = prev_acti_upload_time + (MAX_ENTRIES*60);
      // malloc the data
      int data_size = (sizeof(time_t)*2) + (sizeof(HealthMinuteData)*MAX_ENTRIES);
      uint8_t *data = (uint8_t*) malloc(data_size );

      // add the prev upload and new attempted upload time to the head.
      write_time_to_array_head(prev_acti_upload_time, data+0);
      write_time_to_array_head(attempt_acti_upload_time, data+4);

      // add the period data to the remainder of the block
      // get the data from  prev_acti_upload_time to attempt_acti_upload_time
      //  where attempt_acti_upload_time is prev_acti_upload_time * INTERVAL*60
      // write the data block to the block to be sent over
      int num_entries =  health_service_get_minute_history(
        (HealthMinuteData*) (data+(sizeof(time_t)*2)),
        MAX_ENTRIES, &prev_acti_upload_time, &attempt_acti_upload_time);

      // write data to outbox iterator
      dict_write_data(out, AppKeyActiData, data, data_size);
      dict_write_end(out);
      // if the number of retrieved records matched what is expected, send it
      if(num_entries != MAX_ENTRIES){
        APP_LOG(APP_LOG_LEVEL_ERROR,
          "num_entries returned does not match MAX_ENTRIES, not sending");
      }else{
        if(app_message_outbox_send() != APP_MSG_OK){
          APP_LOG(APP_LOG_LEVEL_ERROR, "Error sending message");
        }
      }
      // free the malloced data
      free(data);
      // AppKeyPinteractData
    }else if(app_key == AppKeyPinteractData){
      // get the address of the next element of the pinteract in pstorage

      // get the size of the next pinteract element in pstorage
      int pstorage_key = get_next_pinteract_element_key();
      int data_size = get_size_of_pinteract_element_data(pstorage_key);
      uint8_t *data = (uint8_t*) malloc(data_size);

      dict_write_data(out, AppKeyPinteractData, data, data_size);
      dict_write_end(out);
      if(app_message_outbox_send() != APP_MSG_OK){
        APP_LOG(APP_LOG_LEVEL_ERROR, "Error sending message");
      }
      free(data);
    // if AppKeyPushToServer
    }else if(app_key == AppKeyPushToServer){
      int msg_to_server = 0;
      dict_write_int(out, AppKeyPushToServer,&msg_to_server, 4, true);
      dict_write_end(out);
      if(app_message_outbox_send() != APP_MSG_OK){
        APP_LOG(APP_LOG_LEVEL_ERROR, "Error sending message");
      }
    }
  }else{
    APP_LOG(APP_LOG_LEVEL_ERROR, "Error beginning message");
  }
}

static void outbox_sent_handler(DictionaryIterator *iter, void *context){
  // attempt to send the data in order of importance
  if(data_to_send_acti() && (retry_count < MAX_RETRY) ){
    send_data_item(AppKeyActiData);
  }else if(data_to_send_pinteract() && (retry_count < MAX_RETRY )){
    send_data_item(AppKeyPinteractData);
  // test if the connection is active to see if we can infact push to the server
  }else if( connection_service_peek_pebble_app_connection() && (retry_count < MAX_RETRY) ){
    send_data_item(AppKeyPushToServer);
  }else{
    countdown_active = false;
    // once reach the end of the countdown, remove the window no matter what
    close_transmit_window();
  }
}

static void outbox_dropped_handler(DictionaryIterator *iterator, AppMessageResult reason, void *context){
  // if the phone is active, and
  retry_count++;
  APP_LOG(APP_LOG_LEVEL_ERROR, "OUTBOUND MESSAGE DROPPED");
  // close_transmit_window();
}

static void inbox_received_handler(DictionaryIterator *iter, void *context) {

  // see if the message sent is to say the phone is ready
  Tuple *js_ready_t = dict_find(iter, AppKeyJSReady);
  if(js_ready_t) {
    // Check that it has been at least INTERVAL_MINUTES since the last upload
    // if there are acti periods to send, start with those
    // if there are no acti periods to send, send the pinteracts
    // if there are no acti or periods to send, send the server push
    if(data_to_send_acti()){
      send_data_item(AppKeyActiData);
    }else if(data_to_send_pinteract()){
      send_data_item(AppKeyPinteractData);
    }else{
      send_data_item(AppKeyPushToServer);
    }
  }
  // check to see if the message has been sent to the server
  Tuple *js_sent_t = dict_find(iter, AppKeySentToServer);
  if(js_sent_t){
    countdown_active = false;
    // once reach the end of the countdown, remove the window no matter what
    close_transmit_window();
  }
}

void comm_begin_upload(){
  // test if BT connection is up at all. If it is NOT, don't even try
  if(bluetooth_connection_service_peek()){
    app_message_register_inbox_received(inbox_received_handler);
    app_message_register_outbox_sent(outbox_sent_handler);
    app_message_register_outbox_failed(outbox_dropped_handler);
    app_message_open(100, 1000);

    s_transmit_phone_window = window_create();
    // app_message_open(app_message_inbox_size_maximum(),app_message_outbox_size_maximum());

    window_set_window_handlers(s_transmit_phone_window, (WindowHandlers) {
      .load = transmit_phone_window_load,
      .unload = transmit_phone_window_unload,
    });

    window_set_click_config_provider(s_transmit_phone_window,
                                     (ClickConfigProvider) click_config_provider);
    window_stack_push(s_transmit_phone_window, false);
  }
}
