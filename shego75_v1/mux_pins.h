#pragma once

#include <stddef.h>  // for NULL
#include <stdint.h>  // for uint8_t

#define KEY_COUNT 81 // 4 rows * 12 columns


typedef enum KeyName{
    // Row 0 (matrix row 0) - 14 Keys: positions [0,0] to [0,13], skip [0,14]
    K_ESC = 1, K_F1, K_F2, K_F3, K_F4, K_F5, K_F6, K_F7, K_F8, K_F9, K_F10, K_F11, K_F12, K_DEL,
    _PAD_R0, // Padding to align to 15-column matrix
    
    // Row 1 (matrix row 1) - 15 Keys: positions [1,0] to [1,14]
    K_GRAVE, K_1, K_2, K_3, K_4, K_5, K_6, K_7, K_8, K_9, K_0, K_MINUS, K_EQUAL, K_BACKSPACE, K_HOME,
    
    // Row 2 (matrix row 2) - 15 Keys: positions [2,0] to [2,14]
    K_TAB, K_Q, K_W, K_E, K_R, K_T, K_Y, K_U, K_I, K_O, K_P, K_LBRC, K_RBRC, K_BSLASH, K_PGUP,
    
    // Row 3 (matrix row 3) - 14 Keys: positions [3,0] to [3,13], skip [3,14]
    K_CAPS, K_A, K_S, K_D, K_F, K_G, K_H, K_J, K_K, K_L, K_SEMI, K_QUOTE, K_ENTER, K_PGDN,
    _PAD_R3, // Padding to align to 15-column matrix
    
    // Row 4 (matrix row 4) - 13 Keys: positions [4,0] to [4,12], skip [4,13-14]
    K_LSHIFT, K_Z, K_X, K_C, K_V, K_B, K_N, K_M, K_COMMA, K_DOT, K_SLASH, K_RSHIFT, K_UP,
    _PAD_R4_1, _PAD_R4_2, // Padding to align to 15-column matrix
    
    // Row 5 (matrix row 5) - 10 Keys: positions [5,0] to [5,9], skip [5,10-14]
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
