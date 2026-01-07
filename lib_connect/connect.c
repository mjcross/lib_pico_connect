#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "lwip/netif.h"         // for netif_ext_callback_t
#include "lwip/inet.h"          // for inet_ntoa()
#include "lwipopts.h"

#include "connect.h"
#include "settings.h"
#include "password.h"

// declare local functions
static void wifi_connect_cb(async_context_t *ctx, async_at_time_worker_t *p_worker);

// define and initialise globals
volatile bool network_is_up = false;

// initialise shared local variables
static async_context_t *ctx = NULL;
static netif_ext_callback_t netif_ext_status = {};
static async_at_time_worker_t wifi_connect_worker = { .do_work = wifi_connect_cb };

// define functions

// callback for network 'extended status' events
static void netif_ext_status_cb(struct netif *netif, u16_t reason, const netif_ext_callback_args_t *args) {
    if (reason & LWIP_NSC_IPV4_ADDR_VALID) {
        // we were given an IP address
        printf("netif_ext_cb: given IP %s\n", inet_ntoa(netif->ip_addr.addr));
        network_is_up = true;
        // todo: look up the MQTT server address
        // todo: start the SNTP client
    }
    if (reason & LWIP_NSC_LINK_CHANGED && args->link_changed.state == 0) {
        // the link went down
        puts("netif_ext_cb: link down");
        network_is_up = false;
        // todo: stop the SNTP client
    }
}

// connect to WiFi and get an IP address
static void wifi_connect_cb(async_context_t *ctx, async_at_time_worker_t *p_worker) {
    if (!netif_ext_status.callback_fn) {
        // our LWIP extended status listener is not yet installed
        puts("wifi_connect_worker_cb: adding lwIP extended status listener");
        netif_add_ext_callback(&netif_ext_status, netif_ext_status_cb);
    }
    if (!network_is_up) {
        // not received an IP address yet
        if(cyw43_wifi_link_status(&cyw43_state, CYW43_ITF_STA) != CYW43_LINK_JOIN) {
            // no connection attempt in progress
            puts("wifi_connect_worker_cb: connect");
            cyw43_arch_wifi_connect_async(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK);
        }
        // keep trying until we get an IP address
        async_context_add_at_time_worker_in_ms(ctx, p_worker, 250);
    }
}

// initialise the WiFi and get the cy43_arch asynchrounous context
void init_wifi() {
    if (cyw43_arch_init_with_country(CYW43_COUNTRY_UK)) {
        panic("could not initialise WiFi");
    }
    cyw43_arch_enable_sta_mode();
    ctx = cyw43_arch_async_context();
}

// user entry point
void connect() {
    // configure the WiFi device and cy43_arch API
    init_wifi();

    // connect to the network and servers
    async_context_add_at_time_worker_in_ms(ctx, &wifi_connect_worker, 0);
}   