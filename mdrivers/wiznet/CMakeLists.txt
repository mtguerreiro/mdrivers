cmake_minimum_required(VERSION 3.12)

add_library(wiznet dhcp.c dns.c socket.c w5500.c wizchip_conf.c tcp_server_echo.c tcp_server.c)

#target_link_libraries(wiznet)

#target_include_directories(wiznet PUBLIC "${PROJECT_SOURCE_DIR}")
