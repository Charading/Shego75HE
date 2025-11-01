// Copyright 2023 QMK
// SPDX-License-Identifier: GPL-2.0-or-later
#include QMK_KEYBOARD_H
#include "../../uart.h"
#include "../../uart_keycodes.h"
#include "../../config.h"
#include "../../lighting.h"

// Note: VIA/Designer expects a PROGMEM encoder_map for encoder assignments.
// We'll provide a default encoder_map so VIA knows the keyboard has an encoder.
// Custom behavior is implemented in encoder_update_user below.

// (Timed hold logic removed for now; encoder press is handled as simple press.)

// Timestamp of last encoder press event (ms). Used to suppress accidental
// matrix key events that occur simultaneously with encoder presses.
uint32_t encoder_last_press = 0;

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [0] = LAYOUT_ortho_4x12(
        KC_ESC,  KC_Q,    KC_W,    KC_E,    KC_R,    KC_T,    KC_Y,    KC_U,    KC_I,    KC_O,    KC_P,    KC_BSPC,
        KC_TAB,  KC_A,    KC_S,    KC_D,    KC_F,    KC_G,    KC_H,    KC_J,    KC_K,    KC_L,    KC_SCLN, KC_ENT,
        KC_LSFT, KC_Z,    KC_X,    KC_C,    KC_V,    KC_B,    KC_N,    KC_M,    KC_COMM, KC_DOT,  KC_UP, KC_RSFT,
        KC_LCTL, KC_LGUI, KC_LALT, MO(1),   TG(3),   KC_SPC,  KC_SPC,  KC_SPC,   KC_RALT, KC_LEFT, KC_DOWN,  KC_RGHT
    ),
    [1] = LAYOUT_ortho_4x12(
        QK_USER_0, SETTINGS_OPEN, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, TIMER_OPEN, SOCD_TOG,
        QK_USER_1, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        QK_USER_2, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, TFT_BRIGHTNESS_UP, KC_TRNS,
        QK_USER_3, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, TFT_BRIGHTNESS_DOWN, KC_TRNS
    ),
    [2] = LAYOUT_ortho_4x12(
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS
    ),
    [3] = LAYOUT_ortho_4x12(
        RESET_ESP, QK_CLEAR_EEPROM, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, DEBUG_KEYS, DEBUG_ADC, DEBUG_RAW,
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS
    )
};

// #define ENCODER_MAP_ENABLE

// Encoder map for VIA - actual behavior in encoder_update_user
#if defined(ENCODER_MAP_ENABLE)
#endif
const uint16_t PROGMEM encoder_map[][NUM_ENCODERS][2] = {
    [0] = { ENCODER_CCW_CW(KC_VOLD, KC_VOLU) },
    [1] = { ENCODER_CCW_CW(MENU_DOWN, MENU_UP) },
    [2] = { ENCODER_CCW_CW(KC_TRNS, KC_TRNS) },
    [3] = { ENCODER_CCW_CW(MENU_DOWN, MENU_UP) }
};

// Per-layer encoder press map (single encoder).
// Change these values to customize what the encoder press does per-layer.
// Use QK_USER_* entries to route the action to process_record_user (UART commands).
// Special encoder actions (not raw keycodes)
#define ENC_TOGGLE_L3 0xFFFE

const uint16_t encoder_press_map[][1] = {
    [0] = { KC_MPLY },      // layer 0: play/pause
    [1] = { ENC_TOGGLE_L3 }, // layer 1: toggle layer 3
    [2] = { QK_USER_4 },    // layer 2: SETTINGS_OPEN (handled in process_record_user)
    [3] = { QK_USER_3 },      // layer 3: MENU_SELECT via QK_USER_3
};

// Initialize UART after QMK is fully initialized
void keyboard_post_init_user(void) {
    // Set encoder switch pin and initialize UART
    // enable internal pull-ups for encoder switch and encoder pins (GP5/GP6)
    setPinInputHigh(ENCODER_SW_PIN);
    setPinInputHigh(GP5);
    setPinInputHigh(GP6);
    // initialize UART TX/RX and send welcome
    uart_init_and_welcome();
    uart_init_rx();
    // ensure RGB matrix is enabled and set a dim red baseline
    rgb_matrix_enable();
    rgb_matrix_set_color_all(26, 0, 0);
}

// Encoder handling - works alongside encoder_map
bool encoder_update_user(uint8_t index, bool clockwise) {
    if (clockwise) {
        tap_code(KC_VOLU);
    } else {
        tap_code(KC_VOLD);
    }
    return false;
}

// Encoder switch handling (simple press only).
void matrix_scan_user(void) {
    static bool encoder_switch_pressed = false;
    // Active low (assuming pullup)
    // Active low (assuming pullup)
    bool switch_state = !readPin(ENCODER_SW_PIN);

    // Detect press edge and debounce
#ifndef ENCODER_SWITCH_DEBOUNCE_MS
#define ENCODER_SWITCH_DEBOUNCE_MS 10
#endif
    static uint32_t last_switch_event = 0;
    if (switch_state && !encoder_switch_pressed) {
        // ignore if within debounce window
        if (timer_elapsed32(last_switch_event) > ENCODER_SWITCH_DEBOUNCE_MS) {
            uint8_t current_layer = get_highest_layer(layer_state);
            // determine number of configured layers in encoder_press_map
            size_t num_layers = sizeof(encoder_press_map) / sizeof(encoder_press_map[0]);
            uint16_t code = KC_MPLY; // default fallback
            if (current_layer < num_layers) {
                code = encoder_press_map[current_layer][0];
            }
            // transparent fallback -> default action
            if (code == KC_TRNS) code = KC_MPLY;
            // Debug print which code we're about to send
            {
                char _buf[48];
                snprintf(_buf, sizeof(_buf), "ENCODER_PRESS: code=0x%04X\n", code);
                uart_send_string(_buf);
            }
            // Special handling for the toggle action even if layer 3 has its own mapping.
            bool l1_active = (layer_state & (1UL << 1));
            bool l3_active = (layer_state & (1UL << 3));
            if (current_layer == 1 && encoder_press_map[1][0] == ENC_TOGGLE_L3) {
                // Press while on layer 1 -> turn on layer 3 and send MENU_OPEN
                if (!l3_active) {
                    layer_on(3);
                    uart_send_string("MENU_OPEN\n");
                }
            } else if (current_layer == 3 && l1_active) {
                // Press while on layer 3 with MO(1) held -> turn off layer 3
                layer_off(3);
            } else if (code == ENC_TOGGLE_L3) {
                // Fallback: toggle semantics when code explicitly contains the sentinel
                if (!l3_active) {
                    if (l1_active || current_layer == 1) {
                        layer_on(3);
                        uart_send_string("MENU_OPEN\n");
                    }
                } else {
                    if (l1_active) {
                        layer_off(3);
                    }
                }
            } else if (code >= QK_USER_0 && code <= QK_USER_4) {
                // Map QK_USER_* to UART commands directly to avoid generating HID keycodes
                switch (code) {
                    case QK_USER_0: uart_send_string("MENU_OPEN\n"); break;
                    case QK_USER_1: uart_send_string("MENU_UP\n"); break;
                    case QK_USER_2: uart_send_string("MENU_DOWN\n"); break;
                    case QK_USER_3: uart_send_string("MENU_SELECT\n"); break;
                    case QK_USER_4: uart_send_string("SETTINGS_OPEN\n"); break;
                }
            } else {
                // send the mapped code as a HID keycode
                tap_code16(code);
            }
            last_switch_event = timer_read32();
        }
    }
    // update pressed state
    encoder_switch_pressed = switch_state;

    // Poll UART for incoming messages
    uart_receive_task();
}

// Called when the active layer state changes. We use this to send TFT_FOCUS
// when layer 3 becomes active and BOARD_FOCUS when it leaves. We also
// manage the lighting override for layer3 so those LEDs are forced.
layer_state_t layer_state_set_user(layer_state_t state) {
    static bool prev_l3 = false;
    bool now_l3 = (state & (1UL << 3));
    if (now_l3 && !prev_l3) {
        // entering layer 3
        send_tft_focus();
        set_layer3_override(true);
    } else if (!now_l3 && prev_l3) {
        // leaving layer 3
        send_board_focus();
        set_layer3_override(false);
    }
    prev_l3 = now_l3;
    return state;
}

// LED update hook (caps lock state changes)
bool led_update_user(led_t led_state) {
    bool caps = led_state.caps_lock;
    set_caps_override(caps);
    return true; // allow default handling
}

// Note: process_record_user is defined in uart_keycodes.c