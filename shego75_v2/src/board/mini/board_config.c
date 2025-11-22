#include "board/board_config.h"

static const board_config_t BOARD_CFG = {
    .name = "Shego Mini",
    .usb_vid = 0xDEAD,
    .usb_pid = 0x1616,
    .total_leds = 16,
    .matrix_rows = 4,
    .matrix_cols = 4,
    .gif_i2c_address = 0x3C,
    .gif_chunk_size = 256,
    .has_encoders = false,
    .has_hall_effect = false,
    .signalrgb_uid = {0x16, 0x00, 0x16},
};

const board_config_t *board_config_get(void) {
    return &BOARD_CFG;
}
