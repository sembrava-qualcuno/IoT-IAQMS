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
            p.sample_frequency = atoi(token);
		else
			p.sample_frequency = -1;

        token = strtok(NULL, ",");
        if (!strcmp(token, " "))
            p.min_gas_value = atoi(token);
		else
			p.min_gas_value = -1;

        token = strtok(NULL, ",");
        if (!strcmp(token, " "))
            p.max_gas_value = atoi(token);
		else
			p.max_gas_value = -1;

        xQueueOverwrite(parameter_queue, (void *)&p);
    }
    else if (strcmp(topic, PROTOCOL_TOPIC))
    {
        int protocol = atoi((char *) payload);

        xQueueOverwrite(protocol_queue, (void *)&protocol);
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
