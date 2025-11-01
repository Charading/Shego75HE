# Quick Reference Guide - ESP32 GIF Player Menus

## 🎮 UART Commands Cheat Sheet

### Menu Navigation
| Command | Description |
|---------|-------------|
| `MENU_OPEN` | Open GIF select menu |
| `SETTINGS_OPEN` | Open settings menu |
| `MENU_CLOSE` | Close menu, return to GIF |
| `MENU_UP` | Navigate up / Decrease timer |
| `MENU_DOWN` | Navigate down / Increase timer |
| `MENU_SELECT` | Select / Toggle / Start-Stop |
| `MENU_CYCLE_RIGHT` | Cycle menus → |
| `MENU_CYCLE_LEFT` | Cycle menus ← |
| `STATUS` | Get current state |

---

## 📋 Settings Menu Items

```
Settings:
  > < Back                      [SELECT = Exit]
    SOCD: ON/OFF                [SELECT = Toggle]
    Debugging                   [Header - not selectable]
      Raw Keycodes: ON/OFF      [SELECT = Toggle]
      Keypress Detect: ON/OFF   [SELECT = Toggle]
      ADC Printing: ON/OFF      [SELECT = Toggle]
```

**Navigation**: Auto-skips "Debugging" header  
**Timeout**: 5 seconds of inactivity

---

## ⏱️ Timer Menu Controls

### When Stopped:
- `MENU_UP` → Decrease minutes (min: 1)
- `MENU_DOWN` → Increase minutes (max: 99)
- `MENU_SELECT` → Start countdown
- **Visual**: Orange arrows ▲▼ visible

### When Running:
- **Any button** → Stop & Reset
- **Auto-update**: Every 100ms
- **No timeout**: Runs indefinitely

### On Expiration:
- Screen flashes 3× (white/black)
- "TIME'S UP!" message
- Returns to stopped state (00:00)

---

## 🎨 Text Offset Editing

### Quick Start:
1. Trigger popup (e.g., press key)
2. Check Serial Monitor:
   ```
   [POPUP] Title: "Keypress" | X_OFFSET: 0
   ```
3. Edit line ~110 in sketch:
   ```cpp
   {"Keypress", 15},  // +15px right
   ```
4. Upload

### Rules:
- **Positive** = shift RIGHT
- **Negative** = shift LEFT
- **0** = left edge (default)
- **Auto**: `0x____` gets +8px

---

## 🚦 Debug Popup Types

| UART Message | Popup Title | Body Color |
|--------------|-------------|------------|
| `RAW_KEYCODE: 0x001C` | Keypress | Yellow |
| `[DEBUG] Key printing: ON` | Key Printing | Green |
| `[DEBUG] Key printing: OFF` | Key Printing | Red |
| `[DEBUG] Raw keycodes: ON` | Raw Keycodes | Green |
| `[DEBUG] Raw keycodes: OFF` | Raw Keycodes | Red |
| `[DEBUG] ADC printing: ON` | ADC printing | Green |
| `[DEBUG] ADC printing: OFF` | ADC printing | Red |
| `SOCD: Enabled` | SOCD status | Green |
| `SOCD: Disabled` | SOCD status | Red |

**Display**: 750ms (keycodes) or 1500ms (settings)  
**Behavior**: No GIF pause, immediate draw

---

## 🔧 Common Tasks

### Change Timer Default (5 min → X min):
Line 79: `int timerMinutes = 5;` → `int timerMinutes = X;`

### Adjust Popup Duration:
Line 76: `unsigned long debugPopupDefaultMs = 1500;`

### Move ALL Keycodes Right 10px:
Line 140-143: Change `return 8;` → `return 10;`

### Add New Setting to Menu:
1. Add bool at line ~72 (e.g., `bool setting_xyz = false;`)
2. Add to items array in `drawSettingsMenu()` (~line 1112)
3. Add handler in `handleSettingsSelection()` (~line 1150)
4. Update total count in MENU_UP/DOWN handlers

---

## 📊 Menu State Variables

```cpp
currentMenuType          // MENU_GIF, MENU_SETTINGS, MENU_TIMER
inMenu                   // true if any menu is open
menuSelection           // GIF menu cursor position
settingsSelection       // Settings menu cursor position
timerMinutes           // Timer duration (adjustable)
timerRunning           // Timer active state
setting_socd_enabled   // SOCD toggle state
// ... more settings
```

---

## 🐛 Troubleshooting

### Popup Text Not Moving:
1. Check Serial Monitor for exact string
2. Ensure entry matches exactly (case-insensitive ok)
3. Verify comma after each entry (except last)
4. Recompile and upload

### Timer Not Starting:
- Ensure you're in TIMER menu (not SETTINGS)
- Check Serial Monitor for "Timer started" message
- Try resetting with any button first

### Menu Stuck/Frozen:
- Send `MENU_CLOSE` via UART
- Menu auto-exits after 5s timeout (except timer)
- Physical buttons also work (if wired)

### Popups Flickering:
- Should be fixed now (immediate draw mode)
- If still flickering, check Serial for errors
- Ensure latest code is uploaded

---

## 💾 Backup Settings Before Editing

### Critical Lines to Backup:
- Lines 95-120: Manual offset table
- Lines 60-84: Menu state and settings
- Lines 410-800: handleQMKCommands() function

### Safe Edit Areas:
- Offset X values (can't break anything)
- Timer default minutes
- Popup duration (milliseconds)
- Color definitions (TFT_GREEN, etc.)

---

## 📞 Quick Debug Commands

```cpp
// In Serial Monitor or QMK:
STATUS                    // Check current menu state
[DEBUG] ADC printing: ON  // Test ADC popup
RAW_KEYCODE: 0x001C       // Test keycode popup
SOCD: Enabled             // Test SOCD popup
SETTINGS_OPEN             // Open settings
MENU_CLOSE                // Force close menu
```

---

## 🎯 Best Practices

1. **Test popups first**: Trigger each type and copy exact strings
2. **Small adjustments**: Move text by ±5px at a time
3. **Backup often**: Save working offset values
4. **Serial Monitor**: Always keep open during testing
5. **One change**: Edit one offset at a time, test, repeat

---

**For detailed information, see `CHANGELOG_2025-10-02.md`**
