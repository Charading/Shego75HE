# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.2.0)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.2.0-a4)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    cmake_minimum_required(VERSION 3.13)

    set(CMAKE_C_STANDARD 11)
    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

    # == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
    if(WIN32)
        set(USERHOME $ENV{USERPROFILE})
    else()
        set(USERHOME $ENV{HOME})
    endif()
    set(sdkVersion 2.2.0)
    set(toolchainVersion 14_2_Rel1)
    set(picotoolVersion 2.2.0-a4)
    set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
    if (EXISTS ${picoVscode})
        include(${picoVscode})
    endif()
    # ====================================================================================
    set(PICO_BOARD pico CACHE STRING "Board type")

    include(pico_sdk_import.cmake)

    project(shego75_v2.1 C CXX ASM)

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

        pico_enable_stdio_usb(${TARGET_NAME} 0)
        pico_enable_stdio_uart(${TARGET_NAME} 0)
        pico_add_extra_outputs(${TARGET_NAME})
    endfunction()

    shego_add_board(shego75_v2_1 src/board/shego75/board_config.c)
    shego_add_board(shego_mini_dev src/board/mini/board_config.c)

