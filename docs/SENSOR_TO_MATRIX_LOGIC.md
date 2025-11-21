# Sensor → Matrix logic

Short reference for how sensor numbers (1–48) map to the keyboard matrix positions used in this project.

## Concept

- Sensors are numbered 1..48. Each sensor maps to a matrix row and column in a 4×12 matrix.
- Formula (integer math):

- matrix_row = (sensor - 1) / 12  // integer division
- matrix_col = (sensor - 1) % 12  // remainder

Or the inverse (given matrix row r and column c):

- sensor = r * 12 + c + 1

Notes:
- Rows and columns are zero-indexed for the matrix (rows 0..3, cols 0..11).
- These formulas are exactly what the scanner code in `mux_adc.c` uses when converting a sensor number to a matrix coordinate.

## Examples

- Sensor 1 → row = (1-1)/12 = 0, col = (1-1)%12 = 0 → matrix[0][0]
- Sensor 6 → row = (6-1)/12 = 0, col = (6-1)%12 = 5 → matrix[0][5]
- Sensor 18 → row = (18-1)/12 = 1, col = (18-1)%12 = 5 → matrix[1][5]
- Sensor 48 → row = (48-1)/12 = 3, col = (48-1)%12 = 11 → matrix[3][11]

Inverse examples:
- Row 0, Col 5 → sensor = 0*12 + 5 + 1 = 6
- Row 1, Col 5 → sensor = 1*12 + 5 + 1 = 18

## Quick code snippets

C (same idea used in the firmware):

```c
// sensor is 1..48
uint8_t matrix_row = (sensor - 1) / 12;
uint8_t matrix_col = (sensor - 1) % 12;
// then use matrix_row and matrix_col to set matrix[row][col]
```

Python (for testing / scripting):

```python
def sensor_to_matrix(sensor):
    assert 1 <= sensor <= 48
    row = (sensor - 1) // 12
    col = (sensor - 1) % 12
    return row, col

def matrix_to_sensor(row, col):
    assert 0 <= row < 4 and 0 <= col < 12
    return row * 12 + col + 1
```

## 4 × 12 sensor grid (sensor number → matrix position)

Rows are left→right (cols 0..11). Each cell shows the sensor number and (r,c).

Row 0:  1 (0,0)   2 (0,1)   3 (0,2)   4 (0,3)   5 (0,4)   6 (0,5)   7 (0,6)   8 (0,7)   9 (0,8)  10 (0,9)  11 (0,10) 12 (0,11)

Row 1: 13 (1,0)  14 (1,1)  15 (1,2)  16 (1,3)  17 (1,4)  18 (1,5)  19 (1,6)  20 (1,7)  21 (1,8) 22 (1,9)  23 (1,10) 24 (1,11)

Row 2: 25 (2,0)  26 (2,1)  27 (2,2)  28 (2,3)  29 (2,4)  30 (2,5)  31 (2,6)  32 (2,7)  33 (2,8) 34 (2,9)  35 (2,10) 36 (2,11)

Row 3: 37 (3,0)  38 (3,1)  39 (3,2)  40 (3,3)  41 (3,4)  42 (3,5)  43 (3,6)  44 (3,7)  45 (3,8) 46 (3,9)  47 (3,10) 48 (3,11)

## Practical notes

- The scanner reads a MUX channel, looks up the configured `sensor` number for that channel (from `mux_pins.c`), and applies the formula above to set the corresponding key in the QMK matrix.
- If two different MUX channels are accidentally configured with the same sensor number, pressing either physical key will activate the same matrix position (causing duplicated/ghosted keypresses).
- If a physical key does nothing, check that the MUX channel's `.sensor` value points to the correct sensor number and that the wiring to that MUX channel/4×4 board is good.

Paste this file anywhere you need; it's meant to be a compact copy/paste reference for quick debugging.
