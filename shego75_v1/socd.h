// socd.h - SOCD handling for A/D and W/S
#pragma once

#include "quantum.h"

// Toggle SOCD state
void toggle_socd(void);
bool get_socd_enabled(void);

// Process a key event for SOCD. Returns true to allow normal processing,
// false to suppress the event.
bool socd_process_key(uint16_t keycode, bool pressed);
