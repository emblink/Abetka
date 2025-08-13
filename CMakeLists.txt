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
set(sdkVersion 2.1.1)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.1.1)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)
set(PICO_EXTRAS_PATH ../lib/pico-extras)
include(pico_extras_import.cmake)

project(Abetka C CXX ASM)

# Set debug build type
set(CMAKE_BUILD_TYPE Debug)

# Use deoptimized debug mode
set(PICO_DEOPTIMIZED_DEBUG 1)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(Abetka
    src/mfrc522.c
    src/Abetka.cpp
    src/ws2812.c
    src/st7789.c
    src/lv_font_ukrainian_48.c
    src/appMode.c
    src/appModeIdle.c
    src/appModeRead.c
    src/appModeWrite.c
    src/mifareHal.c
    src/keyScan.c
    src/uaSymbol.c
    src/battery.c
    usb_mass_storage/hw_config.c
    usb_mass_storage/msc_disk.c
    usb_mass_storage/usb_descriptors.c
    src/sdAudio.c
    src/sdCard.c
    src/pio_spi.c
)

file(MAKE_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/generated)

# generate the header file into the source tree as it is included in the RP2040 datasheet
pico_generate_pio_header(Abetka ${CMAKE_CURRENT_LIST_DIR}/src/ws2812.pio OUTPUT_DIR ${CMAKE_CURRENT_LIST_DIR}/generated)
pico_generate_pio_header(Abetka ${CMAKE_CURRENT_LIST_DIR}/src/i2s.pio OUTPUT_DIR ${CMAKE_CURRENT_LIST_DIR}/generated)
pico_generate_pio_header(Abetka ${CMAKE_CURRENT_LIST_DIR}/src/spi.pio OUTPUT_DIR ${CMAKE_CURRENT_LIST_DIR}/generated)

pico_set_program_name(Abetka "Abetka")
pico_set_program_version(Abetka "0.1")
# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(Abetka 0)
pico_enable_stdio_usb(Abetka 1)

# Add the standard library to the build
target_link_libraries(Abetka
        pico_stdlib
        lvgl
        no-OS-FatFS-SD-SDIO-SPI-RPi-Pico
)

# Add the standard include files to the build
include_directories(Abetka PUBLIC
    lib/no-OS-FatFS-SD-SDIO-SPI-RPi-Pico/src/ff15/source
    lib/no-OS-FatFS-SD-SDIO-SPI-RPi-Pico/src/include
)

target_include_directories(Abetka PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}
        lvgl
        src
        usb_mass_storage
)

target_compile_options(Abetka PRIVATE -O0 -g)

# Add any user requested libraries
target_link_libraries(Abetka
        hardware_spi
        hardware_dma
        hardware_pio
        hardware_adc
        tinyusb_additions
        tinyusb_board
        tinyusb_device
         pico_audio_i2s
        )

target_compile_definitions(Abetka PRIVATE
    PICO_AUDIO_I2S_MONO_INPUT=1
    PICO_AUDIO_I2S_MONO_OUTPUT=1
    USE_AUDIO_I2S=1
    PICO_AUDIO_I2S_PIO=1
)

pico_add_extra_outputs(Abetka)

set(LV_CONF_INCLUDE_SIMPLE OFF)
add_subdirectory(lib/lvgl)
add_subdirectory(lib/no-OS-FatFS-SD-SDIO-SPI-RPi-Pico/src)