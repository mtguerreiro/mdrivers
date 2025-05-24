cmake_minimum_required(VERSION 3.15)

add_library(my_drivers)

target_sources(my_drivers
    PRIVATE
    ds18b20/ds18b20.c
)

target_sources(my_drivers
    PRIVATE
    led/led.c
)

target_sources(my_drivers
    PRIVATE
    modbus/modbushandle.c
    modbus/nanomodbus.c
)

target_sources(my_drivers
    PRIVATE
    rp/rp.c
)

target_sources(my_drivers
    PRIVATE
    temperature/temperature.c
)

target_sources(my_drivers
    PRIVATE
    wiznet/dhcp.c 
    wiznet/dns.c 
    wiznet/socket.c
    wiznet/w5500.c 
    wiznet/wizchip_conf.c
    wiznet/tcp_server_echo.c
    wiznet/tcp_server.c
)

