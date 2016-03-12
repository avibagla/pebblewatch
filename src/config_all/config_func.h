#pragma once
#include <pebble.h>
#include "../modules/helper.h"
#include "../constants.h"
// #include "pinteract_all/pinteract_func.h"


// static const int32_t CONFIG_T_TO_TODAY_S = 337;
//  -> 86400/65536 = 1.3183 -> (86400*10000)/65536 = 13183
static const int32_t CONFIG_T_TO_TODAY_S = 13183;


// configuration scheduling struct
typedef struct {
  int16_t pinteract_code;
  time_t srt;
  time_t end;
} WakeupConfig;


// Config wakeup functions

void write_to_config_wakeup_persistant_storage(WakeupConfig * cs_ary_in, int16_t n_cs_ary_el_in );

WakeupConfig read_config_wakeup_index_persistant_storage( int16_t config_wakeup_i );

int16_t pinteract_code_from_config_wakeup_index(int16_t config_wakeup_i);

void reset_config_wakeup_schedule();

WakeupId read_wakeup_id_at_config_wakeup_index(int16_t config_wakeup_i);

void write_wakeup_id_at_config_wakeup_index(int16_t config_wakeup_i, WakeupId wakeup_id);

WakeupId reschedule_config_wakeup_index(int16_t config_wakeup_i, time_t wakeup_time_t);

void config_wakeup_schedule();
