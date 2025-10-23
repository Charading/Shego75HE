#pragma once
#include "quantum.h"

#define LED_TOG_PIN GP23

// Lighting helper hooks used by the keyboard firmware.
// These functions are implemented in lighting.c and are safe to call from
// other modules (socd.c, keymap.c, etc.).

// Trigger a SOCD blink animation (called when SOCD toggles)
void trigger_socd_blink(bool enabled);

// Set/clear an enforced override for the layer-3 status LEDs (92..94)
void set_layer3_override(bool on);

// Set/clear an enforced override for caps-lock ambient LEDs (82..91)
void set_caps_override(bool on);

// Start a one-shot pulse animation on a single LED index (0-based)
void start_led_pulse(uint8_t led_index);

// Initialize lighting baseline and internal caches. Call from keyboard_post_init_user.
void lighting_init(void);

