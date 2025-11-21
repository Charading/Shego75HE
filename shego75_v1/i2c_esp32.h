/* i2c_esp32.h - I2C communication with ESP32 */
#pragma once

#include <stdint.h>
#include <stdbool.h>

// Initialize I2C communication
void i2c_esp32_init(void);

// Test I2C connection
bool i2c_esp32_test(void);

// Send data to ESP32
bool i2c_esp32_send(const uint8_t *data, uint16_t length);
