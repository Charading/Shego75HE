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
#include <string.h>

#define DEBOUNCE_MS 5

// ESP_RESET_PIN definition (GP3 - bootloader trigger pin, must be HIGH during boot)
#ifndef ESP_RESET_PIN
#define ESP_RESET_PIN GP3
#endif

// ADC debug output buffer (for formatting display)
static uint16_t adc_values[90];  // Store all ADC values for display
static uint32_t last_adc_print_time = 0;
#define ADC_PRINT_INTERVAL_MS 1000  // Print every 1000ms (1 second)

// NOTE: On this board the ADG732 EN pins are grounded (always enabled).
// We therefore select each ADG732 using its CS pin (active low) and latch
// addresses with the WR pin. If you have external EN pins, you can enable
// them by defining MUX1_EN / MUX2_EN / MUX3_EN in your board config.

// Key state tracking for 81 keys (6 rows × 15 cols max)
// Using MATRIX_ROWS * MATRIX_COLS = 6 * 15 = 90 slots to be safe
#define MAX_KEYS (MATRIX_ROWS * MATRIX_COLS)
static bool key_pressed[MAX_KEYS];
static uint32_t key_timer[MAX_KEYS];

// Auto-calibration storage
static uint16_t key_baseline[MAX_KEYS];      // Baseline (resting) ADC value for each key
static uint16_t key_threshold[MAX_KEYS];     // Dynamic threshold for each key
static bool calibration_complete = false;    // Flag indicating calibration status

// Debug counter to limit output
// Debug counters (may be used later); mark volatile to avoid unused warnings
static volatile uint32_t debug_counter = 0;
static volatile uint32_t last_debug_time = 0;

// Helper for reading ADC
// Helper for reading ADC (kept static but may be unused in diagnostic stub)
static inline uint16_t read_adc_pin(pin_t pin) {
    return analogReadPin(pin);
}

// Helper for selecting MUX channel
// Helper to select MUX channel (kept inline to avoid unused-function error)
static inline void select_mux_channel(uint8_t channel) {
    // Set address lines (A0..A4)
    writePin(MUX_A0, (channel & 0x01) ? 1 : 0);
    writePin(MUX_A1, (channel & 0x02) ? 1 : 0);
    writePin(MUX_A2, (channel & 0x04) ? 1 : 0);
    writePin(MUX_A3, (channel & 0x08) ? 1 : 0);
    writePin(MUX_A4, (channel & 0x10) ? 1 : 0);

    // Pulse WR low to latch the address into the ADG732 (falling edge triggers latch)
#ifdef MUX_WR
    writePinLow(MUX_WR);
    wait_us(5); // short pulse to ensure latch
    writePinHigh(MUX_WR);
#else
    // If no WR pin is defined, give address lines time to settle
    wait_us(50);
#endif
}

// Initialize ESP_RESET_PIN early to prevent bootloader trigger
// DISABLED FOR TESTING
/*
void keyboard_pre_init_kb(void) {
    // Ensure ESP reset pin (GP3) stays HIGH to avoid bootloader mode
    setPinOutput(ESP_RESET_PIN);
    writePinHigh(ESP_RESET_PIN);
}
*/

void matrix_init_custom(void) {
    // Ensure ESP_RESET_PIN stays HIGH
    setPinOutput(ESP_RESET_PIN);
    writePinHigh(ESP_RESET_PIN);
    
    // Minimal MUX pin setup
    setPinOutput(MUX_A0);
    setPinOutput(MUX_A1);
    setPinOutput(MUX_A2);
    setPinOutput(MUX_A3);
    setPinOutput(MUX_A4);
    
#ifdef MUX_WR
    setPinOutput(MUX_WR);
    writePinHigh(MUX_WR);
#endif

#ifdef MUX_CS1
    setPinOutput(MUX_CS1);
    writePinHigh(MUX_CS1);
#endif
#ifdef MUX_CS2
    setPinOutput(MUX_CS2);
    writePinHigh(MUX_CS2);
#endif
#ifdef MUX_CS3
    setPinOutput(MUX_CS3);
    writePinHigh(MUX_CS3);
#endif

    // Initialize key state arrays
    for (uint8_t i = 0; i < MAX_KEYS; i++) {
        key_pressed[i] = false;
        key_timer[i] = 0;
        key_baseline[i] = 0;
        key_threshold[i] = SENSOR_THRESHOLD; // Use default until calibration completes
    }
}

// Auto-calibration: Scan all keys and establish baseline + dynamic thresholds
// This should be called after matrix_init_custom, during keyboard_post_init_kb
void calibrate_sensors(void) {
    pin_t adc_pins[3] = {MUX1_ADC_PIN, MUX2_ADC_PIN, MUX3_ADC_PIN};
    const mux32_ref_t* mux_tables[3] = {mux1_channels, mux2_channels, mux3_channels};
    
    // Perform multiple reads per key and average them for stability
    const uint8_t CALIBRATION_SAMPLES = 5;
    uint32_t sample_accumulator[MAX_KEYS] = {0};
    uint8_t sample_count[MAX_KEYS] = {0};
    
    // Collect samples
    for (uint8_t sample = 0; sample < CALIBRATION_SAMPLES; sample++) {
        for (uint8_t mux_idx = 0; mux_idx < 3; mux_idx++) {
            // Select appropriate CS
#ifdef MUX_CS1
            if (mux_idx == 0) { writePinLow(MUX_CS1); } else { writePinHigh(MUX_CS1); }
#endif
#ifdef MUX_CS2
            if (mux_idx == 1) { writePinLow(MUX_CS2); } else { writePinHigh(MUX_CS2); }
#endif
#ifdef MUX_CS3
            if (mux_idx == 2) { writePinLow(MUX_CS3); } else { writePinHigh(MUX_CS3); }
#endif

            for (uint8_t ch = 0; ch < 32; ch++) {
                select_mux_channel(ch);
                wait_us(100);

                uint16_t adc_val = read_adc_pin(adc_pins[mux_idx]);
                
                // Filter out anomalous high values
                const uint16_t ADC_MAX_VALID = 800;
                if (adc_val > ADC_MAX_VALID) {
                    adc_val = 4095;
                }

                const mux32_ref_t* key_mapping = &mux_tables[mux_idx][ch + 1];
                if (!key_mapping || key_mapping->sensor == 0) continue;

                uint16_t sensor = key_mapping->sensor;
                uint8_t matrix_row = (sensor - 1) / MATRIX_COLS;
                uint8_t matrix_col = (sensor - 1) % MATRIX_COLS;

                if (matrix_row >= MATRIX_ROWS || matrix_col >= MATRIX_COLS) continue;

                uint16_t key_idx = (matrix_row * MATRIX_COLS) + matrix_col;
                if (key_idx >= MAX_KEYS) continue;

                // Accumulate valid readings only (skip obvious disconnects)
                if (adc_val < 4000) {
                    sample_accumulator[key_idx] += adc_val;
                    sample_count[key_idx]++;
                }
            }

            // Release CS
#ifdef MUX_CS1
            writePinHigh(MUX_CS1);
#endif
#ifdef MUX_CS2
            writePinHigh(MUX_CS2);
#endif
#ifdef MUX_CS3
            writePinHigh(MUX_CS3);
#endif
        }
        wait_ms(10); // Small delay between calibration samples
    }
    
    // Calculate baseline and threshold for each key
    for (uint8_t key_idx = 0; key_idx < MAX_KEYS; key_idx++) {
        if (sample_count[key_idx] > 0) {
            // Calculate average baseline
            key_baseline[key_idx] = sample_accumulator[key_idx] / sample_count[key_idx];
            
            // Calculate dynamic threshold as percentage of baseline
            // CALIBRATION_THRESHOLD_PERCENT is defined in mux_adc.h (e.g., 85 = 85% of baseline)
            key_threshold[key_idx] = (key_baseline[key_idx] * CALIBRATION_THRESHOLD_PERCENT) / 100;
            
            // Safety clamps: ensure threshold is reasonable
            if (key_threshold[key_idx] < 100) {
                key_threshold[key_idx] = 100; // Minimum threshold
            }
            if (key_threshold[key_idx] > 700) {
                key_threshold[key_idx] = 700; // Maximum threshold
            }
        } else {
            // No valid readings - use default
            key_baseline[key_idx] = 512;
            key_threshold[key_idx] = SENSOR_THRESHOLD;
        }
    }
    
    calibration_complete = true;
}

bool matrix_scan_custom(matrix_row_t current_matrix[]) {
    bool changed = false;
    uint32_t now = timer_read32();

    // Clear matrix output
    for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
        current_matrix[row] = 0;
    }
    
    // Clear ADC values array
    for (uint8_t i = 0; i < 90; i++) {
        adc_values[i] = 0;
    }

    // ADC pins and mapping tables
    pin_t adc_pins[3] = {MUX1_ADC_PIN, MUX2_ADC_PIN, MUX3_ADC_PIN};
    const mux32_ref_t* mux_tables[3] = {mux1_channels, mux2_channels, mux3_channels};

    const uint16_t ADC_GND_THRESHOLD = 100; // skip obviously unconnected channels

    // Scan each mux and channel
    for (uint8_t mux_idx = 0; mux_idx < 3; mux_idx++) {
        // Select appropriate CS
#ifdef MUX_CS1
        if (mux_idx == 0) { writePinLow(MUX_CS1); } else { writePinHigh(MUX_CS1); }
#endif
#ifdef MUX_CS2
        if (mux_idx == 1) { writePinLow(MUX_CS2); } else { writePinHigh(MUX_CS2); }
#endif
#ifdef MUX_CS3
        if (mux_idx == 2) { writePinLow(MUX_CS3); } else { writePinHigh(MUX_CS3); }
#endif

        for (uint8_t ch = 0; ch < 32; ch++) {
            select_mux_channel(ch);
            wait_us(100);  // Increased delay without filter caps to allow signal settling

            uint16_t adc_val = read_adc_pin(adc_pins[mux_idx]);
            
            // Filter out anomalous high values (crosstalk without filter caps)
            // Valid Hall sensor range: 0 (pressed) to ~512 (released)
            // Values above 800 are likely crosstalk
            const uint16_t ADC_MAX_VALID = 800;
            if (adc_val > ADC_MAX_VALID) {
                adc_val = 4095;  // Treat as invalid/unpressed
            }

            const mux32_ref_t* key_mapping = &mux_tables[mux_idx][ch + 1];
            if (!key_mapping) continue;

            // If channel is unmapped and reads near GND, skip
            if (key_mapping->sensor == 0) {
                if (adc_val <= ADC_GND_THRESHOLD) continue;
                else continue;
            }

            uint16_t sensor = key_mapping->sensor;
            if (sensor == 0) continue;

            // Map sensor to matrix position
            uint8_t matrix_row = (sensor - 1) / MATRIX_COLS;
            uint8_t matrix_col = (sensor - 1) % MATRIX_COLS;

            if (matrix_row >= MATRIX_ROWS || matrix_col >= MATRIX_COLS) continue;

            uint16_t key_idx = (matrix_row * MATRIX_COLS) + matrix_col;
            if (key_idx >= MAX_KEYS) continue;
            
            // Store ADC value for debug display (show filtered value)
            if (key_idx < 90) {
                adc_values[key_idx] = adc_val;
            }

            // Use dynamic per-key threshold if calibration is complete
            uint16_t threshold = calibration_complete ? key_threshold[key_idx] : SENSOR_THRESHOLD;
            bool should_press = (adc_val < threshold);

            // Debounce: only change state if debounce time elapsed
            if (timer_elapsed32(key_timer[key_idx]) > DEBOUNCE_MS) {
                if (should_press != key_pressed[key_idx]) {
                    key_pressed[key_idx] = should_press;
                    key_timer[key_idx] = now;
                    changed = true;
                }
            }

            if (key_pressed[key_idx]) {
                current_matrix[matrix_row] |= (1 << matrix_col);
            }
        }

        // Release CS for this MUX (keep others disabled)
#ifdef MUX_CS1
        writePinHigh(MUX_CS1);
#endif
#ifdef MUX_CS2
        writePinHigh(MUX_CS2);
#endif
#ifdef MUX_CS3
        writePinHigh(MUX_CS3);
#endif
    }

    // Print ADC values if debug enabled (every 1000ms = 1 second)
    if (get_adc_debug_enabled() && timer_elapsed32(last_adc_print_time) > ADC_PRINT_INTERVAL_MS) {
        last_adc_print_time = now;
        
        // Build entire display in one giant buffer matching zuart.txt format exactly
        static char adc_display[1200];
        int pos = 0;
        
        // Row 0: F-keys (14 positions, using indices 0-11,12,13 skipping 11 for F11)
        pos += snprintf(adc_display + pos, sizeof(adc_display) - pos, 
                       "|Esc:   %04d |F1:  %04d |F2    %04d |F3:  %04d |F4:   %04d |F5: %04d |F6: %04d |F7: %04d |F8: %04d |F9: %04d |F10: %04d |F12:   %04d |Del: %04d |\n",
                       adc_values[0], adc_values[1], adc_values[2], adc_values[3], adc_values[4], adc_values[5], 
                       adc_values[6], adc_values[7], adc_values[8], adc_values[9], adc_values[10], adc_values[12], adc_values[13]);
        
        // Row 1: Number row (15 positions, indices 15-29)
        pos += snprintf(adc_display + pos, sizeof(adc_display) - pos,
                       "|`:     %04d |1:   %04d |2:    %04d |3:   %04d |4:    %04d |5:  %04d |6:  %04d |7:  %04d |8:  %04d |9:  %04d |0:   %04d |-:     %04d |=:   %04d |Back: %04d |\n",
                       adc_values[15], adc_values[16], adc_values[17], adc_values[18], adc_values[19], adc_values[20], 
                       adc_values[21], adc_values[22], adc_values[23], adc_values[24], adc_values[25], adc_values[26], adc_values[27], adc_values[28]);
        
        // Row 2: QWERTY row (15 positions, indices 30-44)
        pos += snprintf(adc_display + pos, sizeof(adc_display) - pos,
                       "|Tab:   %04d |Q:   %04d |W:    %04d |E:   %04d |R:    %04d |T:  %04d |Y:  %04d |U:  %04d |I:  %04d |O:  %04d |P:   %04d |[:     %04d |]:   %04d |Home: %04d |\n",
                       adc_values[30], adc_values[31], adc_values[32], adc_values[33], adc_values[34], adc_values[35], 
                       adc_values[36], adc_values[37], adc_values[38], adc_values[39], adc_values[40], adc_values[41], adc_values[42], adc_values[44]);
        
        // Row 3: ASDF row (14 positions, indices 45-58)
        pos += snprintf(adc_display + pos, sizeof(adc_display) - pos,
                       "|Caps:  %04d |A:   %04d |S:    %04d |D:   %04d |F:    %04d |G:  %04d |H:  %04d |J:  %04d |K:  %04d |L:  %04d |;:   %04d |':     %04d |Ent: %04d |PgUp: %04d |\n",
                       adc_values[45], adc_values[46], adc_values[47], adc_values[48], adc_values[49], adc_values[50], 
                       adc_values[51], adc_values[52], adc_values[53], adc_values[54], adc_values[55], adc_values[56], adc_values[57], adc_values[58]);
        
        // Row 4: ZXCV row (13 positions, indices 60-72 with padding)
        pos += snprintf(adc_display + pos, sizeof(adc_display) - pos,
                       "|LShft: %04d |Z:   %04d |X:    %04d |C:   %04d |V:    %04d |B:  %04d |N:  %04d |M:  %04d |,:  %04d |.:  %04d |/:   %04d |RShft: %04d |↑:   %04d |\n",
                       adc_values[60], adc_values[61], adc_values[62], adc_values[63], adc_values[64], adc_values[65], 
                       adc_values[66], adc_values[67], adc_values[68], adc_values[69], adc_values[70], adc_values[71], adc_values[72]);
        
        // Row 5: Bottom row (10 positions, indices 75-84 with padding)
        pos += snprintf(adc_display + pos, sizeof(adc_display) - pos,
                       "|Ctrl:  %04d |Win: %04d |RAlt: %04d |                      Spc: %04d                     |Alt:%04d |Fn: %04d |          |←:     %04d |↓:   %04d |→:    %04d |\n\n",
                       adc_values[75], adc_values[76], adc_values[77], adc_values[78], adc_values[79], adc_values[80], 
                       adc_values[82], adc_values[83], adc_values[84]);
        
        // Send entire buffer at once via UART debug (bypasses HID console line buffering)
        uart_debug_print(adc_display);
    }

    (void)debug_counter; (void)last_debug_time;
    return changed;
}