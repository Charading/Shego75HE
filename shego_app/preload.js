const { contextBridge, ipcRenderer } = require('electron');

contextBridge.exposeInMainWorld('api', {
  listDevices: () => ipcRenderer.invoke('list-devices'),
  sendCommand: (params) => ipcRenderer.invoke('send-command', params),
  startConsole: () => ipcRenderer.send('start-console'),
  stopConsole: () => ipcRenderer.send('stop-console'),
  onConsoleData: (callback) => ipcRenderer.on('console-data', (event, data) => callback(data))
});
