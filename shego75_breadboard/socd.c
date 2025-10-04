// socd.c - implement SOCD neutralization for A/D and W/S
#include "socd.h"
#include "uart.h"
#include "wait.h"
#include "lighting.h"
#include "timer.h"

static bool socd_enabled = true;

// track pressed state
static bool a_pressed = false;
static bool d_pressed = false;
static bool w_pressed = false;
static bool s_pressed = false;

// suppressed flags (true when both were neutralized)
static bool a_suppressed = false;
static bool d_suppressed = false;
static bool w_suppressed = false;
static bool s_suppressed = false;

void toggle_socd(void) {
    // Prevent rapid toggles (debounce) — ignore toggles within 1000 ms
    static uint32_t last_toggle_time = 0;
    const uint32_t DEBOUNCE_MS = 1000;
    if (timer_elapsed32(last_toggle_time) < DEBOUNCE_MS) {
        uart_send_string("SOCD: toggle ignored (debounce)\n");
        return;
    }
    last_toggle_time = timer_read32();

    socd_enabled = !socd_enabled;
    if (socd_enabled) uart_send_string("SOCD: Enabled\n");
    else uart_send_string("SOCD: Disabled\n");
    // Trigger lighting blink feedback
    trigger_socd_blink(socd_enabled);
}

bool get_socd_enabled(void) {
    return socd_enabled;
}



bool socd_process_key(uint16_t keycode, bool pressed) {
    // Only handle A/D/W/S
    if (keycode == KC_A) {
        if (pressed) {
            // A pressed now. Last input wins -> if D was down, suppress D and allow A
            a_pressed = true;
            if (socd_enabled && d_pressed) {
                if (!d_suppressed) {
                    unregister_code(KC_D);
                    d_suppressed = true;
                }
                a_suppressed = false;
                return true; // allow A (it wins)
            }
            return true;
        } else {
            // A released. If A had been suppressed previously, just clear flag.
            a_pressed = false;
            if (a_suppressed) {
                a_suppressed = false;
                return false; // nothing to reassert
            }
            // If A was the winning key and D is still held but suppressed, reassert D
            if (d_pressed && d_suppressed) {
                register_code(KC_D);
                d_suppressed = false;
            }
            return true;
        }
    }

    if (keycode == KC_D) {
        if (pressed) {
            // D pressed now. Last input wins -> if A was down, suppress A and allow D
            d_pressed = true;
            if (socd_enabled && a_pressed) {
                if (!a_suppressed) {
                    unregister_code(KC_A);
                    a_suppressed = true;
                }
                d_suppressed = false;
                return true; // allow D
            }
            return true;
        } else {
            d_pressed = false;
            if (d_suppressed) {
                d_suppressed = false;
                return false;
            }
            if (a_pressed && a_suppressed) {
                register_code(KC_A);
                a_suppressed = false;
            }
            return true;
        }
    }

    if (keycode == KC_W) {
        if (pressed) {
            w_pressed = true;
            if (socd_enabled && s_pressed) {
                if (!s_suppressed) {
                    unregister_code(KC_S);
                    s_suppressed = true;
                }
                w_suppressed = false;
                return true;
            }
            return true;
        } else {
            w_pressed = false;
            if (w_suppressed) {
                w_suppressed = false;
                return false;
            }
            if (s_pressed && s_suppressed) {
                register_code(KC_S);
                s_suppressed = false;
            }
            return true;
        }
    }

    if (keycode == KC_S) {
        if (pressed) {
            s_pressed = true;
            if (socd_enabled && w_pressed) {
                if (!w_suppressed) {
                    unregister_code(KC_W);
                    w_suppressed = true;
                }
                s_suppressed = false;
                return true;
            }
            return true;
        } else {
            s_pressed = false;
            if (s_suppressed) {
                s_suppressed = false;
                return false;
            }
            if (w_pressed && w_suppressed) {
                register_code(KC_W);
                w_suppressed = false;
            }
            return true;
        }
    }

    // Not handled here
    return true;
}
