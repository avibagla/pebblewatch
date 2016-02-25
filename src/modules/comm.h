//  This file takes care of the transmission of data to the phone and pushing
// the data to the server.
# pragma once

#include <pebble.h>
#include "helper.h"
#include "data.h"

// this sends the specified data items, of three types.
// 1. send all hourly data
// 2. send all pinteract data
// 3. send signal to send to server

typedef enum{
  AppKeyJSReady = 0,
  AppKeyActiData,
  AppKeyPinteractData,
  AppKeyPushToServer,
  AppKeySentToServer
} AppKey;


static void send_data_item(AppKey app_key);


static void comm_begin_upload();
