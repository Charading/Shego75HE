# Shego75HE - Dev Branch Developer Diary

## 2025-11-21
- Added vendor-class USB descriptors and host-side vendor/bulk support (experimental).
  - Firmware: descriptors added and a `vendor_bridge` stub introduced so host tools can detect a vendor interface; shared HID processing extracted into `hid_process_received_buffer()` to unify raw-HID and vendor paths.
  - Host: Electron app updated to prefer `node-usb` vendor bulk endpoints with a `node-hid` fallback; test utilities created (`test-vendor.js`, `try-claim.js`, `debug-usb-interface.js`) to enumerate, claim, and exercise vendor endpoints on Windows (Zadig/WinUSB or libusbK required).
  - Debugging: one-shot IN transfers replaced `startPoll()` in tests to avoid libusb pending-transfer races; remaining Windows libusb polling race noted for follow-up.

- SOCD and input handling improvements.
  - Fixed SOCD logic to avoid duplicate key registrations on release and ensured `socd_process_key()` handles registrations/unregistrations itself to prevent QMK double-processing.

- Sensor / ADC tuning and crosstalk work.
  - Auto-calibration kept but per-key sensitivity made adjustable; default sensitivity tuned (developer tested values 2%→4% during tuning).
  - Debounce lowered for snappier response (runtime tuned from 5ms down to 2ms in testing); matrix scan throttle adjusted during tuning to free CPU for RGB rendering (various values tried: 2–5ms; 1.5ms target discussed).
  - Investigated MUX-channel crosstalk (adjacent-channel bleed observed). Firmware includes diagnostic output and temporary mitigation strategies, but hardware fixes (filter caps, routing changes, remapping) recommended as the proper solution.

- LED / RGB and power changes.
  - Inverted GP23 LED logic to match new two-transistor (BSS138 + AO3401) gate circuit; initialization and toggle logic updated accordingly.
  - Added `RGB_MATRIX_MAXIMUM_BRIGHTNESS` cap (70%) to reduce heat.

- Encoder fixes and VIA support.
  - Encoder handler fixed so `encoder_map` is honored; direction inversion and resolution options were tested (resolution restored to 4, direction inverted per user preference).
  - Created a `shego75he-via.json` definition and documented loading it into VIA (Design → Load Draft Definition) so VIA recognizes custom keycodes (QK_USER / custom actions).

- Misc and testing notes
  - Multiple host test scripts and small utilities added to `software/` to exercise vendor endpoints and debug claim/release behavior on Windows with libusb-based backends.
  - Ongoing: implement proper ChibiOS/QMK vendor endpoint handling in firmware (non-stub), and finish host-side robust vendor handshake to fully validate end-to-end vendor → I2C → ESP32 GIF streaming.


## 2025-11-04
- Finalized full firmware integration for working board in `shego75_v1/`.
  - Verified full boot sequence and peripheral initialization.
  - Cleaned up debug output and confirmed stable board operation.
- Updated LED logic for consistent startup state.
  - Adjusted light commit routines in `rgb_control.c`.
- Confirmed all modules (ADC, MUX, display, encoder) operate synchronously.
- Added minor refinements to USB HID behavior and timing.

## 2025-11-01
- Refined SOCD cleaning logic in `shego75_v1/holoscan.c`.
  - Prevented double-press behavior on key release.
  - Adjusted key state transitions and debounce edge handling.
- Updated internal variable naming for clarity.
- Verified key scanning stability after SOCD updates.

## 2025-10-26
- Added preliminary I2C support to `esp32_display/`.
  - Implemented base I2C communication functions.
  - Evaluating UART replacement for data streaming.
- Began GUI app logic for key actuation adjustment and GIF upload.
  - Introduced base structure for user configuration commands.
- Updated actuation logic in `shego75_v1/holoscan.c`.
  - Added automatic calibration at startup.
  - Uses 15% deviation (≈85% threshold) for press registration.
- Removed old PCB versions (v2.1, v2.2) and migrated to TinyPico Nano base.
  - Cleaned outdated design references in firmware.

## 2025-10-23
- Added LED MOSFET and encoder pin defines in `shego75_v1/pins.h`.
  - Updated pin mapping for v2.2 hardware.
- Refined scanning behavior and encoder logic.
  - Confirmed new encoder pins functional under revised firmware.
- Minor cleanup across hardware abstraction layer.
  - Adjusted timing constants in `shego75_v1/main.c`.

## 2025-10-15
- Minor schematic and firmware tidying.
  - Simplified internal hardware mapping arrays.
  - Adjusted power pin defines for upcoming PCB rev.

## 2025-10-14
- Introduced v2.1 hardware layout updates (transistors, pin changes).
  - Updated `pins.h` and `board_config.c` to match new schematic.
  - Continued work on firmware compatibility.

## 2025-10-05
- Added SignalRGB plugin files for LED integration (`plugins/signalrgb/`).
  - Added packet encoder for LED frame updates.
- Replaced HC4067 logic with ADG732 in `shego75_v1/holoscan.c`.
  - Rewrote MUX channel handling and pin selection logic.
- Created typedef enums for sensor-mux mapping.
  - Simplified multiplexed ADC reads and loop structure.
- Added `.gitignore` and removed unnecessary assets.
  - Cleaned temporary test scripts and debug files.
- Merged unified `dev` branch with stable foundation.

## 2025-10-03
- Initialized main repo with minimal structure.
  - Added `README.md` and stub firmware directories.
  - Set up initial branch for early development.

---

### Notes
This changelog compiles all commits (including minor tweaks and experiments) across the `dev` branch, excluding hardware CAD files under `pcbs/` and `case/`. Each section reflects functional and structural code changes drawn from commit diffs.

