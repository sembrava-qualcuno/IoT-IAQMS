#include <coap-simple.h>
#include <HardwareSerial.h>
#include "iot_device.h"

#define COAP_SERVER IPAddress(192, 168, 1, 69) // TODO: change this values
#define COAP_PORT 8080
#define COAP_RESOURCE "sensor-data"   

// CoAP client response callback definition
extern void callback_response(CoapPacket &packet, IPAddress ip, int port);
extern void post(IPAddress dest, int port, char *resource, uint8_t * payload, size_t s);
