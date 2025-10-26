// Keyboard Configuration
module.exports = {
  // Replace these with your keyboard's actual VID/PID
  // You can find these in Device Manager or by running the app and checking the console
  KEYBOARD_VID: 0xDEAD, // Vendor ID (hexadecimal)
  KEYBOARD_PID: 0xC0DE, // Product ID (hexadecimal)
  
  // HID Report Configuration
  REPORT_SIZE: 64, // Standard HID report size (adjust if your keyboard uses different size)
  
  // Protocol Commands
  CMD_START_GIF: 0x01,
  CMD_GIF_DATA: 0x02,
  CMD_END_GIF: 0x03,
  
  // Destination Flags
  DEST_SCREEN: 0x01,
  DEST_SD_CARD: 0x02,
  
  // Timing
  PACKET_DELAY_MS: 10, // Delay between packets
  START_DELAY_MS: 50   // Delay after start packet
};
