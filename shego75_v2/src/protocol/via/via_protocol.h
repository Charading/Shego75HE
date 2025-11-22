#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "board/board_config.h"

void via_protocol_init(const board_config_t *cfg);
bool via_protocol_handle_packet(const uint8_t *data, uint16_t length);
