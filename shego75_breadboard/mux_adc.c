#include QMK_KEYBOARD_H
#include "mux_adc.h"
#include "mux_pins.h"
#include "uart.h"
#include "uart_keycodes.h"
#include "quantum.h"
#include "matrix.h"
#include "analog.h"
#include "wait.h"
#include "debug.h"
#include "timer.h"
#include "print.h"
#include <stdio.h>

// Hall effect threshold - key is pressed when ADC is BELOW this value
#define SENSOR_THRESHOLD 413
#define DEBOUNCE_MS 5

// Optional MUX enable pins (comment out if not used)
// #define MUX1_EN GP15
// #define MUX2_EN GP16
// #define MUX3_EN GP17

// Key state tracking for 48 keys (4 rows x 12 cols)
static bool key_pressed[48];
static uint32_t key_timer[48];

// Debug counter to limit output
static uint32_t debug_counter = 0;
static uint32_t last_debug_time = 0;

// Helper for reading ADC
static uint16_t read_adc_pin(pin_t pin) {
    return analogReadPin(pin);
}

// Helper for selecting MUX channel
static void select_mux_channel(uint8_t channel) {
    writePin(MUX_S0, (channel & 0x01) ? 1 : 0);
    writePin(MUX_S1, (channel & 0x02) ? 1 : 0);
    writePin(MUX_S2, (channel & 0x04) ? 1 : 0);
    writePin(MUX_S3, (channel & 0x08) ? 1 : 0);
    wait_us(50);
}

void matrix_init_custom(void) {
    // Setup MUX control pins
    setPinOutput(MUX_S0);
    setPinOutput(MUX_S1);
    setPinOutput(MUX_S2);
    setPinOutput(MUX_S3);
    
    #ifdef MUX1_EN
    setPinOutput(MUX1_EN);
    writePinHigh(MUX1_EN); // Disable (active low)
    #endif
    #ifdef MUX2_EN
    setPinOutput(MUX2_EN);
    writePinHigh(MUX2_EN); // Disable (active low)
    #endif
    #ifdef MUX3_EN
    setPinOutput(MUX3_EN);
    writePinHigh(MUX3_EN); // Disable (active low)
    #endif
    
    // Initialize ADC pins
    setPinInputHigh(MUX1_ADC_PIN);
    setPinInputHigh(MUX2_ADC_PIN);
    setPinInputHigh(MUX3_ADC_PIN);
    
    // Initialize state
    for (uint8_t i = 0; i < 48; i++) {
        key_pressed[i] = false;
        key_timer[i] = 0;
    }
}

bool matrix_scan_custom(matrix_row_t current_matrix[]) {
    bool changed = false;
    uint32_t now = timer_read32();
    
    // Clear matrix
    for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
        current_matrix[row] = 0;
    }
    
    // Debug every 1 second (1000ms) - only if ADC debug is enabled
    bool debug_this_scan = false;
    if (get_adc_debug_enabled() && timer_elapsed32(last_debug_time) >= 1000) {
        debug_this_scan = true;
        last_debug_time = now;
    }
    debug_counter++;

    // Array of ADC pins and MUX tables
    pin_t adc_pins[3] = {MUX1_ADC_PIN, MUX2_ADC_PIN, MUX3_ADC_PIN};
    const mux16_ref_t* mux_tables[3] = {mux1_channels, mux2_channels, mux3_channels};
    
    // Storage for ADC values organized by row/col for debug printing
    typedef struct {
        const char* key_name;
        uint16_t adc_val;
    } row_data_t;
    row_data_t row_data[MATRIX_ROWS][MATRIX_COLS];
    
    // Initialize row data
    for (uint8_t r = 0; r < MATRIX_ROWS; r++) {
        for (uint8_t c = 0; c < MATRIX_COLS; c++) {
            row_data[r][c].key_name = NULL;
            row_data[r][c].adc_val = 0;
        }
    }

    // Scan all 3 MUXes
    for (uint8_t mux_idx = 0; mux_idx < 3; mux_idx++) {
        // Enable current MUX (if enable pins are defined)
        #ifdef MUX1_EN
        if (mux_idx == 0) { writePinLow(MUX1_EN); writePinHigh(MUX2_EN); writePinHigh(MUX3_EN); }
        #endif
        #ifdef MUX2_EN
        if (mux_idx == 1) { writePinHigh(MUX1_EN); writePinLow(MUX2_EN); writePinHigh(MUX3_EN); }
        #endif
        #ifdef MUX3_EN
        if (mux_idx == 2) { writePinHigh(MUX1_EN); writePinHigh(MUX2_EN); writePinLow(MUX3_EN); }
        #endif
        
        // Scan all 16 channels on this MUX
        for (uint8_t ch = 0; ch < 16; ch++) {
            select_mux_channel(ch);
            wait_us(100);
            
            uint16_t adc_val = read_adc_pin(adc_pins[mux_idx]);
            
            // Calculate key index (0-47 for 48 keys)
            uint8_t key_idx = mux_idx * 16 + ch;
            if (key_idx >= 48) continue; // Only handle first 48 keys
            
            // Get key mapping from the table
            const mux16_ref_t* key_mapping = &mux_tables[mux_idx][ch];
            if (!key_mapping->key) continue; // Skip unmapped keys
            
            // Convert sensor number to matrix position (4 rows x 12 cols)
            uint8_t matrix_row = (key_mapping->sensor - 1) / MATRIX_COLS; // sensor numbers start at 1
            uint8_t matrix_col = (key_mapping->sensor - 1) % MATRIX_COLS;
            
            // Check if this is within our 4x12 matrix
            if (matrix_row >= MATRIX_ROWS || matrix_col >= MATRIX_COLS) continue;
            
            // *** KEY LOGIC: Key is pressed when ADC value is BELOW threshold ***
            bool should_press = (adc_val < SENSOR_THRESHOLD);
            
            // Store ADC value for debug printing
            if (debug_this_scan) {
                row_data[matrix_row][matrix_col].key_name = key_mapping->key;
                row_data[matrix_row][matrix_col].adc_val = adc_val;
            }
            
            // Debounce check
            if (timer_elapsed32(key_timer[key_idx]) > DEBOUNCE_MS) {
                if (should_press != key_pressed[key_idx]) {
                    key_pressed[key_idx] = should_press;
                    key_timer[key_idx] = now;
                    changed = true;
                    
                    // Print key event only if key debug is enabled
                    if (get_key_debug_enabled()) {
                        char buf[80];
                        snprintf(buf, sizeof(buf), "Key %s: %s (R%d C%d) ADC=%d\n", 
                                key_mapping->key,
                                should_press ? "PRESS" : "RELEASE", 
                                matrix_row, matrix_col, adc_val);
                        uart_send_string(buf);
                    }
                }
            }
            
            // Set key in matrix if pressed
            if (key_pressed[key_idx]) {
                current_matrix[matrix_row] |= (1 << matrix_col);
            }
        }
        
        // Disable all MUXes after scanning
        #ifdef MUX1_EN
        writePinHigh(MUX1_EN);
        #endif
        #ifdef MUX2_EN
        writePinHigh(MUX2_EN);
        #endif
        #ifdef MUX3_EN
        writePinHigh(MUX3_EN);
        #endif
    }
    
    // Print debug output organized by rows
    if (debug_this_scan) {
        char buf[200];
        uart_send_string("\n=== ADC VALUES BY ROW ===\n");
        
        for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
            snprintf(buf, sizeof(buf), "Row %d: ", row);
            uart_send_string(buf);
            
            for (uint8_t col = 0; col < MATRIX_COLS; col++) {
                if (row_data[row][col].key_name) {
                    snprintf(buf, sizeof(buf), "%s=%d ", 
                             row_data[row][col].key_name,
                             row_data[row][col].adc_val);
                    uart_send_string(buf);
                }
            }
            uart_send_string("\n");
        }
        uart_send_string("\n");
    }
    
    return changed;
}