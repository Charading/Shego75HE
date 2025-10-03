Problem: Encoder press was generating a '.' because the firmware sometimes sent a raw HID key (e.g., KC_DOT) for the encoder press when layer 3 or earlier mappings fell through.

Fixes applied:
- Replaced hardcoded press behavior with a per-layer `encoder_press_map` and a special sentinel `ENC_TOGGLE_L3` to express the "toggle menu-control layer" action instead of using `TG(3)` (which was being sent as HID and produced '.').
- Implemented explicit handling:
	- Press while on layer 1 → turn ON layer 3 and also send the UART "MENU_OPEN" string immediately.
	- Press while on layer 3 while `MO(1)` is held → turn OFF layer 3 (return to normal).
	- If layer 3 has `MENU_SELECT` mapped, it is implemented via `QK_USER_3` and handled by `process_record_user()` so the action uses UART strings instead of HID keycodes.
- Added UART debug output for each encoder press to print the numeric mapped code (`ENCODER_PRESS: code=0xNNNN`) so you can confirm what's being sent.
- Ensured `QK_USER_*` entries are handled in firmware by mapping them to UART strings (no HID keycodes emitted there).
- Added an optional sentinel `ENC_TOGGLE_L3` to express the toggle action and avoid sending it as a HID keycode.

How the new flow works

Start in normal layer (0).
- Hold `MO(1)` to switch to layer 1 (menu control layer).
- While holding `MO(1)`, press the encoder:
	- Layer 3 will be turned ON and the firmware will send "MENU_OPEN\n" over UART so the attached host menu opens.
	- Layer 3 stays active until you press the encoder again while holding `MO(1)`. Then it turns OFF.
	- When in layer 3, the encoder rotation/up/down should navigate the menu (`MENU_UP`/`MENU_DOWN`), and encoder press will send `MENU_SELECT` via UART (mapped via `QK_USER_3`).
	- Encoder press no longer emits arbitrary HID keys like '.' — menu commands are sent via UART strings.

