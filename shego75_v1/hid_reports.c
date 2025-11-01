/* hid_reports.c - HID RAW report handler - forwards GIF data to ESP32 via I2C */
#include QMK_KEYBOARD_H
#include "hid_reports.h"
#include "i2c_esp32.h"
#include "uart.h"
#include "raw_hid.h"
#include "mux_adc.h"
#include "vendor_bridge.h"
#include <string.h>
#include <stdio.h>

#ifndef RAW_EPSIZE
#    define RAW_EPSIZE 32
#endif

// Transfer tracking
static uint32_t bytes_received = 0;
static bool transfer_active = false;
static uint16_t current_chunk_index = 0;

// Initialize HID reports
void hid_reports_init(void) {
    bytes_received = 0;
    transfer_active = false;
    current_chunk_index = 0;
    uart_send_string("[HID] Ready to receive GIF data and forward via I2C\n");
}

// Send status back to host
void send_status_to_host(uint8_t status, uint16_t chunk_index) {
    uint8_t report[RAW_EPSIZE] = {0};
    report[0] = HID_REPORT_ID_STATUS;
    report[1] = status;
    report[2] = chunk_index & 0xFF;
    report[3] = (chunk_index >> 8) & 0xFF;
    // Add a magic number to prove this function was called
    report[4] = 0xDE;
    report[5] = 0xAD;
    report[6] = 0xBE;
    report[7] = 0xEF;
    // Prefer vendor endpoint if available; fall back to raw HID
#ifdef VENDOR_ENABLE
    if (!vendor_send_response(report, RAW_EPSIZE)) {
        raw_hid_send(report, RAW_EPSIZE);
    }
#else
    raw_hid_send(report, RAW_EPSIZE);
#endif
}

// Convert incoming HID packet to an I2C payload for the ESP32 and log it
static void convert_hid_i2c(uint8_t *buf, uint8_t length) {
    if (!buf || length == 0) return;

    // Log basic info to UART/HID console
    char dbg[128];
    // Show first up to 8 bytes for quick inspection
    int show = (length > 8) ? 8 : length;
    char hexbuf[8 * 5 + 1];
    hexbuf[0] = '\0';
    for (int i = 0; i < show; i++) {
        char tmp[6];
        snprintf(tmp, sizeof(tmp), "0x%02X ", buf[i]);
        strncat(hexbuf, tmp, sizeof(hexbuf) - strlen(hexbuf) - 1);
    }
    snprintf(dbg, sizeof(dbg), "[HID->I2C] CMD=0x%02X LEN=%d DATA=%s\n", buf[0], length, hexbuf);
    uart_send_string(dbg);

    // Send a short debug packet to the ESP32 over I2C so the ESP32-side logger can
    // show the fact we received a HID packet without duplicating the full data flow.
    // Debug packet format: [DBG_ID=0xEE][CMD][LEN][first up to 6 bytes of payload]
    uint8_t dbg_pkt[9] = {0};
    dbg_pkt[0] = 0xEE;
    dbg_pkt[1] = buf[0];
    dbg_pkt[2] = length & 0xFF;
    int copy_len = (length > 6) ? 6 : length;
    for (int i = 0; i < copy_len; i++) dbg_pkt[3 + i] = buf[i];
    if (!i2c_esp32_send(dbg_pkt, 3 + copy_len)) {
        uart_send_string("[HID->I2C] debug i2c_esp32_send FAILED\n");
    }
}

// Shared processor for a normalized input buffer (used by raw HID and vendor bridge)
void hid_process_received_buffer(uint8_t *buf, uint8_t length) {
    if (!buf || length == 0) return;

    // Convert and forward the received HID buffer to I2C for debugging/processing
    convert_hid_i2c(buf, length);

    uint8_t report_id = buf[0];

    // Debug logging
    char debug_buf[96];
    snprintf(debug_buf, sizeof(debug_buf), "[HID] RX ID: 0x%02X, len: %d\n", report_id, length);
    uart_send_string(debug_buf);

    // Ignore VIA commands
    if (report_id <= 0x0F) {
        return;
    }

    // Process the command
    switch (report_id) {
        case HID_REPORT_ID_START_GIF: {
            send_status_to_host(STATUS_TRANSFER_STARTED, 0);

            uint8_t destination = buf[3];
            uart_send_string("[HID] START_GIF command\n");
            transfer_active = true;
            bytes_received = 0;
            current_chunk_index = 0;
            
            uint8_t i2c_packet[4];
            i2c_packet[0] = HID_REPORT_ID_START_GIF;
            i2c_packet[1] = buf[1]; // File size byte 1
            i2c_packet[2] = buf[2]; // File size byte 2
            i2c_packet[3] = destination;
            
            if (i2c_esp32_send(i2c_packet, 4)) {
                uart_send_string("[HID] GIF start sent via I2C SUCCESS\n");
            } else {
                uart_send_string("[HID] I2C send FAILED for START\n");
                send_status_to_host(STATUS_ERROR_INVALID, 0);
                transfer_active = false;
            }
            break;
        }
        
        case HID_REPORT_ID_GIF_DATA: {
            uint16_t received_chunk_index = (buf[2] << 8) | buf[1];
            send_status_to_host(STATUS_CHUNK_RECEIVED, received_chunk_index);

            if (!transfer_active) {
                break;
            }
            
            // The actual payload starts after the report ID and chunk index
            uint8_t* payload = &buf[3];
            uint8_t payload_len = (length > 3) ? (length - 3) : 0;

            if (payload_len && i2c_esp32_send(payload, payload_len)) {
                bytes_received += payload_len;
                current_chunk_index++;
            } else {
                uart_send_string("[HID] I2C send FAILED for DATA\n");
                send_status_to_host(STATUS_ERROR_INVALID, received_chunk_index);
            }
            break;
        }
        
        case HID_REPORT_ID_END_GIF: {
            send_status_to_host(STATUS_TRANSFER_COMPLETE, current_chunk_index);

            if (!transfer_active) {
                break;
            }
            
            uint8_t destination = buf[1];
            uart_send_string("[HID] END_GIF command\n");
            
            uint8_t i2c_packet[2];
            i2c_packet[0] = HID_REPORT_ID_END_GIF;
            i2c_packet[1] = destination;
            
            if (i2c_esp32_send(i2c_packet, 2)) {
                uart_send_string("[HID] GIF end sent via I2C SUCCESS\n");
            } else {
                uart_send_string("[HID] I2C send FAILED for END\n");
                send_status_to_host(STATUS_ERROR_INVALID, current_chunk_index);
            }
            transfer_active = false;
            break;
        }

        case HID_REPORT_ID_SET_THRESHOLD: {
            if (length < 4) {
                send_status_to_host(STATUS_ERROR_INVALID, 0);
                break;
            }

            uint8_t row = buf[1];
            uint8_t col = buf[2];
            uint8_t percent = buf[3];

            uint16_t key_idx = (uint16_t)row * MATRIX_COLS + (uint16_t)col;
            if (key_idx >= (MATRIX_ROWS * MATRIX_COLS)) {
                send_status_to_host(STATUS_ERROR_INVALID, 0);
                break;
            }

            set_key_threshold(key_idx, percent);
            send_status_to_host(STATUS_OK, 0);
            break;
        }
        
        // This case handles a status report sent *from* the host, if any.
        case HID_REPORT_ID_STATUS:
            // We can just acknowledge it.
            send_status_to_host(STATUS_OK, 0);
            break;
            
        default:
            snprintf(debug_buf, sizeof(debug_buf), "[HID] Unknown report ID: 0x%02X\n", report_id);
            uart_send_string(debug_buf);
            send_status_to_host(STATUS_ERROR_INVALID, 0);
            break;
    }
}

// Main HID receive handler: keep compatibility for raw_hid path
void raw_hid_receive_user(uint8_t *data, uint8_t length) {
    uint8_t buf[RAW_EPSIZE] = {0};
    uint8_t n = (length > RAW_EPSIZE) ? RAW_EPSIZE : length;
    if (n > 0) memcpy(buf, data, n);
    hid_process_received_buffer(buf, n);
}
