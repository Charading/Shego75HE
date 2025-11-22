#pragma once

#include <stdbool.h>
#include <stdint.h>

#include <tusb.h>

#include "board/board_config.h"
#include "usb/hid_types.h"

typedef struct {
    bool num_lock;
    bool caps_lock;
    bool scroll_lock;
} usb_host_led_state_t;

void usb_device_init(const board_config_t *cfg);
void usb_device_task(void);
bool usb_device_ready(void);
const usb_host_led_state_t *usb_host_led_state(void);

void usb_keyboard_push_report(hid_keyboard_report_t const *report);
