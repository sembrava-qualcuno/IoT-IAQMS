#include "mqtt.h"

// Callback used in case of configuration messages received on the topic
void callback(char *topic, byte *payload, unsigned int length)
{
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");

    if (strcmp(topic, PARAMETERS_TOPIC))
    {
        Parameters p;
        char *token = strtok((char *) payload, ",");
        if (!strcmp(token, " "))
            SAMPLE_FREQUENCY = atoi(token);

        token = strtok(NULL, ",");
        if (!strcmp(token, " "))
            MIN_GAS_VALUE = atoi(token);
		
        token = strtok(NULL, ",");
        if (!strcmp(token, " "))
            MAX_GAS_VALUE = atoi(token);
    }
    else if (strcmp(topic, PROTOCOL_TOPIC))
      PROTOCOL = atoi((char *) payload);
      
    else if (strcmp(topic, PERFORMANCE_READ_TOPIC))
      PERFORMANCE_EVAL = atoi((char *) payload);
      
    else if (strcmp(topic, WRITE_TOPIC))
    {
      // Get packet RTT and update received packet count
      long pkt_RTT = millis() - mqtt_pkt_time;
      mqtt_pkt_delay_tot += pkt_RTT;
      mqtt_pkt_rcv++;
      Serial.println("[MQTT Packet arrived to broker]");
      Serial.print("Packet RTT: "); Serial.println(pkt_RTT);
  
      // Calculate average packets delay and PDR
      Serial.print("Average MQTT packets Delay: "); Serial.println(mqtt_pkt_delay_tot/mqtt_pkt_rcv);
      Serial.print("MQTT Packet Delivery Ratio so far: "); Serial.print((mqtt_pkt_sent/mqtt_pkt_rcv) * 100); Serial.println("%");
    }
}

// Function used to reconnect to the broker
void reconnect()
{
    // Loop until we're reconnected
    while (! mqtt_client.connected())
    {
        Serial.print("Attempting MQTT connection...");

        // Attempt to connect
        if (mqtt_client.connect(MQTT_CLIENT))
        {
            Serial.println("connected");
            mqtt_client.subscribe(PARAMETERS_TOPIC);
            mqtt_client.subscribe(PROTOCOL_TOPIC);
            mqtt_client.subscribe(PERFORMANCE_READ_TOPIC);
        }
        else
        {
            Serial.print("failed, rc=");
            Serial.print(mqtt_client.state());
            Serial.println(" try again in 5 seconds");
            // Wait 5 seconds before retrying
            delay(5000);
        }
    }
}
