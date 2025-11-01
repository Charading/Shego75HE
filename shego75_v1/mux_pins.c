#include "mux_pins.h"
#include "quantum.h"

// MASTER SENSOR-TO-KEYCODE LOOKUP TABLE
// This maps sensor numbers 1-48 directly to QMK keycodes based on your 4x12 layout
// Sensor numbers are calculated as: (matrix_row * 12) + matrix_col + 1
// Your keymap layout:





// Sensor names array (for debug printing)
const char *sensor_names[KEY_COUNT] = {
    "Esc", "F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "F11", "F12", "Delete",
    "Grave", "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "Minus", "Equal", "Backspace", "Home",
    "Tab", "Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P", "LBracket", "RBracket", "Backslash", "PgUp",
    "Caps", "A", "S", "D", "F", "G", "H", "J", "K", "L", "Semicolon", "Quote", "Enter", "PgDn",
    "LShift", "Z", "X", "C", "V", "B", "N", "M", "Comma", "Dot", "Slash", "RShift", "Up",
    "LControl", "LWin", "LAlt", "Space", "RAlt", "Function", "RControl", "Left", "Down", "Right"
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
    [25] = { 0         },
    [26] = { K_K       },
    [27] = { K_DOT     },
    [28] = { K_J       },
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
    [5]  = { 0            },
    [6]  = { K_F9         },
    [7]  = { 0            },
    [8]  = { K_F8         },
    [9]  = { 0            },
    [10] = { K_EQUAL      },
    [11] = { 0            },
    [12] = { K_RBRC       },
    [13] = { K_L          },
    [14] = { K_SEMI       },
    [15] = { K_QUOTE      },
    [16] = { K_SLASH      },
    [17] = { K_HOME       },
    [18] = { K_PGUP       },
    [19] = { K_PGDN       },
    [20] = { 0            },
    [21] = { K_RIGHT      },
    [22] = { 0            },
    [23] = { K_UP         },
    [24] = { 0            },
    [25] = { K_DOWN       },
    [26] = { K_BSLASH     },
    [27] = { K_ENTER      },
    [28] = { K_BACKSPACE  },
    [29] = { K_LEFT       },
    [30] = { K_RCTRL      },
    [31] = { K_RSHIFT     },
    [32] = { K_FN         },
};

