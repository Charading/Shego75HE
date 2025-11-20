# shego_app

Minimal Electron app with a button that sends the custom Raw HID command `0x30` (LED_TOGGLE) to the Shego75 firmware. Node cannot send Raw HID packets reliably, so every command is executed by `python scripts/toggle_led.py`, which uses hidapi under the hood. Firmware maps this command to `toggle_led()`, same effect as pressing the hardware `LED_TOG` key.

How to run

1. Install dependencies (Windows PowerShell):

```powershell
cd X:\Shego75HE\shego_app
npm install
npm start
```

2. Install Python 3 + `hidapi` (`pip install hidapi`) and ensure `python` is available in PATH. The Electron app shells out to the Python helper for every packet.

Manual CLI usage (bypasses Electron UI):

```powershell
cd X:\Shego75HE\shego_app
python scripts\toggle_led.py --vid DEAD --pid C0DE --cmd 30
```

Notes
- Sends report with command byte `0x30` (defined in firmware as `HID_REPORT_ID_LED_TOGGLE`).
- SignalRGB module now forwards unknown/non-module packets, so this command works even when SignalRGB is active.
- If you change the command ID in firmware, update `renderer.js` (`CMD_LED_TOGGLE`) and/or pass a different `--cmd` to `scripts/toggle_led.py`.
