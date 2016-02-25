#include "data.h"

bool data_to_send_acti(){
  // compares the current time to the end of the previous block of data sent up
  return ( (time() - persist_read_int(ACTI_LAST_UPLOAD_TIME_PERSIST_KEY)) >= (MAX_ENTRIES*60))
}

bool data_to_send_pinteract(){
  // this code tells us if th
  return (persist_read_int(PINTERACT_KEY_COUNT_PERSIST_KEY) > 0);
}


int get_next_pinteract_element_key(){

}

int get_size_of_pinteract_element_data(int pstorage_key){
  // get the head of the pinteract block, which will contain the size of the
  // whole block
}

int get_next_pinteract_element(int pstorage_key){


}
