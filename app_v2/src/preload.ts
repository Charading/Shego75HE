import { contextBridge, ipcRenderer } from "electron";
import { Buffer } from "node:buffer";

contextBridge.exposeInMainWorld("shego", {
  listDevices: () => ipcRenderer.invoke("hid:list-devices"),
  sendRaw: (pathId: string, payload: Uint8Array) => ipcRenderer.invoke("hid:send", pathId, Buffer.from(payload)),
});
