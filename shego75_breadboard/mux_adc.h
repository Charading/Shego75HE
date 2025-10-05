#pragma once

#include QMK_KEYBOARD_H

// MUX control pins
#define MUX_S0 GP11
#define MUX_S1 GP12
#define MUX_S2 GP13
#define MUX_S3 GP14

// ADC pins for each MUX
#define MUX1_ADC_PIN GP26
#define MUX2_ADC_PIN GP27
#define MUX3_ADC_PIN GP28

// QMK Matrix functions
void matrix_init_custom(void);
bool matrix_scan_custom(matrix_row_t current_matrix[]);

