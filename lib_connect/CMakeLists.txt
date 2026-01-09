add_library(lib_connect INTERFACE)

target_sources(lib_connect INTERFACE
    connect.c
    mqtt.c
    sntp.c
    )

target_link_libraries(lib_connect INTERFACE
    pico_stdlib
    pico_cyw43_arch_lwip_threadsafe_background
    pico_lwip_mqtt
    pico_lwip_sntp
    pico_aon_timer
    pico_sync
    )