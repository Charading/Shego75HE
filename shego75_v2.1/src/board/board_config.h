#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    const char *name;
    uint16_t usb_vid;
    uint16_t usb_pid;
    uint16_t total_leds;
    uint8_t matrix_rows;
    uint8_t matrix_cols;
    uint8_t gif_i2c_address;
    uint16_t gif_chunk_size;
    bool has_encoders;
    bool has_hall_effect;
    uint8_t signalrgb_uid[3];
} board_config_t;

const board_config_t *board_config_get(void);

void board_gpio_init(const board_config_t *cfg);
void board_gpio_task(const board_config_t *cfg);
