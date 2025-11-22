#pragma once

#include <stdbool.h>
#include <stdint.h>

void raw_hid_init(void);
bool raw_hid_send(const uint8_t *data, uint16_t length);
