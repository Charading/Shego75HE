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
const mux32_ref_t mux1_channels[33] = {
    [1]  = { K_F3      },
    [2]  = { K_3       },
    [3]  = { K_F2      },
    [4]  = { K_F1      },
    [5]  = { K_2       },
    [6]  = { K_ESC     },
    [7]  = { K_W       },
    [8]  = { K_F4      },
    [9]  = { K_1       },
    [10] = { K_GRAVE   },
    [11] = { K_Q       },
    [12] = { K_TAB     },
    [13] = { K_CAPS    },
    [14] = { K_LSHIFT  },
    [15] = { K_A       },
    [16] = { K_LCTRL   },
    [17] = { K_E       },
    [18] = { K_4       },
    [19] = { K_R       },
    [20] = { K_5       },
    [21] = { K_G       },
    [22] = { K_B       },
    [23] = { K_F       },
    [24] = { K_SPACE   },
    [25] = { K_D       },
    [26] = { K_V       },
    [27] = { K_C       },
    [28] = { K_LALT    },
    [29] = { K_LWIN    },
    [30] = { K_X       },
    [31] = { K_Z       },
    [32] = { K_S       },
};

//MUX 2 channel mappings (channels 0-15)
const mux32_ref_t mux2_channels[33] = {
    [1]  = { K_F7      },
    [2]  = { K_7       },
    [3]  = { K_F6      },
    [4]  = { K_6       },
    [5]  = { K_Y       },
    [6]  = { K_8       },
    [7]  = { 0         },
    [8]  = { K_F5      },
    [9]  = { K_T       },
    [10] = { K_N       },
    [11] = { K_M       },
    [12] = { K_RALT    },
    [13] = { K_COMMA   },
    [14] = { K_H       },
    [15] = { 0         },
    [16] = { 0         },
    [17] = { K_U       },
    [18] = { K_I       },
    [19] = { K_9       },
    [20] = { K_O       },
    [21] = { K_0       },
    [22] = { K_P       },
    [23] = { K_MINUS   },
    [24] = { K_LBRC    },
    [25] = { K_K       },
    [26] = { K_DOT     },
    [27] = { K_J       },
    [28] = { 0         },
    [29] = { 0         },
    [30] = { 0         },
    [31] = { 0         },
    [32] = { 0         },
};

//MUX 3 channel mappings (channels 0-15)
const mux32_ref_t mux3_channels[33] = {
    [1]  = { K_DEL        },
    [2]  = { K_F12        },
    [3]  = { K_F11        },
    [4]  = { K_F10        },
    [5]  = { K_F9         },
    [6]  = { K_F8         },
    [7]  = { K_EQUAL      },
    [8]  = { K_RBRC       },
    [9]  = { K_SEMI       },
    [10] = { K_QUOTE      },
    [11] = { K_SLASH      },
    [12] = { K_L          },
    [13] = { 0            },
    [14] = { 0            },
    [15] = { 0            },
    [16] = { 0            },
    [17] = { K_HOME       },
    [18] = { K_PGUP       },
    [19] = { K_PGDN       },
    [20] = { 0            },
    [21] = { K_RIGHT      },
    [22] = { K_UP         },
    [23] = { K_DOWN       },
    [24] = { K_BSLASH     },
    [25] = { K_ENTER      },
    [26] = { K_BACKSPACE  },
    [27] = { K_LEFT       },
    [28] = { K_RCTRL      },
    [29] = { K_RSHIFT     },
    [30] = { K_FN         },
    [31] = { 0            },
    [32] = { 0            },
};

