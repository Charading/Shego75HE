# Vendor Bulk Endpoint Setup Guide

Your Electron app now supports high-speed vendor bulk endpoints with automatic fallback to raw HID.

## 🚀 Quick Start

### 1. Install Dependencies
```bash
cd software
npm install usb
```

### 2. Build & Flash Firmware  
The firmware is already configured with `VENDOR_ENABLE`. Just rebuild and flash:
```bash
qmk compile -kb shego75_v1 -km default
# Flash the resulting firmware to your keyboard
```

### 3. Test Vendor Communication
```bash
cd software
node test-vendor.js
```

## 🔧 How It Works

### Firmware Changes
- **`VENDOR_ENABLE`** - Enables vendor interface in USB descriptors  
- **Vendor Interface** - USB class 0xFF bulk endpoints (64-byte packets)
- **`vendor_task()`** - Polls vendor OUT endpoint, called from `matrix_scan_user()`
- **Shared Processing** - Both vendor and HID use `hid_process_received_buffer()`

### App Changes
- **Primary**: Tries vendor bulk endpoints first (faster, larger packets)
- **Fallback**: Uses raw HID if vendor fails (compatibility)
- **Auto-detection**: Finds vendor interface (class 0xFF) automatically
- **Bigger chunks**: 61-byte vs 29-byte payloads (vendor vs HID)

### Transport Comparison
| Method | Chunk Size | Speed | Driver Req (Win) | Compatibility |
|--------|------------|-------|------------------|---------------|
| Vendor Bulk | 61 bytes | Fast | WinUSB (Zadig) | Modern |
| Raw HID | 29 bytes | Slower | None | Universal |

## 🪟 Windows Setup (Driver Binding)

On Windows, vendor endpoints need a libusb-compatible driver:

### Using Zadig (Recommended)
1. Download [Zadig](https://zadig.akeo.ie/)
2. **With keyboard connected**, run Zadig as Administrator
3. **Options → List All Devices** ✓
4. Find your keyboard (VID: 0xDEAD, PID: 0xC0DE)
5. Look for the **vendor interface** (not HID interface!)
6. Select **WinUSB** driver → **Replace Driver**

### Verification
- Device Manager should show your keyboard with both HID and USB (WinUSB) entries
- `node test-vendor.js` should find vendor interface and send test packets

## 🐛 Troubleshooting

### "Vendor interface not found"
- ✅ Firmware built with `VENDOR_ENABLE` defined
- ✅ Keyboard flashed with new firmware  
- ✅ Device enumerated properly (check Device Manager)

### "Failed to claim vendor interface" (Windows)
- ✅ Use Zadig to bind vendor interface to WinUSB
- ✅ Run Node.js as Administrator if needed

### "USB library not available"
- ✅ Run `npm install usb` in software folder
- ✅ May need build tools on Windows (Visual Studio Build Tools)

### Transfer Issues
- ✅ Check ESP32 debug output for I2C messages
- ✅ Verify `vendor_task()` is called in `matrix_scan_user()`
- ✅ Check firmware console for HID debug messages

## 📊 Performance

Expected transfer speeds for a 100KB GIF:
- **Vendor**: ~2-3 seconds (61-byte chunks @ 10ms delay)
- **HID**: ~4-5 seconds (29-byte chunks @ 10ms delay)

## 🔄 Fallback Behavior

The app automatically tries vendor first, then HID:

```javascript
// Electron automatically tries:
1. Open vendor interface (class 0xFF)
2. If vendor fails → open HID interface  
3. Use appropriate chunk size for chosen method
4. Send same protocol packets (START/DATA/END)
```

Both paths forward to the same `hid_process_received_buffer()` on the device.

## 🧪 Testing

```bash
# Test vendor communication only
node test-vendor.js

# Test with actual Electron app  
npm run dev
# Use "Send GIF" - check console for "Using vendor bulk endpoints"
```

## 📁 Files Modified

### Firmware
- `config.h` - Added `#define VENDOR_ENABLE`  
- `usb_descriptor.h/.c` - Added vendor interface descriptors
- `vendor_bridge.h/.c` - Vendor endpoint polling and packet sending
- `hid_reports.c` - Refactored into shared processor
- `keymap.c` - Added `vendor_task()` call in `matrix_scan_user()`

### Software  
- `main.js` - Added vendor support with HID fallback
- `package.json` - Added `usb` dependency
- `test-vendor.js` - Vendor endpoint test script
