# SHEGO_BREADBOARD (shego75_v1)

Overview
- Keyboard: SHEGO_BREADBOARD (RP2040)
- Manufacturer: Charading
- Layout: 4x12 ortholinear
- MCU: RP2040, bootloader: rp2040
- RGB: WS2812 (vendor driver), GPIO: GP17, LED count: 94
- Encoder: rotary on GP5 / GP6, encoder switch available
- UART: used to communicate with an ESP32 (RX on GP1 is expected for incoming data)
- Features enabled: RGB matrix, encoder, VIA, signalrgb, nkro, mousekey, extrakey

Key runtime behaviors
- On keyboard init the RGB matrix is enabled and baseline color is set to dim red (~10%).
- Layer 3 behavior:
  - When Layer 3 becomes active, keyboard sends "TFT_FOCUS\n" to the ESP32 and forces the last three LEDs (status bar) to 0x00AAFF (blue).
  - When Layer 3 deactivates, keyboard sends "BOARD_FOCUS\n" and releases the forced override so normal RGB effects resume.
- Caps Lock:
  - When Caps Lock is active the ambient strip LEDs (indices 82..91, zero-based) are forced to white.
- Layer transitions:
  - When switching to a higher layer, LED #94 (zero-based index 93) is briefly set to white and then fades back.
  - When switching to a lower layer, LED #92 (zero-based index 91) pulses white then fades.
- SOCD:
  - When SOCD is enabled the ambient strip (LEDs 82..91) blinks green twice.
  - When SOCD is disabled the ambient strip blinks red twice.
- UART receive:
  - The firmware parses incoming lines over UART and can react to commands from the ESP32.
  - Expected plain-text commands (newline-terminated): TFT_FOCUS, BOARD_FOCUS, MENU_OPEN, MENU_UP, MENU_DOWN, MENU_SELECT, SETTINGS_OPEN, TFT_ACK.

Pins and LED indexing
- WS2812 data pin: GP17 (set in `keyboard.json`).
- Encoder: pin_a GP5, pin_b GP6, resolution 2.
- UART RX: GP1 (connect ESP32 TX -> GP1). Connect ESP32 RX to the keyboard TX pin (see `uart.h`/`uart.c` for TX pin).
- LED indices are zero-based: 0 .. 93 (total 94 LEDs).
  - Ambient strip: indices 82 .. 91
  - Status bar (last 3): indices 91 .. 93 (these correspond to physical LEDs 92..94 in 1-based indexing)

Build and flash (Windows)
- From qmk firmware root:
  - Compile:
    qmk compile -kb shego75_v1 -km default
  - Flash (RP2040 options):
    - Option A: Use QMK flash (if configured)
      qmk flash -kb shego75_v1 -km default
    - Option B: Use UF2/Mass Storage:
      - Hold BOOTSEL on the RP2040, copy the compiled `.uf2` to the RP2040 USB mass storage device.
- If compile errors show duplicate `keyboard_post_init_user`, ensure only one definition exists (prefer keeping it in `keymaps/default/keymap.c`). Do not define `keyboard_post_init_user` in `lighting.c`.

Troubleshooting
- Only two LEDs light:
  - Verify `keyboard.json` led_count = 94 and pin = GP17.
  - Verify power to the WS2812 strip and correct data direction (COL2ROW is keyboard diode direction, unrelated to LED strip direction).
  - Test by temporarily forcing all LEDs in code: e.g. call `rgb_matrix_set_color_all(26,0,0)` in `keyboard_post_init_user`.
  - Confirm physical wiring and that the first LED is at the expected strip end.
- Linker errors about undefined `send_tft_focus` / `uart_receive_task`:
  - Ensure `uart_commands.c` is compiled and linked (present in `rules.mk` SRC).
  - Ensure prototypes are declared in headers included by files that call them (e.g., `uart.h`, `lighting.h`).
- Multiple definition of `keyboard_post_init_user`:
  - Remove duplicate definitions. Keep only one `keyboard_post_init_user` (recommended location: the keymap file).
- Encoder behavior:
  - Encoder mapping is provided in the keymap and encoder press handling is implemented in `matrix_scan_user`.
  - If VIA needs the encoder map, ensure `encoder_map` is present (it is included unconditionally in this keymap).

UART protocol (simple reference)
- Commands sent from keyboard (examples):
  - "TFT_FOCUS\n" — request TFT focus (sent when entering layer 3)
  - "BOARD_FOCUS\n" — request board focus (sent when leaving layer 3)
  - "MENU_OPEN\n", "MENU_UP\n", "MENU_DOWN\n", "MENU_SELECT\n", "SETTINGS_OPEN\n"
- Commands from ESP32 the keyboard expects:
  - "TFT_ACK\n" — example acknowledgment used by the firmware
- Implementation detail:
  - Incoming UART lines are processed by `uart_receive_task()` called from `matrix_scan_user()`.

Files of interest
- keyboard.json — pinout, led_count, layout mapping
- rules.mk — build options and SRC list for custom modules
- keymaps/default/keymap.c — keymap, encoder handling, keyboard_post_init_user, layer hooks
- lighting.c / lighting.h — LED override APIs (set_layer3_override, set_caps_override, start_led_pulse, trigger_socd_blink)
- uart.c / uart.h / uart_commands.c — UART init, send/receive helpers, command handling
- socd.c — SOCD toggle calls `trigger_socd_blink(...)`

Notes
- LED indices shown in code are zero-based.
- Keep `keyboard_post_init_user` in exactly one translation unit to avoid linker conflicts.
- If further adjustments are needed for pin assignments, refer to `keyboard.json` and source `uart.c` / `lighting.c`.

Quick test sequence
1. Compile and flash.
2. On boot the RGB matrix should show a dim red baseline.
3. Toggle Caps Lock — ambient strip 82..91 should become white.
4. Activate Layer 3 — last three LEDs should become blue and ESP32 should receive "TFT_FOCUS".
5. Deactivate Layer 3 — last three LEDs return to normal and ESP32 receives "BOARD_FOCUS".
6. Trigger SOCD changes and confirm ambient strip blinks.

License
- This keyboard repository follows QMK license conventions (GPL-2.0-or-later where applicable).