#include <stdio.h>
#include "pico/stdlib.h"
#include "lib_connect/connect.h"

//! NOTE: the library requires some extra lwIP options but
//!       a project can only have one lwipopts.h file

int main()
{
    stdio_init_all();

    puts("connecting");
    connect();

    while(true) sleep_ms(1000);
}
