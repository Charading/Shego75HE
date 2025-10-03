#include "lighting.h"
#include "quantum.h"
#include "rgb_matrix.h"

// State for overrides and simple animations
static bool layer3_override = false;
static bool caps_override = false;

// Pulse state
static bool pulse_active = false;
static uint8_t pulse_led = 0;
static uint32_t pulse_start = 0;
static const uint32_t PULSE_MS = 600;

// SOCD blink state
static bool socd_blink_active = false;
static uint8_t socd_blink_color_r = 0;
static uint8_t socd_blink_color_g = 0;
static uint8_t socd_blink_color_b = 0;
static uint8_t socd_blink_count = 0;
static uint32_t socd_blink_last = 0;

// lighting.c no longer defines keyboard_post_init_user (it's provided by keymap)

void set_layer3_override(bool on) {
	layer3_override = on;
}

void set_caps_override(bool on) {
	caps_override = on;
}

void start_led_pulse(uint8_t led_index) {
	pulse_active = true;
	pulse_led = led_index;
	pulse_start = timer_read32();
}

void trigger_socd_blink(bool enabled) {
	socd_blink_active = true;
	socd_blink_last = timer_read32();
	socd_blink_count = 0;
	if (enabled) {
		socd_blink_color_r = 0;
		socd_blink_color_g = 0xFF; // green
		socd_blink_color_b = 0;
	} else {
		socd_blink_color_r = 0xFF; // red
		socd_blink_color_g = 0;
		socd_blink_color_b = 0;
	}
}

// This hook runs frequently from the main loop. We use it to enforce
// overrides on the RGB matrix. It returns false to let the RGB matrix
// continue its regular processing after we update pixels.
bool rgb_matrix_indicators_user(void) {
	// Apply caps override: LEDs 82..91 -> white when caps lock is on
	if (caps_override) {
		for (uint8_t i = 82; i <= 91; ++i) {
			rgb_matrix_set_color(i, 0xFF, 0xFF, 0xFF);
		}
	}

	// Layer3 override: LEDs 91..93 in 1-based indexing (92..94 zero-based)
	if (layer3_override) {
		// enforce color 0x00AAFF on last three LEDs (indices 91,92,93)
		rgb_matrix_set_color(91, 0x00, 0xAA, 0xFF);
		rgb_matrix_set_color(92, 0x00, 0xAA, 0xFF);
		rgb_matrix_set_color(93, 0x00, 0xAA, 0xFF);
	}

	// Pulse animation handling
	if (pulse_active) {
		uint32_t now = timer_read32();
		uint32_t elapsed = now - pulse_start;
		if (elapsed >= PULSE_MS) {
			pulse_active = false; // done
		} else {
			// simple ease-out fade: scale from 255 -> 0
			float t = 1.0f - (float)elapsed / (float)PULSE_MS;
			uint8_t v = (uint8_t)(t * 255.0f);
			rgb_matrix_set_color(pulse_led, v, v, v);
		}
	}

	// SOCD blink handling: blink twice then stop
	if (socd_blink_active) {
		uint32_t now = timer_read32();
		if (timer_elapsed32(socd_blink_last) > 250) {
			// toggle blink for leds 82..91 (ambient strip)
			bool on = ((socd_blink_count % 2) == 0);
			for (uint8_t i = 82; i <= 91; ++i) {
				if (on) rgb_matrix_set_color(i, socd_blink_color_r, socd_blink_color_g, socd_blink_color_b);
				else rgb_matrix_set_color(i, 0, 0, 0);
			}
			socd_blink_last = now;
			socd_blink_count++;
			if (socd_blink_count >= 4) {
				socd_blink_active = false;
			}
		}
	}

	return false; // allow other rgb effects to run
}
