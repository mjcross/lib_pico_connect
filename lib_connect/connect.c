/**
 * Copyright (c) 2026 mjcross
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "pico/async_context.h"
#include "lwip/netif.h"         // for netif_ext_callback_t
#include "lwip/inet.h"          // for inet_ntoa()

#include "connect.h"
#include "settings.h"
#include "mqtt.h"

// network status flag and network context (globals)
volatile bool network_is_up = false;
async_context_t *ctx = NULL;


// network status callback
// start and stop the services when the network goes up or down
static void network_status_cb(struct netif *netif, u16_t reason, const netif_ext_callback_args_t *args) {
    // DCHP done: network 'up'
    if (reason & LWIP_NSC_IPV4_ADDR_VALID) {
        printf("netif_ext_cb: given IP %s\n", inet_ntoa(netif->ip_addr.addr));
        network_is_up = true;
        start_mqtt();
        start_sntp();
    }
    // link lost: network 'down'
    if (reason & LWIP_NSC_LINK_CHANGED && args->link_changed.state == 0) {
        puts("netif_ext_cb: link down");
        network_is_up = false;
        mqtt_is_up = false;
        stop_sntp();
    }
}

// connect to WiFi and install the network status callback
static void connect_worker_cb(async_context_t *ctx, async_at_time_worker_t *p_worker) {
    static netif_ext_callback_t netif_ext_status;
    if (!netif_ext_status.callback_fn) {
        netif_add_ext_callback(&netif_ext_status, network_status_cb); // install network status callback
    }
    if (!network_is_up) {
        if(cyw43_wifi_link_status(&cyw43_state, CYW43_ITF_STA) != CYW43_LINK_JOIN) {
            // no connection attempt in progress
            puts("wifi_connect_worker_cb: connect");
            cyw43_arch_wifi_connect_async(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK);
        }
        // keep trying until we get an IP address
        async_context_add_at_time_worker_in_ms(ctx, p_worker, 250);
    }
}

// user entry point
void connect() {
    // initialise the WiFi driver in station mode
    if (cyw43_arch_init_with_country(CYW43_COUNTRY_UK)) {
        panic("could not initialise WiFi");
    }
    cyw43_arch_enable_sta_mode();
    ctx = cyw43_arch_async_context();

    // start the connection worker (on the network thread)
    static async_at_time_worker_t connect_worker = { .do_work = connect_worker_cb };
    async_context_add_at_time_worker_in_ms(ctx, &connect_worker, 0);
}