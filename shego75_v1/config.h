/* config.h - keyboard configuration for shego75_v1 */
#pragma once

// ============================================================================
// AUTO-CALIBRATION CONFIGURATION
// ============================================================================
// The keyboard automatically calibrates sensor thresholds on startup.
// Each key's baseline (resting) value is measured, and actuation occurs when
// the ADC drops below CALIBRATION_THRESHOLD_PERCENT of that baseline.
//
// IMPORTANT: Ensure NO KEYS ARE PRESSED during power-on!
//
// Adjusting sensitivity:
// - Lower percentage (e.g., 80) = MORE sensitive (earlier actuation, lighter touch)
// - Higher percentage (e.g., 90) = LESS sensitive (deeper press required)
//
// Uncomment and adjust the value below to customize:
// #define CALIBRATION_THRESHOLD_PERCENT 85
//
// Default is 85% (15% drop from baseline triggers actuation)
// ============================================================================

// UART configuration - Dual UART setup
// UART0 (GP0/GP1): Debug output
// UART1 (GP8/GP9): ESP32 communication
// Note: Pin configuration is handled in uart.c

// Encoder switch (optional - not part of standard encoder config)
#ifndef ENCODER_SW_PIN
#define ENCODER_SW_PIN GP4
#define ENCODER_A_PIN GP5
#define ENCODER_B_PIN GP6
#endif
// #define ENCODER_DIRECTION_FLIP