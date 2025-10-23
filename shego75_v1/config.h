/* config.h - keyboard configuration for shego75_v1 */
#pragma once

// UART configuration - GP8 = UART1 TX, GP9 = UART1 RX
// RP2040 UART1 pins: TX can be GP4, GP8, GP12, GP16, GP20; RX can be GP5, GP9, GP13, GP17, GP21
#define UART_TX_PIN 8
#define UART_RX_PIN 9
#define UART_DRIVER_INDEX 1  // Use uart1 instead of uart0

// Encoder switch (optional - not part of standard encoder config)
#ifndef ENCODER_SW_PIN
#define ENCODER_SW_PIN GP10
#endif
// #define ENCODER_DIRECTION_FLIP