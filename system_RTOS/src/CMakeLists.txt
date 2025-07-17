add_executable(system_RTOS
    system_RTOS.c
    pwm.c
)

# Corrige a saída para build/ em vez de build/src/
set_target_properties(system_RTOS PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

target_sources(system_RTOS
    PRIVATE
    ${FREERTOS_KERNEL_PATH}/portable/MemMang/heap_4.c
)

target_include_directories(system_RTOS PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/../config
    ${CMAKE_CURRENT_LIST_DIR}/../tasks
    ${CMAKE_CURRENT_LIST_DIR}/../drivers
)

target_link_libraries(system_RTOS 
    pico_stdlib 
    FreeRTOS-Kernel
    hardware_adc
    hardware_pwm
)

pico_add_extra_outputs(system_RTOS)
