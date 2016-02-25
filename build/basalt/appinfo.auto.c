#include "pebble_process_info.h"
#include "src/resource_ids.auto.h"

const PebbleProcessInfo __pbl_app_info __attribute__ ((section (".pbl_header"))) = {
  .header = "PBLAPP",
  .struct_version = { PROCESS_INFO_CURRENT_STRUCT_VERSION_MAJOR, PROCESS_INFO_CURRENT_STRUCT_VERSION_MINOR },
  .sdk_version = { PROCESS_INFO_CURRENT_SDK_VERSION_MAJOR, PROCESS_INFO_CURRENT_SDK_VERSION_MINOR },
  .process_version = { 0, 15 },
  .load_size = 0xb6b6,
  .offset = 0xb6b6b6b6,
  .crc = 0xb6b6b6b6,
  .name = "Wearable Psych",
  .company = "Wearable Psych",
  .icon_resource_id = RESOURCE_ID_WATCHTOWER_MENU_ICON_28,
  .sym_table_addr = 0xA7A7A7A7,
  .flags = 0,
  .num_reloc_entries = 0xdeadcafe,
  .uuid = { 0x3A, 0x91, 0x37, 0x0B, 0xAB, 0x7F, 0x42, 0xFB, 0xB4, 0x47, 0x5C, 0xC9, 0x45, 0x27, 0xA3, 0x29 },
  .virtual_size = 0xb6b6
};
