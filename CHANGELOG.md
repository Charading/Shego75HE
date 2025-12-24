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

---

## 24/12/2025 - Merry Christmas! GIANT UPDATE!
I’ve got the software working and a completely new firmware stack built from the ground up. The app is **Electron-based** and (honestly) was mostly vibe-coded while I iterated on UX and device behavior. The firmware is also brand new—written specifically for this project and its multi‑MCU architecture.

A big part of this update is that we **moved fully away from QMK**. Instead of adapting QMK to fit the hardware and features, we started from scratch with a firmware designed around our exact data paths and performance goals: **USB HID on the RP2040**, a clean **UART link** to the ESP32 for UI/display responsibilities, and **SPI** for high-throughput TFT updates. Dropping QMK gave us tighter control over timing, memory, and feature integration, and removed the friction of forcing a general-purpose keyboard firmware into a system that’s doing more than “just key scanning.”

The project is currently in a private repo. If you want to follow along or chat about it, my Discord is **hyupdo**.

---

### App Preview

<h5 align="center">Keys</h5>

![Keys Tab](/assets/app/tab_keys.jpg)

<h5 align="center">Lighting</h5>

![Lighting Tab](/assets/app/tab_lighting2.gif)

<h5 align="center">Performance</h5>

![Performance Tab](/assets/app/tab_performance.gif)


![live preview](/assets/app/livepreview.gif)


<h5 align="center" style="margin-bottom: 0;">Screen</h5>

<div align="center" style="font-style: italic; line-height: 1;">
  <small>This feature is only available on the Shego75</small>
</div>

![live preview](/assets/app/tab_screen.gif)

<div align="center" style="line-height: 2;">
  <small>Storage system using the SD ('Send to SD' yet to be implemented)</small>
</div>

![live preview](/assets/app/gifstoragepreview.gif)


### How to use:

#### Keys

Configure per-key actuation points, dead zones, and key assignments. The live visualization shows real-time magnetic sensor readings and key states. Customize individual key behavior with adjustable sensitivity ranges and enable anti-ghosting features. Export and import key profiles for quick configuration switching.

#### Lighting

In the **Lighting** tab, effects are driven using **SignalRGB**, which treats the keyboard like an addressable LED device and streams color data in real time. Once enabled, SignalRGB can take control of animations (audio-reactive, game integrations, ambient effects) and push them directly to the board so lighting stays perfectly in sync with the rest of your setup. You can still fine-tune how the device presents itself to SignalRGB—especially how LEDs are grouped and mapped—so the software knows what “keys” or regions it’s actually driving.

To change **zones**, use the zone controls to assign LEDs/keys into separate regions (for example: left half, right half, underglow, indicator cluster, or per-row groups). Each zone can be adjusted independently so you can apply different colors/effects per region, or scale brightness and effect intensity where you want it. After updating zone assignments, apply/save the configuration so the device reports the new layout and SignalRGB (or the app’s built-in effects) will target the updated zones correctly.

#### Performance

The **Performance** tab is a live “health dashboard” for the firmware and transport links, meant to answer one question quickly: *is the device keeping up in real time?* It aggregates timing, throughput, and workload counters so you can confirm that input scanning, lighting updates, and display/UI work are all completing within budget—especially when features are enabled simultaneously.

At a high level, the tab focuses on three areas:

1) **Frame / Loop Timing**
- Shows how long the main firmware loop (or “frame”) takes to run and how consistent it is over time.
- Look for **average**, **min/max**, and **spike** behavior. A stable system has tight variance; frequent spikes suggest something occasionally blocks (heavy rendering, long SPI transfers, or bursts of host traffic).
- If you see a “budget” or target interval, the goal is to keep the loop time comfortably below that target so there’s slack for bursts.

2) **Input Responsiveness**
- Tracks the cadence and workload of key scanning and event processing.
- Useful indicators include scan rate, event queue depth, and any dropped/late events.
- If responsiveness metrics degrade while other features are active, it’s a sign you should reduce background load (very high LED update rates, overly frequent display refresh, or excessive logging/telemetry).

3) **Transport + Subsystem Throughput (Host ⇄ RP2040 ⇄ ESP32 ⇄ TFT)**
- **USB/HID**: lets you see how often the host is sending commands and how quickly the device is responding. Excessive command rates can create overhead and jitter.
- **UART (RP2040 ⇄ ESP32)**: shows whether messages are flowing smoothly between MCUs. Watch for rising queue/backlog counters or retry/error counts—those often correlate with delayed UI updates.
- **SPI (ESP32 ⇄ TFT)**: large, frequent screen updates can dominate time. If you see display-related counters tied to spikes, consider lowering refresh frequency or reducing what’s being redrawn per update.

How to use it in practice:
- Start with everything “idle” and note the baseline loop time and stability.
- Enable one feature at a time (lighting effects, animations, screen updates, host polling) and watch what changes.
- If a metric provides “worst” or “spike” values, treat those as the most important—single long stalls are often more noticeable than a slightly higher average.
- Use the tab as a verification step after configuration changes: if you adjust lighting zones, increase effect complexity, or change display behavior, confirm the loop remains stable and queues don’t grow over time.

If the Performance tab includes error counters (CRC, framing, dropped packets, overflow), any value that steadily increases during normal use usually indicates a configuration or bandwidth issue worth addressing before it impacts responsiveness.

#### Screen

The screen is driven as a simple “remote display” in a two‑MCU chain: the **RP2040** acts as the USB/HID-facing coordinator, and the **ESP32** acts as the display/UI controller. The RP2040 collects system state (and any stats it owns), then sends compact status packets over **UART** to the ESP32. The ESP32 turns those packets into UI state (text, graphs, icons, warnings) and renders the result to the TFT over **SPI**.

**Communication path**
- **Host → RP2040 (USB HID):** the desktop app can request status, enable/disable telemetry, or change what the UI should show (page selection, refresh rate, debug toggles).
- **RP2040 → ESP32 (UART):** the RP2040 forwards the “what to display” information as structured messages (think: page id + key/value metrics + flags). UART is used because it’s reliable, low-latency, and keeps the display work off the RP2040’s timing-critical loop.
- **ESP32 → TFT (SPI):** the ESP32 updates the actual pixels. SPI is a high-throughput link commonly used by small TFTs, and it lets the ESP32 push either partial updates (only what changed) or full frames depending on the UI.

**How it shows “memory and stuff”**
What you see on the screen is ultimately just the ESP32 drawing primitives (text, lines, rectangles, images) based on numbers it receives. Those numbers can come from a few places:
- **ESP32-local stats:** free heap/PSRAM, task runtime, Wi‑Fi state, frame time for rendering, SPI throughput, queue depths. These are measured directly on the ESP32 and then displayed immediately.
- **RP2040 stats:** loop timing, scan workload, HID traffic counters, UART throughput/error counters. The RP2040 measures these and periodically ships them to the ESP32 so they appear in the same UI.
- **Derived/aggregated values:** rates and percentages (e.g., “updates per second”, “max spike”, “avg frame time”) computed from counters over a time window to make the data readable at a glance.

**Display model (why it can update smoothly)**
- The ESP32 keeps a small UI state model (current page, last-known metric values, thresholds, formatting).
- On each update tick, it compares new values to the previous ones and redraws only the regions that changed (e.g., a number field or a graph strip). This avoids pushing a full-screen refresh over SPI when it’s not needed.
- Graphs are typically rendered as rolling plots: the ESP32 stores a short history buffer (last N samples) and draws it as a line or bar chart, which makes “performance” data intuitive without flooding the link.

**Features the screen typically supports**
- **Pages / tabs:** status overview, lighting status, performance graphs, debug info, connectivity.
- **Live counters:** loop/frame time, HID packet rate, UART rate, dropped/overflow counts.
- **Memory readouts:** free heap/PSRAM (ESP32), and any reported RP2040 memory stats if exposed.
- **Warnings:** threshold-based indicators (e.g., queue growing, repeated UART errors, refresh budget exceeded).

In short: the RP2040 and ESP32 exchange lightweight telemetry over UART, and the ESP32 turns that telemetry into a responsive on-screen dashboard by drawing UI elements over SPI to the TFT.


### App Flow

#### The way it works:

![order of operations](/assets/app/flowchart.png)

##### Software
The desktop-side software is the user interface and configuration layer. It connects to the device over **USB HID**, provides a way to change settings (layouts, lighting, display options, etc.), and then sends those changes as compact commands to the firmware. Because it uses HID, it can talk to the device without needing a custom driver on most systems, and it can also be used for debugging/telemetry (reading back status, firmware info, and error codes).

##### RP2040
The RP2040 is the **USB-facing controller** and the central coordinator. It presents itself to the computer as a **HID device**, parses incoming configuration commands from the software, and applies settings that belong on the main MCU (key scanning, state management, timing, etc.). It also acts as a bridge to the secondary MCU over **UART**, forwarding display/network-related commands and receiving status updates so the host can see what the rest of the system is doing.

##### ESP32-PICO-D4
The ESP32-PICO-D4 is responsible for the **high-level peripheral side**, primarily driving the display. It receives messages from the RP2040 over **UART**, interprets them as UI/state updates, and renders to the TFT over **SPI**. It can also be used for features that fit better on the ESP32 (background tasks, connectivity, asset handling), while keeping the RP2040 focused on deterministic USB + input timing.


### Test for yourself
I have put the app up online if you want to browse through, WebHID isn't implemented properly since I primarily focused on USB HID but do check out the themes in the Settings tab!

Link: [nova-8tt.pages.dev](https://nova-8tt.pages.dev)

