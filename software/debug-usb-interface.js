// debug-usb-interfaces.js
const usb = require('usb');
const VID = 0xDEAD, PID = 0xC0DE;
const dev = usb.findByIds(VID, PID);
if (!dev) { console.error('Device not found'); process.exit(1); }
try { dev.open(); } catch (e) { console.error('open() failed', e); }
console.log('Device opened OK');
dev.interfaces.forEach((iface, i) => {
  console.log(`iface ${i}: bInterfaceNumber=${iface.descriptor.bInterfaceNumber} bInterfaceClass=0x${iface.descriptor.bInterfaceClass.toString(16)} bInterfaceSubClass=0x${iface.descriptor.bInterfaceSubClass.toString(16)} bInterfaceProtocol=0x${iface.descriptor.bInterfaceProtocol.toString(16)}`);
  try {
    if (typeof iface.isKernelDriverActive === 'function') {
      console.log('  kernel driver active?', iface.isKernelDriverActive());
    } else {
      console.log('  kernel driver active? (no API)');
    }
  } catch (e) { console.log('  kernel driver check failed:', e); }
  iface.endpoints.forEach(ep => {
    console.log(`  ep addr=0x${ep.address.toString(16)} dir=${ep.direction} type=${ep.transferType} max=${ep.descriptor.wMaxPacketSize}`);
  });
});
try { dev.close(); } catch(e) {}