# SPDX-License-Identifier: GPL-3.0-or-later
# Raspberry Pi Pico 2 W (RP2350) firmware for HondaOBD.
#
# Build (once the Pico SDK is installed and PICO_SDK_PATH is set):
#   - copy pico_sdk_import.cmake from the SDK into this directory
#   - set(PICO_BOARD pico2_w)
#   - mkdir build && cd build && cmake .. && make
#
# STATUS: first draft; not yet compiled (no SDK/hardware present during authoring).

cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(hondaobd_firmware C CXX ASM)
set(CMAKE_C_STANDARD 11)

pico_sdk_init()

add_executable(hondaobd
    src/main.c
    src/iso9141.c
    src/honda_frame.c
    src/hal_pico.c
)

target_include_directories(hondaobd PRIVATE src)
target_link_libraries(hondaobd pico_stdlib hardware_uart)

# Logs over USB CDC, not the hardware UART (which the K-line needs).
pico_enable_stdio_usb(hondaobd 1)
pico_enable_stdio_uart(hondaobd 0)

pico_add_extra_outputs(hondaobd)
