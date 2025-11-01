# Windows HID Report ID Fix

## The Problem

When sending HID data from Electron to the keyboard, **Windows was stripping the first byte** before passing it to the QMK firmware's `raw_hid_receive_user()` callback.

### What Was Happening

```
Electron sent:      [0x10][0x8f][0x36][0x01][...]
                     ^^^^  ^^^^  ^^^^  ^^^^
                     CMD   SIZE  SIZE  DEST

Windows stripped:   [0x10] ← Interpreted as Report ID, removed!

Firmware received:        [0x8f][0x36][0x01][...]
                          ^^^^  ^^^^  ^^^^
                          ???   ???   ???
```

The keyboard firmware was seeing garbage because the command byte was missing!

## The Solution

**Always prefix packets with `0x00` as the Report ID:**

```
Electron sends:     [0x00][0x10][0x8f][0x36][0x01][...]
                     ^^^^  ^^^^  ^^^^  ^^^^  ^^^^
                     RPT   CMD   SIZE  SIZE  DEST
                     ID

Windows strips:     [0x00] ← Report ID removed
                     ^^^^

Firmware receives:        [0x10][0x8f][0x36][0x01][...]
                          ^^^^  ^^^^  ^^^^  ^^^^
                          CMD   SIZE  SIZE  DEST  ✓
```

## Implementation

### Before (Broken)
```javascript
const packet = Buffer.alloc(64);
packet[0] = 0x10;  // CMD_START_GIF
packet[1] = size & 0xFF;
packet[2] = (size >> 8) & 0xFF;
packet[3] = destination;
device.write(Array.from(packet));
```

### After (Fixed)
```javascript
const packet = Buffer.alloc(65);  // +1 for Report ID
packet[0] = 0x00;  // Report ID (required by Windows HID)
packet[1] = 0x10;  // CMD_START_GIF
packet[2] = size & 0xFF;
packet[3] = (size >> 8) & 0xFF;
packet[4] = destination;
device.write(Array.from(packet));
```

## Why This Happens

From the Windows HID documentation:

> When an application calls WriteFile to send a HID report to a device, the first byte of the buffer must contain the report ID. If the device doesn't use report IDs, set this to 0x00.

The `node-hid` library on Windows follows this convention, so **every packet must start with a Report ID byte** (typically `0x00` for raw HID).

## What Changed

All three packet types now include the `0x00` prefix:

1. **START packet**: `[0x00][0x10][size_low][size_high][dest][...]`
2. **DATA packet**: `[0x00][0x11][...63 bytes of GIF data...]`
3. **END packet**: `[0x00][0x12][dest][...]`

## Testing

After this fix, you should see debug messages on the keyboard's UART showing:

```
[HID] raw_hid_receive_user called!
[HID] Command: 0x10 (START_GIF)
[HID] Size: 13967 bytes
[HID] Destination: 0x01 (Screen)
[I2C] Forwarding to ESP32...
```

## References

- [Windows HID Concepts](https://docs.microsoft.com/en-us/windows-hardware/drivers/hid/introduction-to-hid-concepts)
- [node-hid Documentation](https://github.com/node-hid/node-hid#usage)
- [QMK Raw HID Documentation](https://docs.qmk.fm/#/feature_rawhid)
