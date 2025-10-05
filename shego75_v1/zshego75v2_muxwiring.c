// SHEGO75 Wiring
#include <stddef.h>   // for NULL


    // MUX Wiring

    // MUX Select Pins
    #define MUX_S0 GP18
    #define MUX_S1 GP19
    #define MUX_S2 GP17
    #define MUX_S3 GP16
    // MUX Chip Select Pins
    #define MUX_EN4 GP22
    #define MUX_EN5 GP23
    #define MUX_EN6 GP24
    
    #define RGB_PIN GP20    // RGB Pin

    #define ENC_A_CLK GP13
    #define ENC_B_DT GP14
    #define ENC_SW GP12

    #define MUX1_ADC GP26 //ADC0
    #define MUX2_ADC GP27 //ADC1
    #define MUX3_ADC GP28 //ADC2
    #define MUX4_ADC GP29 //ADC3

// MUX Channel Wiring (ADG732)

//MUX1

// Enum for indexing by key name
typedef enum {
    KEY_F3, KEY_18_3, KEY_F2, KEY_F1, KEY_17_2, KEY_Esc, KEY_W, KEY_F4,
    KEY_16_1, KEY_Grave, KEY_Q, KEY_Tab, KEY_CapsLock, KEY_ShiftL, KEY_A, KEY_Ctrl,
    KEY_E, KEY_19_4, KEY_R, KEY_20_5, KEY_G, KEY_B, KEY_F, KEY_Space,
    KEY_D, KEY_V, KEY_C, KEY_AltL, KEY_Win, KEY_X, KEY_Z, KEY_S,
    KEY_COUNT
} key_id_t;


typedef struct {
    uint8_t sensor;       // HE number stays the same as 32ch version
    const char *key;      // Key name from 32ch mapping
} mux16_ref_t;

// MUX1 reference table

const mux16_ref_t mux1_channels[16] = {
    [0]  = { 17, "2"          },   // HE17 → 2
    [1]  = {  2, "F1"         },   // HE2 → F1
    [2]  = { 16, "1"          },   // HE16 → 1
    [3]  = {  1, "Esc"        },   // HE1 → Esc
    [4]  = { 15, "Grave"      },   // HE15 → Grave
    [5]  = { 30, "Tab"        },   // HE30 → Tab
    [6]  = { 45, "CapsLock"   },   // HE45 → CapsLock
    [7]  = { 59, "ShiftL"     },   // HE59 → ShiftL
    [8]  = { 60, "Z"          },   // HE60 → Z
    [9]  = { 32, "W"          },   // HE32 → W
    [10] = { 73, "Win"        },   // HE73 → Win
    [11] = { 46, "A"          },   // HE46 → A
    [12] = { 31, "Q"          },   // HE31 → Q
    [13] = {  3, "F2"         },   // HE3 → F2
    [14] = {  4, "F3"         },   // HE4 → F3
    [15] = { 18, "3"          },   // HE18 → 3
};

// MUX2 reference table

const mux16_ref_t mux2_channels[16] = {
    [0]  = { 33, "E"            },   // HE33 → E
    [1]  = { 19, "4"            },   // HE19 → 4
    [2]  = { 47, "S"            },   // HE47 → S
    [3]  = { 74, "AltL"         },   // HE74 → AltL
    [4]  = { 48, "D"            },   // HE48 → D
    [5]  = { 75, "Space"        },   // HE75 → Space
    [6]  = { 50, "G"            },   // HE50 → G
    [7]  = {  6, "F5"           },   // HE6  → F5
    [8]  = { 63, "V"            },   // HE63 → V
    [9]  = { 18, "3"            },   // HE18 → 3
    [10] = { 34, "R"            },   // HE34 → R
    [11] = { 20, "5"            },   // HE20 → 5
    [12] = { 64, "B"            },   // HE64 → B
    [13] = { 49, "F"            },   // HE49 → F
    [14] = {  0, NULL           },   // unused → GND
    [15] = { 72, "Ctrl"         },   // HE72 → Ctrl
};
// MUX3 reference table
const mux16_ref_t mux3_channels[16] = {
    [0]  = { 21, "6"          },  // 6   (HE21)
    [1]  = {  7, "F6"         },  // F6  (HE7)
    [2]  = {  8, "F7"         },  // F7  (HE8)
    [3]  = { 22, "7"          },  // 7   (HE22)
    [4]  = { 51, "H"          },  // H   (HE51)
    [5]  = { 65, "N"          },  // N   (HE65)
    [6]  = { 64, "B"          },  // B   (HE64)
    [7]  = { 75, "Space"      },  // Space (HE75)
    [8]  = {  0, NULL         },
    [9]  = {  0, NULL         },
    [10] = {  0, NULL         },
    [11] = {  0, NULL         },
    [12] = { 52, "J"          },  // J   (HE52)
    [13] = { 37, "U"          },  // U   (HE37)
    [14] = { 36, "Y"          },  // Y   (HE36)
    [15] = {  0, NULL         },
};


// MUX4 reference table
const mux16_ref_t mux4_channels[17] = {   // 0 unused
    [1]  = {  0,  NULL       },  // GND
    [2]  = {  0,  NULL       },  // GND
    [3]  = { 67, "Comma"     },  // HE67
    [4]  = { 66, "M"         },  // HE66
    [5]  = {  0,  NULL       },  // GND
    [6]  = { 68, "Period"    },  // HE68
    [7]  = { 54, "L"         },  // HE54
    [8]  = {  0,  NULL       },  // GND
    [9]  = {  0,  NULL       },  // GND
    [10] = {  0,  NULL       },  // GND
    [11] = { 39, "O"         },  // HE39
    [12] = { 23, "8"         },  // HE23
    [13] = {  9, "1"         },  // HE9
    [14] = { 24, "9"         },  // HE24
    [15] = {  0,  NULL       },  // GND
    [16] = {  0,  NULL       }   // GND
};

// MUX5 reference table
const mux16_ref_t mux5_channels[17] = {   // 0 unused
    [1]  = { 40, "P"         },  // HE40
    [2]  = { 25, "0"         },  // HE25
    [3]  = { 10, "F1"        },  // HE10
    [4]  = {  0,  NULL       },  // GND
    [5]  = {  0,  NULL       },  // GND
    [6]  = { 55, "Semicolon" },  // HE55
    [7]  = { 69, "Quote"     },  // HE69
    [8]  = { 77, "Fn"        },  // HE77
    [9]  = { 76, "AltR"      },  // HE76
    [10] = { 78, "CtrlR"     },  // HE78
    [11] = { 56, "Quote"     },  // HE56
    [12] = { 41, "LeftBracket"}, // HE41
    [13] = { 11, "F2"        },  // HE11
    [14] = { 26, "Minus"     },  // HE26
    [15] = {  0,  NULL       },  // GND
    [16] = {  0,  NULL       }   // GND
};

// MUX6 reference table
const mux16_ref_t mux6_channels[17] = {   // 0 unused
    [1]  = { 13, "Grave"     },  // HE13
    [2]  = { 42, "RightBracket" }, // HE42
    [3]  = { 27, "Equals"    },  // HE27
    [4]  = { 70, "ShiftR"    },  // HE70
    [5]  = { 79, "ArrowLeft" },  // HE79
    [6]  = { 71, "ArrowUp"   },  // HE71
    [7]  = { 57, "Enter"     },  // HE57
    [8]  = { 80, "ArrowDown" },  // HE80
    [9]  = { 81, "?"         },  // HE81 → Not assigned in earlier mapping
    [10] = { 58, "PgDn"      },  // HE58
    [11] = { 44, "PgUp"      },  // HE44
    [12] = { 29, "Home"      },  // HE29
    [13] = { 14, "ShiftL"    },  // HE14
    [14] = { 43, "Backslash" },  // HE43
    [15] = { 28, "Backspace" },  // HE28
    [16] = {  0,  NULL       }   // GND
};

// End of MUX Channel Wiring