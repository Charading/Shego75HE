const HID = require('node-hid');

const RAW_USAGE_PAGE = 0xFF60; // match your config.h
const RAW_USAGE_ID   = 0x61;   // match your config.h
const VID = 0xDEAD; // replace if different
const PID = 0xC0DE;

const devs = HID.devices();
console.log('All HID.devices() (filtered by VID/PID if matched):');
console.log(devs.filter(d => d.vendorId === VID && d.productId === PID));

const matching = devs.find(d => d.vendorId === VID && d.productId === PID &&
                                d.usagePage === RAW_USAGE_PAGE && d.usage === RAW_USAGE_ID);

const deviceInfo = matching || devs.find(d => d.vendorId === VID && d.productId === PID) || devs[0];

if (!deviceInfo) {
  console.error('No HID devices found at all. Please paste HID.devices() output here.');
  process.exit(1);
}

console.log('Using device:', deviceInfo.path || deviceInfo);

try {
  const h = new HID.HID(deviceInfo.path);
  h.on('data', buf => {
    console.log('Received from keyboard:', Buffer.from(buf).toString('hex').match(/.{1,2}/g).join(' 0x'));
  });
  h.on('error', e => console.error('HID error', e));

  // Build a tiny START packet your firmware expects:
  // CMD = 0x10, size low/high = 0x01,0x00 (tiny test), dest = 0x01
  const start = [0x10, 0x01, 0x00, 0x01]; // no leading report id
  console.log('Writing start (no leading 0x00):', start.map(b => '0x' + b.toString(16)));
  h.write(start);

  // Wait and then try with leading 0x00 (Windows style)
  setTimeout(() => {
    const withReport = [0x00, ...start];
    console.log('Writing start (with leading 0x00):', withReport.map(b => '0x' + b.toString(16)));
    h.write(withReport);
  }, 250);

  // Keep open a few seconds to receive anything
  setTimeout(() => {
    console.log('Done; closing device');
    try { h.close(); } catch (_) {}
    process.exit(0);
  }, 2000);

} catch (e) {
  console.error('Open/write failed:', e);
  process.exit(1);
}