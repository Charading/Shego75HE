const HID = require('node-hid');
const config = require('../config');

function findDevicePath() {
  const devices = HID.devices();
  const device = devices.find(d => d.vendorId === config.KEYBOARD_VID && d.productId === config.KEYBOARD_PID && d.interface === 1);
  if (!device) return null;
  return device.path;
}

async function main() {
  const path = findDevicePath();
  if (!path) {
    console.error('Keyboard device not found. Check VID/PID and interface.');
    process.exit(1);
  }

  console.log('Opening device at path:', path);
  const dev = new HID.HID(path);
  dev.setNonBlocking(true);

  // Build a test packet: report ID 0x00, CMD_GIF_DATA (0x11), chunk index 0, then a short payload
  const REPORT_SIZE = config.REPORT_SIZE;
  const buffer = Buffer.alloc(REPORT_SIZE + 1, 0);
  buffer[0] = 0x00; // report id
  buffer[1] = config.CMD_GIF_DATA; // 0x11
  buffer.writeUInt16LE(0, 2); // chunk index 0
  // fill a short payload pattern
  const payload = Buffer.from([0xDE, 0xAD, 0xBE, 0xEF]);
  payload.copy(buffer, 4);

  console.log('Writing packet:', Array.from(buffer).slice(0, 16).map(b => '0x' + b.toString(16).padStart(2,'0')).join(' '), '...');
  try {
    dev.write(Array.from(buffer));
  } catch (e) {
    console.error('Write error:', e);
  }

  console.log('Now listening for up to 2 seconds for device responses...');
  const start = Date.now();
  const interval = setInterval(() => {
    try {
      const data = dev.readTimeout ? dev.readTimeout(100) : null;
    } catch (e) {
      // ignore
    }

    // node-hid doesn't provide a synchronous readTimeout on all platforms via JS binding.
    // Use the event-based .on('data') if available.
  }, 200);

  dev.on('data', (data) => {
    const arr = Array.from(data);
    console.log('READ:', arr.map(b => '0x' + b.toString(16).padStart(2,'0')).join(' '));
  });

  dev.on('error', (err) => {
    console.error('Device error:', err);
  });

  setTimeout(() => {
    clearInterval(interval);
    try { dev.close(); } catch (_) {}
    console.log('Done.');
    process.exit(0);
  }, 2000);
}

main().catch(e => { console.error(e); process.exit(1); });
