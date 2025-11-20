const { app, BrowserWindow, ipcMain, dialog } = require('electron');
const path = require('path');
const fs = require('fs');
const { execFile } = require('child_process');
const { promisify } = require('util');
const config = require('./config');

const execFileAsync = promisify(execFile);
const PYTHON_EXE = process.env.PYTHON || 'python';
const PY_RGB_TOGGLE = path.join(__dirname, 'py_settings', 'rgb_toggle.py');

// Enable hot reload in development
try {
  require('electron-reload')(__dirname, {
    electron: require(`${__dirname}/node_modules/electron`),
    ignored: /node_modules|[\/\\]\./
  });
} catch (_) {}

let mainWindow;
let HID;
let USB;

// Try to load node-hid
try {
  HID = require('node-hid');
} catch (err) {
  console.log('node-hid not available, HID features will be disabled');
}

// Optional: enumerate HID devices for debugging interface selection
if (HID && process.env.HID_ENUM) {
  try {
    const devs = HID.devices();
    console.log('----- HID Device Enumeration -----');
    devs.forEach(d => {
      console.log(`[IF:${d.interface||0}] VID=0x${(d.vendorId||0).toString(16).padStart(4,'0')} PID=0x${(d.productId||0).toString(16).padStart(4,'0')} usagePage=${d.usagePage||''} usage=${d.usage||''} path=${d.path}`);
    });
    console.log('----------------------------------');
  } catch (e) {
    console.log('Failed to enumerate HID devices:', e.message);
  }
}

// Try to load usb (for vendor bulk endpoints)
try {
  USB = require('usb');
} catch (err) {
  console.log('usb library not available, vendor endpoints will be disabled');
}

// HID Configuration
const KEYBOARD_VID = config.KEYBOARD_VID;
const KEYBOARD_PID = config.KEYBOARD_PID;
const REPORT_SIZE = config.REPORT_SIZE;

let hidDevice = null;
let usbDevice = null;
let vendorInterface = null;
let vendorOutEndpoint = null;
let vendorInEndpoint = null;

// Enhanced debug flag
const HID_DEBUG = !!process.env.HID_DEBUG;

// Simple echo tracking with better matching
const sentPacketHistory = [];
const HISTORY_SIZE = 50;
const ECHO_WINDOW_MS = 3000;

function sendHIDPacket(device, reportId, data) {
  const buffer = Buffer.alloc(REPORT_SIZE + 1);
  buffer[0] = 0x00; // Report ID for Windows
  buffer[1] = reportId;
  data.copy(buffer, 2, 0, Math.min(data.length, REPORT_SIZE - 1));

  const packetToSend = Array.from(buffer);
  
  // Store ONLY the first 8 bytes of payload for echo matching
  // (command + a bit of data is enough to identify echoes)
  const signature = buffer.slice(1, 9); // Skip report ID, take next 8 bytes
  sentPacketHistory.push({ 
    signature: Array.from(signature), 
    ts: Date.now(),
    fullPacket: Array.from(buffer.slice(1)) // For debugging
  });
  
  if (sentPacketHistory.length > HISTORY_SIZE) {
    sentPacketHistory.shift();
  }

  if (HID_DEBUG) {
    console.log('[DEBUG] Sending HID packet:', {
      reportId: '0x' + reportId.toString(16),
      signature: signature.toString('hex')
    });
  }

  try {
    device.write(packetToSend);
    return packetToSend;
  } catch (error) {
    console.error('❌ Error writing to HID device:', error);
    sentPacketHistory.pop();
    return null;
  }
}

function sendVendorPacket(reportId, data) {
  if (!vendorOutEndpoint) {
    throw new Error('Vendor endpoint not available');
  }

  const buffer = Buffer.alloc(64); // Standard vendor bulk size
  buffer[0] = reportId;
  data.copy(buffer, 1, 0, Math.min(data.length, 63));

  if (HID_DEBUG) {
    console.log('[DEBUG] Sending vendor packet:', {
      reportId: '0x' + reportId.toString(16),
      length: data.length + 1
    });
  }

  return new Promise((resolve, reject) => {
    vendorOutEndpoint.transfer(buffer, (error) => {
      if (error) {
        console.error('❌ Error writing to vendor endpoint:', error);
        reject(error);
      } else {
        resolve();
      }
    });
  });
}

function openVendorDevice() {
  if (!USB) {
    throw new Error('USB library not available');
  }

  const device = USB.findByIds(KEYBOARD_VID, KEYBOARD_PID);
  if (!device) {
    throw new Error('USB device not found');
  }

  try {
    device.open();
  } catch (error) {
    throw new Error(`Failed to open USB device: ${error.message}`);
  }

  // Look for vendor interface (class 0xFF)
  let vendorIface = null;
  for (let i = 0; i < device.interfaces.length; i++) {
    const iface = device.interfaces[i];
    if (iface.descriptor.bInterfaceClass === 0xFF) {
      vendorIface = iface;
      break;
    }
  }

  if (!vendorIface) {
    device.close();
    throw new Error('Vendor interface not found. Make sure VENDOR_ENABLE is defined in firmware.');
  }

  try {
    // Detach kernel driver if necessary (Linux/macOS)
    if (vendorIface.isKernelDriverActive && vendorIface.isKernelDriverActive()) {
      vendorIface.detachKernelDriver();
    }
    vendorIface.claim();
  } catch (error) {
    device.close();
    throw new Error(`Failed to claim vendor interface: ${error.message}`);
  }

  // Find endpoints
  let outEp = null;
  let inEp = null;
  for (const endpoint of vendorIface.endpoints) {
    if (endpoint.direction === 'out' && endpoint.transferType === USB.LIBUSB_TRANSFER_TYPE_BULK) {
      outEp = endpoint;
    } else if (endpoint.direction === 'in' && endpoint.transferType === USB.LIBUSB_TRANSFER_TYPE_BULK) {
      inEp = endpoint;
    }
  }

  if (!outEp) {
    vendorIface.release();
    device.close();
    throw new Error('Vendor OUT endpoint not found');
  }

  console.log('✅ Vendor interface opened successfully');
  
  usbDevice = device;
  vendorInterface = vendorIface;
  vendorOutEndpoint = outEp;
  vendorInEndpoint = inEp;

  // Setup IN endpoint listener for responses if available
  if (inEp) {
    inEp.startPoll(1, 64);
    inEp.on('data', (data) => {
      const rawArr = Array.from(data);
      if (rawArr.length === 0) return;

      const cmdByte = rawArr[0];
      
      // Check if it's a status message
      if (cmdByte === config.HID_REPORT_ID_STATUS) {
        const statusCode = rawArr.length > 1 ? rawArr[1] : 0;
        let statusMsg = 'Unknown';
        
        if (statusCode === config.STATUS_OK) statusMsg = 'OK';
        else if (statusCode === config.STATUS_CHUNK_RECEIVED) statusMsg = 'Chunk received';
        else if (statusCode === config.STATUS_TRANSFER_STARTED) statusMsg = 'Transfer started';
        else if (statusCode === config.STATUS_TRANSFER_COMPLETE) statusMsg = 'Transfer complete';
        else if (statusCode === config.STATUS_ERROR_INVALID) statusMsg = 'Error: Invalid';
        
        console.log(`✅ Status from keyboard (vendor): ${statusMsg} (0x${statusCode.toString(16)})`);
        mainWindow.webContents.send('hid-status', { code: statusCode, message: statusMsg });
        return;
      }

      console.log(`📨 Message from keyboard (vendor, cmd: 0x${cmdByte.toString(16)}):`, 
        rawArr.slice(0, 16).map(b => '0x' + b.toString(16).padStart(2, '0')).join(' '),
        rawArr.length > 16 ? '...' : ''
      );
      
      mainWindow.webContents.send('hid-data', Buffer.from(rawArr));
    });
    
    inEp.on('error', (err) => {
      console.error('❌ Vendor IN endpoint error:', err);
    });
  }

  return { device, interface: vendorIface, outEndpoint: outEp, inEndpoint: inEp };
}

function closeVendorDevice() {
  if (vendorInEndpoint) {
    try { vendorInEndpoint.stopPoll(); } catch (_) {}
  }
  if (vendorInterface) {
    try { vendorInterface.release(); } catch (_) {}
  }
  if (usbDevice) {
    try { usbDevice.close(); } catch (_) {}
  }
  
  usbDevice = null;
  vendorInterface = null;
  vendorOutEndpoint = null;
  vendorInEndpoint = null;
}

function setupHidListener(hidDevice, webContents) {
  hidDevice.on('data', (data) => {
    let normArr = Array.from(data);
    // Strip Windows leading report ID 0x00
    if (normArr.length > 0 && normArr[0] === 0x00) normArr = normArr.slice(1);

    if (normArr.length === 0) return;

    // Echo suppression signature BEFORE prefix strip (to match what we sent)
    const receivedSig = normArr.slice(0, 8);
    const now = Date.now();
    let isEcho = false;
    for (let i = sentPacketHistory.length - 1; i >= 0; i--) {
      const entry = sentPacketHistory[i];
      if (now - entry.ts > ECHO_WINDOW_MS) continue;
      let matches = true;
      for (let j = 0; j < Math.min(8, receivedSig.length, entry.signature.length); j++) {
        if (receivedSig[j] !== entry.signature[j]) { matches = false; break; }
      }
      if (matches) {
        isEcho = true;
        sentPacketHistory.splice(i, 1);
        if (HID_DEBUG) console.log('[DEBUG] Suppressed echo after', now - entry.ts, 'ms');
        break;
      }
    }
    if (isEcho) return;

    // Strip optional 0xFF prefix inserted by intermediary hosts
    if (normArr.length > 1 && normArr[0] === 0xFF) {
      if (HID_DEBUG) console.log('[DEBUG] Raw HID inbound prefix 0xFF stripped');
      normArr = normArr.slice(1);
    }

    const cmdByte = normArr[0];

    if (cmdByte === config.HID_REPORT_ID_STATUS) {
      const statusCode = normArr.length > 1 ? normArr[1] : 0;
      let statusMsg = 'Unknown';
      if (statusCode === config.STATUS_OK) statusMsg = 'OK';
      else if (statusCode === config.STATUS_CHUNK_RECEIVED) statusMsg = 'Chunk received';
      else if (statusCode === config.STATUS_TRANSFER_STARTED) statusMsg = 'Transfer started';
      else if (statusCode === config.STATUS_TRANSFER_COMPLETE) statusMsg = 'Transfer complete';
      else if (statusCode === config.STATUS_ERROR_INVALID) statusMsg = 'Error: Invalid';
      console.log(`✅ Status from keyboard: ${statusMsg} (0x${statusCode.toString(16)})`);
      webContents.send('hid-status', { code: statusCode, message: statusMsg });
      return;
    }

    console.log(`📨 Message from keyboard (cmd: 0x${cmdByte.toString(16)}):`,
      normArr.slice(0, 16).map(b => '0x' + b.toString(16).padStart(2, '0')).join(' '),
      normArr.length > 16 ? '...' : ''
    );
    webContents.send('hid-data', Buffer.from(normArr));
  });

  hidDevice.on('error', (err) => console.error('❌ HID device error:', err));
}
function createWindow() {
  mainWindow = new BrowserWindow({
    width: 1200,
    height: 800,
    webPreferences: {
      preload: path.join(__dirname, 'preload.js'),
      nodeIntegration: false,
      contextIsolation: true
    }
  });

  mainWindow.loadFile(path.join(__dirname, 'index.html'));

  if (process.env.OPEN_DEVTOOLS) {
    mainWindow.webContents.openDevTools();
  }
}

// Select GIF file dialog
ipcMain.handle('select-gif', async () => {
  const result = await dialog.showOpenDialog({
    title: 'Select GIF',
    properties: ['openFile'],
    filters: [ { name: 'GIF Images', extensions: ['gif'] } ]
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

app.whenReady().then(() => {
  createWindow();
  app.on('activate', () => {
    if (BrowserWindow.getAllWindows().length === 0) createWindow();
  });
});

app.on('window-all-closed', () => {
  // Close HID / USB resources to ensure process can exit cleanly
  try { if (hidDevice) { try { hidDevice.close(); } catch (_) {} hidDevice = null; } } catch (_) {}
  try { closeVendorDevice(); } catch (_) {}
  if (process.platform !== 'darwin') app.quit();
});

// Ensure cleanup on quit (in case other resources remain open)
app.on('before-quit', () => {
  try { if (hidDevice) { try { hidDevice.close(); } catch (_) {} hidDevice = null; } } catch (_) {}
  try { closeVendorDevice(); } catch (_) {}
});

// Handle sending GIF to different destinations
ipcMain.handle('send-gif', async (event, { destination, gifData }) => {
  console.log('\n' + '='.repeat(60));
  console.log(`📤 Starting GIF transfer to ${destination}`);
  console.log(`File: ${gifData.name} (${gifData.size} bytes)`);
  console.log('='.repeat(60));
  
  let useVendor = false;
  let sendPacketFunc = null;
  
  try {
    // Try to open vendor interface first
    try {
      if (!vendorOutEndpoint) {
        openVendorDevice();
      }
      useVendor = true;
      sendPacketFunc = async (reportId, data) => {
        await sendVendorPacket(reportId, data);
      };
      console.log('✅ Using vendor bulk endpoints for transfer');
    } catch (vendorError) {
      console.log(`⚠️  Vendor endpoint failed: ${vendorError.message}`);
      console.log('🔄 Falling back to raw HID...');
      
      // Fallback to HID
      if (!HID) {
        throw new Error('Neither vendor endpoints nor HID library are available. Please run: npm install usb node-hid');
      }

      // Find and connect to keyboard via HID
      if (!hidDevice) {
        const devices = HID.devices();
        const keyboard = devices.find(d => 
          d.vendorId === KEYBOARD_VID && 
          d.productId === KEYBOARD_PID && 
          d.interface === 1
        );
        
        if (!keyboard) {
          console.error('❌ Keyboard not found. Looking for:');
          console.error(`   VID: 0x${KEYBOARD_VID.toString(16).padStart(4, '0').toUpperCase()}`);
          console.error(`   PID: 0x${KEYBOARD_PID.toString(16).padStart(4, '0').toUpperCase()}`);
          console.error(`   Interface: 1`);
          console.error('\nAvailable devices:');
          devices.filter(d => d.vendorId === KEYBOARD_VID && d.productId === KEYBOARD_PID)
            .forEach(d => {
              console.error(`   Interface ${d.interface}: ${d.path}`);
            });
          throw new Error('Keyboard not found. Check VID/PID/Interface in config.js');
        }
        
        console.log(`✅ Connecting to HID: ${keyboard.path}`);
        hidDevice = new HID.HID(keyboard.path);
        
        setupHidListener(hidDevice, mainWindow.webContents);
        console.log('✅ HID device opened and listening\n');
      }
      
      sendPacketFunc = async (reportId, data) => {
        sendHIDPacket(hidDevice, reportId, data);
      };
      console.log('✅ Using raw HID for transfer');
    }

    const destinationFlag = destination === 'Screen' ? config.DEST_SCREEN : config.DEST_SD_CARD;
    const gifBuffer = Buffer.from(gifData.data);
    const chunkPayloadSize = useVendor ? 61 : (REPORT_SIZE - 3); // Vendor: 64-3, HID: 32-3
    const totalChunks = Math.ceil(gifBuffer.length / chunkPayloadSize);
    
    console.log(`📊 Transfer details:`);
    console.log(`   Method: ${useVendor ? 'Vendor Bulk' : 'Raw HID'}`);
    console.log(`   Destination: ${destination} (flag: 0x${destinationFlag.toString(16)})`);
    console.log(`   Chunk size: ${chunkPayloadSize} bytes`);
    console.log(`   Total chunks: ${totalChunks}`);
    console.log(`   Estimated time: ~${(totalChunks * config.PACKET_DELAY_MS / 1000).toFixed(1)}s\n`);
    
    // Send START command
    const startPayload = Buffer.alloc(3);
    startPayload.writeUInt16LE(gifData.size, 0);
    startPayload[2] = destinationFlag;
    
    console.log('📤 Sending START command...');
    await sendPacketFunc(config.CMD_START_GIF, startPayload);
    await sleep(config.START_DELAY_MS);
    
    // Send GIF data in chunks
    console.log('📤 Sending data chunks...');
    for (let i = 0; i < totalChunks; i++) {
      const start = i * chunkPayloadSize;
      const end = Math.min(start + chunkPayloadSize, gifBuffer.length);
      const chunk = gifBuffer.slice(start, end);
      
      const packetPayload = Buffer.alloc(2 + chunk.length);
      packetPayload.writeUInt16LE(i, 0);
      chunk.copy(packetPayload, 2);
      
      await sendPacketFunc(config.CMD_GIF_DATA, packetPayload);
      await sleep(config.PACKET_DELAY_MS);
      
      // Update progress
      if (i % 10 === 0 || i === totalChunks - 1) {
        const progress = Math.round((i + 1) / totalChunks * 100);
        mainWindow.webContents.send('gif-progress', progress);
        
        if (i % 50 === 0 || i === totalChunks - 1) {
          console.log(`   Progress: ${progress}% (${i + 1}/${totalChunks} chunks)`);
        }
      }
    }
    
    // Send END command
    const endPayload = Buffer.alloc(1);
    endPayload[0] = destinationFlag;
    
    console.log('\n📤 Sending END command...');
    await sendPacketFunc(config.CMD_END_GIF, endPayload);
    
    console.log('='.repeat(60));
    console.log('✅ Transfer Complete');
    console.log('='.repeat(60) + '\n');
    
    console.log('⏳ Waiting for firmware response...');
    console.log('   (If you see no status messages, check firmware debug output)\n');
    
    return { success: true, message: `GIF sent to ${destination} via ${useVendor ? 'vendor bulk' : 'raw HID'}` };
    
  } catch (error) {
    console.error('❌ Transfer failed:', error.message);
    
    // Clean up connections
    if (hidDevice) {
      try { hidDevice.close(); } catch (_) {}
      hidDevice = null;
    }
    if (vendorOutEndpoint) {
      closeVendorDevice();
    }
    
    return { success: false, message: error.message };
  }
});

function sleep(ms) {
  return new Promise(resolve => setTimeout(resolve, ms));
}

// Handle actuation threshold updates
ipcMain.handle('update-actuation', async (event, { keyId, threshold }) => {
  console.log(`🔧 Updating key ${keyId} actuation to ${threshold}mm`);

  let useVendor = false;
  let sendPacketFunc = null;

  try {
    // Try vendor first, fallback to HID
    try {
      if (!vendorOutEndpoint) {
        openVendorDevice();
      }
      useVendor = true;
      sendPacketFunc = async (reportId, data) => {
        await sendVendorPacket(reportId, data);
      };
    } catch (vendorError) {
      if (!HID) {
        throw new Error('Neither vendor endpoints nor HID library are available.');
      }

      if (!hidDevice) {
        const devices = HID.devices();
        const keyboard = devices.find(d => 
          d.vendorId === KEYBOARD_VID && 
          d.productId === KEYBOARD_PID && 
          d.interface === 1
        );
        if (!keyboard) throw new Error('Keyboard not found');
        hidDevice = new HID.HID(keyboard.path);
        setupHidListener(hidDevice, mainWindow.webContents);
      }
      
      sendPacketFunc = async (reportId, data) => {
        sendHIDPacket(hidDevice, reportId, data);
      };
    }

    const keyIndex = parseInt(keyId.split('-')[1], 10) - 1;
    if (isNaN(keyIndex) || keyIndex < 0 || keyIndex > 112) {
      throw new Error('Invalid keyId format or index out of range.');
    }

    const thresholdValue = Math.round((threshold / 4.0) * 255);

    const payload = Buffer.alloc(2);
    payload[0] = keyIndex;
    payload[1] = thresholdValue;

    await sendPacketFunc(config.CMD_SET_THRESHOLD, payload);

    return { success: true, message: `Actuation for key ${keyId} updated via ${useVendor ? 'vendor' : 'HID'}.` };
  } catch (error) {
    console.error('❌ Failed to update actuation:', error);
    return { success: false, message: error.message };
  }
});

// Handle simple LED toggle via Python helper (py_settings/rgb_toggle.py)
ipcMain.handle('toggle-led', async () => {
  try {
    const args = [PY_RGB_TOGGLE, '--cmd', config.CMD_LED_TOGGLE.toString(16).padStart(2, '0')];
    const { stdout, stderr } = await execFileAsync(PYTHON_EXE, args, { cwd: __dirname });
    if (stderr && stderr.trim()) {
      console.warn('[py_settings] rgb_toggle stderr:', stderr.trim());
    }
    return { success: true, stdout: stdout.trim(), stderr: stderr.trim() };
  } catch (error) {
    const errMsg = error.stderr ? error.stderr.toString() : error.message;
    console.error('rgb_toggle failed:', errMsg);
    return { success: false, message: errMsg };
  }
});