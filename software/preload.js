const { contextBridge, ipcRenderer } = require('electron');

contextBridge.exposeInMainWorld('electronAPI', {
  selectGif: () => ipcRenderer.invoke('select-gif'),
  sendGif: (destination, gifData) => ipcRenderer.invoke('send-gif', { destination, gifData }),
  updateActuation: (keyId, threshold) => ipcRenderer.invoke('update-actuation', { keyId, threshold }),
  getPorts: () => ipcRenderer.invoke('get-ports')
});
