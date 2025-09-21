cmake_minimum_required(VERSION 3.15)

target_sources(
    my_drivers
    PRIVATE
    temperature.c
    led.c
)
