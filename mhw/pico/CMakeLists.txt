cmake_minimum_required(VERSION 3.15)

add_library(my_hw)

target_sources(
    my_hw
    PRIVATE
    temperatureHw.c
    pwm_irq_handler.c
    onewirehl.c
    wiznet_init.c
)

target_sources(
    my_hw
    PRIVATE
    led/ledws2812.c
)

target_link_libraries(
    my_hw
    PRIVATE
    pico_stdlib
    hardware_pwm
    hardware_spi
    pico_unique_id
    hardware_pio 
    my_drivers
)

target_include_directories(
    my_hw
    PRIVATE
    "${MDRIVERS_PATH}"
    ./
)

pico_generate_pio_header(my_hw ${CMAKE_CURRENT_LIST_DIR}/ws2812.pio OUTPUT_DIR ${CMAKE_CURRENT_LIST_DIR}/generated)
