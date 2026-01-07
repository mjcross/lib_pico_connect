add_library(lib_connect INTERFACE)

target_sources(lib_connect INTERFACE
    connect.c
    )

target_link_libraries(lib_connect INTERFACE
    pico_stdlib
    pico_cyw43_arch_lwip_threadsafe_background
    )