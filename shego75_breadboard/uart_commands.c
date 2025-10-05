#include "uart_keycodes.h"
#include "socd.h"
#include "uart.h"
#include "wait.h"
#include "hardware/gpio.h"
#include "lighting.h"

// Simple RX buffer for incoming UART lines
static char rx_line[128];
static uint8_t rx_idx = 0;

// Called frequently from matrix_scan_user to process any received UART bytes
void uart_receive_task(void) {
	int b = uart_poll_byte();
	while (b >= 0) {
		char c = (char)b;
		if (c == '\n' || c == '\r') {
			if (rx_idx > 0) {
				rx_line[rx_idx] = '\0';
				// handle a received line
				if (strcmp(rx_line, "TFT_ACK") == 0) {
					// example response handling
				}
				// clear buffer
				rx_idx = 0;
			}
		} else if (rx_idx < (sizeof(rx_line) - 1)) {
			rx_line[rx_idx++] = c;
		} else {
			// overflow, reset
			rx_idx = 0;
		}
		b = uart_poll_byte();
	}
}

// Helper functions to send the focus commands
void send_tft_focus(void) { uart_send_string("TFT_FOCUS\n"); }
void send_board_focus(void) { uart_send_string("BOARD_FOCUS\n"); }
