// try-claim.js
const usb = require('usb');
const VID = 0xDEAD, PID = 0xC0DE;
const dev = usb.findByIds(VID, PID);
if (!dev) { console.error('Device not found'); process.exit(1); }
try { dev.open(); } catch (e) { console.error('open() failed', e); }
const idx = 4; // interface index from your test output
const iface = dev.interfaces[idx];
console.log('Attempting on interface index', idx, 'bInterfaceNumber', iface.descriptor.bInterfaceNumber);
try {
  if (iface.isKernelDriverActive && iface.isKernelDriverActive()) {
    console.log('Kernel driver is active, attempting detach()...');
    try { iface.detachKernelDriver(); console.log('detach() ok'); } catch (e) { console.error('detach() failed:', e); }
  } else {
    console.log('Kernel driver not active (or API not available).');
  }
} catch (e) { console.error('isKernelDriverActive() error:', e); }
try {
  iface.claim();
  console.log('claim() OK');
  iface.release(true, () => { console.log('released'); try{ dev.close(); }catch(_){} });
} catch (e) {
  console.error('claim() failed:', e);
  try{ dev.close(); }catch(_){} 
}