const { contextBridge, ipcRenderer } = require('electron');

contextBridge.exposeInMainWorld('electronAPI', {
  selectGif: () => ipcRenderer.invoke('select-gif'),
  sendGif: (destination, gifData) => ipcRenderer.invoke('send-gif', { destination, gifData }),
  updateActuation: (keyId, threshold) => ipcRenderer.invoke('update-actuation', { keyId, threshold }),
  getPorts: () => ipcRenderer.invoke('get-ports'),
  toggleLed: () => ipcRenderer.invoke('toggle-led'),
  onHidStatus: (callback) => ipcRenderer.on('hid-status', (_event, data) => callback(data)),
  onHidData: (callback) => ipcRenderer.on('hid-data', (_event, data) => callback(data))
});
