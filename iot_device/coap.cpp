#include "coap.h"

// CoAP client response callback
void callback_response(CoapPacket &packet, IPAddress ip, int port)
{
    // Get packet RTT and update received packet count
    if(PERFORMANCE_EVAL != 0)
    {
      long pkt_RTT = millis() - coap_pkt_time;
      coap_pkt_delay_tot += pkt_RTT;
      coap_pkt_rcv++;
      Serial.print("Packet RTT: "); Serial.println(pkt_RTT);
    }
    Serial.println("[Coap Response got]");

    char p[packet.payloadlen + 1];
    memcpy(p, packet.payload, packet.payloadlen);
    p[packet.payloadlen] = NULL;

    Serial.print("Packet payload: ");
    Serial.println(p);
    
    // Calculate average packets delay and PDR
    if(PERFORMANCE_EVAL != 0)
    {
      Serial.print("Average COAP packets Delay: "); Serial.println(float(coap_pkt_delay_tot)/coap_pkt_rcv);
      Serial.print("COAP Packet Delivery Ratio so far: "); Serial.print((float(coap_pkt_rcv)/coap_pkt_sent) * 100); Serial.println("%");
    }
}


void post(IPAddress dest, int port, char *resource, uint8_t * payload, size_t size)
{
  coap.send(dest, port, resource, COAP_CON, COAP_POST, NULL, 0, payload, size);
}
