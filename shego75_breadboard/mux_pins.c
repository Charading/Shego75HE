#include "mux_pins.h"
#include "quantum.h"

// MASTER SENSOR-TO-KEYCODE LOOKUP TABLE
// This maps sensor numbers 1-48 directly to QMK keycodes based on your 4x12 layout
// Sensor numbers are calculated as: (matrix_row * 12) + matrix_col + 1
// Your keymap layout:
//   Row 0: ESC  Q    W    E    R    T    Y    U    I    O    P    BSPC
//   Row 1: TAB  A    S    D    F    G    H    J    K    L    SCLN ENT
//   Row 2: LSFT Z    X    C    V    B    N    M    COMM DOT  SLSH RSFT
//   Row 3: LCTL LGUI LALT MO1  SPC  SPC  SPC  MO2  RALT RGUI APP  RCTL

const uint16_t sensor_to_keycode[KEY_COUNT] = {
    // Sensors 1-12 (Row 0): ESC through BSPC
    KC_ESC,  KC_Q,    KC_W,    KC_E,    KC_R,    KC_T,    KC_Y,    KC_U,    KC_I,    KC_O,    KC_P,    KC_BSPC,
    // Sensors 13-24 (Row 1): TAB through ENT
    KC_TAB,  KC_A,    KC_S,    KC_D,    KC_F,    KC_G,    KC_H,    KC_J,    KC_K,    KC_L,    KC_SCLN, KC_ENT,
    // Sensors 25-36 (Row 2): LSFT through RSFT
    KC_LSFT, KC_Z,    KC_X,    KC_C,    KC_V,    KC_B,    KC_N,    KC_M,    KC_COMM, KC_DOT,  KC_SLSH, KC_RSFT,
    // Sensors 37-48 (Row 3): LCTL through RCTL  (use 0x5D00 for MO(1), 0x5D01 for MO(2))
    KC_LCTL, KC_LGUI, KC_LALT, 0x5D00,  KC_SPC,  KC_SPC,  KC_SPC,  0x5D01,  KC_RALT, KC_RGUI, KC_APP,  KC_RCTL
};

// Sensor names array (for debug printing)
const char *sensor_names[KEY_COUNT] = {
    "Esc", "Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P", "Bspc",
    "Tab", "A", "S", "D", "F", "G", "H", "J", "K", "L", "Scln", "Ent",
    "LShft", "Z", "X", "C", "V", "B", "N", "M", "Comm", "Dot", "Slsh", "RShft",
    "LCtrl", "LGui", "LAlt", "MO1", "Spc", "Spc", "Spc", "MO2", "RAlt", "RGui", "App", "RCtrl"
};

// MUX1 channel mappings (channels 0-15)
// Each MUX channel is wired to a specific sensor number.
// The sensor number automatically maps to matrix position via: (sensor-1)/12, (sensor-1)%12
// Update these sensor numbers to match your actual MUX1 wiring.
const mux16_ref_t mux1_channels[16] = {
    [0]  = { K_E       },  // CH0    /* HE4 */
    [1]  = { K_D       },  // CH1    /* HE16 */
    [2]  = { K_C       },  // CH2    /* HE28 */
    [3]  = { K_MO1     },  // CH3    /* HE40 */
    [4]  = { K_W       },  // CH4    /* HE3 */
    [5]  = { K_S       },  // CH5    /* HE15 */
    [6]  = { K_X       },  // CH6    /* HE27 */
    [7]  = { K_LALT    },  // CH7    /* HE39 */
    [8]  = { K_Q       },  // CH8    /* HE2 */
    [9]  = { K_A       },  // CH9    /* HE14 */
    [10] = { K_Z       },  // CH10   /* HE26 */
    [11] = { K_WIN     },  // CH11   /* HE38 */
    [12] = { K_ESC     },  // CH12   /* HE1 */
    [13] = { K_TAB     },  // CH13   /* HE13 */
    [14] = { K_LSFT    },  // CH14   /* HE25 */
    [15] = { K_LCTL    },  // CH15   /* HE37 */
};

// MUX2 channel mappings (channels 0-15)
// Same wiring as MUX1, but add 4 to each sensor number
const mux16_ref_t mux2_channels[16] = {
    [0]  = { K_U       },  // CH0   /* HE8 */
    [1]  = { K_J       },  // CH1   /* HE20 */
    [2]  = { K_M       },  // CH2   /* HE32 */
    [3]  = { K_FN      },  // CH3   /* HE41 */
    [4]  = { K_Y       },  // CH4   /* HE7 */
    [5]  = { K_H       },  // CH5   /* HE19 */
    [6]  = { K_N       },  // CH6   /* HE31 */
    [7]  = { K_SPC2    },  // CH7   /* HE42 */
    [8]  = { K_T       },  // CH8   /* HE6 */
    [9]  = { K_G       },  // CH9   /* HE18 */
    [10] = { K_B       },  // CH10  /* HE30 */
    [11] = { K_SPC1    },  // CH11  /* HE43 */
    [12] = { K_R       },  // CH12  /* HE5 */
    [13] = { K_F       },  // CH13  /* HE17 */
    [14] = { K_V       },  // CH14  /* HE29 */
    [15] = { K_TG3     },  // CH15  /* HE43 */
};

// MUX3 channel mappings (channels 0-15)
// Same wiring as MUX1, but add 8 to each sensor number
const mux16_ref_t mux3_channels[16] = {
    [0]  = { K_BSPC    },  // CH0   /* HE12 */
    [1]  = { K_ENT     },  // CH1   /* HE24 */
    [2]  = { K_RSFT    },  // CH2   /* HE36 */
    [3]  = { K_RGHT    },  // CH3   /* HE48 */
    [4]  = { K_P       },  // CH4   /* HE11 */
    [5]  = { K_SCLN    },  // CH5   /* HE23 */
    [6]  = { K_UP      },  // CH6   /* HE48 */
    [7]  = { K_DOWN    },  // CH7   /* HE44 */
    [8]  = { K_O       },  // CH8   /* HE10 */
    [9]  = { K_L       },  // CH9   /* HE22 */
    [10] = { K_DOT     },  // CH10  /* HE34 */
    [11] = { K_LEFT    },  // CH11  /* HE45 */
    [12] = { K_I       },  // CH12  /* HE9 */
    [13] = { K_K       },  // CH13  /* HE21 */
    [14] = { K_COMM    },  // CH14  /* HE33 */
    [15] = { K_RALT    },  // CH15  /* HE45 */
};
