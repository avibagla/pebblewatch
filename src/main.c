/* Project Kraepelin, Main file
The MIT License (MIT)

Copyright (c) 2015, Nathaniel T. Stockham

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

This license is taken to apply to any other files in the Project Kraepelin
Pebble App roject.
*/


#include "main.h"



static void init() {
  /* ++++++ CHECK IF FIRST TIME APP INSTALLED +++++ */


  /* ++++++ EVERY TIME THE APP STARTS  +++++ */
  // ALWAYS register the wakeup handler each time the app is opened so that,
  // when the app is closed again, that the wakeup handler will take place.
  wakeup_service_subscribe(wakeup_main_response_handler);

  // CURRENTLY UNUSED AS OF VERSION 0.15
  // !! Register the global tick timer function. ALL elements will use this
  // and this tick timer alone!
  // tick_timer_service_subscribe(SECOND_UNIT, fore_app_master_tick_timer_handler);


  /* ++++++ LAUNCH REASONS ++++++ */
  // if the launch was due to the user, then show main dash
  if(launch_reason() == APP_LAUNCH_USER){
    APP_LOG(APP_LOG_LEVEL_ERROR, "app start: heap size: used %d , free %d",
        heap_bytes_used(), heap_bytes_free());
    // display_main_dash();
  }


  // if the launch was due to the wakeup, then show the mood rating.
  // NOTE, the wakeup handler deals with all the details of the wakeup
  if(launch_reason() ==  APP_LAUNCH_WAKEUP){
    // If the app was opened due to wakeup, then give the wakeup handler the
    // needed information to call open the correct pinteract
    WakeupId wakeup_id;
    int32_t wakeup_cookie;
    wakeup_get_launch_event(&wakeup_id, &wakeup_cookie);
    // APP_LOG(APP_LOG_LEVEL_ERROR, "wakeup_id %d  wakeup_cookie %d", (int) wakeup_id, (int) wakeup_cookie);
    wakeup_main_response_handler(wakeup_id, wakeup_cookie);
  }
}

static void deinit() {
  //   app_message_deregister_callbacks();
  tick_timer_service_unsubscribe();
  APP_LOG(APP_LOG_LEVEL_ERROR, "Main Project K rem heap B: %d",(int) heap_bytes_free());
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
