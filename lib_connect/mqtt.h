#ifndef _MQTT_H
#define _MQTT_H

extern volatile bool mqtt_is_up; 

void start_mqtt();
void publish_mqtt(const char *topic, const void *payload, uint16_t payload_length);

#endif