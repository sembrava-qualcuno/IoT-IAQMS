#include "mqtt.h"
#include "StringTokenizer.h"

// Callback used in case of configuration messages received on the topic
void callback(char *topic, byte *payload, unsigned int length)
{
    payload[length] = '\0';
    String topicString = String(topic);
    String payloadString = String((char*)payload);
  
    Serial.print("Message arrived - topic [");
    Serial.print(topicString);
    Serial.println("] ");
    Serial.print("payload: [");
    Serial.print(payloadString);
    Serial.println("]");

    if(topicString == PARAMETERS_TOPIC)
    {
        Parameters p;
        StringTokenizer tokens(payloadString, ",");

        String token = tokens.nextToken();
        if(token != " ") 
            SAMPLE_FREQUENCY = token.toInt();
        token = tokens.nextToken();
        if(token != " ") 
            MIN_GAS_VALUE = token.toInt();
        token = tokens.nextToken();
        if(token != " ") 
            MAX_GAS_VALUE = token.toInt();

        p.sample_frequency = SAMPLE_FREQUENCY;
        p.min_gas_value = MIN_GAS_VALUE;
        p.max_gas_value = MAX_GAS_VALUE;
        EEPROM.put(sizeof(int), p);
        EEPROM.commit();
    }
    else if(topicString == PROTOCOL_TOPIC)
    {
        Serial.println("Switch protocol");
        PROTOCOL = atoi((char *) payload);
        int protocol = PROTOCOL;
        EEPROM.put(0, protocol);
        EEPROM.commit();
    }
    else if (topicString == PERFORMANCE_READ_TOPIC)
    {
        Serial.println("Start performance evaluation");
        PERFORMANCE_EVAL = atoi((char *) payload);
    }
    else if (topicString == WRITE_TOPIC)
    {
      // Get packet RTT and update received packet count
      long pkt_RTT = millis() - mqtt_pkt_time;
      mqtt_pkt_delay_tot += pkt_RTT;
      mqtt_pkt_rcv++;
      Serial.println("[MQTT Packet arrived to broker]");
      Serial.print("Packet RTT: "); Serial.println(pkt_RTT);
  
      // Calculate average packets delay and PDR
      Serial.print("Average MQTT packets Delay: "); Serial.println(float(mqtt_pkt_delay_tot)/mqtt_pkt_rcv);
      Serial.print("MQTT Packet Delivery Ratio so far: "); Serial.print((float(mqtt_pkt_rcv)/mqtt_pkt_sent) * 100); Serial.println("%");
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
