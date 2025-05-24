cmake_minimum_required(VERSION 3.12)

add_library(modbus modbushandle.c nanomodbus.c)

#target_link_libraries(modbus)

#target_include_directories(modbus PUBLIC "${PROJECT_SOURCE_DIR}")
