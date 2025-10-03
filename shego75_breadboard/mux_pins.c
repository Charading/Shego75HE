#include "mux_pins.h"

// Sensor names array
const char *sensor_names[KEY_COUNT] = {
    // Row 0 (Esc + Numbers) - 12 keys (sensors 1-12)
    "Esc", "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "Minus",
    // Row 1 (Tab + QWERTY) - 12 keys (sensors 13-24)
    "Tab", "Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P", "L_Bracket",
    // Row 2 (Caps + ASDF...) - 12 keys (sensors 25-36)
    "Caps", "A", "S", "D", "F", "G", "H", "J", "K", "L", "Semicolon", "Quote",
    // Row 3 (Shift + ZXCV...) - 12 keys (sensors 37-48)
    "L_Shift", "Z", "X", "C", "V", "B", "N", "M", "Comma", "Period", "Slash", "R_Shift",
};

// MUX1 channel mappings (channels 0-15, local sensors 1-16)
const mux16_ref_t mux1_channels[16] = {
    [0]  = { .sensor =  1, .key = "C"       },  // Row 0, Col 0
    [1]  = { .sensor =  2, .key = "X"       },  // Row 0, Col 1  
    [2]  = { .sensor =  3, .key = "Z"       },  // Row 0, Col 2
    [3]  = { .sensor =  4, .key = "L_Shift" },  // Row 0, Col 3
    [4]  = { .sensor =  5, .key = "D"       },  // Row 1, Col 0
    [5]  = { .sensor =  6, .key = "S"       },  // Row 1, Col 1
    [6]  = { .sensor =  7, .key = "A"       },  // Row 1, Col 2
    [7]  = { .sensor =  8, .key = "Caps"    },  // Row 1, Col 3
    [8]  = { .sensor =  9, .key = "E"       },  // Row 2, Col 0
    [9]  = { .sensor = 10, .key = "w"       },  // Row 2, Col 1
    [10] = { .sensor = 11, .key = "Q"       },  // Row 2, Col 2
    [11] = { .sensor = 12, .key = "Tab"     },  // Row 2, Col 3
    [12] = { .sensor = 13, .key = "3"       },  // Row 3, Col 0
    [13] = { .sensor = 14, .key = "2"       },  // Row 3, Col 1
    [14] = { .sensor = 15, .key = "1"       },  // Row 3, Col 2
    [15] = { .sensor = 16, .key = "Esc"     },  // Row 3, Col 3
};

// MUX2 channel mappings (channels 0-15, local sensors 1-16)
const mux16_ref_t mux2_channels[16] = {
    [0]  = { .sensor =  1, .key = "M"       },  // Row 0, Col 4
    [1]  = { .sensor =  2, .key = "N"       },  // Row 0, Col 5
    [2]  = { .sensor =  3, .key = "B"       },  // Row 0, Col 6
    [3]  = { .sensor =  4, .key = "V"       },  // Row 0, Col 7
    [4]  = { .sensor =  5, .key = "J"       },  // Row 1, Col 4
    [5]  = { .sensor =  6, .key = "H"       },  // Row 1, Col 5
    [6]  = { .sensor =  7, .key = "G"       },  // Row 1, Col 6
    [7]  = { .sensor =  8, .key = "F"       },  // Row 1, Col 7
    [8]  = { .sensor =  9, .key = "U"       },  // Row 2, Col 4
    [9]  = { .sensor = 10, .key = "Y"       },  // Row 2, Col 5
    [10] = { .sensor = 11, .key = "T"       },  // Row 2, Col 6
    [11] = { .sensor = 12, .key = "R"       },  // Row 2, Col 7
    [12] = { .sensor = 13, .key = "7"       },  // Row 3, Col 4
    [13] = { .sensor = 14, .key = "6"       },  // Row 3, Col 5
    [14] = { .sensor = 15, .key = "5"       },  // Row 3, Col 6
    [15] = { .sensor = 16, .key = "4"       },  // Row 3, Col 7
};

// MUX3 channel mappings (channels 0-15, local sensors 1-16)
const mux16_ref_t mux3_channels[16] = {
    [0]  = { .sensor =  1, .key = "R_Shift" },  // Row 0, Col 8
    [1]  = { .sensor =  2, .key = "Slash"   },  // Row 0, Col 9
    [2]  = { .sensor =  3, .key = "Period"  },  // Row 0, Col 10
    [3]  = { .sensor =  4, .key = "Comma"   },  // Row 0, Col 11
    [4]  = { .sensor =  5, .key = "Quote"   },  // Row 1, Col 8
    [5]  = { .sensor =  6, .key = "Semicolon"}, // Row 1, Col 9
    [6]  = { .sensor =  7, .key = "L"       },  // Row 1, Col 10
    [7]  = { .sensor =  8, .key = "K"       },  // Row 1, Col 11
    [8]  = { .sensor =  9, .key = "L_Bracket"}, // Row 2, Col 8
    [9]  = { .sensor = 10, .key = "P"       },  // Row 2, Col 9
    [10] = { .sensor = 11, .key = "O"       },  // Row 2, Col 10
    [11] = { .sensor = 12, .key = "I"       },  // Row 2, Col 11
    [12] = { .sensor = 13, .key = "Minus"   },  // Row 3, Col 8
    [13] = { .sensor = 14, .key = "0"       },  // Row 3, Col 9
    [14] = { .sensor = 15, .key = "9"       },  // Row 3, Col 10
    [15] = { .sensor = 16, .key = "8"       },  // Row 3, Col 11
};