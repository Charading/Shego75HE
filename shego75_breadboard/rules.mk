# MCU and bootloader
MCU = RP2040
BOOTLOADER = rp2040

# Features
CONSOLE_ENABLE = yes
ENCODER_MAP_ENABLE = yes

# Use extended matrix scanning
CUSTOM_MATRIX = lite
SRC += mux_adc.c
SRC += mux_pins.c
SRC += uart.c
SRC += uart_keycodes.c
SRC += socd.c
SRC += lighting.c
SRC += uart_commands.c

# Analog driver for RP2040
ANALOG_DRIVER_REQUIRED = yes
ANALOG_DRIVER = rp2040_adc

# Optimizations
LTO_ENABLE = yes