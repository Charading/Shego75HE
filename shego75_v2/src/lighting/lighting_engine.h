#pragma once

#include <stdint.h>

#include "board/board_config.h"

void lighting_engine_init(const board_config_t *cfg);
void lighting_engine_task(void);
void lighting_engine_set_pixel(uint16_t index, uint8_t r, uint8_t g, uint8_t b);
uint16_t lighting_engine_led_count(void);
