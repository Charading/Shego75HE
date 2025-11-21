// uart.c - Pico SDK UART implementation for shego75_v1
#include "uart.h"

#include <stdbool.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/gpio.h"

// Global flag to track if UART has been initialized
static bool uart_initialized = false;

// Some QMK/platform configs may define UART_TX_PIN as the token GP0 etc
// before the Pico SDK headers are available in the include order. If GPn
// tokens aren't defined yet, define safe numeric fallbacks so the token
// expansions work (e.g. UART_TX_PIN == GP0 -> 0).
#ifndef GP0
#define GP0 0
#define GP1 1
#define GP2 2
#define GP3 3
#define GP4 4
#define GP5 5
#define GP6 6
#define GP7 7
#define GP8 8
#define GP9 9
#define GP10 10
#define GP11 11
#define GP12 12
#define GP13 13
#define GP14 14
#define GP15 15
#define GP16 16
#define GP17 17
#define GP18 18
#define GP19 19
#define GP20 20
#define GP21 21
#define GP22 22
#define GP23 23
#define GP24 24
#define GP25 25
#define GP26 26
#define GP27 27
#define GP28 28
#define GP29 29
#endif

// Use uart0 on GP0 as the TX pin (only define if not provided by board headers)
#ifndef UART_ID
#define UART_ID uart0
#endif

#ifndef UART_TX_PIN
#define UART_TX_PIN 0
#endif

#ifndef UART_BAUD
#define UART_BAUD 115200
#endif

void uart_init_and_welcome(void) {
    // Only initialize once
    if (uart_initialized) return;
    
    // Initialize UART peripheral
    uart_init(UART_ID, UART_BAUD);

    // Configure TX pin for UART
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);

    // Disable hardware flow control (CTS/RTS)
    uart_set_hw_flow(UART_ID, false, false);

    // 8 data bits, 1 stop bit, no parity
    uart_set_format(UART_ID, 8, 1, UART_PARITY_NONE);

    // Small delay to stabilise the line
    sleep_ms(10);

    // Mark as initialized before sending (prevents recursive issues)
    uart_initialized = true;

    // Send a welcome message so you can verify UART is working
    uart_puts(UART_ID, "QMK_UART_READY\n");
    uart_tx_wait_blocking(UART_ID);
}

void uart_send_string(const char* str) {
    if (!str) return;
    
    // Lazy-initialize UART if it hasn't been initialized yet. This makes
    // uart_send_string safe to call early (for example from process_record_user)
    // even if keyboard_post_init_user hasn't run yet.
    if (!uart_initialized) {
        uart_init_and_welcome();
    }
    
    // Send character by character with explicit blocking to force immediate TX
    // This is more aggressive than uart_puts + uart_tx_wait_blocking
    while (*str) {
        uart_putc_raw(UART_ID, *str);
        str++;
    }
    
    // Ensure all characters have been transmitted
    uart_tx_wait_blocking(UART_ID);
}

void uart_init_rx(void) {
    // Configure RX pin for UART peripheral so we can receive from ESP32
    // Default RX is GP1 for uart0
#ifndef UART_RX_PIN
#define UART_RX_PIN 1
#endif
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
    // Ensure UART is initialized
    if (!uart_initialized) uart_init_and_welcome();
}

int uart_poll_byte(void) {
    if (!uart_initialized) return -1;
    if (uart_is_readable(UART_ID)) {
        int c = uart_getc(UART_ID);
        return c;
    }
    return -1;
}
