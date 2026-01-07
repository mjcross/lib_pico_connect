#include <stdio.h>
#include "pico/stdlib.h"
#include "lib_connect/connect.h"

int main()
{
    stdio_init_all();

    puts("connecting");
    connect();

    while(true) sleep_ms(1000);
}
