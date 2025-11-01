# MCU and bootloader
MCU = RP2040
BOOTLOADER = rp2040

# Features
# Temporarily disable USB console while debugging RAW HID interface so the
# console HID interface doesn't compete with the RAW HID endpoint.
CONSOLE_ENABLE = yes
RAW_ENABLE = yes           # Enable RAW HID for Electron app communication
ENCODER_MAP_ENABLE = yes
I2C_DRIVER_REQUIRED = yes  # Enable I2C for ESP32 communication

# Use extended matrix scanning
CUSTOM_MATRIX = lite
SRC += mux_adc.c
SRC += mux_pins.c
SRC += uart.c
SRC += uart_keycodes.c
SRC += socd.c
SRC += lighting.c
SRC += uart_commands.c
SRC += hid_reports.c
SRC += i2c_esp32.c 
SRC += hid_reports.c
SRC += vendor_bridge.c

# Analog driver for RP2040
ANALOG_DRIVER_REQUIRED = yes
ANALOG_DRIVER = rp2040_adc

# Optimizations
LTO_ENABLE = yes

# Enable VIA and persistent dynamic keymaps so VIA can save layout changes to EEPROM
DYNAMIC_KEYMAP_ENABLE = yes