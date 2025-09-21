cmake_minimum_required(VERSION 3.15)

add_library(my_hw)

target_sources(
    my_hw
    PRIVATE
    led/ledpwm.c
)

target_include_directories(
    my_hw
    PRIVATE
    "${MDRIVERS_PATH}/"
)

target_include_directories(
    my_hw
    PUBLIC
    ./
)
