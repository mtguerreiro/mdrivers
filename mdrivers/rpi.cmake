cmake_minimum_required(VERSION 3.15)

target_sources(
    my_drivers
    PRIVATE
    temperature.c
)

target_sources(
    my_drivers
    PRIVATE
    led.c
)

target_sources(
    my_drivers
    PRIVATE
    modbus/modbushandle.c
    modbus/nanomodbus.c
)
