#include "lighting.h"
#include "quantum.h"
#include "rgb_matrix.h"
#include "uart_keycodes.h"

// State for overrides and simple animations
static bool layer3_override = false;
static bool caps_override = false;

// Pulse state
static bool pulse_active = false;
static uint8_t pulse_led = 0;
static uint32_t pulse_start = 0;
static const uint32_t PULSE_MS = 600;

// SOCD blink state (new pattern: 50ms on, 50ms off, 2000ms on, then stop)
static bool socd_blink_active = false;
static uint8_t socd_blink_color_r = 0;
static uint8_t socd_blink_color_g = 0;
static uint8_t socd_blink_color_b = 0;
static uint8_t socd_blink_step = 0;
static uint32_t socd_blink_last = 0;
static const uint32_t socd_blink_timings[] = { 50, 50, 2000 }; // ms
static const uint8_t socd_blink_steps = sizeof(socd_blink_timings) / sizeof(socd_blink_timings[0]);

// Helper: clear ambient indices (82..91) respecting layer3_override
// Forward declaration for set_led used by helpers below
static void set_led(uint8_t idx, uint8_t r, uint8_t g, uint8_t b);

// Helper: clear status bar indices (91..93)
static void clear_status_bar(void) {
    for (uint8_t i = 91; i <= 93; ++i) {
        set_led(i, 0, 0, 0);
    }
}

// lighting.c no longer defines keyboard_post_init_user (it's provided by keymap)

// Store previous colors so we can restore after temporary overrides.
static uint8_t saved_colors[94][3];
static bool saved_valid = false;

// Maintain a local cache of the last colors we set so we can restore them.
static uint8_t current_colors[94][3];

static void set_led(uint8_t idx, uint8_t r, uint8_t g, uint8_t b) {
    if (idx < 94) {
        current_colors[idx][0] = r;
        current_colors[idx][1] = g;
        current_colors[idx][2] = b;
        rgb_matrix_set_color(idx, r, g, b);
    }
}

// Snapshot current colors into saved_colors. We approximate by reading the
// rgb_matrix led buffer if available; if not possible, we just mark saved_valid
// and let restores clear to black (best-effort). QMK's RGB API does not provide
// a portable getter for per-LED color, so this is best-effort.
static void snapshot_colors(void) {
    // Copy from our local cache of colors we've set via set_led(). This
    // allows restore_colors() to replay the last known state our code wrote.
    for (int i = 0; i < 94; ++i) {
        saved_colors[i][0] = current_colors[i][0];
        saved_colors[i][1] = current_colors[i][1];
        saved_colors[i][2] = current_colors[i][2];
    }
    saved_valid = true;
}

static void restore_colors(void) {
    if (!saved_valid) {
        // best-effort clear (so effects can repaint); caller may then let
        // rgb effects re-run and paint the strip
        for (int i = 0; i < 94; ++i) {
            set_led(i, 0, 0, 0);
        }
        return;
    }
    for (int i = 0; i < 94; ++i) {
        set_led(i, saved_colors[i][0], saved_colors[i][1], saved_colors[i][2]);
    }
}

void set_layer3_override(bool on) {
    layer3_override = on;
    if (on) {
        // Snapshot existing colors, cancel any ongoing SOCD blink or pulses
        snapshot_colors();
        socd_blink_active = false;
        pulse_active = false;
    } else {
        // When disabling layer3, restore colors so regular effects resume
        restore_colors();
        clear_status_bar();
    }
}

// Initialize lighting state to a sane baseline. Call from keyboard_post_init_user.
void lighting_init(void) {
    // baseline dim red (~5% of 255 -> 13)
    const uint8_t r = 13, g = 0, b = 0;
    for (int i = 0; i < 94; ++i) {
        current_colors[i][0] = r;
        current_colors[i][1] = g;
        current_colors[i][2] = b;
        saved_colors[i][0] = r;
        saved_colors[i][1] = g;
        saved_colors[i][2] = b;
        rgb_matrix_set_color(i, r, g, b);
    }
    saved_valid = true;
}

void set_caps_override(bool on) {
    caps_override = on;
}

void start_led_pulse(uint8_t led_index) {
    // Do not allow pulses to target the status bar while layer3 override is active
    if (layer3_override && (led_index >= 91 && led_index <= 93)) return;
    snapshot_colors();
    pulse_active = true;
    pulse_led = led_index;
    pulse_start = timer_read32();
}

void trigger_socd_blink(bool enabled) {
    // If layer3 override is active, do not start a SOCD blink
    if (layer3_override) return;
    snapshot_colors();
    socd_blink_active = true;
    socd_blink_last = timer_read32();
    socd_blink_step = 0;
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
// overrides on the RGB matrix. Returning true blocks other RGB effects,
// returning false lets other effects run.
bool rgb_matrix_indicators_user(void) {
    // If Layer 3 override is active, always force last three LEDs to solid color
    // but do not block effects on the ambient strip (82..91). This ensures the
    // status bar (91..93) remains solid while ambient LEDs can still show
    // debug or SOCD patterns.
    if (layer3_override) {
           set_led(91, 0x00, 0xAA, 0xFF);
           set_led(92, 0x00, 0xAA, 0xFF);
           set_led(93, 0x00, 0xAA, 0xFF);
        // do not return; allow ambient (82..91) code to run
    }

    // If any debug printing is enabled, force a marquee on ambient strip 82..91
    // alternating #FFC400 and off every 750ms. This takes precedence over SOCD
    // and caps when active.
    bool debug_any = get_adc_debug_enabled() || get_key_debug_enabled() || get_raw_debug_enabled();
    static bool prev_debug_any = false;
    if (debug_any) {
        const uint32_t MARQUEE_MS = 750;
        static uint32_t marquee_last = 0;
        static bool marquee_phase = false;
        uint32_t now = timer_read32();
        if (timer_elapsed32(marquee_last) >= MARQUEE_MS) {
            marquee_last = now;
            marquee_phase = !marquee_phase;
        }
        // When layer3 is active, alternate between layer3 color (0x00AAFF)
        // and the debug color (#FFC400). Otherwise alternate between #FFC400
        // and OFF as before.
        const uint8_t debug_r = 0xFF, debug_g = 0xC4, debug_b = 0x00;
        const uint8_t l3_r = 0x00, l3_g = 0xAA, l3_b = 0xFF;

        // If layer3_override is active, do not write the status-bar indices
        // (91..93) so they remain solid. Ambient end index depends on that.
        uint8_t ambient_end = layer3_override ? 90 : 91;

            for (uint8_t i = 82; i <= ambient_end; ++i) {
                bool phase_bit = ((i & 1) == (marquee_phase ? 0 : 1));
                if (layer3_override) {
                    // alternate between layer3 color and debug color
                    if (phase_bit) set_led(i, l3_r, l3_g, l3_b);
                    else set_led(i, debug_r, debug_g, debug_b);
                } else {
                    // alternate between debug color and off
                    if (phase_bit) set_led(i, debug_r, debug_g, debug_b);
                    else set_led(i, 0, 0, 0);
                }
            }
        // snapshot once when debug starts
        if (!prev_debug_any) snapshot_colors();
        prev_debug_any = true;
        return true; // prevent other effects from overriding ambient while debugging
    }
    // if debug was active and now stopped, restore previous colors
    if (prev_debug_any && !debug_any) {
        prev_debug_any = false;
        restore_colors();
    }

    // Handle SOCD blink pattern (this takes precedence over ambient caps override)
    if (socd_blink_active) {
        uint32_t now = timer_read32();
        uint32_t elapsed = now - socd_blink_last;
        if (elapsed >= socd_blink_timings[socd_blink_step]) {
            // advance step
            socd_blink_step++;
            socd_blink_last = now;
            if (socd_blink_step >= socd_blink_steps) {
                // finished sequence
                socd_blink_active = false;
                // restore saved colors so other effects resume
                restore_colors();
            }
        }

        // Determine whether current step is "on" or "off"
        // Steps layout: step0 = on(50ms), step1 = off(50ms), step2 = on(2000ms)
            if (socd_blink_active) {
                bool on = (socd_blink_step == 0) || (socd_blink_step == 2);
                for (uint8_t i = 82; i <= 91; ++i) {
                    if (on) set_led(i, socd_blink_color_r, socd_blink_color_g, socd_blink_color_b);
                    else set_led(i, 0, 0, 0);
                }
                return true; // prevent other effects from overriding ambient strip during blink
            }
    }

    // Apply caps override: LEDs 82..91 -> white when caps lock is on
    if (caps_override) {
        for (uint8_t i = 82; i <= 91; ++i) {
            set_led(i, 0xFF, 0xFF, 0xFF);
        }
        // allow other effects to run for other LEDs
    }

    // Pulse animation handling (for single-LED pulse feedback)
    if (pulse_active) {
        uint32_t now = timer_read32();
        uint32_t elapsed = now - pulse_start;
        if (elapsed >= PULSE_MS) {
            pulse_active = false; // done
            // clear the pulse LED so other effects can repaint
            set_led(pulse_led, 0, 0, 0);
        } else {
            // simple ease-out fade: scale from 255 -> 0
            float t = 1.0f - (float)elapsed / (float)PULSE_MS;
            uint8_t v = (uint8_t)(t * 255.0f);
            set_led(pulse_led, v, v, v);
        }
    }

    return false; // allow other rgb effects to run for LEDs we didn't explicitly set
}
