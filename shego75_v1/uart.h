/* uart.h - simple UART helper for shego75_v1
 * Provides a small wrapper around the RP2040 UART peripheral
 * to send strings from the keyboard firmware to an attached ESP32.
 */
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void uart_init_and_welcome(void);
void uart_send_string(const char* str);
void uart_debug_print(const char* str);  // Debug output to UART0 (GP0) and HID console
// Initialize UART RX (configure RX pin and enable interrupts/polling as needed)
void uart_init_rx(void);
// Poll for available bytes; returns -1 if none, otherwise returns uint8_t value.
int uart_poll_byte(void);
// Polling task to be called frequently to process incoming UART bytes
void uart_receive_task(void);
// Helpers to send focus commands
void send_tft_focus(void);
void send_board_focus(void);

#ifdef __cplusplus
}
#endif
