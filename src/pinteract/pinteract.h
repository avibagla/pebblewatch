#pragma once

#include <pebble.h>
#include "pinteract_structs.h"



// note, we pass the privacy screen the pinteact_code that we want it to execute
// when the user agrees to begin
void pinteract_priv_scrn(int pinteract_code);

// note, we decouple the privacy screen and pinteract driver because not all
// printeracts will require a privacy screen, but they will all need a driver.
void pinteract_driver(int pinteract_code);
