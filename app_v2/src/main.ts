import { app, BrowserWindow, ipcMain, IpcMainInvokeEvent } from "electron";
import { Buffer } from "node:buffer";
import path from "node:path";
import HID from "node-hid";

const VENDOR_ID = 0xdead;
const SUPPORTED_PIDS = new Map<number, string>([
  [0x0444, "Shego75 v2"],
  [0x1616, "Shego Mini"],
]);

type HidSummary = {
  pid: number;
  vid: number;
  path: string;
  product: string;
  serialNumber?: string;
  usage?: number;
  usagePage?: number;
};

const hidHandles = new Map<string, HID.HID>();

function summarizeDevices(): HidSummary[] {
  return HID.devices()
    .filter((d: HID.Device) => d.vendorId === VENDOR_ID && SUPPORTED_PIDS.has(d.productId ?? -1))
    .map((device: HID.Device) => ({
      pid: device.productId ?? 0,
      vid: device.vendorId ?? 0,
      path: device.path ?? "",
      product: SUPPORTED_PIDS.get(device.productId ?? 0) ?? "Unknown",
      serialNumber: device.serialNumber ?? undefined,
      usage: device.usage,
      usagePage: device.usagePage,
    }));
}

async function handleListDevices(): Promise<HidSummary[]> {
  return summarizeDevices();
}

async function handleSendRaw(_event: IpcMainInvokeEvent, pathId: string, payload: Buffer): Promise<boolean> {
  let handle = hidHandles.get(pathId);
  if (!handle) {
    handle = new HID.HID(pathId);
    hidHandles.set(pathId, handle);
  }

  handle.write(Array.from(payload));
  return true;
}

function createWindow(): BrowserWindow {
  const win = new BrowserWindow({
    width: 900,
    height: 600,
    webPreferences: {
      preload: path.join(__dirname, "preload.js"),
    },
  });

  const rendererPath = path.join(app.getAppPath(), "src", "renderer", "index.html");
  win.loadFile(rendererPath);
  return win;
}

function registerIpc() {
  ipcMain.handle("hid:list-devices", handleListDevices);
  ipcMain.handle("hid:send", handleSendRaw);
}

app.whenReady().then(() => {
  registerIpc();
  createWindow();

  app.on("activate", () => {
    if (BrowserWindow.getAllWindows().length === 0) {
      createWindow();
    }
  });
});

app.on("window-all-closed", () => {
  hidHandles.forEach((handle) => handle.close());
  hidHandles.clear();
  if (process.platform !== "darwin") {
    app.quit();
  }
});
