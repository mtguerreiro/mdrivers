add_executable(ds18b20 main.c)

add_subdirectory(tasks)

# pull in common dependencies
target_link_libraries(
    ds18b20
    tasks
    pico_stdlib
    FreeRTOS-Kernel FreeRTOS-Kernel-Heap4
    mdrivers)

target_include_directories(
    ds18b20
    PRIVATE
   ${MDRIVERS_PATH}
)

# enable usb output, disable uart output
pico_enable_stdio_usb(ds18b20 1)
pico_enable_stdio_uart(ds18b20 0)

# create map/bin/hex/uf2 file etc.
pico_add_extra_outputs(ds18b20)

# add url via pico_set_program_url
example_auto_set_url(ds18b20)

