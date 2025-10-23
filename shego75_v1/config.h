/* config.h - keyboard configuration for shego75_v1 */
#pragma once

// UART configuration - Dual UART setup
// UART0 (GP0/GP1): Debug output
// UART1 (GP8/GP9): ESP32 communication
// Note: Pin configuration is handled in uart.c

// Encoder switch (optional - not part of standard encoder config)
#ifndef ENCODER_SW_PIN
#define ENCODER_SW_PIN GP4
#endif
// #define ENCODER_DIRECTION_FLIP