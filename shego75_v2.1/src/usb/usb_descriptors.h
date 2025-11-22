#pragma once

#include <tusb.h>

#include "board/board_config.h"

enum {
    ITF_NUM_HID_KEYBOARD = 0,
    ITF_NUM_HID_VIA,
    ITF_NUM_HID_SIGNALRGB,
    ITF_NUM_VENDOR_GIF,
    ITF_NUM_TOTAL
};

enum {
    HID_REPORT_ID_KEYBOARD = 1,
    HID_REPORT_ID_VIA = 2,
    HID_REPORT_ID_SIGNALRGB = 3
};

void usb_descriptors_init(const board_config_t *cfg);
const board_config_t *usb_descriptor_board(void);
