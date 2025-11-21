/* hid_reports.c - HID RAW report handler - forwards GIF data to ESP32 via I2C */
#include QMK_KEYBOARD_H
#include "hid_reports.h"
#include "i2c_esp32.h"
#include "uart.h"
#include "uart_keycodes.h" // for toggle_led() prototype
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

    // Verbose debug dump of first 16 bytes before any rewriting
    char pre_dump[128];
    int pre_show = (length > 16) ? 16 : length;
    int pos = 0;
    for (int i = 0; i < pre_show; i++) {
        pos += snprintf(pre_dump + pos, sizeof(pre_dump) - pos, "%02X ", buf[i]);
    }
    uart_debug_print("[HID] RX RAW: ");
    uart_debug_print(pre_dump);
    uart_debug_print("\n");

    // Convert and forward the received HID buffer to I2C for debugging/processing
    convert_hid_i2c(buf, length);

    // ------------------------------------------------------------------------
    // ROBUST COMMAND SCANNING (Ported from adc_matrix_test)
    // ------------------------------------------------------------------------
    // Scan the first 8 bytes for known ASCII commands. This handles cases where
    // offsets vary (0x00 padding, 0xFF prefix, etc.) without fragile stripping.
    uint8_t scanned_cmd = 0;
    const uint8_t known_cmds[] = { 'T' }; // Add others like 'S','C','Q','B' if needed
    
    for (int i = 0; i < length && i < 8; i++) {
        for (size_t k = 0; k < sizeof(known_cmds); k++) {
            if (buf[i] == known_cmds[k]) {
                scanned_cmd = buf[i];
                char msg[64];
                snprintf(msg, sizeof(msg), "[HID] SCAN: Found cmd '%c' (0x%02X) at offset %d\n", scanned_cmd, scanned_cmd, i);
                uart_debug_print(msg);
                break;
            }
        }
        if (scanned_cmd) break;
    }

    if (scanned_cmd == 'T') {
        // Directly toggle LED transistor logic (same effect as LED_TOG key)
        uart_debug_print("\n\n========== LED TOGGLE (SCANNED) ==========\n");
        
        // Log state BEFORE toggle
        {
            char pre[120];
            snprintf(pre, sizeof(pre), "[BEFORE] logical=%d pin_level=%d pin=%d polarity=%s\n",
                        get_led_enabled() ? 1 : 0,
                        get_led_pin_level(),
                        get_led_pin_number(),
                        led_pin_active_high() ? "ACTIVE_HIGH" : "ACTIVE_LOW");
            uart_debug_print(pre);
        }
        
        toggle_led();
        
        // Log state AFTER toggle
        {
            char post[120];
            snprintf(post, sizeof(post), "[AFTER]  logical=%d pin_level=%d pin=%d polarity=%s\n",
                        get_led_enabled() ? 1 : 0,
                        get_led_pin_level(),
                        get_led_pin_number(),
                        led_pin_active_high() ? "ACTIVE_HIGH" : "ACTIVE_LOW");
            uart_debug_print(post);
        }
        
        // Also toggle Pico onboard LED (GP25) so user has visible feedback
        toggle_onboard_led(get_led_enabled());
        uart_debug_print("[HID] Onboard LED (GP25) toggled\n");
        uart_debug_print("==========================================\n\n");
        
        // Send an explicit raw HID response with the LED state
        uint8_t resp[RAW_EPSIZE] = {0};
        resp[0] = HID_REPORT_ID_LED_TOGGLE;
        resp[1] = (get_led_enabled() ? 1 : 0);
        resp[2] = get_led_pin_level();
        resp[3] = get_led_pin_number();
        resp[4] = led_pin_active_high() ? 1 : 0;
        raw_hid_send(resp, RAW_EPSIZE);
        
        // Also send a short status back so host apps can confirm the toggle
        send_status_to_host(STATUS_OK, 0);
        return; // Handled!
    }
    // ------------------------------------------------------------------------

    // Robust prefix stripping for other commands (GIF, etc.):
    // 1. Skip all leading zeros (Windows Report ID or padding)
    while (length > 0 && buf[0] == 0x00) {
        buf++;
        length--;
        uart_debug_print("[HID] Stripped leading 0x00\n");
    }

    if (length == 0) return;

    uint8_t report_id = buf[0];

    // 2. Strip single leading 0xFF (common in some QMK configurations)
    if (report_id == 0xFF && length >= 2) {
        buf++;
        length--;
        report_id = buf[0];
        uart_debug_print("[HID] Stripped prefix 0xFF\n");
    }

    // 3. Strip Magic 0xA5 (Legacy Python helpers)
    if (report_id == HID_PREFIX_APP_MAGIC && length >= 2) {
        buf++;
        length--;
        report_id = buf[0];
        uart_debug_print("[HID] Stripped prefix 0xA5\n");
    }

    // Debug logging - show parsed report ID after prefix stripping
    char debug_buf[128];
    snprintf(debug_buf, sizeof(debug_buf), "[HID] Parsed ID: 0x%02X, remaining len: %d\n", report_id, length);
    uart_debug_print(debug_buf);

    // ECHO BACK the parsed report ID to the host for debugging
    // This helps confirm what the firmware actually sees after stripping prefixes.
    // Only do this for the LED toggle command or unknown commands to avoid spamming during GIF transfer.
    if (report_id == HID_REPORT_ID_LED_TOGGLE || report_id > 0x30) {
        uint8_t echo[RAW_EPSIZE] = {0};
        echo[0] = 0xEE; // Debug Echo Report ID
        echo[1] = report_id;
        echo[2] = length;
        if (length > 0) echo[3] = buf[0]; // First byte of payload
        raw_hid_send(echo, RAW_EPSIZE);
    }

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
        
        case HID_REPORT_ID_LED_TOGGLE: {
            // Directly toggle LED transistor logic (same effect as LED_TOG key)
            uart_debug_print("\n\n========== LED TOGGLE DEBUG ==========\n");
            uart_debug_print("[HID] LED_TOGGLE (0x54/'T') command received\n");
            
            // Log state BEFORE toggle
            {
                char pre[120];
                snprintf(pre, sizeof(pre), "[BEFORE] logical=%d pin_level=%d pin=%d polarity=%s\n",
                         get_led_enabled() ? 1 : 0,
                         get_led_pin_level(),
                         get_led_pin_number(),
                         led_pin_active_high() ? "ACTIVE_HIGH" : "ACTIVE_LOW");
                uart_debug_print(pre);
            }
            
            toggle_led();
            
            // Log state AFTER toggle
            {
                char post[120];
                snprintf(post, sizeof(post), "[AFTER]  logical=%d pin_level=%d pin=%d polarity=%s\n",
                         get_led_enabled() ? 1 : 0,
                         get_led_pin_level(),
                         get_led_pin_number(),
                         led_pin_active_high() ? "ACTIVE_HIGH" : "ACTIVE_LOW");
                uart_debug_print(post);
            }
            
            // Also toggle Pico onboard LED (GP25) so user has visible feedback
            toggle_onboard_led(get_led_enabled());
            uart_debug_print("[HID] Onboard LED (GP25) toggled\n");
            uart_debug_print("======================================\n\n");
            
            // Send an explicit raw HID response with the LED state
            uint8_t resp[RAW_EPSIZE] = {0};
            resp[0] = HID_REPORT_ID_LED_TOGGLE;
            resp[1] = (get_led_enabled() ? 1 : 0);
            resp[2] = get_led_pin_level();
            resp[3] = get_led_pin_number();
            resp[4] = led_pin_active_high() ? 1 : 0;
            raw_hid_send(resp, RAW_EPSIZE);
            
            // Also send a short status back so host apps can confirm the toggle
            send_status_to_host(STATUS_OK, 0);
            break;
        }
            
        default:
            snprintf(debug_buf, sizeof(debug_buf), "[HID] Unknown report ID: 0x%02X\n", report_id);
            uart_debug_print(debug_buf);
            
            // Emergency fallback: if we see 0x30 or 0x54 anywhere in the buffer, force LED toggle
            for (int i = 0; i < length && i < 8; i++) {
                if (buf[i] == 0x30 || buf[i] == 0x54) {
                    char em[64];
                    snprintf(em, sizeof(em), "[HID] EMERGENCY: Found 0x%02X in buffer, forcing LED toggle!\n", buf[i]);
                    uart_debug_print(em);
                    toggle_led();
                    toggle_onboard_led(get_led_enabled());
                    uint8_t resp[RAW_EPSIZE] = {0};
                    resp[0] = HID_REPORT_ID_LED_TOGGLE;
                    resp[1] = (get_led_enabled() ? 1 : 0);
                    resp[2] = 0xEE; // Emergency marker
                    raw_hid_send(resp, RAW_EPSIZE);
                    return;
                }
            }
            
            send_status_to_host(STATUS_ERROR_INVALID, 0);
            break;
    }
}

// --- Minimal Raw HID handler copied from adc_matrix_test (returns handled) ---
static bool process_rawhid_command(uint8_t *data, uint8_t length) {
    uart_debug_print("[HID] process_rawhid_command ENTRY\n");
    if (length == 0) {
        uart_debug_print("[HID] len=0\n");
        return false;
    }

    // Log first 8 bytes
    char first8[64];
    snprintf(first8, sizeof(first8), "%02X %02X %02X %02X %02X %02X %02X %02X",
           data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7]);
    uart_debug_print("[HID] RX: ");
    uart_debug_print(first8);
    uart_debug_print("\n");

    if (length < RAW_EPSIZE) {
        char bufmsg[64];
        snprintf(bufmsg, sizeof(bufmsg), "[HID] Note: received %d bytes (expected %d)\n", length, RAW_EPSIZE);
        uart_debug_print(bufmsg);
    }

    // Heuristic: search first 8 bytes for known ASCII commands
    uint8_t cmd = 0;
    const uint8_t known_cmds[] = { 'T','S','C','Q','B' };
    for (uint8_t i = 0; i < 8 && i < length; ++i) {
        for (uint8_t k = 0; k < sizeof(known_cmds); ++k) {
            if (data[i] == known_cmds[k]) {
                cmd = data[i];
                char m[64];
                snprintf(m, sizeof(m), "[HID] Found cmd at offset %d: 0x%02X\n", i, cmd);
                uart_debug_print(m);
                break;
            }
        }
        if (cmd) break;
    }

    if (!cmd) {
        if (data[0] == 0xFF && data[1] != 0x00) {
            cmd = data[1];
            uart_debug_print("[HID] Using VIA marker cmd at data[1]\n");
        } else if (data[1] != 0x00) {
            cmd = data[1];
            uart_debug_print("[HID] Using data[1] as cmd\n");
        } else {
            cmd = data[0];
            uart_debug_print("[HID] Defaulting to data[0] as cmd\n");
        }
    }

    bool handled = true;
    char dbg[64];
    snprintf(dbg, sizeof(dbg), "[HID] Processing cmd=0x%02X ('%c')\n", cmd, (cmd >= 32 && cmd < 127) ? cmd : '?');
    uart_debug_print(dbg);

    switch (cmd) {
        case 'T':
            uart_debug_print("[HID] Toggling LED (T)\n");
            toggle_led();
            break;
        case 'B':
            uart_debug_print("[HID] BLINK test - pulsing LED\n");
            // Pulse the LED pin visibly
#ifdef LED_TOG_PIN
            setPinOutput(LED_TOG_PIN);
            for (int i = 0; i < 4; ++i) {
                writePinLow(LED_TOG_PIN);
                wait_ms(120);
                writePinHigh(LED_TOG_PIN);
                wait_ms(120);
            }
            // restore logical state to ON
            led_set_state(true);
#endif
            break;
        case 'S':
            uart_debug_print("[HID] Setting LED HIGH (S)\n");
            led_set_state(true);
            break;
        case 'C':
            uart_debug_print("[HID] Setting LED LOW (C)\n");
            led_set_state(false);
            break;
        case 'Q':
            uart_debug_print("[HID] Query only (Q)\n");
            break;
        default:
            uart_debug_print("[HID] Unknown cmd - not handled here\n");
            handled = false;
            break;
    }

    if (!handled) return false;

    uint8_t response[RAW_EPSIZE];
    memset(response, 0, sizeof(response));
    response[0] = 0x01; // Success marker
    response[1] = cmd;  // Echo command
    response[2] = get_led_enabled() ? 1 : 0;
    // Echo first 8 bytes of received payload for debugging on host side
    for (uint8_t i = 0; i < 8 && i < length; ++i) response[3 + i] = data[i];

    raw_hid_send(response, sizeof(response));
    uart_debug_print("[HID] Response sent\n");
    return true;
}

// Main HID receive handler
// We implement multiple hooks to ensure we catch the packet regardless of
// whether VIA, SignalRGB, or no module is active.

// 1. Core logic wrapper
void hid_receive_handler(uint8_t *data, uint8_t length) {
    uint8_t buf[RAW_EPSIZE] = {0};
    uint8_t n = (length > RAW_EPSIZE) ? RAW_EPSIZE : length;
    if (n > 0) memcpy(buf, data, n);
    // First, offer the simple adc_matrix_test-style raw HID handler a chance
    // to process short toggle commands. If it handles the packet, we're done.
    if (process_rawhid_command(buf, n)) return;

    // Otherwise fall back to the more complete HID processing flow
    hid_process_received_buffer(buf, n);
}

// 2. Keyboard-level hook (called by VIA if unhandled, or by QMK core)
void raw_hid_receive_kb(uint8_t *data, uint8_t length) {
    // uart_send_string("[HID] raw_hid_receive_kb called\n");
    hid_receive_handler(data, length);
    // Call user hook if needed, but we are handling it here
    // raw_hid_receive_user(data, length); 
}

// 3. User-level hook (called by SignalRGB patch or default QMK)
void raw_hid_receive_user(uint8_t *data, uint8_t length) {
    // uart_send_string("[HID] raw_hid_receive_user called\n");
    hid_receive_handler(data, length);
}

// 4. Top-level hook (Weak) - Fallback if no other module (VIA/SignalRGB) claims it
__attribute__((weak)) void raw_hid_receive(uint8_t *data, uint8_t length) {
    // uart_send_string("[HID] raw_hid_receive (weak) called\n");
    hid_receive_handler(data, length);
}
