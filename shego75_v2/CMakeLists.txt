cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif ()

set(PICO_BOARD pico CACHE STRING "Pico board")

include(pico_sdk_import.cmake)

project(shego75_v2 C CXX ASM)

pico_sdk_init()

set(COMMON_SOURCES
    src/main.c
    src/board/board_hw.c
    src/lighting/lighting_engine.c
    src/protocol/signalrgb/signalrgb_patched.c
    src/protocol/via/via_protocol.c
    src/protocol/gif/gif_bridge.c
    src/usb/usb_device.c
    src/usb/usb_descriptors.c
    src/usb/raw_hid.c
)

function(shego_add_board TARGET_NAME BOARD_CONFIG_SRC)
    add_executable(${TARGET_NAME}
        ${COMMON_SOURCES}
        ${BOARD_CONFIG_SRC}
    )

    target_include_directories(${TARGET_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/src
        ${CMAKE_CURRENT_LIST_DIR}/src/usb
    )

    target_link_libraries(${TARGET_NAME}
        pico_stdlib
        pico_unique_id
        hardware_flash
        hardware_i2c
        hardware_pwm
        hardware_spi
        tinyusb_board
        tinyusb_device
    )

    target_compile_definitions(${TARGET_NAME} PRIVATE
        PICO_XOSC_STARTUP_DELAY_MULTIPLIER=64
    )

    pico_enable_stdio_usb(${TARGET_NAME} 0)
    pico_enable_stdio_uart(${TARGET_NAME} 0)
    pico_add_extra_outputs(${TARGET_NAME})
endfunction()

shego_add_board(shego75_v2 src/board/shego75/board_config.c)
shego_add_board(shego_mini_dev src/board/mini/board_config.c)
