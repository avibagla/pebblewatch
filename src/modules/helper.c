#include "helper.h"

void write_time_to_array_head(time_t ts, uint8_t * buf){
  /* put timestamp into 1st 4 bytes of array, assumw little endian encoding*/
  for(int16_t i=0; i < 4; i ++ ){ buf[i] = (uint8_t) (ts >> (8*i) ); }
}

time_t  read_time_from_array_head(uint8_t * buf){
  /* get time from array head */
  return (time_t)( buf[0] | (buf[1]<<8) | (buf[2]<<16) | (buf[3]<<24) );
}
