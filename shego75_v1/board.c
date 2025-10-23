// board.c - Early hardware initialization for RP2040
// This runs BEFORE QMK's keyboard initialization

#include "quantum.h"
#include "hardware/gpio.h"

// ESP_RESET_PIN (GP3) - Critical: must be HIGH during boot to avoid bootloader mode
#ifndef ESP_RESET_PIN
#define ESP_RESET_PIN 3
#endif

// This function is called very early in the boot process, before any QMK initialization
void early_hardware_init_pre_platform(void) {
    // CRITICAL: GP3 is a bootloader trigger pin on RP2040
    // If GP3 is LOW during reset/power-on, the RP2040 enters bootloader mode
    // Set it as OUTPUT HIGH immediately to prevent this
    
    gpio_init(ESP_RESET_PIN);
    gpio_set_dir(ESP_RESET_PIN, GPIO_OUT);
    gpio_put(ESP_RESET_PIN, 1);  // Drive HIGH
}
