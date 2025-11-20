const { app, BrowserWindow, ipcMain } = require('electron');
const path = require('path');
const hid = require('./hid');
const { spawn } = require('child_process');

let consoleProcess = null;
let mainWindow = null;

function createWindow() {
  const win = new BrowserWindow({
    width: 900,
    height: 700,
    resizable: true,
    webPreferences: {
      preload: path.join(__dirname, 'preload.js'),
      contextIsolation: true,
      nodeIntegration: false,
    }
  });

  win.loadFile('index.html');
  // win.webContents.openDevTools(); // Uncomment for debugging
  mainWindow = win;
}

app.whenReady().then(() => {
  createWindow();
  app.on('activate', () => {
    if (BrowserWindow.getAllWindows().length === 0) createWindow();
  });
});

app.on('window-all-closed', () => {
  if (process.platform !== 'darwin') app.quit();
});

// IPC Handlers
ipcMain.handle('list-devices', async () => {
  try {
    const devices = hid.listDevices();
    return { success: true, devices };
  } catch (error) {
    return { success: false, error: error.message };
  }
});

ipcMain.handle('send-command', async (event, { vendorId, productId, commandByte }) => {
  try {
    const result = await hid.sendCommand(vendorId, productId, commandByte);
    return { success: true, ...result };
  } catch (error) {
    return { success: false, error: error.message };
  }
});

ipcMain.on('start-console', (event) => {
  if (consoleProcess) return;

  const pythonScript = path.join(__dirname, 'scripts', 'read_serial.py');
  // Use 'python' or 'python3' depending on environment. 
  // Ideally this should be configurable or detected.
  const pythonExe = process.env.PYTHON || 'python'; 
  
  consoleProcess = spawn(pythonExe, [pythonScript]);

  consoleProcess.stdout.on('data', (data) => {
    if (mainWindow) {
      mainWindow.webContents.send('console-data', data.toString());
    }
  });

  consoleProcess.stderr.on('data', (data) => {
    if (mainWindow) {
      mainWindow.webContents.send('console-data', `[ERR] ${data.toString()}`);
    }
  });

  consoleProcess.on('close', (code) => {
    if (mainWindow) {
      mainWindow.webContents.send('console-data', `[EXIT] Process exited with code ${code}`);
    }
    consoleProcess = null;
  });
});

ipcMain.on('stop-console', () => {
  if (consoleProcess) {
    consoleProcess.kill();
    consoleProcess = null;
  }
});
