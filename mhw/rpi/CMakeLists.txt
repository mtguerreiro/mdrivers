cmake_minimum_required(VERSION 3.15)

add_library(my_hw)

target_sources(
    my_hw
    PRIVATE
    ledpwm.c
)

target_link_libraries(
    my_hw
    PRIVATE
    rt
    pigpio
)

target_include_directories(
    my_hw
    PUBLIC
    ./
)
