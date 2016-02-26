
#pragma once
// This file is to contain the struct specifications for each datatype

typedef struct{
  uint16_t pinteract_code; // NOTE: must set this to 11 on initialization
  uint16_t data_size; // NOTE: must set this to sizeof(Pinteract11Data) when initialize
  time_t time_srt_priv_scrn; // when the privacy screen first prompts the user
  time_t time_srt_pi; // when the user enters into the pinteract
  time_t time_end_pi; // when the user ends the pinteract
  uint8_t mood_res; // the response to the pinteract
}__attribute__((__packed__)) Pinteract11Data;

typedef struct{
  uint16_t pinteract_code; // NOTE: must set this to 11 on initialization
  uint16_t data_size; // NOTE: must set this to sizeof(Pinteract11Data) when initialize
  time_t time_srt_priv_scrn; // when the privacy screen first prompts the user
  time_t time_srt_pi; // when the user enters into the pinteract
  time_t time_end_pi; // when the user ends the pinteract
  uint16_t sleep_duration_min_res; // the response to the pinteract
  uint8_t sleep_quality_res; // the response to the pinteract
}__attribute__((__packed__)) Pinteract12Data;


typedef struct{
  uint16_t pinteract_code; // NOTE: must set this to 11 on initialization
  uint16_t data_size; // NOTE: must set this to sizeof(Pinteract11Data) when initialize
  time_t time_srt_priv_scrn; // when the privacy screen first prompts the user
  time_t time_srt_pi; // when the user enters into the pinteract
  time_t time_end_pi; // when the user ends the pinteract
  uint16_t sleep_srt_min_res; // the response to the pinteract
  uint16_t sleep_end_min_res; // the response to the pinteract
}__attribute__((__packed__)) Pinteract121Data;
