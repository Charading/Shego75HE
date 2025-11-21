// Helper script to find your keyboard's VID/PID
// Run this with: node find-keyboard.js

try {
  const HID = require('node-hid');
  
  console.log('Scanning for HID devices...\n');
  
  const devices = HID.devices();
  
  if (devices.length === 0) {
    console.log('No HID devices found.');
  } else {
    console.log(`Found ${devices.length} HID device(s):\n`);
    
    devices.forEach((device, index) => {
      console.log(`Device ${index + 1}:`);
      console.log(`  Vendor ID:  0x${device.vendorId.toString(16).toUpperCase().padStart(4, '0')}`);
      console.log(`  Product ID: 0x${device.productId.toString(16).toUpperCase().padStart(4, '0')}`);
      console.log(`  Manufacturer: ${device.manufacturer || 'N/A'}`);
      console.log(`  Product: ${device.product || 'N/A'}`);
      console.log(`  Path: ${device.path}`);
      console.log('');
    });
    
    console.log('\nTo use your keyboard:');
    console.log('1. Find your keyboard in the list above');
    console.log('2. Copy the Vendor ID and Product ID');
    console.log('3. Update config.js with these values\n');
  }
} catch (err) {
  console.error('Error: node-hid is not installed.');
  console.error('Please run: npm install node-hid');
}
