#pragma once

#include <stddef.h>  // for NULL
#include <stdint.h>  // for uint8_t

#define KEY_COUNT SENSOR_COUNT

typedef enum KeyName {
    K_ESC = 0, K_Q, K_W, K_E, K_R, K_T, K_Y, K_U, K_I, K_O, K_P, K_BSPC,
    K_TAB, K_A, K_S, K_D, K_F, K_G, K_H, K_J, K_K, K_L, K_SCLN, K_ENT,
    K_LSFT, K_Z, K_X, K_C, K_V, K_B, K_N, K_M, K_COMM, K_DOT, K_UP, K_RSFT,
    K_LCTL, K_WIN, K_LALT, K_MO1, K_TG3, K_SPC1, K_SPC2, K_FN, K_RALT, K_LEFT, K_DOWN, K_RGHT,

    SENSOR_COUNT,
} Keys;

// Struct for each channel entry
typedef struct {
    Keys sensor;       // Key associated with this channel
} mux16_ref_t;

// Extern declarations for each MUX table
extern const char *sensor_names[KEY_COUNT];
extern const uint16_t sensor_to_keycode[KEY_COUNT];
extern const mux16_ref_t mux1_channels[16];
extern const mux16_ref_t mux2_channels[16];
extern const mux16_ref_t mux3_channels[16];
