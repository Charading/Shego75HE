#include "uart_keycodes.h"
#include "socd.h"
#include "uart.h"
#include "wait.h"
#include "lighting.h"
#include <stdio.h>

// Pin used to reset external ESP device (active low pulse)
#ifndef ESP_RESET_PIN
#define ESP_RESET_PIN GP3
#endif

// Debug state variables
static bool adc_debug_enabled = false;   // Start disabled (no UART debug output)
static bool key_debug_enabled = false;   // Start disabled
static bool raw_debug_enabled = false;    // Start disabled
static bool led_enabled = true;          // Logical LED state
// SOCD lives in socd.c

// Toggle functions
void toggle_adc_debug(void) {
    adc_debug_enabled = !adc_debug_enabled;
    if (adc_debug_enabled) {
        uart_debug_print("[DEBUG] ADC printing: ON\n");
    } else {
        uart_debug_print("[DEBUG] ADC printing: OFF\n");
    }
}

void toggle_key_debug(void) {
    key_debug_enabled = !key_debug_enabled;
    if (key_debug_enabled) {
        uart_debug_print("[DEBUG] Key printing: ON\n");
    } else {
        uart_debug_print("[DEBUG] Key printing: OFF\n");
    }
}

void toggle_raw_debug(void) {
    raw_debug_enabled = !raw_debug_enabled;
    if (raw_debug_enabled) {
        uart_debug_print("[DEBUG] Raw keycodes: ON\n");
    } else {
        uart_debug_print("[DEBUG] Raw keycodes: OFF\n");
    }
}

static void drive_led_pin(bool on) {
#ifdef LED_TOG_PIN
    setPinOutput(LED_TOG_PIN);
    bool level = LED_TOG_ACTIVE_HIGH ? on : !on;
    writePin(LED_TOG_PIN, level ? 1 : 0);
    char buf[80];
    snprintf(buf, sizeof(buf), "[LED] logical=%s level=%s (pin %d)\n", on ? "ON" : "OFF", level ? "HIGH" : "LOW", LED_TOG_PIN);
    uart_debug_print(buf);
#else
    (void)on;
#endif
}

void led_set_state(bool on) {
    led_enabled = on;
    drive_led_pin(led_enabled);
}

void toggle_led(void) {
    led_enabled = !led_enabled;
    drive_led_pin(led_enabled);
}

// Small helper: toggle onboard Pico LED (GP25) for a visible ACK
void toggle_onboard_led(bool on) {
    // Some QMK/boards may not define GP25; ensure macro exists
#ifdef GP25
    setPinOutput(GP25);
    if (on) writePinHigh(GP25);
    else writePinLow(GP25);
#endif
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

bool get_led_enabled(void) {
    return led_enabled;
}

uint8_t get_led_pin_level(void) {
#ifdef LED_TOG_PIN
    return readPin(LED_TOG_PIN) ? 1 : 0;
#else
    return 0;
#endif
}

uint8_t get_led_pin_number(void) {
#ifdef LED_TOG_PIN
    return LED_TOG_PIN;
#else
    return 0xFF;
#endif
}

bool led_pin_active_high(void) {
#ifdef LED_TOG_ACTIVE_HIGH
    return LED_TOG_ACTIVE_HIGH;
#else
    return true;
#endif
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    bool pressed = record->event.pressed;

    if (pressed && get_raw_debug_enabled()) {
        // Print the raw keycode to debug UART (GP0)
        char buf[64];
        snprintf(buf, sizeof(buf), "RAW_KEYCODE: 0x%04X\n", keycode);
        uart_debug_print(buf);
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
    else if (keycode == 0x7E0D) kc = LED_TOG;              // VIA slot for LED_TOG

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

        case LED_TOG: // LED toggle (VIA)
            if (pressed) toggle_led();
            return false;

        case RESET_ESP: {
            if (pressed) {
                // ESP32 reset control via AO3400 N-channel MOSFET on GP3
                // Normal: Hi-Z = MOSFET off, Reset: HIGH = MOSFET on
                
                // Send notification before reset
                uart_send_string("RESET_ESP: resetting ESP32 via N-channel MOSFET\n");

                // Assert reset: Drive GP3 HIGH to turn on N-channel MOSFET
                // This pulls ESP_EN low, putting ESP32 in reset
                setPinOutput(ESP_RESET_PIN);
                writePinHigh(ESP_RESET_PIN);  // Turn on MOSFET -> ESP_EN goes LOW
                wait_ms(500);                 // Hold reset for 500ms
                
                // Release reset: Make GP3 Hi-Z (no pull-up) to turn off N-channel MOSFET  
                // This releases ESP_EN, allowing ESP32 to boot
                setPinInput(ESP_RESET_PIN);     // Hi-Z without pull-up -> MOSFET off -> ESP_EN goes HIGH
                wait_ms(150);                   // Wait for ESP32 to start booting
            }
            return false;
        }
        
    }

    return true;
}