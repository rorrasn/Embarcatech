add_executable(semaf_RTOS
    semaf_RTOS.c
)

# Corrige a saída para build/ em vez de build/src/
set_target_properties(semaf_RTOS PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

target_sources(semaf_RTOS
    PRIVATE
    ${FREERTOS_KERNEL_PATH}/portable/MemMang/heap_4.c
)

target_include_directories(semaf_RTOS PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/../config
    ${CMAKE_CURRENT_LIST_DIR}/../tasks
    ${CMAKE_CURRENT_LIST_DIR}/../drivers
)

target_link_libraries(semaf_RTOS 
    pico_stdlib 
    FreeRTOS-Kernel
)

pico_add_extra_outputs(semaf_RTOS)
