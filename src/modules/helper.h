#include <pebble.h>

void write_time_to_array_head(time_t ts, uint8_t * buf);

time_t  read_time_from_array_head(uint8_t * buf);
