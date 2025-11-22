#include "board/board_config.h"

static const board_config_t BOARD_CFG = {
    .name = "Shego75 v2",
    .usb_vid = 0xDEAD,
    .usb_pid = 0x0444,
    .total_leds = 120,
    .matrix_rows = 6,
    .matrix_cols = 16,
    .gif_i2c_address = 0x3C,
    .gif_chunk_size = 512,
    .has_encoders = true,
    .has_hall_effect = true,
    .signalrgb_uid = {0x75, 0x00, 0x02},
};

const board_config_t *board_config_get(void) {
    return &BOARD_CFG;
}
