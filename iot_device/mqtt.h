#include <PubSubClient.h>
#include "iot_device.h"

// Configuration parameters
#define MQTT_BROKER "something" // TODO: change this values
#define MQTT_PORT 1883          // TODO: change this values
#define MQTT_CLIENT "ESP32-" DEVICE_ID

// MQTT Topics
#define WRITE_TOPIC "/sensor-data"
#define PARAMETERS_TOPIC "/devices/" DEVICE_ID "/parameters"
#define PROTOCOL_TOPIC "/devices/" DEVICE_ID "/protocol"
#define PERFORMANCE_READ_TOPIC "/devices/" DEVICE_ID "/performance/activate"
#define PERFORMANCE_WRITE_TOPIC "/devices/" DEVICE_ID "/performance/evaluation"

// Variables declaration
extern PubSubClient mqtt_client;

// Callback activated when message received
extern void callback(char *topic, byte *payload, unsigned int length);

// Function used to reconnect to the broker
extern void reconnect();
