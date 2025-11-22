#include "board/board_config.h"

#include "pico/stdlib.h"

void board_gpio_init(const board_config_t *cfg) {
    (void)cfg;
    // TODO: configure matrix rows/cols, encoders, hall sensors, etc.
}

void board_gpio_task(const board_config_t *cfg) {
    (void)cfg;
    // TODO: scan matrix, read encoders, hall sensors, and push events into the HID pipeline.
}
