# ESP32 GIF Player (ST7735)

Small ESP32 sketch to browse GIFs on an SD card, copy a selected GIF into SPIFFS as `/current.gif`, and play it on a ST7735 TFT display.

## Features
- Browse GIFs stored on SD card (up to 20 entries).
- Menu UI on the TFT (navigate with buttons for now. *QMK serial commands soon to be implemented*).
- Copy selected GIF from SD card to SPIFFS atomically (writes a temp file then renames).
- Visual progress bar while copying.
- `Clear GIF` menu option removes `/current.gif` from SPIFFS.
- Plays GIFs from SPIFFS only (prevents playing directly from SD) *I had so many issues trying to get that to work.*



## Pin assignments
These are the defaults used in `esp32_tft_sd_menu.ino`:

| Signal / Purpose | Sketch define | Default GPIO |
|---|:---|---:|
| SD Card CS (VSPI) | `SD_CS_PIN` | 5 |
| SD MOSI (VSPI) | `SD_MOSI` | 22 |
| SD MISO (VSPI) | `SD_MISO` | 19 |
| SD SCK (VSPI) | `SD_SCK` | 21 |
| Button - Up / Previous | `BUTTON_UP` | 14 |
| Button - Down / Next | `BUTTON_DOWN` | 13 |
| Button - Select | `BUTTON_SELECT` | 15 |
| TFT reset (optional) | (toggled via GPIO) | 4 |
| QMK UART RX (ESP32 receives) | `QMK_RX_PIN` | 16 |
| QMK UART TX (ESP32 sends) | `QMK_TX_PIN` | 17 |

> Note: TFT pin mappings (SPI, DC, CS, MOSI/MISO/SCLK) are configured via the `TFT_eSPI` library `User_Setup.h`. The sketch only toggles GPIO 4 as an optional display reset line if wired.

## Behavior / Usage
1. On boot the frimware initializes SD and SPIFFS and scans the SD root for `.gif` files.
2. If a valid `/current.gif` exists in SPIFFS the frimware will attempt to play it. If not, the menu is shown for gif selection.
3. In the menu:
   - Use `BUTTON_UP` and `BUTTON_DOWN` to change selection.
   - Short press `BUTTON_SELECT` to copy the chosen GIF into SPIFFS as `/current.gif` and start playback.
   - The bottom menu item is `Clear GIF`, which deletes `/current.gif` from SPIFFS.
4. Copying is atomic: the file is written to a temporary file then renamed to `/current.gif` on success. A progress bar is shown on the TFT while copying.

## QMK Serial Commands
You can control the menu remotely over the second hardware serial port (pins 16/17):*

- `MENU_OPEN`  – open menu
- `MENU_CLOSE` – close menu
- `MENU_UP`    – move selection up
- `MENU_DOWN`  – move selection down
- `MENU_SELECT`– select current item (copy / clear)
- `STATUS`     – request status string

The firmware replies with simple status messages (e.g. `MENU_OPENED`, `GIF_SELECTED:...`).

**Still yet to be implemented properly.*

## Upload / Troubleshooting
- Use the Arduino IDE or PlatformIO with the ESP32 board package installed.
- If upload fails with "Wrong boot mode detected (0x13)", put the board into download mode by holding BOOT (IO0) while plugging USB, then upload.
- Serial output (115200) prints helpful debug messages during SD/SPIFFS init, menu actions and copying.
- If copying freezes:
  - Verify SD card wiring and power.
  - Make sure the TFT and SD do not share conflicting pins in your `TFT_eSPI` configuration.
  - Check that SPIFFS has enough free space for the selected GIF.
  - Clear GIF to free up SPIFFS then try again with another smaller gif.

## Files
- `esp32_tft_sd_menu.ino` — main sketch (menu, copy logic, GIF playback callbacks).

## Notes
- The TFT driver uses `TFT_eSPI` and its pin mapping is controlled by the `User_Setup.h` / `TFT_eSPI` config. Adjust that file if your display uses different pins.
- SPI transactions and a mutex are used to serialize SD/TFT/SPIFFS accesses. Avoid changing SPI pins at runtime.

If you want any additional documentation (example wiring diagram, change log, or a shorter quickstart), tell me what to include and I will add it.


*i am still very new to coding and all this and github copilot has been a huge saviour for me, so please do forgive my spaghetti code 🍝.*
