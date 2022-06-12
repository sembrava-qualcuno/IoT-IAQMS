#include "iot_device.h"
#include "coap.h"
#include "mqtt.h"

/*
 *
 * Global variable definition
 *
 */

/********************************** WiFi variables ******************************/
const char *ssid = "your-ssid";         // TODO: change this values
const char *password = "your-password"; // TODO: change this values

/****************************** Configuration variables ******************************/
volatile int SAMPLE_FREQUENCY = 60; // in seconds
volatile int MIN_GAS_VALUE = 1;     // TODO: change this values
volatile int MAX_GAS_VALUE = 3;     // TODO: change this values
volatile int PROTOCOL = 0;
volatile int PERFORMANCE_EVAL = 0;
unsigned long previousMillis = 0;

/****************************** COAP class definition ******************************/
WiFiUDP udp;
Coap coap(udp);

/******************************* MQTT class definition  ******************************/
WiFiClient espClient;
PubSubClient mqtt_client(espClient);

/******************************* MQ2 class definition  ******************************/
MQUnifiedsensor MQ2(Board, Voltage_Resolution, ADC_Bit_Resolution, Pin, Type);

/******************************* DHT class definition  ******************************/
DHT_Unified dht(DHTPIN, DHTTYPE);

/*
 *
 *   Sketch setup
 *
 */
void setup()
{
  Serial.begin(115200);
  Serial.println("Setup Started");
  setup_wifi();
  setup_MQ2(&MQ2);
  setup_DHT(&dht);

  Serial.println("Setting up COAP Response Callback");
  coap.response(callback_response);

  Serial.println("Starting COAP Client");
  coap.start();
  
  Serial.println("Setting up MQTT broker and message callback");
  mqtt_client.setServer(MQTT_BROKER, MQTT_PORT);
  mqtt_client.setCallback(callback);

  // Get previously stored configurations from EEPROM
  Serial.println("Getting conf variables from EEPROM");
  EEPROM.begin(EEPROM_SIZE);
  get_conf_eeprom();
   
  Serial.println("Setup completed.");  
}

void loop()
{
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= SAMPLE_FREQUENCY * 1000)
  {
    
/*
 *
 *   Data task functions
 *
 */

    previousMillis = currentMillis;

    // Get data from sensors
    String data = String(DEVICE_ID)+String(",")+String(GPS)+String(",");
    get_dht_data(&dht, &data);
    float gas = get_mq2_data(&MQ2, &data);
  
    //  Compute AQI
    int AQI = computeAQI(gas);
  
    if (AQI == -1)
      data += " ,";
    else
      data += String(AQI) + ",";
  
    long RSSI = WiFi.RSSI();
    data += String(RSSI);
    // Send data to data-microservice
    send_data(data);
  
    
  }
  coap.loop();
/*
 *
 *   Configuration task functions
 *
 */
  if (!mqtt_client.connected())
  {
    reconnect();
  }
  mqtt_client.loop();
}
