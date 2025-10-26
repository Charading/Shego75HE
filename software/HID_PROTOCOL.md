# HID Protocol Documentation

## Overview
This document describes the HID communication protocol for sending GIF files from the Electron app to your keyboard, which forwards them to the ESP32.

## Packet Structure
Each HID report is 64 bytes (configurable in `config.js`):

```
Byte 0:    Report ID (typically 0x00)
Byte 1:    Command byte
Byte 2:    Destination flag
Bytes 3+:  Command-specific data
```

## Commands

### 1. Start GIF Transfer (0x01)
Initiates a GIF transfer session.

**Packet Layout:**
```
[0x00] Report ID
[0x01] Command: Start GIF
[0x01/0x02] Destination: 0x01=Screen, 0x02=SD Card
[4 bytes] Total file size (little-endian uint32)
[2 bytes] Total chunks (little-endian uint16)
[remaining] Padding
```

### 2. GIF Data Chunk (0x02)
Sends a chunk of GIF data.

**Packet Layout:**
```
[0x00] Report ID
[0x02] Command: GIF Data
[0x01/0x02] Destination flag
[2 bytes] Chunk index (little-endian uint16)
[1 byte] Chunk size (0-59 bytes)
[59 bytes] Chunk data
```

### 3. End GIF Transfer (0x03)
Signals completion of GIF transfer.

**Packet Layout:**
```
[0x00] Report ID
[0x03] Command: End GIF
[0x01/0x02] Destination flag
[remaining] Padding
```

## Destination Flags
- `0x01`: Send to Screen (ESP32 displays immediately)
- `0x02`: Save to SD Card (ESP32 stores on SD card)

## Timing
- **Start delay**: 50ms after sending start packet
- **Packet delay**: 10ms between data chunks
- These can be adjusted in `config.js`

## Keyboard Implementation

Your keyboard firmware should:

1. **Receive HID reports** via USB
2. **Parse the command byte** to determine action
3. **Forward to ESP32** via UART/SPI/I2C with the destination flag
4. **Handle acknowledgments** (optional, for reliability)

### Example Keyboard Code (Pseudocode)
```c
void handle_hid_report(uint8_t* data, uint16_t len) {
    uint8_t cmd = data[1];
    uint8_t dest = data[2];
    
    switch(cmd) {
        case 0x01: // Start GIF
            uint32_t size = *(uint32_t*)&data[3];
            uint16_t chunks = *(uint16_t*)&data[7];
            esp32_start_gif_transfer(size, chunks, dest);
            break;
            
        case 0x02: // Data chunk
            uint16_t index = *(uint16_t*)&data[3];
            uint8_t chunk_size = data[5];
            esp32_send_gif_chunk(index, &data[6], chunk_size, dest);
            break;
            
        case 0x03: // End
            esp32_end_gif_transfer(dest);
            break;
    }
}
```

## Setup Instructions

1. **Install node-hid**:
   ```bash
   npm install node-hid
   ```

2. **Find your keyboard's VID/PID**:
   ```bash
   node find-keyboard.js
   ```

3. **Update config.js** with your keyboard's VID/PID

4. **Test the connection** by running the app and sending a small GIF

## Troubleshooting

### "Keyboard not found"
- Check VID/PID in `config.js`
- Run `find-keyboard.js` to verify device is detected
- Ensure keyboard is connected and recognized by Windows

### Transfer fails or is slow
- Adjust timing in `config.js`
- Check HID report size matches your keyboard
- Verify keyboard firmware is processing packets

### ESP32 doesn't receive data
- Check UART/SPI/I2C connection between keyboard and ESP32
- Verify baud rate and protocol
- Add debug logging in keyboard firmware
