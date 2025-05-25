cmake_minimum_required(VERSION 3.12)

add_library(tasks
    task_blink.c
    task_temperature.c
    )

target_link_libraries(tasks
    pico_stdlib hardware_spi
    FreeRTOS-Kernel FreeRTOS-Kernel-Heap4
    mdrivers
    )

target_include_directories(
    tasks
    PRIVATE 
    "${PROJECT_SOURCE_DIR}" 
    "${MDRIVERS_PATH}"
)
