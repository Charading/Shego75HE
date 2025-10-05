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

// QMK Matrix functions
void matrix_init_custom(void);
bool matrix_scan_custom(matrix_row_t current_matrix[]);

