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

#define DEBOUNCE_MS 5

// NOTE: On this board the ADG732 EN pins are grounded (always enabled).
// We therefore select each ADG732 using its CS pin (active low) and latch
// addresses with the WR pin. If you have external EN pins, you can enable
// them by defining MUX1_EN / MUX2_EN / MUX3_EN in your board config.

// Key state tracking for 81 keys (6 rows × 15 cols max)
// Using MATRIX_ROWS * MATRIX_COLS = 6 * 15 = 90 slots to be safe
#define MAX_KEYS (MATRIX_ROWS * MATRIX_COLS)
static bool key_pressed[MAX_KEYS];
static uint32_t key_timer[MAX_KEYS];

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

void matrix_init_custom(void) {
    // Diagnostic stub: do minimal init and skip full MUX setup.
    // Debug print so we can see whether init completes
    uart_send_string("[mux_adc] matrix_init_custom start\n");
    // This prevents the custom scanning code from running until we verify it's safe.
    // Keep pin setup minimal so other code can run without touching ADC/MUXs.
    setPinOutput(MUX_A0);
    setPinOutput(MUX_A1);
    setPinOutput(MUX_A2);

    // Initialize state for all possible keys (safe defaults)
    for (uint8_t i = 0; i < MAX_KEYS; i++) {
        key_pressed[i] = false;
        key_timer[i] = 0;
    }
    uart_send_string("[mux_adc] matrix_init_custom done\n");
}

bool matrix_scan_custom(matrix_row_t current_matrix[]) {
    bool changed = false;
    uint32_t now = timer_read32();

    // Clear matrix output
    for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
        current_matrix[row] = 0;
    }

    // ADC pins and mapping tables
    pin_t adc_pins[3] = {MUX1_ADC_PIN, MUX2_ADC_PIN, MUX3_ADC_PIN};
    const mux32_ref_t* mux_tables[3] = {mux1_channels, mux2_channels, mux3_channels};

    const uint16_t ADC_GND_THRESHOLD = 20; // skip obviously unconnected channels

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
            wait_us(50);

            uint16_t adc_val = read_adc_pin(adc_pins[mux_idx]);

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

            bool should_press = (adc_val < SENSOR_THRESHOLD);

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

    (void)debug_counter; (void)last_debug_time;
    return changed;
}