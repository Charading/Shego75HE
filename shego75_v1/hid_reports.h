/* hid_reports.h - HID RAW report handler for Electron app communication */
#pragma once

#include QMK_KEYBOARD_H
#include <stdint.h>
#include <stdbool.h>

// HID Report IDs (must match your Electron app)
#define HID_REPORT_ID_GIF_DATA     0x01  // GIF data transfer
#define HID_REPORT_ID_BUTTON_CMD   0x02  // Button press commands
#define HID_REPORT_ID_STATUS       0x03  // Status responses

// Button Commands
#define BUTTON_CMD_SEND_TO_SCREEN  0x01  // Display GIF on screen
#define BUTTON_CMD_SAVE_TO_SD      0x02  // Save GIF to SD card

// Storage flags
#define STORAGE_SPIFFS   0x01    // Store in ESP32 flash
#define STORAGE_SD_CARD  0x02    // Save to SD card

// Function prototypes
void hid_reports_init(void);
void raw_hid_receive_user(uint8_t *data, uint8_t length);
void send_status_to_host(uint8_t status_code);

// Status codes
#define STATUS_OK                  0x00
#define STATUS_TRANSFER_STARTED    0x01
#define STATUS_CHUNK_RECEIVED      0x02
#define STATUS_TRANSFER_COMPLETE   0x03
#define STATUS_ERROR_INVALID       0xE3
