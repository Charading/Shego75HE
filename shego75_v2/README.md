# Shego75 V2 Firmware Rewrite

This directory hosts the from-scratch RP2040 firmware that replaces the legacy QMK build.  The focus is to keep feature parity with `shego75_v1`, while unlocking a more flexible USB stack (TinyUSB) that can coexist with custom tooling (SignalRGB + Electron `app_v2`).

## High-Level Architecture

| Layer | Responsibility |
| --- | --- |
| **Hardware Abstraction** | Matrix scan scheduler, per-key debounce, rotary encoders, hall-effect hooks, GPIO/I2C/SPI helpers shared by all form factors. |
| **Lighting Engine** | RGB matrix + underglow control, shared buffer of `TOTAL_LEDS`, translation from SignalRGB streaming data, animation fallbacks. |
| **USB Services** | TinyUSB device stack exposing (1) standard HID keyboard, (2) VIA-compatible vendor HID (ID `0xFEED/0x0001` reports), (3) SignalRGB RAW HID endpoint, (4) bulk endpoint for GIF transfer. |
| **Protocol Bridge** | Dispatches host packets to the appropriate service (keymap changes, macros, GIF streaming) and forwards GIF payloads to the ESP32 companion over I2C. |
| **Board Targets** | `shego75_v2` (full board, VID `0xDEAD`, PID `0x0444`) and `shego_mini` (4×4 tester, PID `0x1616`). |

## USB Endpoints

1. **Keyboard HID (Interface 0)**
   - 6KRO/NKRO selectable via VIA.
   - Standard boot protocol compatibility.
2. **VIA Config Interface (Interface 1)**
   - Reuses the VIA protocol (0xFEED usage page) implemented manually.  Report handlers mirror what existed in Shego16 Rust firmware, allowing the VIA desktop app to discover layouts and send keymap edits.
3. **SignalRGB RAW HID (Interface 2)**
   - Includes the provided `signalrgb_patched.c/.h` logic.  Packets route to the lighting engine and reuse the LED bounds/error responses.
4. **GIF Bulk Endpoint (Interface 3)**
   - Vendor-specific IN/OUT endpoints.  Host streams chunked GIF data which is buffered in external/QSPI flash and relayed to the ESP32 display controller via I2C commands.

## Data Flow

```
Electron app_v2 -> USB vendor endpoint -> RP2040 GIF bridge -> I2C -> ESP32 display firmware
SignalRGB plugin -> RAW HID -> LED engine -> local LED drivers
VIA desktop -> VIA vendor HID -> Keymap storage -> Flash + runtime layers
```

## Build Targets

- `shego75_v2`: full keyboard.  Defines matrix size, number of LEDs, hall sensors, trackball, encoder, etc.
- `shego_mini`: 4×4 dev board that reuses the same core but with a drastically smaller matrix definition and different PID.
- `shego75_v2.1`: Pico Extension sandbox.  Eventually it will reuse `src/` and only override board config / `pico_sdk_import.cmake`.

## Directory Layout (planned)

```
src/
  board/
    shego75_v2/board_config.h
    shego_mini/board_config.h
  drivers/
    matrix.c
    rgb.c
    hall.c
    encoder.c
    i2c_bridge.c
  usb/
    descriptors.c
    via_protocol.c
    signalrgb_patched.c
    gif_endpoint.c
  app/
    keymap_storage.c
    lighting_engine.c
    gif_router.c
CMakeLists.txt
cmake/
  pico_sdk_import.cmake (symlink or copy)
```

## Next Steps

1. Author the top-level `CMakeLists.txt` pulling in Pico SDK + TinyUSB configuration.
2. Carve out reusable core modules (matrix scan, RGB, USB descriptors).
3. Port the SignalRGB code into `src/protocol/signalrgb/signalrgb_patched.c` with minimal adjustments (namespacing, header path tweaks).
4. Implement VIA protocol responses (device info, layout data, dynamic keymaps).
5. Bring up GIF streaming endpoint and I2C forwarding.
6. Mirror the build system for `shego_mini` to guarantee the shared core compiles for both targets.
7. Start `app_v2` (Electron) with a shared USB transport library that can detect PIDs and push lighting/GIF packets.

## Building the targets

```
cd shego75_v2
cmake -B build -G "Ninja" -DPICO_SDK_PATH="<path-to-sdk>"
cmake --build build --target shego75_v2

# For the breadboard/PICO tester
cmake --build build --target shego_mini_dev
```

Both binaries share the same core and differ only by the board configuration source that injects PID/VID pairs, matrix geometry, LED counts, and GIF/I2C limits.
```
