#include "protocol/signalrgb/signalrgb_patched.h"

#include <string.h>

#include "lighting/lighting_engine.h"
#include "usb/raw_hid.h"
#include "usb/usb_device.h"

#define FW_VERSION_BYTE_1 0
#define FW_VERSION_BYTE_2 1
#define FW_VERSION_BYTE_3 0

static const board_config_t *g_cfg;
static uint8_t packet[32];

static void srgb_send(void) {
    raw_hid_send(packet, sizeof(packet));
}

void signalrgb_init(const board_config_t *cfg) {
    g_cfg = cfg;
    memset(packet, 0, sizeof(packet));
}

static void get_qmk_version(void) {
    packet[0] = GET_QMK_VERSION;
    packet[1] = FW_VERSION_BYTE_1;
    packet[2] = FW_VERSION_BYTE_2;
    packet[3] = FW_VERSION_BYTE_3;
    srgb_send();
}

static void get_signalrgb_protocol_version(void) {
    packet[0] = GET_PROTOCOL_VERSION;
    packet[1] = PROTOCOL_VERSION_BYTE_1;
    packet[2] = PROTOCOL_VERSION_BYTE_2;
    packet[3] = PROTOCOL_VERSION_BYTE_3;
    srgb_send();
}

static void get_unique_identifier(void) {
    packet[0] = GET_UNIQUE_IDENTIFIER;
    if (g_cfg) {
        packet[1] = g_cfg->signalrgb_uid[0];
        packet[2] = g_cfg->signalrgb_uid[1];
        packet[3] = g_cfg->signalrgb_uid[2];
    }
    srgb_send();
}

static void led_streaming(const uint8_t *data, uint16_t length) {
    if (!g_cfg) {
        return;
    }

    const uint8_t index = data[1];
    const uint8_t numberofleds = data[2];
    const uint16_t available = lighting_engine_led_count();
    const uint16_t expected = 3 + (numberofleds * 3);

    if (expected > length) {
        return;
    }

    if ((uint16_t)index + numberofleds > available) {
        packet[0] = STREAM_RGB_DATA;
        packet[1] = DEVICE_ERROR_LED_BOUNDS;
        srgb_send();
        return;
    }

    if (numberofleds >= 10) {
        packet[0] = STREAM_RGB_DATA;
        packet[1] = DEVICE_ERROR_LED_COUNT;
        srgb_send();
        return;
    }

    const usb_host_led_state_t *host_leds = usb_host_led_state();

    for (uint8_t i = 0; i < numberofleds; i++) {
        const uint8_t offset = (i * 3) + 3;
        uint8_t r = data[offset];
        uint8_t g = data[offset + 1];
        uint8_t b = data[offset + 2];

        (void)host_leds;
        lighting_engine_set_pixel(index + i, r, g, b);
    }
}

static void signalrgb_mode_enable(void) {
    (void)g_cfg;
    // TODO: pause local animations and dedicate LED updates to SignalRGB streaming.
}

static void signalrgb_mode_disable(void) {
    (void)g_cfg;
    // TODO: restore previous lighting effect from non-volatile storage.
}

static void get_total_leds(void) {
    packet[0] = GET_TOTAL_LEDS;
    packet[1] = (uint8_t)lighting_engine_led_count();
    srgb_send();
}

static void get_firmware_type(void) {
    packet[0] = GET_FIRMWARE_TYPE;
    packet[1] = FIRMWARE_TYPE_BYTE;
    srgb_send();
}

bool signalrgb_handle_packet(const uint8_t *data, uint16_t length) {
    if (!length) {
        return false;
    }

    switch (data[0]) {
        case GET_QMK_VERSION:
            get_qmk_version();
            return true;
        case GET_PROTOCOL_VERSION:
            get_signalrgb_protocol_version();
            return true;
        case GET_UNIQUE_IDENTIFIER:
            get_unique_identifier();
            return true;
        case STREAM_RGB_DATA:
            led_streaming(data, length);
            return true;
        case SET_SIGNALRGB_MODE_ENABLE:
            signalrgb_mode_enable();
            return true;
        case SET_SIGNALRGB_MODE_DISABLE:
            signalrgb_mode_disable();
            return true;
        case GET_TOTAL_LEDS:
            get_total_leds();
            return true;
        case GET_FIRMWARE_TYPE:
            get_firmware_type();
            return true;
        default:
            return false;
    }
}
