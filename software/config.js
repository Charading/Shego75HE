module.exports = {
    // Replace these with your keyboard's actual VID/PID
    KEYBOARD_VID: 0xDEAD,
    KEYBOARD_PID: 0xC0DE,

    // HID Report Configuration
    REPORT_SIZE: 32, // RAW HID report size for QMK

    // Protocol Commands (avoid 0x00-0x0F for VIA)
    CMD_START_GIF: 0x10,
    CMD_GIF_DATA: 0x11,
    CMD_END_GIF: 0x12,
    HID_REPORT_ID_STATUS: 0x13, // Report ID for firmware status updates
    // Set per-key actuation threshold (row, col, percent)
    CMD_SET_THRESHOLD: 0x20,
    // Custom: firmware toggles LED transistor/boolean
    CMD_LED_TOGGLE: 0x30,

    // Status Codes from Firmware
    STATUS_OK: 0x01,
    STATUS_CHUNK_RECEIVED: 0x02,
    STATUS_TRANSFER_STARTED: 0x03,
    STATUS_TRANSFER_COMPLETE: 0x04,
    STATUS_ERROR_INVALID: 0x05,

    // Destination Flags
    DEST_SCREEN: 0x01,
    DEST_SD_CARD: 0x02,

    // Timing
    PACKET_DELAY_MS: 10,
    START_DELAY_MS: 50
};