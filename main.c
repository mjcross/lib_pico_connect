#include <stdio.h>
#include "string.h"         // for strlen()
#include "pico/stdlib.h"
#include "lib_connect/connect.h"

#define POSIX_TIMEZONE "BST0GMT,M3.5.0/1,M10.5.0/2"     // POSIX timezone for UK GMT/BST 

// Note: the library requires some additional configuration options
// for lwIP, so be sure to #include lib_connect/extra_lwipopts.h in
// your lwipopts.h file.

int main()
{
    stdio_init_all();

    puts("connecting");
    connect();

    const char *timestamp;
    while(true) {
        if(aon_timer_is_initialised) {
            timestamp = get_timestamp();
            puts(timestamp);
        }

        if(mqtt_is_up) {
            publish_mqtt("time", timestamp, 1 + strlen(timestamp));
        }

        sleep_ms(5000);
    }
}
