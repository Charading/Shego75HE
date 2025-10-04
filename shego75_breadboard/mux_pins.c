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
    [0]  = { .sensor =  4, .key = "E"    },  // CH0  wired to sensor 4
    [1]  = { .sensor =  16, .key = "D"    },  // CH1  wired to sensor 2
    [2]  = { .sensor =  28, .key = "C"    },  // CH2  wired to sensor 3
    [3]  = { .sensor =  40, .key = "MO(1)"  },  // CH3  wired to sensor 1
    [4]  = { .sensor =  3, .key = "W"  },  // CH4  wired to sensor 13
    [5]  = { .sensor =  15, .key = "S"    },  // CH5  wired to sensor 6
    [6]  = { .sensor =  27, .key = "X"    },  // CH6  wired to sensor 7
    [7]  = { .sensor =  39, .key = "LAlt"    },  // CH7  wired to sensor 8r
    [8]  = { .sensor =  2, .key = "Q"    },  // CH8  wired to sensor 9
    [9]  = { .sensor = 14, .key = "A"    },  // CH9  wired to sensor 10
    [10] = { .sensor = 26, .key = "Z"    },  // CH10 wired to sensor 11
    [11] = { .sensor = 38, .key = "Win" },  // CH11 wired to sensor 12
    [12] = { .sensor =  1, .key = "Esc"    },  // CH12 wired to sensor 5
    [13] = { .sensor = 13, .key = "Tab"    },  // CH13 wired to sensor 14
    [14] = { .sensor = 25, .key = "LShift"    }, 
    [15] = { .sensor = 37, .key = "LCtrl"    },  
};
// MUX2 channel mappings (channels 0-15)
// Same wiring as MUX1, but add 4 to each sensor number
const mux16_ref_t mux2_channels[16] = {
    [0]  = { .sensor =  4+4, .key = "U"      },  // CH0: sensor 8
    [1]  = { .sensor = 16+4, .key = "J"      },  // CH1: sensor 20
    [2]  = { .sensor = 28+4, .key = "M"      },  // CH2: sensor 32
    [3]  = { .sensor = 40+4, .key = "MO(2)"  },  // CH3: sensor 44
    [4]  = { .sensor =  3+4, .key = "Y"      },  // CH4: sensor 7
    [5]  = { .sensor = 15+4, .key = "H"      },  // CH5: sensor 19
    [6]  = { .sensor = 27+4, .key = "N"      },  // CH6: sensor 31
    [7]  = { .sensor = 39+4, .key = "Spc"    },  // CH7: sensor 43
    [8]  = { .sensor =  2+4, .key = "T"      },  // CH8: sensor 6
    [9]  = { .sensor = 14+4, .key = "G"      },  // CH9: sensor 18
    [10] = { .sensor = 26+4, .key = "B"      },  // CH10: sensor 30
    [11] = { .sensor = 38+4, .key = "Spc"    },  // CH11: sensor 42
    [12] = { .sensor =  1+4, .key = "R"      },  // CH12: sensor 5
    [13] = { .sensor = 13+4, .key = "F"      },  // CH13: sensor 17
    [14] = { .sensor = 25+4, .key = "V"      },  // CH14: sensor 29
    [15] = { .sensor = 37+4, .key = "Spc"    },  // CH15: sensor 41
};

// MUX3 channel mappings (channels 0-15)
// Same wiring as MUX1, but add 8 to each sensor number
const mux16_ref_t mux3_channels[16] = {
    [0]  = { .sensor =  4+8, .key = "Bspc"   },  // CH0: sensor 12
    [1]  = { .sensor = 16+8, .key = "Ent"    },  // CH1: sensor 24
    [2]  = { .sensor = 28+8, .key = "RShft"  },  // CH2: sensor 36
    [3]  = { .sensor = 40+8, .key = "RCtrl"  },  // CH3: sensor 48
    [4]  = { .sensor =  3+8, .key = "P"      },  // CH4: sensor 11
    [5]  = { .sensor = 15+8, .key = "Scln"   },  // CH5: sensor 23
    [6]  = { .sensor = 27+8, .key = "Slsh"   },  // CH6: sensor 35
    [7]  = { .sensor = 39+8, .key = "App"    },  // CH7: sensor 47
    [8]  = { .sensor =  2+8, .key = "O"      },  // CH8: sensor 10
    [9]  = { .sensor = 14+8, .key = "L"      },  // CH9: sensor 22
    [10] = { .sensor = 26+8, .key = "Dot"    },  // CH10: sensor 34
    [11] = { .sensor = 38+8, .key = "RGui"   },  // CH11: sensor 46
    [12] = { .sensor =  1+8, .key = "I"      },  // CH12: sensor 9
    [13] = { .sensor = 13+8, .key = "K"      },  // CH13: sensor 21
    [14] = { .sensor = 25+8, .key = "Comm"   },  // CH14: sensor 33
    [15] = { .sensor = 37+8, .key = "RAlt"   },  // CH15: sensor 45
};