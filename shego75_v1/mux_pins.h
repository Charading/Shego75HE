#pragma once

#include <stddef.h>  // for NULL
#include <stdint.h>  // for uint8_t

#define KEY_COUNT 81 // 4 rows * 12 columns


typedef enum KeyName{
    // Row 1 - 14 Keys
    K_ESC = 1, K_F1, K_F2, K_F3, K_F4, K_F5, K_F6, K_F7, K_F8, K_F9, K_F10, K_F11, K_F12, K_DEL,
    // Row 2 - 15 Keys
    K_GRAVE, K_1, K_2, K_3, K_4, K_5, K_6, K_7, K_8, K_9, K_0, K_MINUS, K_EQUAL, K_BACKSPACE, K_HOME,
    // Row 3 - 15 Keys
    K_TAB, K_Q, K_W, K_E, K_R, K_T, K_Y, K_U, K_I, K_O, K_P, K_LBRC, K_RBRC, K_BSLASH, K_PGUP,
    // Row 4 - 14 Keys
    K_CAPS, K_A, K_S, K_D, K_F, K_G, K_H, K_J, K_K, K_L, K_SEMI, K_QUOTE, K_ENTER, K_PGDN,
    // Row 5 - 13 Keys
    K_LSHIFT, K_Z, K_X, K_C, K_V, K_B, K_N, K_M, K_COMMA, K_DOT, K_SLASH, K_RSHIFT, K_UP,
    // Row 6 - 10 Keys
    K_LCTRL, K_LWIN, K_LALT, K_SPACE, K_RALT, K_FN, K_RCTRL, K_LEFT, K_DOWN, K_RIGHT,

    SENSOR_COUNT_PLUS_1
} KeyName;

// Struct for each channel entry
typedef struct {
    KeyName sensor;   // which key/sensor this channel corresponds to
} mux32_ref_t;



// Extern declarations for each MUX table
extern const char *sensor_names[KEY_COUNT];
extern const uint16_t sensor_to_keycode[KEY_COUNT];
extern const mux32_ref_t mux1_channels[33];
extern const mux32_ref_t mux2_channels[33];
extern const mux32_ref_t mux3_channels[33];
