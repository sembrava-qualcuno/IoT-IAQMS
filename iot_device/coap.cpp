#include "coap.h"

// CoAP client response callback
void callback_response(CoapPacket &packet, IPAddress ip, int port)
{
    Serial.println("[Coap Response got]");

    char p[packet.payloadlen + 1];
    memcpy(p, packet.payload, packet.payloadlen);
    p[packet.payloadlen] = NULL;

    Serial.println(p);
}


void post(IPAddress dest, int port, char *resource, uint8_t * payload, size_t size)
{
  coap.send(dest, port, resource, COAP_CON, COAP_POST, NULL, 0, payload, size);
}
