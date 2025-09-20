cmake_minimum_required(VERSION 3.12)

if( PLATFORM_NAME STREQUAL "pi-zero-2w" )
    add_subdirectory(pi_zero_2w)
elseif( PLATFORM_NAME STREQUAL "pico" OR PLATFORM_NAME STREQUAL "picow" )
    add_subdirectory(pico)
else()
    message("-- MQTT_MNG: platform ${PLATFORM_NAME} not supported.")
endif()
