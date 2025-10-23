// Copyright 2023 QMK
// SPDX-License-Identifier: GPL-2.0-or-later
#include QMK_KEYBOARD_H
#include "../../uart.h"
#include "../../uart_keycodes.h"
#include "../../config.h"
#include "../../lighting.h"



// Custom layout macro for SHEGO75HE (81-key full-size exploded layout)
// Row 0: 14 keys, Row 1: 15 keys, Row 2: 15 keys, Row 3: 14 keys, Row 4: 13 keys, Row 5: 10 keys
#define SHEGO75HE( \
  K000, K001, K002, K003, K004, K005, K006, K007, K008, K009, K00A, K00B, K00C, K00D, \
  K100, K101, K102, K103, K104, K105, K106, K107, K108, K109, K10A, K10B, K10C, K10D, K10E, \
  K200, K201, K202, K203, K204, K205, K206, K207, K208, K209, K20A, K20B, K20C, K20D, K20E, \
  K300, K301, K302, K303, K304, K305, K306, K307, K308, K309, K30A, K30B, K30C, K30D, \
  K400, K401, K402, K403, K404, K405, K406, K407, K408, K409, K40A, K40B, K40C, \
  K500, K501, K502, K503, K504, K505, K506, K507, K508, K509 \
) \
LAYOUT_shego75he( \
  K000, K001, K002, K003, K004, K005, K006, K007, K008, K009, K00A, K00B, K00C, K00D, \
  K100, K101, K102, K103, K104, K105, K106, K107, K108, K109, K10A, K10B, K10C, K10D, K10E, \
  K200, K201, K202, K203, K204, K205, K206, K207, K208, K209, K20A, K20B, K20C, K20D, K20E, \
  K300, K301, K302, K303, K304, K305, K306, K307, K308, K309, K30A, K30B, K30C, K30D, \
  K400, K401, K402, K403, K404, K405, K406, K407, K408, K409, K40A, K40B, K40C, \
  K500, K501, K502, K503, K504, K505, K506, K507, K508, K509 \
)

// Note: VIA/Designer expects a PROGMEM encoder_map for encoder assignments.
// We'll provide a default encoder_map so VIA knows the keyboard has an encoder.
// Custom behavior is implemented in encoder_update_user below.

// (Timed hold logic removed for now; encoder press is handled as simple press.)

// Timestamp of last encoder press event (ms). Used to suppress accidental
// matrix key events that occur simultaneously with encoder presses.
uint32_t encoder_last_press = 0;

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [0] = SHEGO75HE(
        KC_ESC,  KC_F1,    KC_F2,    KC_F3,    KC_F4,    KC_F5,    KC_F6,    KC_F7,    KC_F8,    KC_F9,    KC_F10,  KC_F11, KC_F12, KC_DEL,
        KC_GRV,  KC_1,    KC_2,    KC_3,    KC_4,    KC_5,    KC_6,    KC_7,    KC_8,    KC_9,    KC_0,    KC_MINS, KC_EQL, KC_BSPC, KC_HOME,
        KC_TAB,  KC_Q,    KC_W,    KC_E,    KC_R,    KC_T,    KC_Y,    KC_U,    KC_I,    KC_O,    KC_P,    KC_LBRC, KC_RBRC, KC_BSLS, KC_MNXT,
        KC_CAPS,  KC_A,    KC_S,    KC_D,    KC_F,    KC_G,    KC_H,    KC_J,    KC_K,    KC_L,    KC_SCLN, KC_QUOT, KC_ENT, KC_MPRV,
        KC_LSFT, KC_Z,    KC_X,    KC_C,    KC_V,    KC_B,    KC_N,    KC_M,    KC_COMM, KC_DOT,  KC_SLSH, KC_RSFT, KC_UP,
        KC_LCTL, KC_LGUI, KC_LALT, KC_SPC,   KC_RALT,   MO(1),  KC_RCTL, KC_LEFT, KC_DOWN,  KC_RGHT
    ),
    [1] = SHEGO75HE(
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, SOCD_TOG, KC_TRNS,
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_PSCR, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_PGUP,
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, MENU_SELECT, KC_PGDN, 
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, MENU_UP,
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, TG(3), SETTINGS_OPEN, MENU_DOWN, TIMER_OPEN
    ),
    [2] = SHEGO75HE(
        QK_BOOT, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, 
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS
    ),
    [3] = SHEGO75HE(
        QK_CLEAR_EEPROM, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, SOCD_TOG, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, DEBUG_KEYS, DEBUG_ADC, DEBUG_RAW, KC_TRNS,
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, RESET_ESP,
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, TFT_BRIGHTNESS_UP,
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, TFT_BRIGHTNESS_DOWN,
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS
    )
    
};

// Encoder map for VIA - actual behavior in encoder_update_user
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
    // Set encoder switch pin and enable internal pull-ups for encoder pins (GP5/GP6)
    setPinInputHigh(ENCODER_SW_PIN);
    setPinInputHigh(GP5);
    setPinInputHigh(GP6);
    
    // Initialize LED control pin (GP23) as output, start with LED off
    setPinOutput(GP23);
    writePinLow(GP23);
    
    // Initialize UART on GP8/GP9 (uart1)
    uart_init_and_welcome();
    uart_init_rx();
    
    // Now safe to send debug messages
    uart_send_string("[keymap] keyboard_post_init_user\n");
    
    // RGB and lighting still disabled for now
    // rgb_matrix_enable();
    // lighting_init();
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