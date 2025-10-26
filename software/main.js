const { app, BrowserWindow, ipcMain, dialog } = require('electron');
const path = require('path');
const fs = require('fs');
const config = require('./config');

// Enable hot reload in development
try {
  require('electron-reload')(__dirname, {
    electron: require(`${__dirname}/node_modules/electron`),
    ignored: /node_modules|[\/\\]\./
  });
} catch (_) {}

let mainWindow;
let HID;

// Try to load node-hid
try {
  HID = require('node-hid');
} catch (err) {
  console.log('node-hid not available, HID features will be disabled');
}

// HID Configuration
const KEYBOARD_VID = config.KEYBOARD_VID;
const KEYBOARD_PID = config.KEYBOARD_PID;
const REPORT_SIZE = config.REPORT_SIZE;

let hidDevice = null;

function createWindow() {
  mainWindow = new BrowserWindow({
    width: 1200,
    height: 800,
    webPreferences: {
      preload: path.join(__dirname, 'preload.js'),
      contextIsolation: true,
      nodeIntegration: false
    }
  });

  mainWindow.loadFile('index.html');
  
  // Open DevTools in development
  if (process.argv.includes('--dev')) {
    mainWindow.webContents.openDevTools();
  }
}

app.whenReady().then(() => {
  createWindow();

  app.on('activate', () => {
    if (BrowserWindow.getAllWindows().length === 0) {
      createWindow();
    }
  });
});

app.on('window-all-closed', () => {
  if (process.platform !== 'darwin') {
    app.quit();
  }
});

// Handle file selection
ipcMain.handle('select-gif', async () => {
  const result = await dialog.showOpenDialog(mainWindow, {
    properties: ['openFile'],
    filters: [
      { name: 'GIF Images', extensions: ['gif'] }
    ]
  });

  if (!result.canceled && result.filePaths.length > 0) {
    const filePath = result.filePaths[0];
    const fileBuffer = fs.readFileSync(filePath);
    return {
      path: filePath,
      name: path.basename(filePath),
      data: Array.from(fileBuffer),
      size: fileBuffer.length
    };
  }
  return null;
});

// Handle sending GIF to different destinations
ipcMain.handle('send-gif', async (event, { destination, gifData }) => {
  console.log(`Sending GIF to ${destination}`, gifData.name, `Size: ${gifData.size} bytes`);
  
  try {
    if (!HID) {
      throw new Error('HID library not available. Please install node-hid: npm install node-hid');
    }

    // Find and connect to keyboard
    if (!hidDevice) {
      const devices = HID.devices();
      const keyboard = devices.find(d => d.vendorId === KEYBOARD_VID && d.productId === KEYBOARD_PID);
      
      if (!keyboard) {
        throw new Error('Keyboard not found. Check VID/PID configuration.');
      }
      
      hidDevice = new HID.HID(keyboard.path);
    }

    // Determine destination flag
    const destinationFlag = destination === 'Screen' ? config.DEST_SCREEN : config.DEST_SD_CARD;
    
    // Send GIF data over HID in chunks
    const gifBuffer = Buffer.from(gifData.data);
    const chunkSize = REPORT_SIZE - 5; // Reserve bytes for: [ReportID, Command, Flag, ChunkIndex, ChunkSize]
    const totalChunks = Math.ceil(gifBuffer.length / chunkSize);
    
    // Send start command
    const startPacket = Buffer.alloc(REPORT_SIZE);
    startPacket[0] = 0x00; // Report ID (adjust as needed)
    startPacket[1] = config.CMD_START_GIF;
    startPacket[2] = destinationFlag;
    startPacket.writeUInt32LE(gifData.size, 3); // Total size
    startPacket.writeUInt16LE(totalChunks, 7); // Total chunks
    
    hidDevice.write(Array.from(startPacket));
    await sleep(config.START_DELAY_MS);
    
    // Send GIF data in chunks
    for (let i = 0; i < totalChunks; i++) {
      const start = i * chunkSize;
      const end = Math.min(start + chunkSize, gifBuffer.length);
      const chunk = gifBuffer.slice(start, end);
      
      const packet = Buffer.alloc(REPORT_SIZE);
      packet[0] = 0x00; // Report ID
      packet[1] = config.CMD_GIF_DATA;
      packet[2] = destinationFlag;
      packet.writeUInt16LE(i, 3); // Chunk index
      packet.writeUInt8(chunk.length, 5); // Chunk size
      chunk.copy(packet, 6); // Copy chunk data
      
      hidDevice.write(Array.from(packet));
      await sleep(config.PACKET_DELAY_MS);
      
      // Update progress (optional)
      const progress = Math.round((i + 1) / totalChunks * 100);
      console.log(`Progress: ${progress}%`);
    }
    
    // Send end command
    const endPacket = Buffer.alloc(REPORT_SIZE);
    endPacket[0] = 0x00;
    endPacket[1] = config.CMD_END_GIF;
    endPacket[2] = destinationFlag;
    
    hidDevice.write(Array.from(endPacket));
    
    return { success: true, message: `GIF sent to ${destination}` };
  } catch (error) {
    console.error('HID Error:', error);
    
    // Close device on error
    if (hidDevice) {
      try {
        hidDevice.close();
      } catch (_) {}
      hidDevice = null;
    }
    
    return { success: false, message: error.message };
  }
});

function sleep(ms) {
  return new Promise(resolve => setTimeout(resolve, ms));
}

// Handle actuation threshold updates
ipcMain.handle('update-actuation', async (event, { keyId, threshold }) => {
  console.log(`Updating key ${keyId} actuation to ${threshold}mm`);
  
  // TODO: Implement actual serial communication to keyboard
  
  try {
    // Placeholder for actual implementation
    await new Promise(resolve => setTimeout(resolve, 100));
    
    return { success: true };
  } catch (error) {
    return { success: false, message: error.message };
  }
});

// Get available serial ports
ipcMain.handle('get-ports', async () => {
  try {
    if (!HID) {
      return [];
    }
    
    const devices = HID.devices();
    return devices.map(d => ({
      vendorId: d.vendorId,
      productId: d.productId,
      manufacturer: d.manufacturer,
      product: d.product,
      path: d.path
    }));
  } catch (error) {
    console.error('Error listing HID devices:', error);
    return [];
  }
});

// Clean up HID device on app quit
app.on('will-quit', () => {
  if (hidDevice) {
    try {
      hidDevice.close();
    } catch (_) {}
  }
});
