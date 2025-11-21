/* hid_recieve.h
 * Simple helper to print/debug received HID packets.
 * Provides helpers to emit the received bytes to the QMK console,
 * to the UART debug/ESP UART, and to the ESP32 over I2C.
 */
#pragma once

#include QMK_KEYBOARD_H
#include <stdint.h>

/**
 * Print the received HID packet to console, UART and I2C (all).
 * data: pointer to received bytes (may be NULL if length==0)
 * length: number of bytes in data
 */
void hid_recieve_debug_all(const uint8_t *data, uint8_t length);

/** Print to UART only (uses uart_send_string) */
void hid_recieve_debug_to_uart(const uint8_t *data, uint8_t length);

/** Send a compact debug packet to ESP32 over I2C (prefix 0xEE, len, bytes...) */
void hid_recieve_debug_to_i2c(const uint8_t *data, uint8_t length);

/** Print to QMK console (uprintf) if CONSOLE_ENABLE is defined */
void hid_recieve_debug_to_console(const uint8_t *data, uint8_t length);
