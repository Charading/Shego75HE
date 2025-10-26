/* hid_reports.c - HID RAW report handler - forwards GIF data to ESP32 via UART */
#include "hid_reports.h"
#include "uart.h"
#include "raw_hid.h"
#include <string.h>
#include <stdio.h>

// Transfer tracking
static uint32_t bytes_received = 0;
static bool transfer_active = false;

// Helper to send a single byte via UART
static void uart_send_raw_byte(uint8_t byte) {
    char buf[2] = {byte, 0};
    uart_send_string(buf);
}

// Initialize HID reports
void hid_reports_init(void) {
    bytes_received = 0;
    transfer_active = false;
    uart_send_string("[HID] Ready to receive GIF data from Electron app\n");
}

// Main HID receive handler - forwards everything to ESP32 via UART
// Using raw_hid_receive_user to work alongside VIA
void raw_hid_receive_user(uint8_t *data, uint8_t length) {
    if (length == 0) return;
    
    uint8_t report_id = data[0];
    
    // Ignore VIA commands (report IDs 0x00-0x0F)
    if (report_id <= 0x0F) return;
    
    switch (report_id) {
        case HID_REPORT_ID_GIF_DATA: {
            // GIF data chunk received
            uint8_t chunk_num = data[1];
            
            if (chunk_num == 0) {
                // First chunk - includes total size and storage flag
                // Format: [0]=0x01, [1]=0, [2-3]=size, [4]=storage, [5+]=data
                uint8_t storage = data[4];
                uint8_t payload_len = length - 5;
                
                transfer_active = true;
                bytes_received = 0;
                
                // Send header to ESP32: <GIF_START:size:storage>
                uart_send_string("<GIF_START:");
                uart_send_raw_byte(data[2]);  // size low
                uart_send_raw_byte(data[3]);  // size high
                uart_send_raw_byte(storage);
                uart_send_string(">");
                
                // Send first chunk data
                if (payload_len > 0) {
                    for (uint8_t i = 0; i < payload_len; i++) {
                        uart_send_raw_byte(data[5 + i]);
                    }
                    bytes_received += payload_len;
                }
                
                send_status_to_host(STATUS_TRANSFER_STARTED);
                
            } else {
                // Subsequent chunks - just data
                // Format: [0]=0x01, [1]=chunk_num, [2+]=data
                uint8_t payload_len = length - 2;
                
                if (payload_len > 0 && transfer_active) {
                    for (uint8_t i = 0; i < payload_len; i++) {
                        uart_send_raw_byte(data[2 + i]);
                    }
                    bytes_received += payload_len;
                    
                    // Status update every 10 chunks
                    if (chunk_num % 10 == 0) {
                        send_status_to_host(STATUS_CHUNK_RECEIVED);
                    }
                }
            }
            break;
        }
        
        case HID_REPORT_ID_BUTTON_CMD: {
            // Button command received
            // Format: [0]=0x02, [1]=button_cmd, [2]=storage
            uint8_t button_cmd = data[1];
            uint8_t storage = data[2];
            
            if (transfer_active) {
                // End the GIF transfer
                uart_send_string("<GIF_END>");
                transfer_active = false;
                send_status_to_host(STATUS_TRANSFER_COMPLETE);
            }
            
            // Send button command to ESP32
            if (button_cmd == BUTTON_CMD_SEND_TO_SCREEN) {
                uart_send_string("<CMD:SCREEN:");
                uart_send_raw_byte(storage);
                uart_send_string(">");
            } else if (button_cmd == BUTTON_CMD_SAVE_TO_SD) {
                uart_send_string("<CMD:SAVE:");
                uart_send_raw_byte(storage);
                uart_send_string(">");
            }
            
            send_status_to_host(STATUS_OK);
            break;
        }
        
        case HID_REPORT_ID_STATUS:
            // Status request
            send_status_to_host(STATUS_OK);
            break;
            
        default:
            send_status_to_host(STATUS_ERROR_INVALID);
            break;
    }
}

// Send status back to host
void send_status_to_host(uint8_t status_code) {
    uint8_t report[32] = {0};
    report[0] = HID_REPORT_ID_STATUS;
    report[1] = status_code;
    report[2] = (bytes_received >> 0) & 0xFF;
    report[3] = (bytes_received >> 8) & 0xFF;
    report[4] = (bytes_received >> 16) & 0xFF;
    report[5] = (bytes_received >> 24) & 0xFF;
    
    raw_hid_send(report, sizeof(report));
}
