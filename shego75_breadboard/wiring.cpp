//SPI control pins
#define SPI_SCL GP2
#define SPI_SDA GP3
#define SPI_RES GP4
// Mux to sensor wiring, every unused channel is GND. Mux is HC4067, sensors are SS49E.
// The following maps each MUX channel to the HE sensor number and the
// canonical key name from `annotations_for_shego75_wiring`.
//
----- MUX1 -----
// i0:  HE17 -> 2
// i1:  HE2  -> F1
// i2:  HE16 -> 1
// i3:  HE1  -> Esc
// i4:  HE15 -> `
// i5:  HE30 -> Tab
// i6:  HE45 -> Caps Lock
// i7:  HE59 -> Shift (L)
// i8:  HE60 -> Z
// i9:  HE32 -> W
// i10: HE73 -> Win
// i11: HE46 -> A
// i12: HE31 -> Q
// i13: HE3  -> F2
// i14: HE4  -> F3
// i15: HE18 -> 3

----- MUX2 -----
// i0:  HE33 -> E
// i1:  HE19 -> 4
// i2:  HE5  -> F4
// i3:  HE47 -> S
// i4:  HE74 -> Alt (L)
// i5:  HE61 -> X
// i6:  HE48 -> D
// i7:  HE62 -> C
// i8:  HE63 -> V
// i9:  HE49 -> F
// i10: HE50 -> G
// i11: HE6  -> F5
// i12: HE20 -> 5
// i13: HE34 -> R
// i14: GND
// i15: GND

----- MUX3 -----
// i0:  HE21 -> 6
// i1:  HE7  -> F6
// i2:  HE8  -> F7
// i3:  HE22 -> 7
// i4:  HE51 -> H
// i5:  HE65 -> N
// i6:  HE64 -> B
// i7:  HE75 -> Space
// i8:  GND
// i9:  GND
// i10: GND
// i11: GND
// i12: GND
// i13: HE52 -> J
// i14: HE37 -> U
// i15: HE36 -> Y

----- MUX4 -----
// i0:  GND
// i1:  GND
// i2:  GND
// i3:  GND
// i4:  HE67 -> , (comma)
// i5:  HE66 -> M
// i6:  GND
// i7:  GND
// i8:  HE68 -> . (period)
// i9:  HE54 -> L
// i10: GND
// i11: GND
// i12: HE39 -> O
// i13: HE23 -> 8
// i14: HE9  -> F8
// i15: HE24 -> 9

----- MUX5 -----
// i0:  HE40 -> P
// i1:  HE25 -> 0
// i2:  HE10 -> F9
// i3:  GND
// i4:  GND
// i5:  GND
// i6:  GND
// i7:  HE55 -> ;
// i8:  HE69 -> /
// i9:  HE77 -> Fn
// i10: HE76 -> Alt (R)
// i11: HE78 -> Ctrl (R)
// i12: HE56 -> '
// i13: HE41 -> [
// i14: HE11 -> F10
// i15: HE26 -> -   // (fixed: 26 is minus, not 24)

----- MUX6 -----
// i0:  HE13 -> F12
// i1:  HE42 -> ]
// i2:  HE27 -> =
// i3:  HE12 -> F11
// i4:  HE70 -> Shift (R)
// i5:  HE79 -> Left Arrow
// i6:  HE71 -> Up Arrow
// i7:  HE57 -> Enter
// i8:  HE80 -> Down Arrow
// i9:  HE81 -> Right Arrow
// i10: HE58 -> PgDn
// i11: HE44 -> PgUp
// i12: HE29 -> Home
// i13: HE14 -> Delete
// i14: HE43 -> Backslash
// i15: HE28 -> Backspace




static void annotations_for_shego75_wiring(void) {
// HE1  -> Esc
// HE2  -> F1
// HE3  -> F2
// HE4  -> F3
// HE5  -> F4
// HE6  -> F5
// HE7  -> F6
// HE8  -> F7
// HE9  -> F8
// HE10 -> F9
// HE11 -> F10
// HE12 -> F11
// HE13 -> F12
// HE14 -> Del

// HE15 -> `
// HE16 -> 1
// HE17 -> 2
// HE18 -> 3
// HE19 -> 4
// HE20 -> 5
// HE21 -> 6
// HE22 -> 7
// HE23 -> 8
// HE24 -> 9
// HE25 -> 0
// HE26 -> -
// HE27 -> =
// HE28 -> Backspace
// HE29 -> Home

// HE30 -> Tab
// HE31 -> Q
// HE32 -> W
// HE33 -> E
// HE34 -> R
// HE35 -> T
// HE36 -> Y
// HE37 -> U
// HE38 -> I
// HE39 -> O
// HE40 -> P
// HE41 -> [
// HE42 -> ]
// HE43 -> \
// HE44 -> PgUp

// HE45 -> Caps Lock
// HE46 -> A
// HE47 -> S
// HE48 -> D
// HE49 -> F
// HE50 -> G
// HE51 -> H
// HE52 -> J
// HE53 -> K
// HE54 -> L
// HE55 -> ;
// HE56 -> '
// HE57 -> Enter
// HE58 -> PgDn

// HE59 -> Shift (L)
// HE60 -> Z
// HE61 -> X
// HE62 -> C
// HE63 -> V
// HE64 -> B
// HE65 -> N
// HE66 -> M
// HE67 -> ,
// HE68 -> .
// HE69 -> /
// HE70 -> Shift (R)
// HE71 -> ↑

// HE72 -> Ctrl (L)
// HE73 -> Win
// HE74 -> Alt (L)
// HE75 -> Space
// HE76 -> Alt (R)
// HE77 -> Fn
// HE78 -> Ctrl (R)
// HE79 -> ←
// HE80 -> ↓
// HE81 -> →
}

const uint16_t key_thresholds[32] = {
    // MUX1 thresholds
    540, 540, 540, 540, 540, 540, 540, 540, 540, 540, 540, 540, 540, 540, 540, 540,
    // MUX2 thresholds
    540, 540, 540, 540, 540, 540, 540, 540, 540, 540, 540, 540, 540, 540, 0, 0
    //MUX3 thresholds
    540, 540, 540, 540, 540, 540, 540, 540, 0, 0, 0, 0, 0, 540, 540, 540,
    //MUX4 thresholds
    0, 0, 0, 0, 540, 540, 0, 0, 540, 540, 0, 0, 540, 540, 540, 540
    //MUX5 thresholds
    540, 540, 540, 0, 0, 0, 0, 540, 540, 540, 540, 540, 540, 540, 540, 540,
    //MUX6 thresholds
    540, 540, 540, 540, 540, 540, 540, 540, 540, 540, 540, 540, 540, 540, 540, 540


};

Okay, so, is there a way to instead of using a static threshold, to make it so the keyboard scans all adc keys when plugged in, and then it only does a key press when the value deviates up OR down 10% away