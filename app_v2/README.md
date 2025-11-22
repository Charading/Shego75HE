# Shego App V2

Electron-based companion utility for Shego75 v2 hardware.  The app identifies connected keyboards via VID/PID, exposes a simple status panel, and will orchestrate GIF/light payload streaming over the custom USB endpoints.

## Goals

- Detect both boards (VID `0xDEAD`, PIDs `0x0444` and `0x1616`).
- Surface device metadata (serial, firmware version, bootloader state).
- Offer a transport abstraction capable of:
  - VIA-style configuration packets.
  - SignalRGB streaming (RAW HID interface).
  - GIF bulk transfers to the ESP32 bridge.
- Provide hooks so future UI panels (layout editor, animation browser) can plug into the same transport without duplicating USB logic.

## Development

```
cd app_v2
npm install
npm run start
```

`npm run start` compiles the TypeScript sources to `dist/` and launches Electron pointing at the compiled entrypoint.  The renderer HTML is currently loaded directly from `src/renderer/index.html`, so hot-reload is as simple as saving the file and refreshing (`Ctrl+R`).

## Architecture

- **main process** (`src/main.ts`): boots Electron, wires IPC handlers, and manages `node-hid` handles for each keyboard.
- **preload** (`src/preload.ts`): exposes a minimal API (`window.shego.*`) to the renderer while keeping the surface area typed and secure.
- **renderer** (`src/renderer/index.html`): lightweight UI that lists detected devices and shows their transport capabilities.  React/Vite can be layered later.

## Next Steps

1. Flesh out the transport layer (queue writes, maintain open handles, auto-reconnect).
2. Implement GIF upload controls that translate local files into vendor packets.
3. Mirror VIA packet formats so advanced layout editing is possible without the VIA desktop app.
4. Integrate a modern frontend stack (likely Vite + React) once UX requirements solidify.
```
