#include "lighting/lighting_engine.h"

#include <string.h>

#define SHEGO_LED_BUFFER_MAX 256

typedef struct {
    const board_config_t *cfg;
    uint16_t led_count;
    uint8_t pixels[SHEGO_LED_BUFFER_MAX][3];
} lighting_state_t;

static lighting_state_t g_state;

void lighting_engine_init(const board_config_t *cfg) {
    g_state.cfg = cfg;
    g_state.led_count = cfg->total_leds;
    if (g_state.led_count > SHEGO_LED_BUFFER_MAX) {
        g_state.led_count = SHEGO_LED_BUFFER_MAX;
    }
    memset(g_state.pixels, 0, sizeof(g_state.pixels));
}

void lighting_engine_task(void) {
    // TODO: push buffered colors into the actual LED drivers (PIO, SPI, etc.).
}

void lighting_engine_set_pixel(uint16_t index, uint8_t r, uint8_t g, uint8_t b) {
    if (index >= g_state.led_count) {
        return;
    }
    g_state.pixels[index][0] = r;
    g_state.pixels[index][1] = g;
    g_state.pixels[index][2] = b;
}

uint16_t lighting_engine_led_count(void) {
    return g_state.led_count;
}
