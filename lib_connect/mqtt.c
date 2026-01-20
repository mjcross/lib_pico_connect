/**
 * Copyright (c) 2026 mjcross
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include "pico/stdlib.h"
#include "lwip/inet.h"
#include "string.h"
#include "pico/unique_id.h"
#include "lwip/apps/mqtt_priv.h"    // mqtt_client_t
#include "lwip/dns.h"

#include "settings.h"
#include "connect.h"
#include "mqtt.h"

// type definitions
typedef struct {
    char topic[MQTT_TOPIC_LEN];
    uint8_t data[MQTT_OUTPUT_RINGBUF_SIZE];
    uint32_t len;
    bool isTruncated;
} mqtt_msg_buf_t;

// shared local variables
volatile bool mqtt_is_up = false;
static mqtt_client_t mqtt_client;
static struct mqtt_connect_client_info_t mqtt_connect_client_info = { .keep_alive = 60 };   // client ID added in start_mqtt()
static mqtt_msg_buf_t mqtt_msg_buf;
static ip_addr_t mqtt_server_ip;

// function declarations
static void mqtt_status_cb(mqtt_client_t *client, void *userdata, mqtt_connection_status_t status);


// function definitions

static void mqtt_pub_request_cb(__unused void *arg, err_t err) {
    if (err != 0) {
        printf("pub_request_cb failed %d", err);
    }
}

static void mqtt_sub_request_cb(void *arg, err_t err) {
    if (err != 0) {
        printf("subscribe request failed %d", err);
    }
}

// called by lwIP MQTT when something is published to a subscribed topic
static void mqtt_incoming_publish_cb(void *arg, const char *topic, u32_t tot_len) {
    mqtt_msg_buf_t *msg = (mqtt_msg_buf_t *)arg;
    if (strlcpy(msg->topic, topic, sizeof(msg->topic)) >= sizeof(msg->topic)) {
        puts("MQTT topic truncated");
    }
    msg->len = 0;
    msg->isTruncated = false;
}

// called by lwIP MQTT with the payload of an incoming message
// may be called multiple times if the payload won't fit into a single buffer
static void mqtt_incoming_data_cb(void *arg, const u8_t *data, u16_t len, u8_t flags) {
    mqtt_msg_buf_t *msg = (mqtt_msg_buf_t *)arg;
    printf("%d bytes for topic %s (core %d)\n", len, msg->topic, get_core_num());
    if (msg->len + len > sizeof(msg->data)) {
        len = sizeof(msg->data) - msg->len;
        msg->isTruncated = true;
    }
    memcpy(&msg->data[msg->len], data, len);
    msg->len += len;

    if (flags & MQTT_DATA_FLAG_LAST) {
        // ToDo: process incoming message
        printf("topic %s: ", msg->topic);
        for (u32_t i = 0; i < msg->len; i += 1) {
            if (!iscntrl(msg->data[i])) {
                putchar(msg->data[i]);
            } else {
                printf("[%d]", msg->data[i]);
            }
        }
        if (msg->isTruncated) {
            printf(" (truncated)");
        }
        putchar('\n');
    }
}

static void mqtt_subscribe_worker_cb(async_context_t *ctx, async_at_time_worker_t *p_mqtt_subscribe_worker) {
    char *topic = (char *)p_mqtt_subscribe_worker->user_data;
    if (mqtt_client_is_connected(&mqtt_client)) {
        err_t err = mqtt_subscribe(&mqtt_client, topic, 1, mqtt_sub_request_cb, topic);
        if (err == ERR_OK) {
            printf("mqtt_subscribe_worker_cb: subscribed %s\n", topic);
        } else {            
            printf("mqtt_subscribe_worker_cb: error %d for topic %s\n", err, topic);
            // reschedule another attempt
            async_context_add_at_time_worker_in_ms(ctx, p_mqtt_subscribe_worker, MQTT_CONNECT_RETRY_MS);
        }
    }
}

static void mqtt_connect_cb(async_context_t *ctx, async_at_time_worker_t *p_mqtt_connect_worker) {
    if (network_is_up) {
        printf("mqtt_connect_worker_cb: connecting as %s\n", mqtt_connect_client_info.client_id);
        err_t err = mqtt_client_connect(
            &mqtt_client,               // client
            &mqtt_server_ip,            // ip_addr
            MQTT_PORT,                  // port
            mqtt_status_cb,             // cb (connection state change callback)
            NULL,                       // arg (userdata passed to callback)
            &mqtt_connect_client_info   // client_info
        );
        if (err == ERR_OK || err == ERR_INPROGRESS || err == ERR_ALREADY) {
            puts("mqtt_connect_worker_cb: in progress");
        } else {
            printf("mqtt_connect_worker_cb: error %d\n", err);
            async_context_add_at_time_worker_in_ms(ctx, p_mqtt_connect_worker, MQTT_CONNECT_RETRY_MS);
        }
    } else {
        // network isn't connected: this shouldn't happen, but the dns worker or callback should
        // start a new connection attempt after we get an IP address
        puts("mqtt_connect_worker_cb: network down or no address");
    }
}
static async_at_time_worker_t mqtt_connect_worker = { .do_work = mqtt_connect_cb };

// function called by lwIP MQTT when the status of the server connection changes
static void mqtt_status_cb(mqtt_client_t *client, void *userdata, mqtt_connection_status_t status) {
    static async_at_time_worker_t mqtt_subscribe_worker = { 
        .do_work = mqtt_subscribe_worker_cb,
        .user_data = MQTT_SUBSCRIBE_TOPIC 
    };
    if (status == MQTT_CONNECT_ACCEPTED) {
        puts("mqtt_status_cb: connection accepted");
        mqtt_is_up = true;
        // subscribe to topics (one worker per topic)
        async_context_add_at_time_worker_in_ms(ctx, &mqtt_subscribe_worker, 0);
    } else {
        if (status == MQTT_CONNECT_DISCONNECTED) {
            puts("mqtt_status_cb: disconnected");
            mqtt_is_up = false;
        } else if (status == MQTT_CONNECT_TIMEOUT) {
            puts("mqtt_status_cb: timed out");
        } else {
            printf("mqtt_status_cb: refused (%d)\n", status);
        }
        // if the network is up then try to reconnect, otherwise netif_ext_cb() will launch
        // another attempt when the network returns
        if (network_is_up) {
            async_context_add_at_time_worker_in_ms(ctx, &mqtt_connect_worker, MQTT_CONNECT_RETRY_MS);
        }
    }
}

static void dns_query_cb(const char *hostname, const ip_addr_t *addr_ptr, void *arg) {
    async_at_time_worker_t *p_dns_lookuworker_ptr = (async_at_time_worker_t *)arg;
    if (addr_ptr) {
        mqtt_server_ip = *addr_ptr;
        printf("%s is at %s\n", hostname, inet_ntoa(mqtt_server_ip));
        // connect to server
        async_context_add_at_time_worker_in_ms(ctx, &mqtt_connect_worker, 0);
    } else {
        puts("dns error");
        async_context_add_at_time_worker_in_ms(ctx, (async_at_time_worker_t *)arg, DNS_RETRY_MS);
    }
}

// look up MQTT server address
static void get_server_ip(async_context_t *ctx, async_at_time_worker_t *p_dns_lookuworker_ptr) {
    printf("dns lookup %s\n", MQTT_SERVER);
    err_t result = dns_gethostbyname(MQTT_SERVER, &mqtt_server_ip, dns_query_cb, p_dns_lookuworker_ptr);
    if (result == ERR_OK) {
        printf("%s is at %s\n", MQTT_SERVER, inet_ntoa(mqtt_server_ip));
        // connect to server
        async_context_add_at_time_worker_in_ms(ctx, &mqtt_connect_worker, 0);
    } else if (result == ERR_INPROGRESS) {
        puts("dns query in progress");
    } else {
        puts("dns error");
        async_context_add_at_time_worker_in_ms(ctx, p_dns_lookuworker_ptr, DNS_RETRY_MS);
    }
}

void start_mqtt() {
    puts("starting MQTT");

    // initialise the MQTT client ID in mqtt_connect_client_info
    if (mqtt_connect_client_info.client_id == NULL) {
        size_t prefix_len = strlen(MQTT_CLIENT_NAME_PREFIX);
        size_t id_sz = prefix_len + 2 * PICO_UNIQUE_BOARD_ID_SIZE_BYTES + 1;
        char *id_str = malloc(id_sz);
        strlcpy(id_str, MQTT_CLIENT_NAME_PREFIX, id_sz);
        pico_get_unique_board_id_string(id_str + prefix_len, id_sz - prefix_len);
        mqtt_connect_client_info.client_id = (const char *)id_str;
        printf("MQTT client id %s\n", mqtt_connect_client_info.client_id);
    }

    // link to our functions for incoming messages
    mqtt_set_inpub_callback(&mqtt_client, mqtt_incoming_publish_cb, mqtt_incoming_data_cb, &mqtt_msg_buf);

    // look up the MQTT server IP and start a connection
    static async_at_time_worker_t dns_lookup_worker = { .do_work = get_server_ip };
    async_context_add_at_time_worker_in_ms(ctx, &dns_lookup_worker, 0);
}

// simple publish function (may be called asynchronously) 
typedef struct {
    const char *topic;
    const void *payload;
    uint16_t payload_length;
} mqtt_msg_t;
static void publish_cb(async_context_t *ctx, async_at_time_worker_t *worker_ptr) {
    mqtt_msg_t *msg_ptr = (mqtt_msg_t *)worker_ptr->user_data;
    err_t err = mqtt_publish(
        &mqtt_client, 
        msg_ptr->topic,
        msg_ptr->payload,
        msg_ptr->payload_length,
        0,                      // QoS
        0,                      // retain
        mqtt_pub_request_cb,    // callback
        NULL                    // callback arg
    );
}
void publish_mqtt(const char *topic, const void *payload, uint16_t payload_length) {
    static mqtt_msg_t msg;
    static async_at_time_worker_t publish_worker = { .do_work = publish_cb, .user_data = &msg };
    printf("MQTT publish: %s\n", topic);
    msg.topic = topic;
    msg.payload = payload;
    msg.payload_length = payload_length;
    async_context_add_at_time_worker_in_ms(ctx, &publish_worker, 0);
}