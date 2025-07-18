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
set(sdkVersion 1.5.1)
set(toolchainVersion 13_2_Rel1)
set(picotoolVersion 2.0.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico_w CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(tarefa_3 C CXX ASM)
set(CMAKE_C_STANDARD 11)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(tarefa_3 
        tarefa_3.c 
        blink.pio
        )

pico_set_program_name(tarefa_3 "tarefa_3")
pico_set_program_version(tarefa_3 "0.1")

# Generate PIO header
pico_generate_pio_header(tarefa_3 ${CMAKE_CURRENT_LIST_DIR}/blink.pio)

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(tarefa_3 0)
pico_enable_stdio_usb(tarefa_3 1)

# Add the standard library to the build
target_link_libraries(tarefa_3
        pico_stdlib
        hardware_adc
        )

# Add the standard include files to the build
target_include_directories(tarefa_3 PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
)

# Add any user requested libraries
target_link_libraries(tarefa_3 
        hardware_pio
        hardware_timer
        
        )

pico_add_extra_outputs(tarefa_3)

