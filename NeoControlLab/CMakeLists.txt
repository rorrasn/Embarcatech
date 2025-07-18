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
set(PICO_BOARD pico_w CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(NeoControlLab C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(NeoControlLab NeoControlLab.c testes_cores.c LabNeoPixel/util.c LabNeoPixel/neopixel_driver.c LabNeoPixel/efeitos.c efeito_curva_ar.c numeros_neopixel.c)

pico_set_program_name(NeoControlLab "NeoControlLab")
pico_set_program_version(NeoControlLab "0.1")

# Generate PIO header
pico_generate_pio_header(NeoControlLab ${CMAKE_CURRENT_LIST_DIR}/LabNeoPixel/ws2818b.pio)

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(NeoControlLab 0)
pico_enable_stdio_usb(NeoControlLab 1)

# Add the standard library to the build
target_link_libraries(NeoControlLab pico_stdlib hardware_pio hardware_clocks hardware_adc hardware_dma hardware_pwm)

# Add the standard include files to the build
target_include_directories(NeoControlLab PRIVATE ${CMAKE_CURRENT_LIST_DIR} ${CMAKE_CURRENT_LIST_DIR}/LabNeoPixel)

# Add any user requested libraries
#target_link_libraries(NeoControlLab)

pico_add_extra_outputs(NeoControlLab)

