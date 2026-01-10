#ifndef _CONNECT_H
#define _CONNECT_H

#include "pico/async_context.h"
#include "sntp.h"   // include public API for SNTP

// public API
extern volatile bool network_is_up;
extern async_context_t *ctx;

void connect();
void publish_mqtt(const char *topic, const void *payload, uint16_t payload_length);

#endif // CONNECT_H