cmake_minimum_required(VERSION 3.15)

add_library(mdrivers INTERFACE)

#add_subdirectory(mdrivers)
add_subdirectory(mhw)

target_link_libraries(mdrivers
    INTERFACE
    #my_drivers
    my_hw
)
