// Simple test script - sends ONE packet to verify communication
// Run with: node test-single-packet.js

const HID = require('node-hid');
const config = require('./config');

console.log('=== Single Packet Test ===\n');

// Find keyboard
const devices = HID.devices();
console.log(`Looking for VID: 0x${config.KEYBOARD_VID.toString(16)}, PID: 0x${config.KEYBOARD_PID.toString(16)}`);

const keyboard = devices.find(d => 
  d.vendorId === config.KEYBOARD_VID && 
  d.productId === config.KEYBOARD_PID && 
  d.interface === 1
);

if (!keyboard) {
  console.error('❌ Keyboard not found!');
  console.error('\nAvailable devices:');
  devices.forEach(d => {
    if (d.vendorId === config.KEYBOARD_VID && d.productId === config.KEYBOARD_PID) {
      console.error(`  Interface ${d.interface}: ${d.path}`);
    }
  });
  process.exit(1);
}

console.log(`✅ Found keyboard: ${keyboard.path}\n`);

// Open device
let hid;
try {
  hid = new HID.HID(keyboard.path);
  console.log('✅ Device opened\n');
} catch (err) {
  console.error('❌ Failed to open device:', err.message);
  process.exit(1);
}

// Listen for responses (including echoes)
let receivedCount = 0;
hid.on('data', (data) => {
  receivedCount++;
  const arr = Array.from(data);
  
  // Strip leading 0x00 if present
  const normalized = arr[0] === 0x00 ? arr.slice(1) : arr;
  
  console.log(`\n[RECEIVED #${receivedCount}]`);
  console.log('  First 10 bytes:', normalized.slice(0, 10).map(b => '0x' + b.toString(16).padStart(2, '0')).join(' '));
  
  const cmd = normalized[0];
  if (cmd === config.CMD_START_GIF) {
    console.log('  Type: ECHO of START packet (this is normal on Windows)');
  } else if (cmd === config.HID_REPORT_ID_STATUS) {
    const status = normalized[1];
    console.log('  Type: STATUS message from firmware');
    console.log('  Status code:', '0x' + status.toString(16));
  } else {
    console.log('  Type: Unknown (0x' + cmd.toString(16) + ')');
  }
});

hid.on('error', (err) => {
  console.error('❌ HID Error:', err);
});

// Build START packet
const packet = Buffer.alloc(config.REPORT_SIZE + 1);
packet[0] = 0x00; // Report ID (required for Windows)
packet[1] = config.CMD_START_GIF; // 0x10
packet.writeUInt16LE(100, 2); // Small test size
packet[4] = config.DEST_SCREEN; // 0x01

console.log('📤 Sending START packet:');
console.log('  Report ID: 0x00 (Windows requirement)');
console.log('  Command: 0x' + config.CMD_START_GIF.toString(16) + ' (START_GIF)');
console.log('  Size: 100 bytes (test value)');
console.log('  Destination: 0x' + config.DEST_SCREEN.toString(16) + ' (Screen)');
console.log('  Full packet:', Array.from(packet).slice(0, 10).map(b => '0x' + b.toString(16).padStart(2, '0')).join(' '), '...\n');

try {
  hid.write(Array.from(packet));
  console.log('✅ Packet sent!\n');
} catch (err) {
  console.error('❌ Write failed:', err.message);
  hid.close();
  process.exit(1);
}

console.log('⏳ Waiting 2 seconds for responses...');
console.log('\n📋 What to check now:');
console.log('  1. Do you see a "RECEIVED" message above? (Echo is normal)');
console.log('  2. Check your keyboard debug output for: [HID] Received 32 bytes, CMD=0x10');
console.log('  3. Check ESP32 debug output for: [ESP] Starting GIF transfer');
console.log('  4. If you see NOTHING, the firmware is not receiving the packet\n');

setTimeout(() => {
  console.log('=== Test Complete ===');
  
  if (receivedCount === 0) {
    console.log('\n⚠️  No data received from keyboard.');
    console.log('This could mean:');
    console.log('  - Wrong interface selected (try interface 0?)');
    console.log('  - Keyboard firmware is not responding');
    console.log('  - HID communication is one-way only');
  } else if (receivedCount === 1) {
    console.log('\n✅ Received 1 packet (likely an echo - normal on Windows)');
    console.log('👉 Now check keyboard and ESP32 debug output!');
  } else {
    console.log(`\n✅ Received ${receivedCount} packets`);
    console.log('This might include status messages from firmware.');
  }
  
  console.log('\n💡 Next steps:');
  console.log('  1. If keyboard firmware received the packet → Check I2C/UART to ESP32');
  console.log('  2. If ESP32 received the packet → Check GIF decoding/display code');
  console.log('  3. If nothing received → Add debug output to keyboard firmware\n');
  
  hid.close();
  process.exit(0);
}, 2000);