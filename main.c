#include <stdio.h>
#include <stdlib.h>     // for setenv()
#include "string.h"     // for strlen()
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

    // set up timestamp
    struct timespec ts;
    struct tm tm;
    char timestamp[20];
    setenv("TZ", POSIX_TIMEZONE, 1);

    while(true) {
        if(aon_timer_is_initialised) {
            get_time_utc(&ts);
            pico_localtime_r(&(ts.tv_sec), &tm);
            strftime(timestamp, sizeof(timestamp), "%a %H:%M:%S", &tm);
        } else {
            snprintf(timestamp, sizeof(timestamp), "--:--:--");
        }
        puts(timestamp);

        // use the library's async-safe MQTT publish function (QoS = 0)
        publish_mqtt("time", timestamp, 1 + strlen(timestamp));

        sleep_ms(5000);
    }
}
