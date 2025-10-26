# Keyboard Configurator - Electron App

A desktop application for configuring your custom keyboard, including GIF uploads via HID and per-key actuation threshold adjustment.

## Features

### 1. GIF Upload Tab
- Select GIF files from your computer
- Preview selected GIF
- Send GIF to two destinations via HID:
  - **Screen**: Display directly on ESP32 screen
  - **SD Card**: Save to SD card for later use

### 2. Actuation Settings Tab
- Interactive 75% keyboard layout
- Click any key to adjust its actuation threshold (0.1mm - 4.0mm)
- Visual feedback for modified keys
- Real-time updates sent to keyboard

## Installation

1. Install dependencies:
```bash
npm install
```

2. Install HID library (required for keyboard communication):
```bash
npm install node-hid
```

## Setup

### Find Your Keyboard's VID/PID

Run the helper script to find your keyboard:
```bash
node find-keyboard.js
```

This will list all HID devices. Find your keyboard and note the Vendor ID and Product ID.

### Configure Your Keyboard

Edit `config.js` and update with your keyboard's information:
```javascript
KEYBOARD_VID: 0x1234, // Your keyboard's Vendor ID
KEYBOARD_PID: 0x5678, // Your keyboard's Product ID
```

## Running the App

Development mode (with DevTools and auto-reload):
```bash
npm run dev
```

Production mode:
```bash
npm start
```

## HID Communication Protocol

The app sends GIF data over HID to your keyboard, which forwards it to the ESP32. The protocol includes:

- **Destination Flag**: Indicates whether to send to screen (0x01) or SD card (0x02)
- **Chunked Transfer**: Large GIFs are split into 64-byte packets
- **Commands**: Start (0x01), Data (0x02), End (0x03)

See `HID_PROTOCOL.md` for detailed protocol documentation.

## Project Structure

```
software/
├── main.js              # Electron main process with HID communication
├── preload.js           # Secure bridge between main and renderer
├── index.html           # Main UI structure
├── renderer.js          # UI logic and interactions
├── styles.css           # Styling
├── config.js            # HID configuration (VID/PID, timing)
├── find-keyboard.js     # Helper to find keyboard VID/PID
├── HID_PROTOCOL.md      # Protocol documentation
├── package.json         # Dependencies and scripts
└── README.md            # This file
```

## Customization

### Keyboard Layout
Modify the `keyLayout` array in `renderer.js` to match your keyboard's physical layout.

### Actuation Range
Adjust the slider range in `index.html`:
```html
<input type="range" id="threshold-slider" min="0.1" max="4.0" step="0.1">
```

### Communication Protocol
Implement your keyboard's specific protocol in the IPC handlers in `main.js`:
- `send-gif`: For GIF data transfer
- `update-actuation`: For actuation threshold updates

## Building for Distribution

To create distributable packages, you can add electron-builder:

```bash
npm install --save-dev electron-builder
```

Then add build scripts to `package.json`.

## License

MIT
