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
static bool transmit_window_active = false;
static const int16_t NUM_SEC_TRANSMIT_SERVER = 15;

static void countdown_timer_handler(void *data);

static void send_data_item(AppKey app_key);

static uint32_t OUTBOX_SIZE = (sizeof(time_t)*2) + (sizeof(HealthMinuteData)*MAX_ENTRIES);
bool health_events_sent;
AppKey cur_app_key;
int num_entries;

// These variables contain information about the endpoints of upload on
// acti, health_events, and pinteract data blocks.
static int pinteract_count;
static time_t prev_health_events_upload_time; // previous time that the
static time_t attempt_health_events_upload_time; // previous time that the
static time_t prev_acti_upload_time;
static time_t attempt_acti_upload_time;

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//    TRANMISSION WINDOW CODE
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


// NOTE ! We don't want to give them any options to get out of this.
// WE have to assume that this is going to be alright, and that
// the tick handlers will kill the window when it is time.

static void shutdown_comm(){
  // desregister the callbacks LAST. Why?
  app_message_deregister_callbacks();
  if(transmit_window_active){
    window_stack_remove(s_transmit_phone_window,false);
  }
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
    shutdown_comm();
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
  // NOTE : we can't rewrite this as a second tick tock handler because might
  //  conflict with other tick tock handlers we want to use
  countdown_timer_handler(NULL);
}

static void transmit_phone_window_unload(Window *window) {
  // get rid of text layer
  text_layer_destroy(s_transmit_text_layer);
  // IF we get to the transmit count down timer, then we destroy those elements
  // if(countdown_active){ app_timer_cancel(app_timer_push_to_server); }
  app_timer_cancel(app_timer_push_to_server);
  text_layer_destroy(s_transmit_countdown_text_layer);
  // Destroy the banner layer
}


// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//    TRANMISSION CODE
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++



static void send_data_router(){
  // attempt to send the data in order of importance, keep sending the respective
  // data until we don't have anymore of that datatype to send, where the datatype
  // data types are
  // 1. acti -> motion
  // 2. pinteract -> patient interactino
  // 3. push to server flag
  // NOTE : once we have no more data, we close by default
  if( num_entries >= 1){
    send_data_item(AppKeyActiData);
  }else if(!health_events_sent){
    send_data_item(AppKeyHealthEventData);
  }else if(data_to_send_pinteract()){
    send_data_item(AppKeyPinteractData);
  // test if the connection is active to see if we can infact push to the server
  }else if( connection_service_peek_pebble_app_connection() ){
    send_data_item(AppKeyPushToServer);
  }else{
    countdown_active = false;
    // once reach the end of the countdown, remove the window no matter what
    shutdown_comm();
  }
}

static bool health_events_iter_cb(HealthActivity activity, time_t time_start,
  time_t time_end, void *context){

  // treat the first two bytes as a call on how many bytes are in the buffer
  uint8_t *data = (uint8_t*) context;
  uint16_t *n_bytes = (uint16_t*) context;
  // if the number of bytes to be added
  if((*n_bytes +sizeof(HealthEventData) )< OUTBOX_SIZE ){
    int16_t activity_int = -1; // if this is an unknown activity, mark as -1
    if(activity == HealthActivityNone){
      activity_int = 0;
    }else if(activity == HealthActivitySleep){
      activity_int = 1;
    }else if(activity == HealthActivityRestfulSleep){
      activity_int = 2;
    }
    HealthEventData health_event_data = {
      .health_activity = activity_int,
      .time_start = time_start,
      .time_end = time_end
    };

    memcpy(data+ *n_bytes, &health_event_data ,sizeof(HealthEventData) );
    *n_bytes = *n_bytes + sizeof(HealthEventData);
    // we update the update time, gate on how many have been written to buffer,
    // add 1 second here, assuming that iterate forward in time
    attempt_health_events_upload_time = time_start + 1;
    // if we have space in the outbox buffer, we get more health events
    return true;
  }else{
    // if we have no more space in the outbox buffer, then we stop getting events
    return false;
  }
}


static void send_app_message_full(DictionaryIterator *out, AppKey app_key, uint8_t *data, int data_start, int data_size){
  if(app_message_outbox_begin(&out) == APP_MSG_OK){
    dict_write_data(out, app_key, data+data_start, data_size);
    free(data);
    dict_write_end(out);

    if(app_message_outbox_send() == APP_MSG_OK){
      // that if we sent the data, we mark it as the current key
      cur_app_key = app_key;
    }else{
      APP_LOG(APP_LOG_LEVEL_ERROR, "Error sending message");
    }
  }else{
    free(data);
    APP_LOG(APP_LOG_LEVEL_ERROR, "Error beginning message");
  }
}

static void send_data_item(AppKey app_key){
  // initialize the out write
  DictionaryIterator *out = NULL;

  // ++++++++++++++++++ AppKeyActiData ++++++++++++++++++
  if(app_key == AppKeyActiData){
    prev_acti_upload_time = persist_read_int(ACTI_LAST_UPLOAD_TIME_PERSIST_KEY);
    attempt_acti_upload_time = time(NULL); // This is okay because the
    // health_service_get_minute_history rewrites the time to be at the end of
    // the health period
    // malloc the data
    HealthMinuteData* data_hmd = (HealthMinuteData*) malloc(OUTBOX_SIZE);

    // add the period data to the remainder of the block
    // get the data from  prev_acti_upload_time to attempt_acti_upload_time
    //  where attempt_acti_upload_time is prev_acti_upload_time * INTERVAL*60
    // write the data block to the block to be sent over
    APP_LOG(APP_LOG_LEVEL_ERROR,"try times prev %d :: attempt %d",
      (int)prev_acti_upload_time, (int)attempt_acti_upload_time);

    num_entries =  health_service_get_minute_history( data_hmd,
      MAX_ENTRIES, &prev_acti_upload_time, &attempt_acti_upload_time);

    // if the number of retrieved records matched what is expected, send it
    APP_LOG(APP_LOG_LEVEL_ERROR,"num_entries %d :: prev %d :: attempt %d",
      (int) num_entries, (int)prev_acti_upload_time, (int)attempt_acti_upload_time);


    // ONLY TRANSMIT IF BLOCK PROPER SIZE
    if(num_entries >= 1){
      // re-write the data so that we know the structure of the data exactly
      int data_size = (sizeof(time_t)*2) + num_entries * sizeof(TrunHealthMinuteData);
      uint8_t *data = (uint8_t*) malloc( data_size);
      TrunHealthMinuteData* trun_data  = (TrunHealthMinuteData*) (data+(sizeof(time_t)*2));

      for(int i = 0; i < num_entries; i++){
        trun_data[i].steps = data_hmd[i].steps;
        trun_data[i].orientation = data_hmd[i].orientation;
        trun_data[i].vmc = data_hmd[i].vmc;
        trun_data[i].is_invalid = data_hmd[i].is_invalid;
        trun_data[i].light = data_hmd[i].light;
        trun_data[i].steps = data_hmd[i].steps;
      }

      free(data_hmd);
      // NOTE: if prev_acti_upload_time on entry to health_service_get_minute_history.
      // is somewhere in the middle of a minute interval, then the function behaves
      // as if the caller passed in the start of that minute.
      write_time_to_array_head(prev_acti_upload_time, data+0);
      // NOTE: if attempt_acti_upload_time on entry to health_service_get_minute_history.
      // is somewhere in the middle of a minute interval, then the function behaves
      // as if the caller passed in the end of that minute.
      // HENCE, when we record the previous last upload time in order to
      // setup for the next upload, we have to add one second to get the next minute
      write_time_to_array_head(attempt_acti_upload_time, data+sizeof(time_t));

      send_app_message_full(out, AppKeyActiData, data, 0,data_size);

    }else{
      APP_LOG(APP_LOG_LEVEL_ERROR,
        "num_entries not at least one, not sending");
      // if fewer than expected entries, then we must skip down to the next
      // level of data to send, the pinteracts. This shouldn't happen because
      // the gate of data to send on the sent handler, but just in case
      // free the malloced data first
      free(data_hmd);
      send_data_router();
    }

    // add pebble event data here.

  // ++++++++++++++++++ AppKeyHealthEventData ++++++++++++++++++
  }else if(app_key == AppKeyHealthEventData){
    // iterate over the health events and place them into an array the same size as the maximum
    prev_health_events_upload_time = persist_read_int(HEALTH_EVENTS_LAST_UPLOAD_TIME_PERSIST_KEY);
    uint8_t* data = (uint8_t*) malloc(OUTBOX_SIZE );
    // first two bytes are the number of bytes in the buffer
    uint16_t* n_bytes = (uint16_t*) data;
    *n_bytes = 2;
    health_service_activities_iterate(HealthActivityMaskAll,
     prev_health_events_upload_time, time(NULL), HealthIterationDirectionFuture,
     health_events_iter_cb, (void*) data);

    // send data via appmessage
    // NOTE, we DONT skip the first two bytes because we could find a count of
    // the number of bytes that are SUPPOSED to be in the array useful
    send_app_message_full(out, AppKeyHealthEventData, data,0, *n_bytes);

  // ++++++++++++++++++ AppKeyPinteractData ++++++++++++++++++
  }else if(app_key == AppKeyPinteractData){
    // get the address of the next element of the pinteract in pstorage
    // get the size of the next pinteract element in pstorage
    uint8_t* data = (uint8_t*) malloc(OUTBOX_SIZE );
    // first two bytes are the number of bytes in the buffer
    uint16_t* n_bytes = (uint16_t*) data;
    *n_bytes = 2;
    // get the initial key count
    pinteract_count = persist_read_int( PINTERACT_KEY_COUNT_PERSIST_KEY);
    while( true){
      uint32_t pinteract_data_size = get_data_size_of_pinteract_element(pinteract_count);
      // test if
      //  1. adding the pinteract to the data will cause it to overload the buffer
      //  2. if we have any keys left, ie:  pinteract_key == 0 is out
      if( ( (*n_bytes + pinteract_data_size) > OUTBOX_SIZE) || (pinteract_count == 0)){
        break;
      }
      // read the pinteract entry into the buffer
      persist_read_data(pinteract_count, data + *n_bytes, pinteract_data_size);
      *n_bytes = *n_bytes + pinteract_data_size; // update the number of valid bytes in buffer
      pinteract_count--; // decrement the counter
    }
    // send data via appmessage
    // NOTE, we DONT skip the first two bytes because we could find a count of
    // the number of bytes that are SUPPOSED to be in the array useful
    send_app_message_full(out, AppKeyPinteractData, data,0, *n_bytes);

  // ++++++++++++++++++ AppKeyPushToServer ++++++++++++++++++
  }else if(app_key == AppKeyPushToServer){
    int data_size = 4;
    uint8_t *data = (uint8_t*) malloc(data_size);
    data[0] = 0;
    send_app_message_full(out, AppKeyPushToServer, data, 0,data_size);
  }

}

static void outbox_sent_handler(DictionaryIterator *iter, void *context){
  APP_LOG(APP_LOG_LEVEL_ERROR, "OUTBOUND MESSAGE SEND");
  // reset the retry counter once we have a successful transmisstion
  retry_count = 0;
  // modify the data counts based on the message that was just sent
  if(cur_app_key == AppKeyActiData){
    // add one second to move it into the next minute
    persist_write_int(ACTI_LAST_UPLOAD_TIME_PERSIST_KEY ,attempt_acti_upload_time);
  }else if(cur_app_key == AppKeyHealthEventData){
    health_events_sent = true;
    persist_write_int(HEALTH_EVENTS_LAST_UPLOAD_TIME_PERSIST_KEY,attempt_health_events_upload_time);
  }else if(cur_app_key == AppKeyPinteractData){
    // if send the data, then update with the current count of elements in the pstorage
    persist_write_int(PINTERACT_KEY_COUNT_PERSIST_KEY,pinteract_count);
  }else if(cur_app_key == AppKeyPushToServer){
    return;
  }
  // determine what data to attempt to send next
  send_data_router();
}

static void outbox_dropped_handler(DictionaryIterator *iterator, AppMessageResult reason, void *context){
  APP_LOG(APP_LOG_LEVEL_ERROR, "OUTBOUND MESSAGE DROPPED");
  // if we failed to send, we increment the retry count
  retry_count++;

  // attempt based on what type of data was attempting to be sent when failed
  // NOTE : once we have no more data, we close by default
  if((cur_app_key == AppKeyActiData) && (retry_count < MAX_RETRY)){
    send_data_item(AppKeyActiData);
  }else if( (cur_app_key == AppKeyHealthEventData) && (retry_count < MAX_RETRY)){
   send_data_item(AppKeyHealthEventData);
  }else if((cur_app_key == AppKeyPinteractData) && (retry_count < MAX_RETRY) ){
    send_data_item(AppKeyPinteractData);
  // test if the connection is active to see if we can infact push to the server
  }else if( connection_service_peek_pebble_app_connection() && (retry_count < MAX_RETRY)){
    send_data_item(AppKeyPushToServer);
  }else{
    countdown_active = false;
    // once reach the end of the countdown, remove the window no matter what
    shutdown_comm();
  }
}

static void inbox_received_handler(DictionaryIterator *iter, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "INBOUND MESSAGE RECIEVED");
  // see if the message sent is to say the phone is ready
  Tuple *js_ready_t = dict_find(iter, AppKeyJSReady);
  if(js_ready_t) {
    // Check that it has been at least INTERVAL_MINUTES since the last upload
    // if there are acti periods to send, start with those
    // if there are no acti periods to send, send the pinteracts
    // if there are no acti or periods to send, send the server push
    send_data_router();
  }
  // check to see if the message has been sent to the server
  Tuple *js_sent_t = dict_find(iter, AppKeySentToServer);
  if(js_sent_t){
    countdown_active = false;
    // once reach the end of the countdown, remove the window no matter what
    shutdown_comm();
  }
}


// for use when the
void comm_begin_upload_active_window(){


  // if JS app is running, then try to send the data
  if(connection_service_peek_pebble_app_connection()){
    APP_LOG(APP_LOG_LEVEL_ERROR, "num pinteract %d ",(int) persist_read_int( PINTERACT_KEY_COUNT_PERSIST_KEY));
    num_entries = 1; // initialize
    health_events_sent = false; // that haven't sent the health events yet
    // test if BT connection is up at all. If it is NOT, don't even try

    app_message_register_inbox_received(inbox_received_handler);
    app_message_register_outbox_sent(outbox_sent_handler);
    app_message_register_outbox_failed(outbox_dropped_handler);
    // NOTE : incoming data for configuration must match the size of the config
    app_message_open(INCOMING_DATA_SIZE, OUTBOX_SIZE);
    send_data_router();
    transmit_window_active = false;
  }
}



void comm_begin_upload_inactive_window(){
  num_entries = 1; // initialize
  health_events_sent = false; // that haven't sent the health events yet

  app_message_register_inbox_received(inbox_received_handler);
  app_message_register_outbox_sent(outbox_sent_handler);
  app_message_register_outbox_failed(outbox_dropped_handler);
  // NOTE : incoming data for configuration must match the size of the config
  app_message_open(INCOMING_DATA_SIZE, OUTBOX_SIZE);

  s_transmit_phone_window = window_create();
  // app_message_open(app_message_inbox_size_maximum(),app_message_outbox_size_maximum());

  window_set_window_handlers(s_transmit_phone_window, (WindowHandlers) {
    .load = transmit_phone_window_load,
    .unload = transmit_phone_window_unload,
  });

  window_set_click_config_provider(s_transmit_phone_window,
                                   (ClickConfigProvider) click_config_provider);
  window_stack_push(s_transmit_phone_window, false);
  transmit_window_active = true;
}

// if(bluetooth_connection_service_peek()){
//   app_message_register_inbox_received(inbox_received_handler);
//   app_message_register_outbox_sent(outbox_sent_handler);
//   app_message_register_outbox_failed(outbox_dropped_handler);
//   // NOTE : incoming data for configuration must match the size of the config
//   app_message_open(INCOMING_DATA_SIZE, OUTBOX_SIZE);
//
//   // app_message_open(INCOMING_DATA_SIZE,app_message_outbox_size_maximum());
//
//   s_transmit_phone_window = window_create();
//   // app_message_open(app_message_inbox_size_maximum(),app_message_outbox_size_maximum());
//
//   window_set_window_handlers(s_transmit_phone_window, (WindowHandlers) {
//     .load = transmit_phone_window_load,
//     .unload = transmit_phone_window_unload,
//   });
//
//   window_set_click_config_provider(s_transmit_phone_window,
//                                    (ClickConfigProvider) click_config_provider);
//   window_stack_push(s_transmit_phone_window, false);
// }


// if(app_message_outbox_begin(&out) == APP_MSG_OK){
//   // write data to outbox iterator
//   // dict_write_data(out, AppKeyActiData, data, (sizeof(time_t)*2) + (trun_hmd_size*num_entries));
//   dict_write_data(out, AppKeyActiData, data, (sizeof(time_t)*2)
//     + (sizeof(TrunHealthMinuteData)*num_entries));
//   // free malloced data
//   free(data);
//   dict_write_end(out);
//   if(app_message_outbox_send() == APP_MSG_OK){
//     // that if we sent the data, we mark it as the current key
//     cur_app_key = app_key;
//   }else{
//     APP_LOG(APP_LOG_LEVEL_ERROR, "Error sending message");
//   }
// }else{
//   APP_LOG(APP_LOG_LEVEL_ERROR, "Error beginning message");
//   // free malloced data
//   free(data);
// }

// if(app_message_outbox_begin(&out) == APP_MSG_OK){
//   dict_write_data(out, AppKeyPinteractData, data, data_size);
//   free(data);
//   dict_write_end(out);
//
//   if(app_message_outbox_send() == APP_MSG_OK){
//     // that if we sent the data, we mark it as the current key
//     cur_app_key = app_key;
//   }else{
//     APP_LOG(APP_LOG_LEVEL_ERROR, "Error sending message");
//   }
// }else{
//   free(data);
//   APP_LOG(APP_LOG_LEVEL_ERROR, "Error beginning message");
// }


// if(app_message_outbox_begin(&out) == APP_MSG_OK){
//   dict_write_int(out, AppKeyPushToServer,&msg_to_server, 4, true);
//   dict_write_end(out);
//   if(app_message_outbox_send() == APP_MSG_OK){
//     // that if we sent the data, we mark it as the current key
//     cur_app_key = app_key;
//   }else{
//     APP_LOG(APP_LOG_LEVEL_ERROR, "Error sending message");
//   }
// }else{
//   APP_LOG(APP_LOG_LEVEL_ERROR, "Error beginning message");
// }
