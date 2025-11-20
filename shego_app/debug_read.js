const HID = require('node-hid');

function toHex(buf) {
  return Array.from(buf).map(b => b.toString(16).padStart(2, '0')).join(' ');
}

function listDevices() {
  const devices = HID.devices();
  devices.forEach(d => console.log(JSON.stringify(d, null, 2)));
}

function listenPath(path) {
  if (!path) throw new Error('Usage: node debug_read.js <device.path>');
  const device = new HID.HID(path);
  device.on('data', data => {
    console.log('IN:', toHex(data));
  });
  device.on('error', err => {
    console.error('ERROR', err);
    try { device.close(); } catch (e) {}
    process.exit(2);
  });
  console.log('Listening on', path);
  // keep process alive
  process.stdin.resume();
}

if (require.main === module) {
  const arg = process.argv[2];
  if (!arg) {
    console.log('Listing HID devices:');
    listDevices();
    console.log('\nTo listen on a device path: node debug_read.js "<device.path>"');
    process.exit(0);
  }
  listenPath(arg);
}

module.exports = { listDevices, listenPath };
