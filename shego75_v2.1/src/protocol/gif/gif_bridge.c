#include "protocol/gif/gif_bridge.h"

#include <string.h>

#include "hardware/i2c.h"

#define GIF_BUFFER_SIZE 512

static const board_config_t *g_cfg;
static uint8_t g_chunk_buffer[GIF_BUFFER_SIZE];
static uint16_t g_chunk_length;

void gif_bridge_init(const board_config_t *cfg) {
    g_cfg = cfg;
    g_chunk_length = 0;
}

void gif_bridge_task(void) {
    if (!g_chunk_length) {
        return;
    }

    // TODO: forward buffered GIF data across I2C to the ESP32 display controller.
    g_chunk_length = 0;
}

void gif_bridge_handle_chunk(const uint8_t *data, uint16_t length) {
    if (length > GIF_BUFFER_SIZE) {
        length = GIF_BUFFER_SIZE;
    }
    memcpy(g_chunk_buffer, data, length);
    g_chunk_length = length;
    (void)g_cfg;
}
