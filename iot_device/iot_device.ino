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
static volatile int SAMPLE_FREQUENCY = 60; // in seconds
static volatile int MIN_GAS_VALUE = 1;     // TODO: change this values
static volatile int MAX_GAS_VALUE = 3;     // TODO: change this values
static volatile int PROTOCOL = 0;
static volatile int PERFORMANCE_EVAL = 0;

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
 *   Tasks managament
 *
 */

/******************************* Tasks definition  ******************************/
TaskHandle_t DataTask;
TaskHandle_t ConfigurationTask;
QueueHandle_t parameter_queue;
QueueHandle_t protocol_queue;
QueueHandle_t performance_queue;

/*
 *
 *   Configuration task loop function
 *
 */
void configuration_task(void *)
{
  if (!mqtt_client.connected())
  {
    reconnect();
  }

  mqtt_client.subscribe(PARAMETERS_TOPIC);
  mqtt_client.subscribe(PROTOCOL_TOPIC);
  mqtt_client.subscribe(PERFORMANCE_READ_TOPIC);

  mqtt_client.loop();
}

/*
 *
 *   Data task loop function
 *
 */
void data_task(void *)
{
  // Check the queues for configuration updates
  check_conf_updates(&SAMPLE_FREQUENCY, &MIN_GAS_VALUE, &MAX_GAS_VALUE, &PROTOCOL, &PERFORMANCE_EVAL);

  // Get data from sensors
  String data = String(DEVICE_ID)+String(",")+String(GPS)+String(",");
  get_dht_data(&dht, &data);
  float gas = get_mq2_data(&MQ2, &data);

  //  Compute AQI
  int AQI = computeAQI(gas, MIN_GAS_VALUE, MAX_GAS_VALUE);

  if (AQI == -1)
    data += " ,";
  else
    data += String(AQI) + ",";

  long RSSI = WiFi.RSSI();
  data += String(RSSI);
  // Send data to data-microservice
  send_data(data, PROTOCOL, &PERFORMANCE_EVAL);

  coap.loop();

  delay(1000 * SAMPLE_FREQUENCY);
}

/*
 *
 *   Sketch setup
 *
 */
void setup()
{
  setup_wifi();
  setup_MQ2(&MQ2);
  setup_DHT(&dht);

  Serial.println("Setting up COAP Response Callback");
  coap.response(callback_response);

  Serial.println("Setting up MQTT server and message callback");
  mqtt_client.setServer(MQTT_BROKER, MQTT_PORT);
  mqtt_client.setCallback(callback);

  // Task scheduling

  xTaskCreatePinnedToCore(
      data_task,   /* Function to implement the task */
      "Data Task", /* Name of the task */
      1000,        /* Stack size in words */
      NULL,        /* Task input parameter */
      1,           /* Priority of the task */
      &DataTask,   /* Task handle. */
      0);          /* Core where the task should run */

  xTaskCreatePinnedToCore(
      configuration_task,   /* Function to implement the task */
      "Configuration Task", /* Name of the task */
      1000,                 /* Stack size in words */
      NULL,                 /* Task input parameter */
      10,                   /* Task priority*/
      &ConfigurationTask,   /* Task handle. */
      1);                   /* Core where the task should run */

  parameter_queue = xQueueCreate(1, 3 * sizeof(int));
  protocol_queue = xQueueCreate(1, sizeof(int));
  performance_queue = xQueueCreate(1, sizeof(int));
  

  Serial.println("Setup completed.");

  // Get previously stored configurations from EEPROM
  EEPROM.begin(EEPROM_SIZE);
  get_conf_eeprom(&PROTOCOL, &SAMPLE_FREQUENCY, &MIN_GAS_VALUE, &MAX_GAS_VALUE);

  // start coap client
  coap.start();
}

void loop(){}
