cmake_minimum_required(VERSION 3.12)

add_library(led 
led.c
ledIf.c
)

target_link_libraries(led rp)

target_include_directories(led PRIVATE "${TIF_PATH}")
#target_include_directories(led PUBLIC "${PROJECT_SOURCE_DIR}")