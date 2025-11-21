#pragma once

#include QMK_KEYBOARD_H

// MUX control pins
#define MUX_A0 GP11
#define MUX_A1 GP12
#define MUX_A2 GP13
#define MUX_A3 GP14
#define MUX_A4 GP15

// ADC pins for each MUX
#define MUX1_ADC_PIN GP26
#define MUX2_ADC_PIN GP27
#define MUX3_ADC_PIN GP28

// MUX chip select pins (active low)
#define MUX_CS1 GP18
#define MUX_CS2 GP19
#define MUX_CS3 GP10

// Mux write/latch pin (active low, latch on rising edge)
#define MUX_WR GP22

// Hall effect threshold - key is pressed when ADC is BELOW this value
#define SENSOR_THRESHOLD 480

// Auto-calibration settings
// Threshold percentage: key press triggers when ADC drops below this % of baseline
// Example: 85 means key actuates when value drops to 85% of resting state (15% drop)
// Lower = more sensitive (earlier actuation), Higher = less sensitive (deeper press required)
// NOTE: This is the legacy fallback. The new system uses DEFAULT_SENSITIVITY_PERCENT in mux_adc.c
#ifndef CALIBRATION_THRESHOLD_PERCENT
#define CALIBRATION_THRESHOLD_PERCENT 85
#endif



// QMK Matrix functions
void matrix_init_custom(void);
bool matrix_scan_custom(matrix_row_t current_matrix[]);

// Auto-calibration function
void calibrate_sensors(void);

// Set a per-key sensitivity percent by key index
// percent: sensitivity percent (e.g., 10 => trigger when value deviates +/-10% from baseline)
void set_key_threshold(uint16_t key_idx, uint8_t percent);

