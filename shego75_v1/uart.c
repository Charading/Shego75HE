// uart.c - Dual UART implementation for shego75_v1
// UART0 (GP0/GP1): Debug output
// UART1 (GP8/GP9): ESP32 communication
#include "uart.h"

#include <stdbool.h>
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/gpio.h"
#include "print.h"
#include "wait.h"

// Track initialization state for both UARTs
static bool uart0_initialized = false;  // Debug UART
static bool uart1_initialized = false;  // ESP32 UART

// UART0: Debug output (GP0 TX, GP1 RX - RX not used)
#define DEBUG_UART_ID uart0
#define DEBUG_TX_PIN 0
#define DEBUG_BAUD 115200

// UART1: ESP32 communication (GP8 TX, GP9 RX)
#define ESP_UART_ID uart1
#define ESP_TX_PIN 8
#define ESP_RX_PIN 9
#define ESP_BAUD 115200

void uart_init_and_welcome(void) {
    // Initialize UART0 for debug output
    if (!uart0_initialized) {
        uart_init(DEBUG_UART_ID, DEBUG_BAUD);
        gpio_set_function(DEBUG_TX_PIN, GPIO_FUNC_UART);
        uart_set_hw_flow(DEBUG_UART_ID, false, false);
        uart_set_format(DEBUG_UART_ID, 8, 1, UART_PARITY_NONE);
        uart0_initialized = true;
    }
    
    // Initialize UART1 for ESP32 communication
    if (!uart1_initialized) {
        uart_init(ESP_UART_ID, ESP_BAUD);
        gpio_set_function(ESP_TX_PIN, GPIO_FUNC_UART);
        uart_set_hw_flow(ESP_UART_ID, false, false);
        uart_set_format(ESP_UART_ID, 8, 1, UART_PARITY_NONE);
        sleep_ms(10);
        uart1_initialized = true;
        
        // Send welcome to ESP32
        uart_puts(ESP_UART_ID, "QMK_UART_READY\n");
        uart_tx_wait_blocking(ESP_UART_ID);
    }
}

// Send string to ESP32 (UART1) and also to HID console
void uart_send_string(const char* str) {
    if (!str) return;
    if (!uart1_initialized) uart_init_and_welcome();
    
    // Send to ESP32 on UART1 (GP8/GP9)
    const char* p = str;
    while (*p) {
        uart_putc_raw(ESP_UART_ID, *p);
        p++;
    }
    uart_tx_wait_blocking(ESP_UART_ID);
    
    // Also send to HID console for visibility
    #ifdef CONSOLE_ENABLE
    printf("%s", str);
    #endif
}

// Send debug string to UART0 and HID console
void uart_debug_print(const char* str) {
    if (!str) return;
    
    // Send to UART0 (GP0) in small chunks to prevent fragmentation
    // RP2040 UART has 32-byte FIFO, so we send in 16-byte chunks with delays
    if (!uart0_initialized) uart_init_and_welcome();
    
    const char* p = str;
    int chunk_count = 0;

    while (*p) {
        uart_putc_raw(DEBUG_UART_ID, *p);
        p++;
        chunk_count++;

        // Every 16 characters, wait for TX FIFO to empty
        if (chunk_count >= 16) {
            uart_tx_wait_blocking(DEBUG_UART_ID);
            wait_us(500);  // Wait 500us between chunks
            chunk_count = 0;
        }
    }

    // Final wait to ensure everything is sent
    uart_tx_wait_blocking(DEBUG_UART_ID);
    wait_us(1000);  // 1ms final delay

    // Also send to HID console (requires CONSOLE_ENABLE = yes in rules.mk)
    #ifdef CONSOLE_ENABLE
    uprintf("%s", str);
    #endif
}

void uart_init_rx(void) {
    // Configure RX pin for ESP32 UART (UART1 GP9)
    if (!uart1_initialized) uart_init_and_welcome();
    gpio_set_function(ESP_RX_PIN, GPIO_FUNC_UART);
}

int uart_poll_byte(void) {
    if (!uart1_initialized) return -1;
    if (uart_is_readable(ESP_UART_ID)) {
        return uart_getc(ESP_UART_ID);
    }
    return -1;
}
