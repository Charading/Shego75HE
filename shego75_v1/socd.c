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
                register_code(KC_A);
                return false; // handled manually
            }
            a_suppressed = false;
            register_code(KC_A);
            return false; // handled manually
        } else {
            // A released
            a_pressed = false;
            if (a_suppressed) {
                // A was suppressed, so it was never sent - don't send release
                a_suppressed = false;
                return false;
            }
            // A was active. Release it and reassert D if needed
            unregister_code(KC_A);
            if (d_pressed && d_suppressed) {
                register_code(KC_D);
                d_suppressed = false;
            }
            return false; // handled manually
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
                register_code(KC_D);
                return false; // handled manually
            }
            d_suppressed = false;
            register_code(KC_D);
            return false; // handled manually
        } else {
            d_pressed = false;
            if (d_suppressed) {
                // D was suppressed, so it was never sent - don't send release
                d_suppressed = false;
                return false;
            }
            // D was active. Release it and reassert A if needed
            unregister_code(KC_D);
            if (a_pressed && a_suppressed) {
                register_code(KC_A);
                a_suppressed = false;
            }
            return false; // handled manually
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
                register_code(KC_W);
                return false; // handled manually
            }
            w_suppressed = false;
            register_code(KC_W);
            return false; // handled manually
        } else {
            w_pressed = false;
            if (w_suppressed) {
                // W was suppressed, so it was never sent - don't send release
                w_suppressed = false;
                return false;
            }
            // W was active. Release it and reassert S if needed
            unregister_code(KC_W);
            if (s_pressed && s_suppressed) {
                register_code(KC_S);
                s_suppressed = false;
            }
            return false; // handled manually
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
                register_code(KC_S);
                return false; // handled manually
            }
            s_suppressed = false;
            register_code(KC_S);
            return false; // handled manually
        } else {
            s_pressed = false;
            if (s_suppressed) {
                // S was suppressed, so it was never sent - don't send release
                s_suppressed = false;
                return false;
            }
            // S was active. Release it and reassert W if needed
            unregister_code(KC_S);
            if (w_pressed && w_suppressed) {
                register_code(KC_W);
                w_suppressed = false;
            }
            return false; // handled manually
        }
    }

    // Not handled here
    return true;
}
