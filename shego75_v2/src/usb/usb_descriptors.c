#include "usb/usb_descriptors.h"

#include <string.h>

#include "pico/unique_id.h"

#include "usb/tusb_config.h"

// Ensure HID/vendor helper macros are available from TinyUSB
#include "class/hid/hid.h"
#include "class/vendor/vendor.h"

#define EPNUM_HID_KEYBOARD 0x81
#define EPNUM_HID_VIA_OUT  0x02
#define EPNUM_HID_VIA_IN   0x82
#define EPNUM_HID_SIGNAL_OUT 0x03
#define EPNUM_HID_SIGNAL_IN  0x83
#define EPNUM_VENDOR_GIF_OUT 0x04
#define EPNUM_VENDOR_GIF_IN  0x84

#ifndef RAW_HID_DESC_LEN
#define RAW_HID_DESC_LEN (9 + 9 + 7 + 7)
#endif

#define CONFIG_TOTAL_LEN (TUD_CONFIG_DESC_LEN + TUD_HID_DESC_LEN + RAW_HID_DESC_LEN * 2 + TUD_VENDOR_DESC_LEN)

#define TUD_HID_INOUT_DESCRIPTOR(_itfnum, _stridx, _protocol, _report_desc_len, _epin, _epout, _size, _interval) \
    9, TUSB_DESC_INTERFACE, _itfnum, 0, 2, TUSB_CLASS_HID, HID_SUBCLASS_NONE, _protocol, _stridx, \
    9, HID_DESC_TYPE_HID, U16_TO_U8S_LE(0x0111), 0x00, 1, HID_DESC_TYPE_REPORT, U16_TO_U8S_LE(_report_desc_len), \
    7, TUSB_DESC_ENDPOINT, _epin, TUSB_XFER_INTERRUPT, U16_TO_U8S_LE(_size), _interval, \
    7, TUSB_DESC_ENDPOINT, _epout, TUSB_XFER_INTERRUPT, U16_TO_U8S_LE(_size), _interval

static const board_config_t *g_cfg;

void usb_descriptors_init(const board_config_t *cfg) {
    g_cfg = cfg;
}

const board_config_t *usb_descriptor_board(void) {
    return g_cfg;
}

static tusb_desc_device_t desc_device;

uint8_t const *tud_descriptor_device_cb(void) {
    const uint16_t vid = g_cfg ? g_cfg->usb_vid : 0xDEAD;
    const uint16_t pid = g_cfg ? g_cfg->usb_pid : 0x0000;

    desc_device.bLength            = sizeof(tusb_desc_device_t);
    desc_device.bDescriptorType    = TUSB_DESC_DEVICE;
    desc_device.bcdUSB             = 0x0200;
    desc_device.bDeviceClass       = TUSB_CLASS_MISC;
    desc_device.bDeviceSubClass    = MISC_SUBCLASS_COMMON;
    desc_device.bDeviceProtocol    = MISC_PROTOCOL_IAD;
    desc_device.bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE;
    desc_device.idVendor           = vid;
    desc_device.idProduct          = pid;
    desc_device.bcdDevice          = 0x0100;
    desc_device.iManufacturer      = 0x01;
    desc_device.iProduct           = 0x02;
    desc_device.iSerialNumber      = 0x03;
    desc_device.bNumConfigurations = 0x01;

    return (uint8_t const *)&desc_device;
}

static const uint8_t hid_report_desc_keyboard[] = {
    TUD_HID_REPORT_DESC_KEYBOARD(HID_REPORT_ID_KEYBOARD)
};

#define RAW_HID_REPORT_DESCRIPTOR(_report_id) \
    0x06, 0x60, 0xFF, /* Usage Page (Vendor Defined) */ \
    0x09, 0x61,       /* Usage (Vendor Usage 0x61) */ \
    0xA1, 0x01,       /* Collection (Application) */ \
    0x85, _report_id, /* Report ID */ \
    0x75, 0x08,       /* Report Size: 8 bits */ \
    0x95, 0x20,       /* Report Count: 32 bytes */ \
    0x15, 0x00,       /* Logical Minimum 0 */ \
    0x26, 0xFF, 0x00, /* Logical Maximum 255 */ \
    0x09, 0x62,       /* Usage */ \
    0x81, 0x02,       /* Input (Data, Var, Abs) */ \
    0x09, 0x63,       /* Usage */ \
    0x91, 0x02,       /* Output (Data, Var, Abs) */ \
    0xC0              /* End Collection */

static const uint8_t hid_report_desc_via[] = { RAW_HID_REPORT_DESCRIPTOR(HID_REPORT_ID_VIA) };
static const uint8_t hid_report_desc_signalrgb[] = { RAW_HID_REPORT_DESCRIPTOR(HID_REPORT_ID_SIGNALRGB) };

static const uint8_t desc_configuration[] = {
    TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL, 0, CONFIG_TOTAL_LEN, TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP, 500),
    TUD_HID_DESCRIPTOR(ITF_NUM_HID_KEYBOARD, 4, HID_ITF_PROTOCOL_KEYBOARD, sizeof(hid_report_desc_keyboard), EPNUM_HID_KEYBOARD, CFG_TUD_HID_EP_BUFSIZE, 1),
    TUD_HID_INOUT_DESCRIPTOR(ITF_NUM_HID_VIA, 5, HID_ITF_PROTOCOL_NONE, sizeof(hid_report_desc_via), EPNUM_HID_VIA_IN, EPNUM_HID_VIA_OUT, CFG_TUD_HID_EP_BUFSIZE, 1),
    TUD_HID_INOUT_DESCRIPTOR(ITF_NUM_HID_SIGNALRGB, 6, HID_ITF_PROTOCOL_NONE, sizeof(hid_report_desc_signalrgb), EPNUM_HID_SIGNAL_IN, EPNUM_HID_SIGNAL_OUT, CFG_TUD_HID_EP_BUFSIZE, 1),
    TUD_VENDOR_DESCRIPTOR(ITF_NUM_VENDOR_GIF, 7, EPNUM_VENDOR_GIF_OUT, EPNUM_VENDOR_GIF_IN, CFG_TUD_VENDOR_TX_BUFSIZE)
};

const uint8_t *tud_descriptor_configuration_cb(uint8_t index) {
    (void)index;
    return desc_configuration;
}

uint8_t const *tud_hid_descriptor_report_cb(uint8_t itf) {
    switch (itf) {
        case ITF_NUM_HID_KEYBOARD:
            return hid_report_desc_keyboard;
        case ITF_NUM_HID_VIA:
            return hid_report_desc_via;
        case ITF_NUM_HID_SIGNALRGB:
            return hid_report_desc_signalrgb;
        default:
            return NULL;
    }
}

static const char *const string_desc[] = {
    "",
    "Charading",
    "Shego75",
    "0000000000000000",
    "Shego Keymap",
    "VIA Config",
    "SignalRGB",
    "GIF Bridge"
};

uint16_t const *tud_descriptor_string_cb(uint8_t index, uint16_t langid) {
    static uint16_t desc_str[32];
    (void)langid;

    uint8_t count;
    if (index == 0) {
        desc_str[1] = 0x0409;
        desc_str[0] = (TUSB_DESC_STRING << 8) | (2 + 2);
        return desc_str;
    }

    const char *str = NULL;
    if (index < sizeof(string_desc) / sizeof(string_desc[0])) {
        str = string_desc[index];
    }
    static char product_name[32];
    static char serial_buf[2 * PICO_UNIQUE_BOARD_ID_SIZE_BYTES + 1];

    if (index == 2 && g_cfg) {
        strncpy(product_name, g_cfg->name, sizeof(product_name) - 1);
        product_name[sizeof(product_name) - 1] = '\0';
        str = product_name;
    } else if (index == 3) {
        pico_get_unique_board_id_string(serial_buf, sizeof(serial_buf));
        str = serial_buf;
    }

    if (!str) {
        return NULL;
    }

    count = (uint8_t)strnlen(str, 31);

    desc_str[0] = (TUSB_DESC_STRING << 8) | ((count + 1) * 2);
    for (uint8_t i = 0; i < count; ++i) {
        desc_str[1 + i] = str[i];
    }

    return desc_str;
}
