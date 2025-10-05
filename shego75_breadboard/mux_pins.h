#pragma once

#include <stddef.h>  // for NULL
#include <stdint.h>  // for uint8_t

#define KEY_COUNT 48 // 4 rows * 12 columns

// Struct for each channel entry
typedef struct {
    uint8_t sensor;      // Sensor number (HE#)
    const char *key;     // Key name for debug ("F1", "A", etc.)
} mux16_ref_t;

// Extern declarations for each MUX table
extern const char *sensor_names[KEY_COUNT];
extern const uint16_t sensor_to_keycode[KEY_COUNT];
extern const mux16_ref_t mux1_channels[16];
extern const mux16_ref_t mux2_channels[16];
extern const mux16_ref_t mux3_channels[16];
