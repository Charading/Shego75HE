#pragma once

#include <stdint.h>

#ifdef VENDOR_ENABLE
void vendor_task(void); // call frequently from main loop (matrix_scan_user)
// send a response back to host over vendor IN endpoint
bool vendor_send_response(const uint8_t *buf, uint16_t len);
#else
// stubs when vendor not enabled
static inline void vendor_task(void) { (void)0; }
static inline bool vendor_send_response(const uint8_t *buf, uint16_t len) { (void)buf; (void)len; return false; }
#endif
