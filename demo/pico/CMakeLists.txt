cmake_minimum_required(VERSION 3.12)

set(PLATFORM_NAME "pico")
set(FREERTOS_KERNEL_PATH "/home/marco/projects/freertos")
set(MDRIVERS_PATH "/home/marco/projects/mdrivers")

# Pull in SDK (must be before project)
include(pico_sdk_import.cmake)
include(${FREERTOS_KERNEL_PATH}/portable/ThirdParty/GCC/RP2040/FreeRTOS_Kernel_import.cmake)

project(pico_examples C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(PICO_DEOPTIMIZED_DEBUG 0)

if (PICO_SDK_VERSION_STRING VERSION_LESS "1.3.0")
    message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.3.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

message("Raspberry Pi Pico SDK version is ${PICO_SDK_VERSION_STRING}")

set(PICO_EXAMPLES_PATH ${PROJECT_SOURCE_DIR})

# Initialize the SDK
pico_sdk_init()
include(example_auto_set_url.cmake)

add_compile_options(-Wall
        -Wno-format          # int != int32_t as far as the compiler is concerned because gcc has int32_t as long int
        -Wno-unused-function # we have some for the docs that aren't called
        -Wno-maybe-uninitialized
        -O2
        )

# Libraries
add_subdirectory("${MDRIVERS_PATH}" "${PROJECT_BINARY_DIR}/mdrivers")

# Add projects
add_subdirectory(ds18b20)
