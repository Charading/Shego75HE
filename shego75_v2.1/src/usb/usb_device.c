#include "usb/usb_device.h"

#include <string.h>

#include "protocol/gif/gif_bridge.h"
#include "protocol/signalrgb/signalrgb_patched.h"
#include "protocol/via/via_protocol.h"
#include "usb/raw_hid.h"
#include "usb/tusb_config.h"
#include "usb/usb_descriptors.h"

static const board_config_t *g_cfg;
static usb_host_led_state_t g_leds;
static hid_keyboard_report_t g_keyboard_report;
static bool g_keyboard_pending;

void usb_device_init(const board_config_t *cfg) {
    g_cfg = cfg;
    usb_descriptors_init(cfg);
    raw_hid_init();
    tud_init(0);
    g_keyboard_pending = false;
}

void usb_device_task(void) {
    tud_task();
}

bool usb_device_ready(void) {
    return tud_mounted();
}

const usb_host_led_state_t *usb_host_led_state(void) {
    return &g_leds;
}

void usb_keyboard_push_report(hid_keyboard_report_t const *report) {
    g_keyboard_report = *report;
    g_keyboard_pending = true;
    if (tud_ready()) {
        tud_hid_n_report(ITF_NUM_HID_KEYBOARD, HID_REPORT_ID_KEYBOARD, &g_keyboard_report, sizeof(g_keyboard_report));
        g_keyboard_pending = false;
    }
}

void tud_mount_cb(void) {
    (void)g_cfg;
}

void tud_umount_cb(void) {
    g_keyboard_pending = false;
}

void tud_hid_report_complete_cb(uint8_t itf, uint8_t const *report, uint16_t len) {
    (void)itf;
    (void)report;
    (void)len;
    if (g_keyboard_pending) {
        tud_hid_n_report(ITF_NUM_HID_KEYBOARD, HID_REPORT_ID_KEYBOARD, &g_keyboard_report, sizeof(g_keyboard_report));
        g_keyboard_pending = false;
    }
}

uint16_t tud_hid_get_report_cb(uint8_t itf, uint8_t report_id, hid_report_type_t report_type, uint8_t *buffer, uint16_t reqlen) {
    (void)itf;
    (void)report_id;
    (void)report_type;
    (void)buffer;
    (void)reqlen;
    return 0;
}

void tud_hid_set_report_cb(uint8_t itf, uint8_t report_id, hid_report_type_t report_type, uint8_t const *buffer, uint16_t bufsize) {
    (void)report_id;
    (void)report_type;

    if (itf == ITF_NUM_HID_KEYBOARD && report_type == HID_REPORT_TYPE_OUTPUT && bufsize) {
        const uint8_t leds = buffer[0];
        g_leds.num_lock = leds & KEYBOARD_LED_NUMLOCK;
        g_leds.caps_lock = leds & KEYBOARD_LED_CAPSLOCK;
        g_leds.scroll_lock = leds & KEYBOARD_LED_SCROLLLOCK;
        return;
    }

    if (itf == ITF_NUM_HID_VIA) {
        via_protocol_handle_packet(buffer, bufsize);
        return;
    }

    if (itf == ITF_NUM_HID_SIGNALRGB) {
        signalrgb_handle_packet(buffer, bufsize);
        return;
    }
}

void tud_vendor_rx_cb(uint8_t itf) {
    if (itf != ITF_NUM_VENDOR_GIF) {
        return;
    }

    uint8_t buffer[CFG_TUD_VENDOR_RX_BUFSIZE];
    const uint32_t count = tud_vendor_n_read(itf, buffer, sizeof(buffer));
    if (count) {
        gif_bridge_handle_chunk(buffer, (uint16_t)count);
    }
}
