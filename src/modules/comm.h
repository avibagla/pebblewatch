//  This file takes care of the transmission of data to the phone and pushing
// the data to the server.
# pragma once

#include <pebble.h>
#include "helper.h"
#include "data.h"
#include "../constants.h"

// this sends the specified data items, of three types.
// 1. send all hourly data
// 2. send all pinteract data
// 3. send signal to send to server

typedef enum{
  AppKeyJSReady = 0,
  AppKeyActiData = 1,
  AppKeyHealthEventsData = 2,
  AppKeyConfigData = 3,
  AppKeyPinteractData = 4,
  AppKeyPushToServer = 5,
  AppKeySentToServer = 6,
  NumAppKeys
} AppKey;


void comm_begin_upload();

void comm_begin_upload_no_window();
