#pragma once

#include <stdint.h>

#include "board/board_config.h"

void gif_bridge_init(const board_config_t *cfg);
void gif_bridge_task(void);
void gif_bridge_handle_chunk(const uint8_t *data, uint16_t length);
