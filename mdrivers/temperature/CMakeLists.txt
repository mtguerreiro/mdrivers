cmake_minimum_required(VERSION 3.12)

add_library(temperature 
temperature.c
temperatureIf.c
)

target_link_libraries(temperature rp)

target_include_directories(temperature PRIVATE "${TIF_PATH}")
#target_include_directories(temperature PUBLIC "${PROJECT_BINARY_DIR}/tif")