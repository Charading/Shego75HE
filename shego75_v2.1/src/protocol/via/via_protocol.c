#include "protocol/via/via_protocol.h"

#include <string.h>

static const board_config_t *g_cfg;

void via_protocol_init(const board_config_t *cfg) {
    g_cfg = cfg;
}

bool via_protocol_handle_packet(const uint8_t *data, uint16_t length) {
    (void)data;
    (void)length;
    (void)g_cfg;
    // TODO: parse VIA commands (ID 0x01..0x0F) and respond using the VIA packet map.
    return false;
}
