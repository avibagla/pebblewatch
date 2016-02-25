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


#include "main_worker.h"

/* META DATA CODES */


static void init() {
}

static void deinit() {

}

int main(void) {
  init();
  worker_event_loop();
  deinit();
}



//
// void tick_summ_datalog_hander(struct tm *tick_time, TimeUnits units_changed){
//   static int8_t cur_min = -1; // this tracks the current min, 0-59 values
//   // this is to protect against summ_datalog() being called more than once a min
//
//   /* >>>>> EXECUTE TOP OF EACH SECOND, XX:XX:XX <<<<< */
//
//   /* >>>>> EXECUTE TOP OF EACH MINUTE, XX:XX:00 <<<<< */
//   if( (tick_time->tm_sec == 0) && (tick_time->tm_min != cur_min)){
//     // reset the minute gate
//     cur_min = tick_time->tm_min;
//     /* perform the epoch analysis */
//     epoch_analysis();
//     /* write the summary to the persist storage */
//     summ_datalog();
//     /* reset the summary metric arrays after each write to storage */
//     reset_summ_metrics();
//   }
//
//   /* >>>>> RESET THE DAILY METRICS AND CHANGE LONG TERM DATA @ 12:01AM <<<<< */
//   if( (tick_time->tm_hour == 0 ) && (tick_time->tm_min < 1 )){
//     reset_daily_metrics();
//   }
//   /* >>>>> EXECUTE, SEND TO SERVER <<<<< */
//   if( summ_since_trans_server > SUMM_BTWN_TRANSMIT_SERVER){
//     persist_write_int(WORKER_START_FORE_APP_REASON_PERSIST_KEY, 2);
//     summ_since_trans_server = 0; // once try to go to server, reset
//   }
// }
//
//
// /* >>>>>> TOP OF MINUTE <<<<< */
// /* attempt to send a message if bluetooth connection is there */
// if(bluetooth_connection_service_peek()  ){
//   // attempt to send all data to phone
//   if(summ_since_trans_server > SUMM_BTWN_TRANSMIT_SERVER){
//     persist_write_int(WORKER_START_FORE_APP_REASON_PERSIST_KEY,
//       WFAWR_PUSH_ALL_DATA_TO_SERVER);
//       summ_since_trans_server = 0; // once try to go to server, reset
//       worker_launch_app();
//   }else if((summ_since_trans_phone > SUMM_BTWN_TRANSMIT_PHONE)
//       && (persist_read_int(I_BLK_PERSIST_KEY) > 0) ){
//     persist_write_int(WORKER_START_FORE_APP_REASON_PERSIST_KEY,
//       WFAWR_PUSH_ALL_DATA_TO_PHONE);
//       summ_since_trans_phone = 0; // once try to go to server, reset
//       worker_launch_app();
//   }
// }
