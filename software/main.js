const { app, BrowserWindow, ipcMain, dialog } = require('electron');
const path = require('path');
const fs = require('fs');

let mainWindow;

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
  console.log(`Sending GIF to ${destination}`, gifData.name);
  
  // TODO: Implement actual serial communication to keyboard
  // This is where you'd use serialport to communicate with your keyboard
  
  try {
    // Placeholder for actual implementation
    // const { SerialPort } = require('serialport');
    // const port = new SerialPort({ path: 'COM3', baudRate: 115200 });
    
    // Simulate sending data
    await new Promise(resolve => setTimeout(resolve, 1000));
    
    return { success: true, message: `GIF sent to ${destination}` };
  } catch (error) {
    return { success: false, message: error.message };
  }
});

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
    // TODO: Implement with serialport
    // const { SerialPort } = require('serialport');
    // const ports = await SerialPort.list();
    // return ports;
    
    return []; // Placeholder
  } catch (error) {
    console.error('Error listing ports:', error);
    return [];
  }
});
