# UART Debug Toggle System

## Overview
The keyboard now has custom keycodes to toggle debug output via UART, keeping the logic cleanly separated from keymap.c for visual simplicity.

## File Structure

### uart_keycodes.h
- Defines custom keycodes: `DEBUG_ADC`, `DEBUG_KEYS`, `MENU_OPEN`, etc.
- Declares helper functions for debug control
- Can be included anywhere you need the custom keycodes

### uart_keycodes.c
- Implements `process_record_user()` with all custom keycode logic
- Contains state variables: `adc_debug_enabled`, `key_debug_enabled`
- Helper functions:
  - `toggle_adc_debug()` - Toggles ADC value printing (every 1 second)
  - `toggle_key_debug()` - Toggles keypress event printing
  - `get_adc_debug_enabled()` - Returns current ADC debug state
  - `get_key_debug_enabled()` - Returns current key debug state

### mux_adc.c
- Calls `get_adc_debug_enabled()` to check if ADC values should be printed
- Calls `get_key_debug_enabled()` to check if key events should be printed
- No direct dependency on the toggle logic - uses clean getter functions

### keymap.c
- Includes `uart_keycodes.h` to use custom keycodes
- Maps keys to custom keycodes (e.g., X → DEBUG_ADC, Slash → DEBUG_KEYS)
- Does NOT implement `process_record_user()` - that's in uart_keycodes.c

## Custom Keycodes

| Keycode | Value | Mapped to | Function |
|---------|-------|-----------|----------|
| DEBUG_ADC | SAFE_RANGE | X key | Toggle ADC row printing (every 1s) |
| DEBUG_KEYS | SAFE_RANGE+5 | Slash key | Toggle keypress event messages |
| MENU_OPEN | SAFE_RANGE | - | Send "MENU_OPEN\n" via UART |
| MENU_UP | SAFE_RANGE+1 | - | Send "MENU_UP\n" via UART |
| MENU_DOWN | SAFE_RANGE+2 | - | Send "MENU_DOWN\n" via UART |
| MENU_SELECT | SAFE_RANGE+3 | - | Send "MENU_SELECT\n" via UART |

## Usage

1. **Toggle ADC printing**: Press X key
   - When enabled: Prints all ADC values by row every 1 second
   - When disabled: No periodic ADC output
   - Sends confirmation message: "[DEBUG] ADC printing: ON/OFF"

2. **Toggle key event printing**: Press Slash key
   - When enabled: Prints "Key X: PRESS/RELEASE (R# C#) ADC=###" for each keypress
   - When disabled: Keys still work, but no UART messages
   - Sends confirmation message: "[DEBUG] Key event printing: ON/OFF"

## VIA Compatibility
The code also supports VIA custom keycodes:
- 0xffff → MENU_OPEN
- 0xfffe → MENU_UP
- 0xfffd → MENU_DOWN
- 0xfffc → MENU_SELECT
- 0xfffb → DEBUG_ADC
- 0xfffa → DEBUG_KEYS

## Why This Structure?

✅ **Visual simplicity**: keymap.c stays clean with just key layouts
✅ **Organization**: All UART logic in uart_keycodes.c
✅ **Reusability**: Helper functions can be called from anywhere
✅ **Maintainability**: Easy to add more debug toggles or menu commands
✅ **Clean separation**: mux_adc.c doesn't know about toggle implementation

## Initial State
Both debug outputs start **enabled** by default:
- `adc_debug_enabled = true`
- `key_debug_enabled = true`

Change the initial values in uart_keycodes.c if you want them disabled by default.
