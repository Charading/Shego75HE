# Troubleshooting Guide: GIF Transfer Not Working

## Current Status (Based on Your Screenshot)

✅ **What's Working:**
- Electron app sends data to keyboard
- HID communication is established
- Transfer completes without errors

❌ **What's Not Working:**
- You're seeing echoes of sent packets logged as "Received from keyboard"
- You don't know if ESP32 is actually receiving/displaying the GIF
- No clear feedback from the firmware

## The Problem

The console shows packets like:
```
Received from keyboard: 0x11 0x04 0x01 0x0a 0x04 0x00 0x88 0x46 ...
```

That `0x11` is `CMD_GIF_DATA` - meaning Windows is echoing back the DATA packets you sent. This is normal behavior, but they should be suppressed.

## Step-by-Step Diagnosis

### 1. Check if Echoes are Being Suppressed

**Replace your current main.js with the improved version**:
```bash
cp /mnt/user-data/outputs/main.js /path/to/your/project/main.js
```

The new version:
- ✅ Better echo suppression using packet signatures
- ✅ Clearer logging (echoes are silent, real messages are shown)
- ✅ Status message detection
- ✅ Better error messages

### 2. Verify Keyboard Firmware is Receiving

**Add debug output to your keyboard firmware** (in `raw_hid_receive_user()`):

```c
void raw_hid_receive_user(uint8_t *data, uint8_t length) {
    // Add this at the very start:
    printf("[HID] Received %d bytes, CMD=0x%02X\n", length, data[0]);
    
    // Your existing code...
}
```

**What to expect:**
- You should see: `[HID] Received 32 bytes, CMD=0x10` (START)
- Then many: `[HID] Received 32 bytes, CMD=0x11` (DATA)
- Finally: `[HID] Received 32 bytes, CMD=0x12` (END)

**If you don't see these messages:**
- ❌ HID data is not reaching your firmware
- Check: USB cable, keyboard enumeration, QMK configuration

### 3. Check ESP32 Communication

**Add debug to your I2C/UART forwarding code**:

```c
// In the function that sends to ESP32:
printf("[I2C] Forwarding CMD=0x%02X, %d bytes to ESP32\n", data[0], length);

// After sending:
printf("[I2C] ESP32 acknowledged: %s\n", ack ? "YES" : "NO");
```

**What to expect:**
```
[I2C] Forwarding CMD=0x10, 32 bytes to ESP32
[I2C] ESP32 acknowledged: YES
[I2C] Forwarding CMD=0x11, 32 bytes to ESP32
[I2C] ESP32 acknowledged: YES
...
```

**If ESP32 doesn't acknowledge:**
- ❌ I2C/UART connection problem
- Check: wiring, pull-ups, baud rate, ESP32 is running

### 4. Verify ESP32 is Processing Data

**Add debug to ESP32 firmware**:

```c
// In your GIF receive function:
Serial.printf("[ESP] Received CMD=0x%02X, size=%d\n", cmd, dataSize);

switch(cmd) {
    case 0x10: // START
        Serial.printf("[ESP] Starting GIF transfer, size=%d, dest=%d\n", totalSize, destination);
        break;
    case 0x11: // DATA
        Serial.printf("[ESP] Chunk %d received\n", chunkIndex);
        break;
    case 0x12: // END
        Serial.printf("[ESP] Transfer complete, displaying GIF\n");
        break;
}
```

**What to expect:**
```
[ESP] Received CMD=0x10, size=13967
[ESP] Starting GIF transfer, size=13967, dest=1
[ESP] Chunk 0 received
[ESP] Chunk 1 received
...
[ESP] Chunk 481 received
[ESP] Received CMD=0x12, size=1
[ESP] Transfer complete, displaying GIF
```

**If ESP32 gets corrupted data:**
- ❌ Data corruption during I2C/UART transfer
- Try: slower I2C speed, shorter wires, better power supply

### 5. Test with Minimal Example

**Create a test script to send just ONE packet**:

```javascript
// test-single-packet.js
const HID = require('node-hid');
const config = require('./config');

const devices = HID.devices();
const keyboard = devices.find(d => 
  d.vendorId === config.KEYBOARD_VID && 
  d.productId === config.KEYBOARD_PID && 
  d.interface === 1
);

if (!keyboard) {
  console.error('Keyboard not found!');
  process.exit(1);
}

const hid = new HID.HID(keyboard.path);

// Listen for responses
hid.on('data', (data) => {
  console.log('Received:', Array.from(data).map(b => '0x' + b.toString(16).padStart(2, '0')).join(' '));
});

// Send a single START packet
const packet = Buffer.alloc(33);
packet[0] = 0x00; // Report ID
packet[1] = config.CMD_START_GIF; // 0x10
packet.writeUInt16LE(100, 2); // Small size
packet[4] = config.DEST_SCREEN; // 0x01

console.log('Sending:', Array.from(packet).slice(0, 10).map(b => '0x' + b.toString(16).padStart(2, '0')).join(' '), '...');
hid.write(Array.from(packet));

setTimeout(() => {
  console.log('Done. Check keyboard/ESP32 debug output.');
  hid.close();
}, 1000);
```

Run it and check debug output on keyboard AND ESP32.

## Common Issues and Solutions

### Issue 1: "Received from keyboard" shows echoes
**Solution:** Use the improved main.js provided above

### Issue 2: No debug output from keyboard firmware
**Solution:** 
- Check USB serial monitor is connected at correct baud rate
- Enable `CONSOLE_ENABLE = yes` in QMK rules.mk
- Add `printf()` calls as shown above

### Issue 3: Keyboard receives data but ESP32 doesn't
**Solution:**
- Check I2C/UART wiring
- Verify ESP32 is powered and running
- Test I2C with a simple ping/pong before transferring GIFs
- Try slower I2C clock speed (100kHz instead of 400kHz)

### Issue 4: ESP32 receives data but doesn't display GIF
**Solution:**
- Check GIF decoding library is working
- Test with a tiny GIF (e.g., 64x64, 2 frames)
- Verify screen initialization
- Check memory allocation (large GIFs need lots of RAM)

### Issue 5: Transfer works sometimes, fails other times
**Solution:**
- Increase `PACKET_DELAY_MS` in config.js (try 20ms or 50ms)
- Add handshaking (wait for ACK after each chunk)
- Check USB cable quality
- Try different USB port

## Expected Console Output (After Fix)

With the improved main.js, you should see:

```
============================================================
📤 Starting GIF transfer to Screen
File: example.gif (13967 bytes)
============================================================
📊 Transfer details:
   Destination: Screen (flag: 0x1)
   Chunk size: 29 bytes
   Total chunks: 482
   Estimated time: ~4.8s

📤 Sending START command...
📤 Sending data chunks...
   Progress: 10% (50/482 chunks)
   Progress: 20% (100/482 chunks)
   ...
   Progress: 100% (482/482 chunks)

📤 Sending END command...
============================================================
✅ Transfer Complete
============================================================

⏳ Waiting for firmware response...
   (If you see no status messages, check firmware debug output)
```

**If your firmware sends status messages**, you'll also see:
```
✅ Status from keyboard: Transfer started (0x3)
✅ Status from keyboard: Chunk received (0x2)
✅ Status from keyboard: Transfer complete (0x4)
```

## Next Steps

1. **Install the improved main.js**
2. **Add debug output to keyboard firmware**
3. **Add debug output to ESP32 firmware**
4. **Run the app and compare output**
5. **Share the keyboard AND ESP32 debug logs** if still not working

## Quick Test Command

```bash
# Run with debug enabled
HID_DEBUG=1 npm run dev
```

This will show detailed packet information.