#include "iot_device.h"
#include "coap.h"
#include "mqtt.h"

static volatile float last_gas[5] = {-1, -1, -1, -1, -1};

volatile long coap_pkt_time = 0;
volatile long coap_pkt_delay_tot = 0;
volatile int coap_pkt_sent = 0;
volatile int coap_pkt_rcv = 0;

volatile long mqtt_pkt_time = 0;
volatile long mqtt_pkt_delay_tot = 0;
volatile int mqtt_pkt_sent = 0;
volatile int mqtt_pkt_rcv = 0;

void setup_wifi()
{
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
}

void setup_MQ2(MQUnifiedsensor *MQ2)
{
    // Set math model to calculate the PPM concentration and the value of constants
    MQ2->setRegressionMethod(1); //_PPM =  a*ratio^b
    MQ2->setA(36974);
    MQ2->setB(-3.109); // Configure the equation to to calculate H2 concentration

    /*
        Exponential regression:
        Gas    | a      | b
        H2     | 987.99 | -2.162
        LPG    | 574.25 | -2.222
        CO     | 36974  | -3.109
        Alcohol| 3616.1 | -2.675
        Propane| 658.71 | -2.168
      */
    /*****************************  MQ Init ********************************************/
    // Remarks: Configure the pin of arduino as input.
    /************************************************************************************/
    MQ2->init();

    /*
      //If the RL value is different from 10K please assign your RL value with the following method:
      MQ2.setRL(10);
    */
    /*****************************  MQ CAlibration ********************************************/
    // Explanation:
    // In this routine the sensor will measure the resistance of the sensor supposedly before being pre-heated
    // and on clean air (Calibration conditions), setting up R0 value.
    // We recomend executing this routine only on setup in laboratory conditions.
    // This routine does not need to be executed on each restart, you can load your R0 value from eeprom.
    // Acknowledgements: https://jayconsystems.com/blog/understanding-a-gas-sensor
    Serial.print("Calibrating please wait.");
    float calcR0 = 0;
    for (int i = 1; i <= 10; i++)
    {
        MQ2->update(); // Update data, the arduino will read the voltage from the analog pin
        calcR0 += MQ2->calibrate(RatioMQ2CleanAir);
        Serial.print(".");
    }
    MQ2->setR0(calcR0 / 10);
    Serial.println("  done!.");

    if (isinf(calcR0))
    {
        Serial.println("Warning: Conection issue, R0 is infinite (Open circuit detected) please check your wiring and supply");
        while (1)
            ;
    }
    if (calcR0 == 0)
    {
        Serial.println("Warning: Conection issue found, R0 is zero (Analog pin shorts to ground) please check your wiring and supply");
        while (1)
            ;
    }
    /*****************************  MQ CAlibration ********************************************/
    // MQ2->serialDebug(true); uncomment if you want to print the table on the serial port
}

void setup_DHT(DHT_Unified *dht)
{
    dht->begin();
    Serial.println(F("DHT11 Sensor Initialization"));
    // Print temperature sensor details.
    sensor_t sensor;
    dht->temperature().getSensor(&sensor);
    Serial.println(F("------------------------------------"));
    Serial.println(F("Temperature Sensor"));
    Serial.print(F("Sensor Type: "));
    Serial.println(sensor.name);
    Serial.print(F("Driver Ver:  "));
    Serial.println(sensor.version);
    Serial.print(F("Unique ID:   "));
    Serial.println(sensor.sensor_id);
    Serial.print(F("Max Value:   "));
    Serial.print(sensor.max_value);
    Serial.println(F("°C"));
    Serial.print(F("Min Value:   "));
    Serial.print(sensor.min_value);
    Serial.println(F("°C"));
    Serial.print(F("Resolution:  "));
    Serial.print(sensor.resolution);
    Serial.println(F("°C"));
    Serial.println(F("------------------------------------"));
    // Print humidity sensor details.
    dht->humidity().getSensor(&sensor);
    Serial.println(F("Humidity Sensor"));
    Serial.print(F("Sensor Type: "));
    Serial.println(sensor.name);
    Serial.print(F("Driver Ver:  "));
    Serial.println(sensor.version);
    Serial.print(F("Unique ID:   "));
    Serial.println(sensor.sensor_id);
    Serial.print(F("Max Value:   "));
    Serial.print(sensor.max_value);
    Serial.println(F("%"));
    Serial.print(F("Min Value:   "));
    Serial.print(sensor.min_value);
    Serial.println(F("%"));
    Serial.print(F("Resolution:  "));
    Serial.print(sensor.resolution);
    Serial.println(F("%"));
    Serial.println(F("------------------------------------"));
}

void send_data(String data)
{
    bool performance_eval = ((PERFORMANCE_EVAL == 0) ? false : true);

    char payload[data.length()+1];
    data.toCharArray(payload, data.length()+1);
  
    switch (PROTOCOL)
    {
    case 0:
        Serial.println("Sending COAP Request");
        if(performance_eval && coap_pkt_sent == PERFORMANCE_EVAL)
        {            
            // Send the report to the MQTT broker
            String pe = String(coap_pkt_delay_tot/coap_pkt_rcv) + String(",") + String((float(coap_pkt_rcv)/coap_pkt_sent) * 100);
            char perf_eval[pe.length()+1];
            pe.toCharArray(perf_eval, pe.length()+1);
            mqtt_client.publish(PERFORMANCE_WRITE_TOPIC, perf_eval);

            Serial.print("Performance evaluation end. Results: ");
            Serial.println(pe);
            
            // Performance evaluation completed, reset all pe variables
            PERFORMANCE_EVAL = 0;
            performance_eval = false;
            coap_pkt_time = 0;
            coap_pkt_delay_tot = 0;
            coap_pkt_sent = 0;
            coap_pkt_rcv = 0;
        }
        if(performance_eval)
          coap_pkt_time = millis(); 
        post(COAP_SERVER, COAP_PORT, COAP_RESOURCE, (uint8_t *) payload, strlen(payload));
        if(performance_eval)
          coap_pkt_sent++;
        break;
    case 1:
        Serial.println("Sending data to /sensor-data topic");
        if(performance_eval && mqtt_pkt_sent == PERFORMANCE_EVAL)
        {            
            // Send the report to the MQTT broker
            String pe = String(mqtt_pkt_delay_tot/mqtt_pkt_rcv) + String(",") + String((float(mqtt_pkt_rcv)/mqtt_pkt_sent) * 100);
            char perf_eval[pe.length()+1];
            pe.toCharArray(perf_eval, pe.length()+1);
            mqtt_client.publish(PERFORMANCE_WRITE_TOPIC, perf_eval);

            Serial.print("Performance evaluation end. Results: ");
            Serial.println(pe);

            // Performance evaluation completed, reset all pe variables
            PERFORMANCE_EVAL = 0;
            performance_eval = false;
            mqtt_pkt_time = 0;
            mqtt_pkt_delay_tot = 0;
            mqtt_pkt_sent = 0;
            mqtt_pkt_rcv = 0;
            mqtt_client.unsubscribe(WRITE_TOPIC);
        }
        else if (performance_eval && mqtt_pkt_sent == 0)
          mqtt_client.subscribe(WRITE_TOPIC);
        
        if(performance_eval)
          mqtt_pkt_time = millis();
        mqtt_client.publish(WRITE_TOPIC, payload);
        Serial.print("Successfully sent sensor data through MQTT");
        if(performance_eval)
          mqtt_pkt_sent++;
        break;
    default:
        Serial.println("Wrong protocol!"); break;
    }
}

int computeAQI(float gas)
{
    int AQI = -1;

    // Average window shifting
    for (int i = 4; i >= 1; i--)
    {
        last_gas[i] = last_gas[i - 1];
    }
    last_gas[0] = gas;

    // AQI computation
    if (last_gas[4] != -1)
    {
        float avg = (last_gas[0] + last_gas[1] + last_gas[2] + last_gas[3] + last_gas[4]) / 5;

        if (avg >= MAX_GAS_VALUE)
            AQI = 0;
        else if (avg < MAX_GAS_VALUE && avg >= MIN_GAS_VALUE)
            AQI = 1;
        else
            AQI = 2;
    }

    return AQI;
}

void get_dht_data(DHT_Unified *dht, String *data)
{
    // Get temperature event and print its value.
    sensors_event_t event;
    dht->temperature().getEvent(&event);
    if (isnan(event.temperature))
    {
        Serial.println(F("Error reading temperature!"));
    }
    else
    {
        Serial.print(F(" Got Temperature: "));
        Serial.print(event.temperature);
        Serial.println(F("°C"));
        data->concat(String(event.temperature) + ',');
    }
    // Get humidity event and print its value.
    dht->humidity().getEvent(&event);
    if (isnan(event.relative_humidity))
    {
        Serial.println(F("Error reading humidity!"));
    }
    else
    {
        Serial.print(F("Got Humidity: "));
        Serial.print(event.relative_humidity);
        Serial.println(F("%"));
        data->concat(String(event.relative_humidity) + ',');
    }
}

float get_mq2_data(MQUnifiedsensor *MQ2, String *data)
{
    // Update data, the arduino will read the voltage from the analog pin
    MQ2->update();
    // MQ2->serialDebug(); // Will print the table on the serial port

    // Sensor will read PPM concentration using the model, a and b values set previously or from the setup
    float gas = MQ2->readSensor();
    Serial.print("Got Gas reading:");
    Serial.print(gas);
    Serial.println(" PPM");
    data->concat(String(gas) + ',');
    return gas;
}

void get_conf_eeprom()
{
  Parameters p;
  int proto = PROTOCOL;
  // Empty EEPROM, initialize with default values
  if(EEPROM.read(0) == 255)
  {
    Serial.println("Empty EEPROM, initialize with default values");
    EEPROM.put(0, proto);
    
    p.sample_frequency = SAMPLE_FREQUENCY;
    p.min_gas_value = MIN_GAS_VALUE;
    p.max_gas_value = MAX_GAS_VALUE;
    EEPROM.put(sizeof(int), p);
  }
  else
  {
    EEPROM.get(0, proto);
    PROTOCOL = proto;
    EEPROM.get(sizeof(int), p);
    SAMPLE_FREQUENCY = p.sample_frequency;
    MIN_GAS_VALUE = p.min_gas_value;
    MAX_GAS_VALUE = p.max_gas_value;
    Serial.println("Got conf from EEPROM:");
    Serial.print("SAMPLE_FREQUENCY: "); Serial.println(SAMPLE_FREQUENCY);
    Serial.print("MIN_GAS_VALUE: "); Serial.println(MIN_GAS_VALUE);
    Serial.print("MAX_GAS_VALUE: "); Serial.println(MAX_GAS_VALUE);
    Serial.print("PROTOCOL: "); Serial.println(PROTOCOL);
  }
}
