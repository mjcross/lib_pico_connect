#ifndef _SETTINGS_H
#define _SETTINGS_H

#include "private_settings.h"                   // MQTT server name, WiFi passwords etc

#define DNS_RETRY_MS                30000       // retry interval if DNS query fails

#define MQTT_TOPIC_LEN              100
#define MQTT_CONNECT_RETRY_MS       5000        // retry interval on MQTT connection error
#define MQTT_SUBSCRIBE_TOPIC        "test/#"
#define MQTT_CLIENT_NAME_PREFIX     "client-"

#endif