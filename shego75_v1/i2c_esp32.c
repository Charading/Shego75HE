/* i2c_esp32.c - Simple I2C communication with ESP32 */
#include "i2c_esp32.h"
#include "i2c_master.h"
#include "wait.h"
#include "uart.h"
#include <stdio.h>

#define ESP32_I2C_ADDR 0x42  // 7-bit address - adjust to match your ESP32
#define ESP32_I2C_TIMEOUT_MS 100

// Initialize I2C
void i2c_esp32_init(void) {
    i2c_init();
    uart_send_string("[I2C] Initialized on GP20/GP21\n");
}

// Send a simple test byte to ESP32
bool i2c_esp32_test(void) {
    uint8_t test_data = 0xAA;
    i2c_status_t status = i2c_transmit(ESP32_I2C_ADDR << 1, &test_data, 1, ESP32_I2C_TIMEOUT_MS);
    
    if (status == I2C_STATUS_SUCCESS) {
        uart_send_string("[I2C] Test SUCCESS\n");
        return true;
    } else {
        uart_send_string("[I2C] Test FAILED\n");
        return false;
    }
}

// Send data to ESP32
bool i2c_esp32_send(const uint8_t *data, uint16_t length) {
    if (!data || length == 0) {
        uart_send_string("[I2C] Invalid data or length\n");
        return false;
    }
    
    // Debug: log what we're sending
    char debug_buf[64];
    snprintf(debug_buf, sizeof(debug_buf), "[I2C] Sending %d bytes to addr 0x%02X\n", length, ESP32_I2C_ADDR);
    uart_send_string(debug_buf);
    
    i2c_status_t status = i2c_transmit(ESP32_I2C_ADDR << 1, data, length, ESP32_I2C_TIMEOUT_MS);
    
    if (status == I2C_STATUS_SUCCESS) {
        uart_send_string("[I2C] Transmit SUCCESS\n");
        return true;
    } else {
        snprintf(debug_buf, sizeof(debug_buf), "[I2C] Transmit FAILED with status: %d\n", status);
        uart_send_string(debug_buf);
        return false;
    }
}
