#include "uart_keycodes.h"
#include "socd.h"
#include "uart.h"
#include "wait.h"

// Pin used to reset external ESP device (active low pulse)
#ifndef ESP_RESET_PIN
#define ESP_RESET_PIN GP3
#endif

// Debug state variables
static bool adc_debug_enabled = false;   // Start disabled
static bool key_debug_enabled = false;   // Start disabled
static bool raw_debug_enabled = false;    // Start disabled
// SOCD lives in socd.c

// Toggle functions
void toggle_adc_debug(void) {
    adc_debug_enabled = !adc_debug_enabled;
    if (adc_debug_enabled) {
        uart_send_string("[DEBUG] ADC printing: ON\n");
    } else {
        uart_send_string("[DEBUG] ADC printing: OFF\n");
    }
}

void toggle_key_debug(void) {
    key_debug_enabled = !key_debug_enabled;
    if (key_debug_enabled) {
        uart_send_string("[DEBUG] Key printing: ON\n");
    } else {
        uart_send_string("[DEBUG] Key printing: OFF\n");
    }
}

void toggle_raw_debug(void) {
    raw_debug_enabled = !raw_debug_enabled;
    if (raw_debug_enabled) {
        uart_send_string("[DEBUG] Raw keycodes: ON\n");
    } else {
        uart_send_string("[DEBUG] Raw keycodes: OFF\n");
    }
}

// Getters for debug state
bool get_adc_debug_enabled(void) {
    return adc_debug_enabled;
}

bool get_key_debug_enabled(void) {
    return key_debug_enabled;
}

bool get_raw_debug_enabled(void) {
    return raw_debug_enabled;
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    bool pressed = record->event.pressed;

    if (pressed && get_raw_debug_enabled()) {
        // Print the raw keycode so you can see what VIA sends for a custom key
        char buf[64];
        snprintf(buf, sizeof(buf), "RAW_KEYCODE: 0x%04X\n", keycode);
        uart_send_string(buf);
    }

    // Delegate SOCD handling (WASD) to socd.c; if it returns false the event
    // should be suppressed.
    if (keycode == KC_A || keycode == KC_D || keycode == KC_W || keycode == KC_S) {
        if (!socd_process_key(keycode, pressed)) return false;
    }

    // First, handle VIA user slots (QK_USER_0..3) directly so VIA works without extra mapping.
    if (keycode == QK_USER_0) {
        if (record->event.pressed) {
            uart_send_string("MENU_OPEN\n");
            wait_ms(200);
        }
        return false;
    }
    if (keycode == QK_USER_1) {
        if (record->event.pressed) {
            uart_send_string("MENU_UP\n");
            wait_ms(200);
        }
        return false;
    }
    if (keycode == QK_USER_2) {
        if (record->event.pressed) {
            uart_send_string("MENU_DOWN\n");
            wait_ms(200);
        }
        return false;
    }
    if (keycode == QK_USER_3) {
        if (record->event.pressed) {
            uart_send_string("MENU_SELECT\n");
            wait_ms(200);
        }
        return false;
    }
    if (keycode == QK_USER_4) {
        if (record->event.pressed) {
            uart_send_string("SETTINGS_OPEN\n");
            wait_ms(200);
        }
        return false;
    }
    

    // Map VIA numeric custom keycodes to our enum-based custom keycodes.
    // VIA uses high-value codes (0xffff..0xfffc) to represent custom keys.
    uint16_t kc = keycode;
    if (keycode == 0x7E00) kc = MENU_OPEN;
    else if (keycode == 0x7E01) kc = MENU_UP;
    else if (keycode == 0x7E02) kc = MENU_DOWN;
    else if (keycode == 0x7E03) kc = MENU_SELECT;
    else if (keycode == 0x7E04) kc = DEBUG_ADC;    // VIA slot for ADC debug
    else if (keycode == 0x7E05) kc = DEBUG_KEYS;   // VIA slot for key debug
    else if (keycode == 0x7E06) kc = DEBUG_RAW;
    else if (keycode == 0x7E07) kc = RESET_ESP;   // VIA slot for ESP reset (sets GPIO3 low briefly)
    else if (keycode == 0x7E08) kc = SOCD_TOG;     // VIA slot for SOCD toggle
    else if (keycode == 0x7E09) kc = SETTINGS_OPEN; // VIA slot for SETTINGS_OPEN
    else if (keycode == 0x7E0A) kc = TIMER_OPEN;    // VIA slot for TIMER_OPEN
    else if (keycode == 0x7E0B) kc = TFT_BRIGHTNESS_UP;    // VIA slot for TFT_BRIGHTNESS_UP
    else if (keycode == 0x7E0C) kc = TFT_BRIGHTNESS_DOWN;  // VIA slot for TFT_BRIGHTNESS_DOWN

    switch (kc) {
               
        // New UART commands for menu control
        case MENU_OPEN: // MENU_OPEN (VIA)
            if (pressed) {
                uart_send_string("MENU_OPEN\n");
                wait_ms(200);
            }
            return false;
            
        case MENU_UP: // MENU_UP (VIA)
            if (pressed) {
                uart_send_string("MENU_UP\n");
                wait_ms(200);
            }
            return false;
            
        case MENU_DOWN: // MENU_DOWN (VIA)
            if (pressed) {
                uart_send_string("MENU_DOWN\n");
                wait_ms(200);
            }
            return false;
            
        case MENU_SELECT: // MENU_SELECT (VIA)
            if (pressed) {
                uart_send_string("MENU_SELECT\n");
                wait_ms(200);
            }
            return false;
        case SETTINGS_OPEN: // SETTINGS_OPEN (VIA)
            if (pressed) {
                uart_send_string("SETTINGS_OPEN\n");
                wait_ms(200);
            }
            return false;
        case TIMER_OPEN: // TIMER_OPEN (VIA)
            if (pressed) {
                uart_send_string("TIMER_OPEN\n");
                wait_ms(200);
            }
        case TFT_BRIGHTNESS_UP: // TFT_BRIGHTNESS_UP (VIA)
            if (pressed) {
                uart_send_string("TFT_BRIGHTNESS_UP\n");
                wait_ms(200);
            }
            return false;
        case TFT_BRIGHTNESS_DOWN: // TFT_BRIGHTNESS_DOWN (VIA)
            if (pressed) {
                uart_send_string("TFT_BRIGHTNESS_DOWN\n");
                wait_ms(200);
            }
            return false;
        // Debug toggle keycodes
        case DEBUG_ADC:
            if (pressed) toggle_adc_debug();
            return false;
            
        case DEBUG_KEYS:
            if (pressed) toggle_key_debug();
            return false;
            
        case DEBUG_RAW:
            if (pressed) toggle_raw_debug();
            return false;

        case SOCD_TOG: // SOCD toggle (VIA)
            if (pressed) toggle_socd();
            return false;

        case RESET_ESP: {
            if (pressed) {
                // Safer reset pulse for external ESP device.
                // Idle state: configure pin as input with pull-up so we don't
                // actively drive the line (avoids contention with target).
                setPinInputHigh(ESP_RESET_PIN);

                // Send notification before pulsing so it appears on UART
                uart_send_string("RESET_ESP: pulsing ESP reset pin\n");

                // Determine assert/release levels depending on wiring.
                #if ESP_RESET_ACTIVE_HIGH
                            // Active HIGH: drive pin HIGH to assert reset
                            setPinOutput(ESP_RESET_PIN);
                            writePinHigh(ESP_RESET_PIN); // assert (active high)
                            wait_ms(500);
                            // Release: go back to input+pull-up
                            setPinInputHigh(ESP_RESET_PIN);
                            wait_ms(150);
                #else
                            // Active LOW (default): drive pin LOW to assert reset
                            setPinOutput(ESP_RESET_PIN);
                            writePinLow(ESP_RESET_PIN); // assert (active low)
                            wait_ms(500);
                            // Release: go back to input+pull-up
                            setPinInputHigh(ESP_RESET_PIN);
                            wait_ms(150);
                #endif
            }
            return false;
        }
        
    }

    return true;
}