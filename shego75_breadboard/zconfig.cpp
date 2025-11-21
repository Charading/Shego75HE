// ===============================
// ====== SENSOR THRESHOLDS ======
// ===============================
static const uint16_t key_thresholds[96] = {
    // ----- MUX1 -----
    940, // i0  -> HE17 -> "2"
    940, // i1  -> HE2  -> "F1"
    940, // i2  -> HE16 -> "1"
    940, // i3  -> HE1  -> "Esc"
    940, // i4  -> HE15 -> "`"
    940, // i5  -> HE30 -> "Tab"
    940, // i6  -> HE45 -> "Caps Lock"
    940, // i7  -> HE59 -> "Shift (L)"
    940, // i8  -> HE60 -> "Z"
    940, // i9  -> HE32 -> "W"
    940, // i10 -> HE73 -> "Win"
    940, // i11 -> HE46 -> "A"
    940, // i12 -> HE31 -> "Q"
    940, // i13 -> HE3  -> "F2"
    940, // i14 -> HE4  -> "F3"
    940, // i15 -> HE18 -> "3"

    // ----- MUX2 -----
    940, // i0  -> HE33 -> "E"
    940, // i1  -> HE19 -> "4"
    940, // i2  -> HE5  -> "F4"
    940, // i3  -> HE47 -> "S"
    940, // i4  -> HE74 -> "Alt (L)"
    940, // i5  -> HE61 -> "X"
    940, // i6  -> HE48 -> "D"
    940, // i7  -> HE62 -> "C"
    940, // i8  -> HE63 -> "V"
    940, // i9  -> HE49 -> "F"
    940, // i10 -> HE50 -> "G"
    940, // i11 -> HE6  -> "F5"
    940, // i12 -> HE20 -> "5"
    940, // i13 -> HE34 -> "R"
      0, // i14 -> GND
      0, // i15 -> GND

    // ----- MUX3 -----
    940, // i0  -> HE21 -> "6"
    940, // i1  -> HE7  -> "F6"
    940, // i2  -> HE8  -> "F7"
    940, // i3  -> HE22 -> "7"
    940, // i4  -> HE51 -> "H"
    940, // i5  -> HE65 -> "N"
    940, // i6  -> HE64 -> "B"
    940, // i7  -> HE75 -> "Space"
      0, // i8  -> GND
      0, // i9  -> GND
      0, // i10 -> GND
      0, // i11 -> GND
      0, // i12 -> GND
    11540, // i13 -> HE52 -> "J"
    11540, // i14 -> HE37 -> "U"
    2940, // i15 -> HE36 -> "Y"

    // ----- MUX4 -----
      0, // i0  -> GND
      0, // i1  -> GND
      0, // i2  -> GND
      0, // i3  -> GND
    1940, // i4  -> HE67 -> ","
    1940, // i5  -> HE66 -> "M"
      0, // i6  -> GND
      0, // i7  -> GND
    1940, // i8  -> HE68 -> "."
    1940, // i9  -> HE54 -> "L"
      0, // i10 -> GND
      0, // i11 -> GND
    1940, // i12 -> HE39 -> "O"
    1940, // i13 -> HE23 -> "8"
    1940, // i14 -> HE9  -> "F8"
    1940, // i15 -> HE24 -> "9"

    // ----- MUX5 -----
    1940, // i0  -> HE40 -> "P"
    1940, // i1  -> HE25 -> "0"
    1940, // i2  -> HE10 -> "F9"
      0, // i3  -> GND
      0, // i4  -> GND
      0, // i5  -> GND
      0, // i6  -> GND
    1940, // i7  -> HE55 -> ";"
    1940, // i8  -> HE69 -> "/"
    1940, // i9  -> HE77 -> "Fn"
    1940, // i10 -> HE76 -> "Alt (R)"
    1940, // i11 -> HE78 -> "Ctrl (R)"
    1940, // i12 -> HE56 -> "'"
    1940, // i13 -> HE41 -> "["
    1940, // i14 -> HE11 -> "F10"
    1940, // i15 -> HE26 -> "-"

    // ----- MUX6 -----
    1940, // i0  -> HE13 -> "F12"
    1940, // i1  -> HE42 -> "]"
    1940, // i2  -> HE27 -> "="
    1940, // i3  -> HE12 -> "F11"
    1940, // i4  -> HE70 -> "Shift (R)"
    1940, // i5  -> HE79 -> "Left Arrow"
    1940, // i6  -> HE71 -> "Up Arrow"
    1940, // i7  -> HE57 -> "Enter"
    1940, // i8  -> HE80 -> "Down Arrow"
    1940, // i9  -> HE81 -> "Right Arrow"
    1940, // i10 -> HE58 -> "PgDn"
    1940, // i11 -> HE44 -> "PgUp"
    1940, // i12 -> HE29 -> "Home"
    1940, // i13 -> HE14 -> "Delete"
    1940, // i14 -> HE43 -> "Backslash"
    1940, // i15 -> HE28 -> "Backspace"
};

// ===============================
// ====== SENSOR KEYMAPPING ======
// ===============================

// Dynamic baselines populated at init - used for 20% up/down deviation detection
// 6 muxes * 16 channels = 96 sensors
static uint16_t key_baseline[96] = {0};

// Map MUX channels to matrix positions
typedef struct {
    uint8_t row;
    uint8_t col;
    uint16_t keycode;
} key_mapping_t;

// MUX1 channel mappings
static const key_mapping_t mux1_keys[16] = {
    {0, 3, KC_2},        // i0 -> HE17 -> "2"
    {0, 1, KC_F1},       // i1 -> HE2  -> "F1"
    {0, 2, KC_1},        // i2 -> HE16 -> "1"
    {0, 0, KC_ESC},      // i3 -> HE1  -> "Esc"
    {1, 0, KC_GRV},      // i4 -> HE15 -> "`"
    {1, 1, KC_TAB},      // i5 -> HE30 -> "Tab"
    {2, 0, KC_CAPS},     // i6 -> HE45 -> "Caps Lock"
    {3, 0, KC_LSFT},     // i7 -> HE59 -> "Shift (L)"
    {4, 0, KC_Z},        // i8 -> HE60 -> "Z"
    {1, 2, KC_W},        // i9 -> HE32 -> "W"
    {5, 0, KC_LGUI},     // i10 -> HE73 -> "Win"
    {2, 1, KC_A},        // i11 -> HE46 -> "A"
    {1, 3, KC_Q},        // i12 -> HE31 -> "Q"
    {0, 2, KC_F2},       // i13 -> HE3  -> "F2"
    {0, 3, KC_F3},       // i14 -> HE4  -> "F3"
    {0, 4, KC_3},        // i15 -> HE18 -> "3"
};

// MUX2 channel mappings
static const key_mapping_t mux2_keys[16] = {
    {1, 4, KC_E},        // i0 -> HE33 -> "E"
    {0, 5, KC_4},        // i1 -> HE19 -> "4"
    {0, 4, KC_F4},       // i2 -> HE5  -> "F4"
    {2, 2, KC_S},        // i3 -> HE47 -> "S"
    {5, 1, KC_LALT},     // i4 -> HE74 -> "Alt (L)"
    {4, 1, KC_X},        // i5 -> HE61 -> "X"
    {2, 3, KC_D},        // i6 -> HE48 -> "D"
    {4, 2, KC_C},        // i7 -> HE62 -> "C"
    {4, 3, KC_V},        // i8 -> HE63 -> "V"
    {2, 4, KC_F},        // i9 -> HE49 -> "F"
    {2, 5, KC_G},        // i10 -> HE50 -> "G"
    {0, 5, KC_F5},       // i11 -> HE6  -> "F5"
    {0, 6, KC_5},        // i12 -> HE20 -> "5"
    {1, 5, KC_R},        // i13 -> HE34 -> "R"
    {255, 255, KC_NO},   // i14 -> GND
    {255, 255, KC_NO},   // i15 -> GND
};

// MUX3 channel mappings
static const key_mapping_t mux3_keys[16] = {
    {0, 7, KC_6},        // i0 -> HE21 -> "6"
    {0, 6, KC_F6},       // i1 -> HE7  -> "F6"
    {0, 7, KC_F7},       // i2 -> HE8  -> "F7"
    {0, 8, KC_7},        // i3 -> HE22 -> "7"
    {2, 6, KC_H},        // i4 -> HE51 -> "H"
    {4, 6, KC_N},        // i5 -> HE65 -> "N"
    {4, 5, KC_B},        // i6 -> HE64 -> "B"
    {5, 6, KC_SPC},      // i7 -> HE75 -> "Space"
    {255, 255, KC_NO},   // i8 -> GND
    {255, 255, KC_NO},   // i9 -> GND
    {255, 255, KC_NO},   // i10 -> GND
    {255, 255, KC_NO},   // i11 -> GND
    {255, 255, KC_NO},   // i12 -> GND
    {2, 7, KC_J},        // i13 -> HE52 -> "J"
    {1, 7, KC_U},        // i14 -> HE37 -> "U"
    {1, 6, KC_Y},        // i15 -> HE36 -> "Y"
};

// MUX4 channel mappings
static const key_mapping_t mux4_keys[16] = {
    {255, 255, KC_NO},   // i0 -> GND
    {255, 255, KC_NO},   // i1 -> GND
    {255, 255, KC_NO},   // i2 -> GND
    {255, 255, KC_NO},   // i3 -> GND
    {4, 8, KC_COMM},     // i4 -> HE67 -> ","
    {4, 7, KC_M},        // i5 -> HE66 -> "M"
    {255, 255, KC_NO},   // i6 -> GND
    {255, 255, KC_NO},   // i7 -> GND
    {4, 9, KC_DOT},      // i8 -> HE68 -> "."
    {2, 9, KC_L},        // i9 -> HE54 -> "L"
    {255, 255, KC_NO},   // i10 -> GND
    {255, 255, KC_NO},   // i11 -> GND
    {1, 9, KC_O},        // i12 -> HE39 -> "O"
    {0, 9, KC_8},        // i13 -> HE23 -> "8"
    {0, 8, KC_F8},       // i14 -> HE9  -> "F8"
    {0, 10, KC_9},       // i15 -> HE24 -> "9"
};

// MUX5 channel mappings
static const key_mapping_t mux5_keys[16] = {
    {1, 10, KC_P},       // i0 -> HE40 -> "P"
    {0, 11, KC_0},       // i1 -> HE25 -> "0"
    {0, 9, KC_F9},       // i2 -> HE10 -> "F9"
    {255, 255, KC_NO},   // i3 -> GND
    {255, 255, KC_NO},   // i4 -> GND
    {255, 255, KC_NO},   // i5 -> GND
    {255, 255, KC_NO},   // i6 -> GND
    {2, 10, KC_SCLN},    // i7 -> HE55 -> ";"
    {4, 10, KC_SLSH},    // i8 -> HE69 -> "/"
    {5, 9, KC_NO},       // i9 -> HE77 -> "Fn"
    {5, 8, KC_RALT},     // i10 -> HE76 -> "Alt (R)"
    {5, 7, KC_RCTL},     // i11 -> HE78 -> "Ctrl (R)"
    {2, 11, KC_QUOT},    // i12 -> HE56 -> "'"
    {1, 11, KC_LBRC},    // i13 -> HE41 -> "["
    {0, 10, KC_F10},     // i14 -> HE11 -> "F10"
    {0, 12, KC_MINS},    // i15 -> HE26 -> "-"
};

// MUX6 channel mappings
static const key_mapping_t mux6_keys[16] = {
    {0, 12, KC_F12},     // i0 -> HE13 -> "F12"
    {1, 12, KC_RBRC},    // i1 -> HE42 -> "]"
    {0, 13, KC_EQL},     // i2 -> HE27 -> "="
    {0, 11, KC_F11},     // i3 -> HE12 -> "F11"
    {3, 11, KC_RSFT},    // i4 -> HE70 -> "Shift (R)"
    {5, 10, KC_LEFT},    // i5 -> HE79 -> "Left Arrow"
    {5, 11, KC_UP},      // i6 -> HE71 -> "Up Arrow"
    {2, 12, KC_ENT},     // i7 -> HE57 -> "Enter"
    {5, 12, KC_DOWN},    // i8 -> HE80 -> "Down Arrow"
    {5, 13, KC_RGHT},    // i9 -> HE81 -> "Right Arrow"
    {3, 13, KC_PGDN},    // i10 -> HE58 -> "PgDn"
    {3, 12, KC_PGUP},    // i11 -> HE44 -> "PgUp"
    {3, 10, KC_HOME},    // i12 -> HE29 -> "Home"
    {0, 14, KC_DEL},     // i13 -> HE14 -> "Delete"
    {1, 13, KC_BSLS},    // i14 -> HE43 -> "Backslash"
    {0, 15, KC_BSPC},    // i15 -> HE28 -> "Backspace"
};
