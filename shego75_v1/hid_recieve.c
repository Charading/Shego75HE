/* hid_recieve.c
 * Implementations of simple helpers to show what HID packets look like
 * when received. They forward formatted output to the QMK console,
 * to the UART (ESP32 UART) and to the ESP32 over I2C.
 */

#include "hid_recieve.h"
#include "uart.h"
#include "i2c_esp32.h"
#include <stdio.h>
#include <string.h>

// Maximum bytes we will pretty-print
#define HID_RECIEVE_MAX_PRINT 64

void hid_recieve_debug_to_console(const uint8_t *data, uint8_t length) {
#ifdef CONSOLE_ENABLE
    if (!data || length == 0) {
        uprintf("HID RX: <empty>\n");
        return;
    }

    // Build a human-readable hex string
    char buf[HID_RECIEVE_MAX_PRINT];
    int pos = 0;
    pos += snprintf(buf + pos, sizeof(buf) - pos, "HID RX (%d):", length);
    for (uint8_t i = 0; i < length && pos < (int)sizeof(buf) - 4; i++) {
        pos += snprintf(buf + pos, sizeof(buf) - pos, " %02X", data[i]);
    }
    pos += snprintf(buf + pos, sizeof(buf) - pos, "\n");
    uprintf("%s", buf);
#else
    (void)data; (void)length;
#endif
}

void hid_recieve_debug_to_uart(const uint8_t *data, uint8_t length) {
    if (!data || length == 0) {
        uart_debug_print("HID RX: <empty>\n");
        return;
    }

    // Build into a buffer then send in one call to uart helpers
    char buf[HID_RECIEVE_MAX_PRINT];
    int pos = 0;
    pos += snprintf(buf + pos, sizeof(buf) - pos, "HID RX (%d):", length);
    for (uint8_t i = 0; i < length && pos < (int)sizeof(buf) - 4; i++) {
        pos += snprintf(buf + pos, sizeof(buf) - pos, " %02X", data[i]);
    }
    pos += snprintf(buf + pos, sizeof(buf) - pos, "\n");

    uart_debug_print(buf);
}

void hid_recieve_debug_to_i2c(const uint8_t *data, uint8_t length) {
    if (!data || length == 0) {
        uint8_t empty_pkt[2] = {0xEE, 0};
        i2c_esp32_send(empty_pkt, 2);
        return;
    }

    // Compose a compact debug packet: 0xEE, len, bytes[0..N]
    uint8_t pkt[34];
    uint8_t send_len = (length > 31) ? 31 : length; // leave room for prefix/len
    pkt[0] = 0xEE;
    pkt[1] = send_len;
    memcpy(&pkt[2], data, send_len);
    i2c_esp32_send(pkt, send_len + 2);
}

void hid_recieve_debug_all(const uint8_t *data, uint8_t length) {
    // Mirror to console, UART and I2C for maximum visibility
    hid_recieve_debug_to_console(data, length);
    hid_recieve_debug_to_uart(data, length);
    hid_recieve_debug_to_i2c(data, length);
}
