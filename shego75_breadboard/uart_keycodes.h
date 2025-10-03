// uart_keycodes.h - Custom keycodes for UART debugging
#pragma once

#include "quantum.h"

// Define custom keycodes starting from SAFE_RANGE
enum custom_keycodes {
    MENU_OPEN = SAFE_RANGE,
    MENU_UP,
    MENU_DOWN,
    MENU_SELECT,
    DEBUG_ADC,      // Toggle ADC value printing
    DEBUG_KEYS,     // Toggle keypress event printing
    DEBUG_RAW,      // Toggle raw keycode printing
    RESET_ESP,      // Reset connected ESP32 device via UART and transistive logic on GPIO3
    SOCD_TOG,       // SOCD toggle (VIA)
    SETTINGS_OPEN,  // Open settings menu (VIA)
    TIMER_OPEN,      // Open timer menu (VIA)
    TFT_BRIGHTNESS_UP,    // Increase TFT brightness (VIA)
    TFT_BRIGHTNESS_DOWN  // Decrease TFT brightness (VIA)
};

// If your external reset transistor inverts the MCU GPIO (eg. BSS138 with gate pulled to
// ground and MOSFET pulling ESP_EN low when MCU drives the gate high), define
// ESP_RESET_ACTIVE_HIGH to 1 so the firmware will drive the pin HIGH to assert reset.
// Default behavior matches earlier code: active-low reset (drive pin LOW to reset).
#ifndef ESP_RESET_ACTIVE_HIGH
// Default to 1 because the breadboard uses a BSS138 that inverts the MCU GPIO
// (MCU HIGH -> MOSFET ON -> ESP_EN pulled LOW). Set to 0 if your circuit is
// non-inverting (drive LOW to reset).
#define ESP_RESET_ACTIVE_HIGH 1
#endif

// Function declarations
bool process_record_user(uint16_t keycode, keyrecord_t *record);

// Debug control functions
void toggle_adc_debug(void);
void toggle_key_debug(void);
void toggle_raw_debug(void);
bool get_adc_debug_enabled(void);
bool get_key_debug_enabled(void);
bool get_raw_debug_enabled(void);
