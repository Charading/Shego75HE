# ESP32 GIF Player - Development Log (October 2, 2025)

## 🎯 Major Features Added Today

### 1. **On-Screen Debug Popups System** 
Real-time semi-transparent overlays that appear over GIF playback without interrupting animation.

#### Features:
- **Multi-line popups**: Title + 2 body lines
- **Color-coded text**: 
  - Green for "ON/Enabled" states
  - Red for "OFF/Disabled" states  
  - Yellow for keycodes
- **Immediate draw mode**: Popups render instantly without pausing GIF (no flicker/dissolve)
- **Auto-positioning**: Left-aligned text with manual per-string X-offset control
- **Smart detection**: Handles both raw UART messages and `[DEBUG]` prefixed messages

#### Supported Debug Messages:
```
RAW_KEYCODE: 0x____      → "Keypress" popup with yellow hex code
[DEBUG] Key printing: ON/OFF     → "Key Printing" popup
[DEBUG] Raw keycodes: ON/OFF     → "Raw Keycodes" popup  
[DEBUG] ADC printing: ON/OFF     → "ADC printing" popup
SOCD: Enabled/Disabled           → "SOCD status" popup
```

---

### 2. **Manual Text Offset System**
Pixel-perfect positioning for every text string in popups.

#### Location: Lines 95-147
```cpp
ManualOffset manualOffsets[] = {
  {"Keypress detection", 10},   // Shift title right 10px
  {"0x001C", -5},                // Shift keycode left 5px
  {"ADC printing", 0},           // No shift (left edge)
};
```

#### Features:
- **Per-string control**: Each exact text string can have its own X offset
- **Positive = right shift**, **negative = left shift**
- **Automatic hex detection**: Any string starting with `0x` gets +8px offset
- **Case-insensitive matching**: Works with uppercase/lowercase variants
- **Serial Monitor debug**: Prints exact strings when popups appear for easy copy-paste

---

### 3. **New Multi-Menu System**
Complete menu overhaul with settings, timer, and future stopwatch support.

#### Menu Types:
1. **GIF Select Menu** (existing, enhanced)
2. **Settings Menu** (NEW)
3. **Timer Menu** (NEW)
4. **Stopwatch Menu** (planned, not implemented yet)

---

### 4. **Settings Menu** (`SETTINGS_OPEN`)

Full-screen black menu with persistent toggle settings.

#### Menu Structure:
```
Settings:
  > < Back               ← Select to exit
    SOCD: ON             ← Toggle SOCD (green=ON, red=OFF)
    Debugging            ← Non-selectable header
      Raw Keycodes: OFF
      Keypress Detect: OFF
      ADC Printing: OFF
```

#### Navigation:
- `MENU_UP` / `MENU_DOWN`: Navigate (auto-skips "Debugging" header)
- `MENU_SELECT`: Toggle selected setting or exit (if "Back" selected)
- Settings persist across menu opens/closes
- 5-second timeout (returns to GIF playback)

#### Persistent Settings:
- `setting_socd_enabled`
- `setting_raw_keycodes_enabled`
- `setting_keypress_detection_enabled`
- `setting_adc_printing_enabled`

---

### 5. **Timer Menu** (`TIMER_OPEN`)

Full-screen countdown timer with real-time updates.

#### Features:
- **Big centered digital display**: `MM:SS` format (size 4 font)
- **Adjustable duration**:
  - `MENU_UP`: Decrease minutes (min: 1 minute)
  - `MENU_DOWN`: Increase minutes (max: 99 minutes)
- **Visual indicators**: Small orange arrows (▲/▼) shown when stopped
- **Start/Stop**:
  - `MENU_SELECT` when stopped: Starts countdown
  - Any button when running: Resets/stops timer
- **Real-time countdown**: Updates every 100ms
- **Expiration effect**: 
  - Screen flashes 3 times (white background, black "TIME'S UP!" text)
  - 200ms flash intervals
  - Returns to stopped timer display

#### Timer Persistence:
- Timer continues running even when cycling between menus
- Current time/state preserved across menu navigation
- No timeout while timer is running

---

### 6. **Menu Cycling System** (For Future Stopwatch)

Horizontal navigation between timer and stopwatch menus.

#### Commands:
- `MENU_CYCLE_RIGHT`: Cycle to next menu (timer → stopwatch)
- `MENU_CYCLE_LEFT`: Cycle to previous menu (stopwatch → timer)

**Note**: Stopwatch not yet implemented (placeholder ready)

---

## 🔧 Technical Improvements

### Bug Fixes:
1. **Popup flicker eliminated**: Changed from request/pause pattern to immediate inline draw
2. **GIF dissolve artifact fixed**: Proper mutex management and immediate draws
3. **Variable scope errors**: Fixed `tableTitleX/Line1X/Line2X` declarations
4. **Duplicate popup titles**: Separated "Key Printing" vs "Raw Keycodes" vs "Keypress" popups
5. **Text centering issues**: Switched to left-alignment with manual offsets
6. **Serial spam reduction**: Removed "starting playback loop" repeated message

### Code Architecture:
- **Menu type enum**: `MENU_GIF`, `MENU_SETTINGS`, `MENU_TIMER`, `MENU_STOPWATCH`
- **Context-aware commands**: MENU_UP/DOWN/SELECT behave differently per menu type
- **Mutex-protected draws**: All immediate popup draws use proper SPI mutex
- **State persistence**: Settings and timer state preserved across menu changes
- **No blocking**: Timer updates in `loop()` without blocking GIF playback

---

## 📡 New UART Commands

### Settings & Menus:
```
SETTINGS_OPEN       → Open settings menu
TIMER_OPEN          → Open timer menu (not yet implemented, use after SETTINGS_OPEN + MENU_CYCLE)
MENU_CYCLE_RIGHT    → Cycle menus right
MENU_CYCLE_LEFT     → Cycle menus left
```

### Existing Commands (Enhanced):
```
MENU_OPEN           → Open GIF select menu
MENU_CLOSE          → Close any menu, return to GIF
MENU_UP             → Navigate up / Decrease timer minutes
MENU_DOWN           → Navigate down / Increase timer minutes
MENU_SELECT         → Select item / Toggle setting / Start-Stop timer
STATUS              → Get current menu state and GIF info
```

**Context-aware behavior**: UP/DOWN/SELECT change function based on active menu type.

---

## 📝 Code Locations

### Key Files:
- **Main sketch**: `esp32_tft_sd_menu_lowpower.ino`
- **GIF draw callback**: `GIFDraw.ino` (handles transparency and frame rendering)

### Important Sections:
- **Lines 95-147**: Manual text offset table (edit here for positioning)
- **Lines 60-84**: Menu state variables and settings toggles
- **Lines 410-720**: `handleQMKCommands()` - UART command parsing
- **Lines 1073-1188**: Settings menu implementation
- **Lines 1193-1309**: Timer menu implementation
- **Lines 1388-1461**: Debug popup drawing functions

---

## 🎨 Visual Design

### Settings Menu:
- Black background
- White text (default)
- Yellow selected item background
- Green "ON" text
- Red "OFF" text
- Thin white scrollbar thumb (right edge)

### Timer Menu:
- Black background
- White large timer digits (size 4 font)
- Orange triangular arrows (▲/▼)
- Green "Running..." status
- Yellow "UP/DOWN=Adjust" instructions
- White/black flash effect on expiration

### Debug Popups:
- Near-black rounded rectangle background (RGB 8,8,8)
- White border
- White title text
- Color-coded body text (green/red/yellow/white)
- Left-aligned with manual X offsets
- No longer centered (easier pixel-perfect positioning)

---

## 🔮 Planned Features (Not Yet Implemented)

### Stopwatch Menu:
- Start/pause with MENU_SELECT
- Reset with MENU_UP or MENU_DOWN
- Big centered display (MM:SS.ms format)
- Persistent state across menu cycles
- Access via MENU_CYCLE_LEFT/RIGHT from timer

---

## 🚀 How to Use

### 1. Upload the sketch to your ESP32

### 2. Open Serial Monitor (115200 baud)

### 3. Send UART commands from QMK or Serial Monitor:

#### View Settings:
```
SETTINGS_OPEN
```
Navigate with MENU_UP/DOWN, toggle with MENU_SELECT

#### Use Timer:
```
SETTINGS_OPEN
MENU_CYCLE_RIGHT    (once stopwatch implemented, or add TIMER_OPEN command)
```
Adjust minutes with UP/DOWN, start with SELECT

#### Customize Popup Positions:
1. Trigger a popup (e.g., press a key with raw keycodes enabled)
2. Check Serial Monitor for exact string:
   ```
   [POPUP] Title: "Keypress" | X_OFFSET: 0
   [POPUP] Line1: "0x001C" | X_OFFSET: 8
   ```
3. Add entry to `manualOffsets[]` array (line ~110):
   ```cpp
   {"Keypress", 15},   // Shift right 15px
   {"0x001C", -5},     // Shift left 5px
   ```
4. Recompile and upload

---

## 🐛 Known Issues

### Fixed Today:
- ✅ Popup flicker/dissolve
- ✅ GIF pause artifacts
- ✅ Duplicate popup titles
- ✅ Text centering inconsistencies
- ✅ Compile errors (variable scope)

### Remaining:
- ⏱️ Stopwatch not implemented
- 📍 No direct `TIMER_OPEN` command yet (workaround: use SETTINGS_OPEN + MENU_CYCLE)

---

## 💡 Tips & Tricks

### Perfect Text Positioning:
1. Use Serial Monitor to see exact strings
2. Start with offset `0` (left edge)
3. Adjust by ±5px increments
4. Positive = right, negative = left
5. Automatic +8px for hex codes (`0x____`)

### Timer Best Practices:
- Set duration before starting
- Any button stops/resets running timer
- No timeout while timer runs
- Timer persists if you cycle menus

### Settings Persistence:
- Toggle settings in menu
- Settings remain even after exiting
- Check Serial Monitor for state changes
- Settings don't auto-send to QMK (read-only display)

---

## 📊 Statistics

### Lines of Code Added: ~400+
### New Functions: 8
- `enterSettingsMenu()`
- `drawSettingsMenu()`
- `handleSettingsSelection()`
- `enterTimerMenu()`
- `drawTimerMenu()`
- `handleTimerSelection()`
- `updateTimer()`
- `getManualOffset()`

### New UART Commands: 4
### New State Variables: 12
### Bug Fixes: 6+

---

## 🙏 Credits

**Development Session**: October 2, 2025  
**Developer**: GitHub Copilot + User Collaboration  
**Platform**: ESP32 + TFT_eSPI + AnimatedGIF  
**Branch**: `exp` (experimental)

---

## 📄 License

Same as parent project (ESP32-GIF-Player)

---

**Happy coding! 🎮✨**
