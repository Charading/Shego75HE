#include "usb/raw_hid.h"

#include <string.h>

#include <tusb.h>

#include "usb/usb_descriptors.h"

void raw_hid_init(void) {}

bool raw_hid_send(const uint8_t *data, uint16_t length) {
    if (!tud_hid_n_ready(ITF_NUM_HID_SIGNALRGB)) {
        return false;
    }
    return tud_hid_n_report(ITF_NUM_HID_SIGNALRGB, HID_REPORT_ID_SIGNALRGB, data, length);
}
