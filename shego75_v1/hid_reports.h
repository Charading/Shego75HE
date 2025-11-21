/* hid_reports.h - HID RAW report handler for Electron app communication */
#pragma once

#include QMK_KEYBOARD_H
#include <stdint.h>
#include <stdbool.h>

// HID Report IDs (must match your Electron app)
// Using 0x10+ to avoid conflict with VIA (0x00-0x0F)
#define HID_REPORT_ID_START_GIF    0x10  // Start GIF transfer (CMD_START_GIF)
#define HID_REPORT_ID_GIF_DATA     0x11  // GIF data chunks (CMD_GIF_DATA)
#define HID_REPORT_ID_END_GIF      0x12  // End GIF transfer (CMD_END_GIF)
#define HID_REPORT_ID_STATUS       0x13  // Status responses
// New: set per-key threshold
#define HID_REPORT_ID_SET_THRESHOLD 0x20
// Custom: trigger LED_TOG keycode from host
// Historically this used 0x30; accept 0x54 ('T') as the primary command now.
#define HID_REPORT_ID_LED_TOGGLE    0x54

// Destination flags (matches your config.js)
#define DEST_SCREEN    0x01  // Display on screen
#define DEST_SD_CARD   0x02  // Save to SD card

// Storage flags
#define STORAGE_SPIFFS   0x01    // Store in ESP32 flash
#define STORAGE_SD_CARD  0x02    // Save to SD card

// Function prototypes
void hid_reports_init(void);
void raw_hid_receive_user(uint8_t *data, uint8_t length);
void send_status_to_host(uint8_t status, uint16_t chunk_index);
// Process a received buffer (shared by raw HID and vendor bulk handler)
void hid_process_received_buffer(uint8_t *buf, uint8_t length);

// Status codes
#define STATUS_OK                  0x01
#define STATUS_TRANSFER_STARTED    0x02
#define STATUS_CHUNK_RECEIVED      0x03
#define STATUS_TRANSFER_COMPLETE   0x04
#define STATUS_ERROR_INVALID       0x05

// Some host utilities prepend a magic prefix byte before the actual command
#define HID_PREFIX_APP_MAGIC        0xA5
