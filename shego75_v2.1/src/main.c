#include "pico/stdlib.h"

#include "board/board_config.h"
#include "board/board_hw.h"
#include "lighting/lighting_engine.h"
#include "protocol/gif/gif_bridge.h"
#include "protocol/via/via_protocol.h"
#include "protocol/signalrgb/signalrgb_patched.h"
#include "usb/usb_device.h"

int main(void) {
    stdio_init_all();

    const board_config_t *cfg = board_config_get();
    board_gpio_init(cfg);
    lighting_engine_init(cfg);
    via_protocol_init(cfg);
    gif_bridge_init(cfg);
    signalrgb_init(cfg);
    usb_device_init(cfg);

    while (true) {
        board_gpio_task(cfg);
        lighting_engine_task();
        gif_bridge_task();
        usb_device_task();
        tight_loop_contents();
    }
}
