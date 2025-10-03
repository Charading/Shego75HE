# Using SHEGO75 Breadboard with VIA

## Setup

### 1. Update the Firmware
The keyboard.json has been updated with:
- **VID**: `0xDEAD`
- **PID**: `0x5460` (was `0xDEAD`)

Compile and flash your firmware:
```bash
qmk compile -kb shego75_v1 -km default
qmk flash -kb shego75_v1 -km default
```

### 2. Load the VIA Definition

#### Option A: Use VIA's Design Tab (Recommended)
1. Open VIA (https://usevia.app or desktop app)
2. Go to the **Settings** tab
3. Enable **"Show Design tab"**
4. Go to the **Design** tab
5. Click **"Load"** and select `shego_breadboard.json`
6. Your keyboard should now appear in VIA!

#### Option B: Sideload JSON
1. Open VIA
2. Go to **Settings** → **Show Design tab** (enable)
3. **File** → **Import Keymap**
4. Select `shego_breadboard.json`

### 3. Verify Connection
- Plug in your keyboard
- VIA should detect it as "SHEGO75 Breadboard"
- You should see a 4x12 ortholinear layout

## Custom Keycodes in VIA

Your custom keycodes will appear in VIA's keycode picker under the "CUSTOM" tab:

| VIA Name | Function | Description |
|----------|----------|-------------|
| **M_OPEN** | MENU_OPEN | Send "MENU_OPEN\n" via UART |
| **M_UP** | MENU_UP | Send "MENU_UP\n" via UART |
| **M_DOWN** | MENU_DOWN | Send "MENU_DOWN\n" via UART |
| **M_SEL** | MENU_SELECT | Send "MENU_SELECT\n" via UART |
| **ADC** | DEBUG_ADC | Toggle ADC value printing (every 1s) |
| **KEYS** | DEBUG_KEYS | Toggle keypress event printing |

## Current Keymap (Layer 0)

```
┌─────┬─────┬─────┬─────┬─────┬─────┬─────┬─────┬─────┬─────┬─────┬─────┐
│ ESC │  1  │  2  │  3  │  4  │  5  │  6  │  7  │  8  │  9  │  0  │  -  │
├─────┼─────┼─────┼─────┼─────┼─────┼─────┼─────┼─────┼─────┼─────┼─────┤
│ TAB │  Q  │  W  │  E  │  R  │  T  │  Y  │  U  │  I  │  O  │  P  │  [  │
├─────┼─────┼─────┼─────┼─────┼─────┼─────┼─────┼─────┼─────┼─────┼─────┤
│CAPS │  A  │  S  │  D  │  F  │  G  │  H  │  J  │  K  │  L  │  ;  │  '  │
├─────┼─────┼─────┼─────┼─────┼─────┼─────┼─────┼─────┼─────┼─────┼─────┤
│SHIFT│  Z  │ ADC │  C  │  V  │  B  │  N  │  M  │  ,  │  .  │KEYS │SHIFT│
└─────┴─────┴─────┴─────┴─────┴─────┴─────┴─────┴─────┴─────┴─────┴─────┘

ADC = Toggle ADC debug printing
KEYS = Toggle key event printing
```

## Troubleshooting

### VIA doesn't detect the keyboard
1. Make sure you compiled and flashed with the new PID (`0x5460`)
2. Check that VIA is enabled in your firmware (it should be - `"via": true` in keyboard.json)
3. Try unplugging and replugging the keyboard
4. Make sure you loaded the `shego_breadboard.json` in VIA

### Custom keycodes don't appear
1. Reload the JSON file in VIA's Design tab
2. Make sure you're looking in the "CUSTOM" tab in the keycode picker
3. Try closing and reopening VIA

### Changes in VIA don't save
- VIA stores keymaps in EEPROM on the keyboard
- Changes should persist across reboots automatically
- If not, check that EEPROM is working (might need to clear EEPROM: hold Space+Backspace while plugging in)

## USB Device Info

After flashing, your keyboard will identify as:
- **Vendor ID**: 0xDEAD
- **Product ID**: 0x5460
- **Device Name**: SHEGO75
- **Manufacturer**: Charading

You can verify with `lsusb` (Linux), Device Manager (Windows), or System Information (macOS).

## Notes

- The PID `0x5460` is arbitrary. You can change it to any value you want in both files.
- Make sure the PID in `keyboard.json` matches the PID in `shego_breadboard.json`
- VIA will only detect your keyboard if the PIDs match!
