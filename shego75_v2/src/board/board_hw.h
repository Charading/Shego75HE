#pragma once

#include "board/board_config.h"

void board_gpio_init(const board_config_t *cfg);
void board_gpio_task(const board_config_t *cfg);
