# Keyboard Configurator - Electron App

A desktop application for configuring your custom keyboard, including GIF uploads and per-key actuation threshold adjustment.

## Features

### 1. GIF Upload Tab
- Select GIF files from your computer
- Preview selected GIF
- Send GIF to three destinations:
  - **Screen**: Display directly on keyboard screen
  - **SPIFFS**: Store in flash memory
  - **SD Card**: Store on SD card

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

## Running the App

Development mode (with DevTools):
```bash
npm run dev
```

Production mode:
```bash
npm start
```

## Project Structure

```
software/
├── main.js          # Electron main process
├── preload.js       # Secure bridge between main and renderer
├── index.html       # Main UI structure
├── renderer.js      # UI logic and interactions
├── styles.css       # Styling
├── package.json     # Dependencies and scripts
└── README.md        # This file
```

## Serial Communication

The app includes placeholder code for serial communication with your keyboard. To implement:

1. The `serialport` library is already in dependencies
2. Uncomment the serial communication code in `main.js`
3. Configure the correct COM port for your keyboard
4. Implement your keyboard's communication protocol

### Example Serial Implementation

```javascript
const { SerialPort } = require('serialport');

// Open port
const port = new SerialPort({ 
  path: 'COM3',  // Adjust to your keyboard's port
  baudRate: 115200 
});

// Send data
port.write(dataBuffer);
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
