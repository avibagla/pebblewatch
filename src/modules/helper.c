#include "helper.h"

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
/* ++++++++++++++++ STORAGE FUNCTIONS +++++++++++++++ */
/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */

void init_persistent_storage(){
  // struct config_general cg = get_config_general();

}

void reset_pinteract_persistent_storage(){
  // if(persist_exists(PIRPS_B1_PERSIST_KEY)){
  //   persist_delete(PIRPS_B1_PERSIST_KEY);
  // }
  //
  // uint8_t ps_buf[MAX_PINTERACT_PS_B_SIZE];
  // for(int16_t i = 0; i < MAX_PINTERACT_PS_B_SIZE; i++){ ps_buf[i] = 0; }
  //
  // write_time_to_array_head(time(NULL),ps_buf);
  // write_res_buf_byte_count(ps_buf, PINTERACT_PS_B_COUNT_IND,
  //                          PINTERACT_PS_HEAD_B_SIZE);
  // persist_write_data(PIRPS_B1_PERSIST_KEY, ps_buf,sizeof(ps_buf));
  // // logging
  // // uint8_t *tmp = ps_buf;
  // // APP_LOG(APP_LOG_LEVEL_ERROR, "reset pinteract ps: %u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u",
  // //         tmp[0],tmp[1],tmp[2],tmp[3],tmp[4],tmp[5],tmp[6],tmp[7],tmp[8],
  // //         tmp[9],tmp[10],tmp[11],tmp[12],tmp[13],tmp[14],tmp[15],tmp[16],tmp[17]);
}




void reset_config_wakeup_persistent_storage(){
  if(persist_exists(CONFIG_WAKEUP_IDS_PERSIST_KEY)){
    persist_delete(CONFIG_WAKEUP_IDS_PERSIST_KEY);
  }

  // write the current day of the month
  time_t cur_time = time(NULL);
  // struct tm *cur_time_tm =  localtime(&cur_time);
  // persist_write_int(CUR_MDAY_PERSIST_KEY,cur_time_tm->tm_mday);

  WakeupConfig cs_ary[2];

  // SUPER ROUGH, but good enough for user testing
  cs_ary[0].pinteract_code = 14;
  cs_ary[0].srt = 540;
  cs_ary[0].end = 600;
  cs_ary[1].pinteract_code = 11;
  cs_ary[1].srt = 1080;
  cs_ary[1].end = 1140;

  reset_config_wakeup_schedule();
  write_to_config_wakeup_persistant_storage(cs_ary, 2);
  // call config_wakeup_schedule
  config_wakeup_schedule();

  // DEBUGGING
  // WakeupId wakeup_id = read_wakeup_id_at_config_wakeup_index(3);
  // APP_LOG(APP_LOG_LEVEL_ERROR, "config wakeup i=0, wakeup_id %d ", (int) wakeup_id);
}


/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
/* +++++++++++++++ FORE APP MASTER TICK FUNCTIONS +++++++++++++++ */
/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */

/* +++++++ GLOBAL VARIABLES +++++*/
/* FORE APP MASTER TICK VARIABLES */
static TickHandler tick_timer_clock_minute_handler;
static TickHandler tick_timer_clock_second_handler;
static TickHandler tick_timer_aux_minute_handler;
static TickHandler tick_timer_aux_second_handler;


void worker_start_fore_app_reason_exec(){

}




void fore_app_master_tick_timer_handler(struct tm *tick_time, TimeUnits units_changed){
  // guards to prevent functions from being called more than once in a given
  // second/minute
  // static int8_t cur_sec = -1;
  // static int8_t cur_min = -1;
  //
  // // execute each second, if it is a new second
  // if( tick_time->tm_sec != cur_sec){
  //   cur_sec = tick_time->tm_sec; // update the current second
  //   // execute the assigned function, IF it is not null
  //   if(tick_timer_clock_second_handler != NULL){
  //     tick_timer_clock_second_handler(tick_time, units_changed);
  //   }
  //   if(tick_timer_aux_second_handler != NULL){
  //     tick_timer_aux_second_handler(tick_time, units_changed);
  //   }
  //   // execute based on what has changed in the app state
  //   app_state_change_trigger_second();
  // }
  //
  //
  // // execute at the top of the minute, if it is a new minute
  // if((tick_time->tm_sec == 0) && (tick_time->tm_min != cur_min )){
  //   cur_min =  tick_time->tm_min; //update the current minute
  //   // execute the assigned function, IF it is not null
  //   if(tick_timer_clock_minute_handler != NULL){
  //     tick_timer_clock_minute_handler(tick_time, units_changed);
  //   }
  //   if(tick_timer_aux_minute_handler != NULL){
  //     tick_timer_aux_minute_handler(tick_time, units_changed);
  //   }
  //   // execute based on what has changed in the app state
  //   app_state_change_trigger_minute();
  // }

}


void fore_app_master_tick_timer_service_clock_subscribe(TimeUnits tick_units, TickHandler handler){
  // depending on the time_unit, we determine if the handlers have been assigned or not
  // and if they are free, we assign the new handler
  if(tick_units == SECOND_UNIT){
    if(tick_timer_clock_second_handler == NULL){
      tick_timer_clock_second_handler = handler;
    }else{
      APP_LOG(APP_LOG_LEVEL_ERROR, "FA_TTS:E1");
    }
  }else if(tick_units == MINUTE_UNIT){
    if(tick_timer_clock_minute_handler == NULL){
      tick_timer_clock_minute_handler = handler;
    }else{
      APP_LOG(APP_LOG_LEVEL_ERROR, "FA_TTS:E2");
    }
  }else{
    APP_LOG(APP_LOG_LEVEL_ERROR, "FA_TTS:E3");
  }
}

void fore_app_master_tick_timer_service_clock_unsubscribe(TimeUnits tick_units){
  if(tick_units == SECOND_UNIT){
    tick_timer_clock_second_handler = NULL;
  }else if(tick_units == MINUTE_UNIT){
    tick_timer_clock_minute_handler = NULL;
  }else{
    APP_LOG(APP_LOG_LEVEL_ERROR, "FA_TTS:E4");
  }
}


void fore_app_master_tick_timer_service_aux_subscribe(TimeUnits tick_units, TickHandler handler){
  // depending on the time_unit, we determine if the handlers have been assigned or not
  // and if they are free, we assign the new handler
  if(tick_units == SECOND_UNIT){
    if(tick_timer_aux_second_handler == NULL){
      tick_timer_aux_second_handler = handler;
    }else{
      APP_LOG(APP_LOG_LEVEL_ERROR, "FA_TTS:E5");
    }
  }else if(tick_units == MINUTE_UNIT){
    if(tick_timer_aux_minute_handler == NULL){
      tick_timer_aux_minute_handler = handler;
    }else{
      APP_LOG(APP_LOG_LEVEL_ERROR, "FA_TTS:E6");
    }
  }else{
    APP_LOG(APP_LOG_LEVEL_ERROR, "FA_TTS:E7");
  }
}

void fore_app_master_tick_timer_service_aux_unsubscribe(TimeUnits tick_units){
  if(tick_units == SECOND_UNIT){
    tick_timer_aux_second_handler = NULL;
  }else if(tick_units == MINUTE_UNIT){
    tick_timer_aux_minute_handler = NULL;
  }else{
    APP_LOG(APP_LOG_LEVEL_ERROR, "FA_TTS:E8");
  }
}



/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
/* +++++++++++++++ CONVIENCE FUNCTIONS +++++++++++++++ */
/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */





/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
/* +++++++++++++++ TIMER FUNCTIONS +++++++++++++++ */
/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */




/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
/* +++++++++++++++ MAJOR OPERATIONS FUNCTIONS +++++++++++++++ */
/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
void wakeup_main_response_handler(WakeupId wakeup_id, int32_t wakeup_cookie){
  // get cookie that this wakeup correponds to. At this point
  // we assume that it directly correponds to pinteract_code of the
  // pinteract that we want to happen now, but latter we might use an
  // encoding scheme to have different kinds of wakeup

  // reschedule all events in the day.
  config_wakeup_schedule();

  // APP_LOG(APP_LOG_LEVEL_ERROR, "wakeup_id %d  wakeup_cookie %d",
  //     (int) wakeup_id, (int) wakeup_cookie);

  // IF the wakeup_cookie is < NUM_CONFIG_WAKEUP and non-negative, then we know
  // it refers to a config wakeup schedule index
  if(( wakeup_cookie < NUM_CONFIG_WAKEUP) && ( wakeup_cookie >= 0) ) {
    vibes_enqueue_custom_pattern(pinteract_vibe_pat);
    uint16_t config_wakeup_i = (uint16_t)wakeup_cookie; // for clarity
    persist_write_int(ACTIVE_WAKEUP_CONFIG_I_PERSIST_KEY,config_wakeup_i);
    uint16_t pinteract_code = pinteract_code_from_config_wakeup_index(config_wakeup_i); //

    // we assume that pinteract is the most important thing is to have the
    // pinteract, so we close all the windows in favor of opening the pinteract
    window_stack_pop_all(false);
    // pinteract_driver(pinteract_code);


    // APP_LOG(APP_LOG_LEVEL_ERROR, "pinteract_code : %d", (int) pinteract_code );
    // psleep(100); // a slight delay to let any concurrent transmissions finish
      // their business.
    // if(heap_bytes_free()<1500){
      // pinteract_driver(pinteract_code);
    // }else{
    //   time_t wakeup_time_t = time(NULL) + 60*(up_delay_mins); // minutes;
    //   reschedule_config_wakeup_index(
    //     persist_read_int(ACTIVE_WAKEUP_CONFIG_I_PERSIST_KEY), wakeup_time_t);
    //   persist_write_int(ACTIVE_WAKEUP_CONFIG_I_PERSIST_KEY,-1);
    //   window_stack_pop_all(false);
    // }

  }
}

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
/* +++++++++++++++ MATHEMATICS FUNCTIONS +++++++++++++++ */
/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */

int32_t rand_lw_up_bounds(int32_t lw_bnd, int32_t up_bnd){
  srand(time(NULL)); // seed a new random generator, comment out for debugging
  return (rand()%(up_bnd - lw_bnd + 1)) ;
}


// TEST : PASSED
int16_t pow_int(int16_t x, int16_t y){
  // Returns x^y if y>0 and x,y are integers
  int16_t r = 1; // result
  for(int16_t i =1; i<= abs(y) ; i++ ){ r = x*r; }
  return r;
}


// TEST : PASSED
/* Take square roots */
uint32_t isqrt(uint32_t x){
  uint32_t op, res, one;

  op = x;
  res = 0;

  /* "one" starts at the highest power of four <= than the argument. */
  one = 1 << 30;  /* second-to-top bit set */
  while (one > op) one >>= 2;

  while (one != 0) {
    if (op >= res + one) {
      op -= res + one;
      res += one << 1;  // <-- faster than 2 * one
    }
    res >>= 1;
    one >>= 2;
  }
  return res;
}


/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
/* +++++++++++++++ TIME FUNCTIONS +++++++++++++++ */
/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */

void write_time_to_array_head(time_t ts, uint8_t * buf){
  /* put timestamp into 1st 4 bytes of array, assumw little endian encoding*/
  for(int16_t i=0; i < 4; i ++ ){ buf[i] = (uint8_t) (ts >> (8*i) ); }
}

time_t  read_time_from_array_head(uint8_t * buf){
  /* get time from array head */
  return (time_t)( buf[0] | (buf[1]<<8) | (buf[2]<<16) | (buf[3]<<24) );
}



int32_t today_srt_time_t_today_s(time_t * today_srt_time_t, int32_t *out_today_s){
  // time_t * today_srt_t -> the unix timestamp of the start of today
  // int32_t today_s -> the # of seconds that have elapsed today
  // return : # of seconds that have elapsed today

  time_t cur_time = time(NULL);
  struct tm *tt = localtime(&cur_time);
  int32_t today_s = (int32_t)( (tt->tm_sec) + (tt->tm_min)*60 + (tt->tm_hour)*(60*60) );

  if( out_today_s != NULL){
    *out_today_s = today_s;
  }
  if(today_srt_time_t != NULL){
    *today_srt_time_t = (time_t) (cur_time - today_s);
  }

  return today_s;
}

int32_t today_ms(int32_t * out_today_ms){
  *out_today_ms = today_srt_time_t_today_s(NULL,NULL)*1000 + time_ms(NULL,NULL);
  return *out_today_ms;
}


time_t today_s_to_time_t(int32_t today_s){
  time_t today_srt_time_t;
  today_srt_time_t_today_s(&today_srt_time_t, NULL);
  return (int32_t)(today_srt_time_t + today_s);
}


int32_t time_t_to_today_s(time_t t){
  time_t today_srt_time_t;
  today_srt_time_t_today_s(&today_srt_time_t, NULL);
  return (int32_t)(t - today_srt_time_t);
}


uint16_t today_s_to_today_m(int32_t today_s){
  return today_s/60;
}

int32_t today_m_to_today_s(uint16_t today_m){
  return today_m*60;
}


/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
/* +++++++++++++++ SORTING FUNCTIONS +++++++++++++++ */
/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */


// >> the pointer to the array we want the the sorted indicies
static int16_t *sortpt;

// >> directly compares values *a and *b
static int cmpfunc_ascend(const void *a, const void *b){
  return(  (*(int16_t*)a) - (*(int16_t*)b) ); // DESCENDING ORDER
}
static int cmpfunc_descend(const void *a, const void *b){
  return(  (*(int16_t*)b) - (*(int16_t*)a) ); // DESCENDING ORDER
}

// >> compares the values of an array at sortpt, where, a & b are indicies
// into that array. Used with an auxillary array ind_d to modify d_ind such that
// ind_d contains the sorted indicies of the sortpt array
static int cmpfunc_ascend_index(const void *a, const void *b){
  return (  (sortpt[*(int16_t*)a ] ) - (sortpt[*(int16_t*)b ])  ); // ASCENDING ORDER
}
static int cmpfunc_descend_index(const void *a, const void *b){
  return (  (sortpt[*(int16_t*)b ] ) - (sortpt[*(int16_t*)a ])  ); // DESCENDING ORDER
}
// >> takes a
static void maxminval(int16_t d[], int16_t p, int16_t r, int16_t *max, int16_t *min, int (*compar)(const void*,const void*) ){
  // p and r are INDICIES, not sizes, so we must touch all
  // this is p & r inclusive
  *max = d[p]; // the first element
  *min = d[p]; // the first element

  for(int16_t i = p; i <= r; i++ ){
    if(   compar(max,&(d[i])) > 0){  *max = d[i]; }
    if( compar(min,&(d[i])) < 0){  *min = d[i]; }
  }
}

// >> the swap function for qsort
static void swapf(int16_t *d, int16_t i, int16_t j){
  int16_t tmp = d[i];
  d[i] = d[j];
  d[j] = tmp;
}
// >> basic implementation of
static void qsortf(int16_t* d, int16_t p, int16_t r, int (*compar)(const void*,const void*)){
  /* test if base case */
  if( (p >= r) || ( r <= p ) ){
    return;
  }
  /* initialize the variables for pivot and array length */
  int16_t n = r - p + 1;
  int16_t q = 0 + p;
  /* parition into < and >= arrays with first element as pivot */
  for(int16_t j = 0; j < n; j++ ){
    if( compar( &d[p + j], &d[q]) < 0 ){
      swapf(d, p+j ,q+1);
      swapf(d, q+1 ,q);
      q = q + 1;
    }
  }
  /* recursive call */
  qsortf(d, p, q-1, compar);
  qsortf(d, q+1, r, compar);
}

// >> sorts the input array d and puts its sorted indicies into ind_d
static void sort_order_descend(int16_t *d, int16_t *ind_d, int16_t dlen){
  for(int16_t i = 0; i<dlen; i++) { ind_d[i] = i;} // initialize ind_d array
  sortpt = d;
  qsortf(ind_d, 0, (dlen-1), cmpfunc_descend_index);
  qsortf(d, 0, (dlen-1), cmpfunc_descend);
}
static void sort_order_ascend(int16_t *d, int16_t *ind_d, int16_t dlen){
  for(int16_t i = 0; i<dlen; i++) { ind_d[i] = i;} // initialize ind_d array
  sortpt = d;
  qsortf(ind_d, 0, (dlen-1), cmpfunc_ascend_index);
  qsortf(d, 0, (dlen-1), cmpfunc_ascend);
}
