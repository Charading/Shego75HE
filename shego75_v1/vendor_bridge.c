#include QMK_KEYBOARD_H
#include "vendor_bridge.h"
#include "hid_reports.h"

#ifdef VENDOR_ENABLE

#ifndef VENDOR_EPSIZE
#    define VENDOR_EPSIZE 64
#endif

// NOTE: QMK doesn't provide high-level vendor endpoint APIs like raw_hid_send/receive.
// For now, these are stubs that allow the descriptors to be present for host detection,
// but actual communication falls back to raw HID.
// To implement actual vendor endpoints, you'd need to write low-level ChibiOS USB driver code.

// Poll vendor OUT endpoint and forward payload into hid processor
void vendor_task(void) {
    // Stub - vendor endpoints are present in descriptors but not yet functional
    // Host apps will detect vendor interface but communication falls back to raw HID
}

// Send via vendor IN endpoint. Returns true if write initiated.
bool vendor_send_response(const uint8_t *buf, uint16_t len) {
    (void)buf; 
    (void)len;
    // Stub - return false to indicate vendor send failed, triggering HID fallback
    return false;
}

#endif // VENDOR_ENABLE
