#ifndef _CONNECT_H
#define _CONNECT_H

#include "pico/cyw43_arch.h"

// public API
extern volatile bool network_is_up;
extern async_context_t *ctx;
void connect();

#endif // CONNECT_H