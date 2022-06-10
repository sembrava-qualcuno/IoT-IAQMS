#include <WiFi.h>
#include <WiFiUdp.h>
#include <MQUnifiedsensor.h>
#include <string.h>
#include <string>
#include <stdlib.h>
#include <HardwareSerial.h>
#include <stddef.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <time.h>
#include <PubSubClient.h>
#include <coap-simple.h>
#include <EEPROM.h>

/********************************** Device information ***************************************/
#ifndef IOT_DEVICE
#define IOT_DEVICE
#define GPS "44.494887,11.3426163"
#define DEVICE_ID "1"
#define EEPROM_SIZE sizeof(int)*4

/********************************** Needed by the MQ-2 library ******************************/
#define Board "ESP-32"
#define Pin 36
#define Type "MQ-2"
#define Voltage_Resolution 3.3 // 3V3 <- IMPORTANT. Source: https://randomnerdtutorials.com/esp32-adc-analog-read-arduino-ide/
#define ADC_Bit_Resolution 12  // ESP-32 bit resolution. Source: https://randomnerdtutorials.com/esp32-adc-analog-read-arduino-ide/
#define RatioMQ2CleanAir 9.83

/********************************** Needed by the DHT library ******************************/
#define DHTPIN 2 // Digital pin connected to the DHT sensor
// Pin 15 can work but DHT must be disconnected during program upload.
// Uncomment the type of sensor in use:
#define DHTTYPE DHT11 // DHT 11
//#define DHTTYPE    DHT22     // DHT 22 (AM2302)
//#define DHTTYPE    DHT21     // DHT 21 (AM2301)

// See guide for details on sensor wiring and usage:
//   https://learn.adafruit.com/dht/overview

typedef struct Parameters
{
  int sample_frequency;
  int min_gas_value;
  int max_gas_value;
} Parameters;

/*
 * 
 * Global variables declaration
 * 
 */
extern const char *ssid;
extern const char *password;
extern Coap coap;
extern PubSubClient mqtt_client;

extern volatile int SAMPLE_FREQUENCY; // in seconds
extern volatile int MIN_GAS_VALUE;    
extern volatile int MAX_GAS_VALUE;    
extern volatile int PROTOCOL;
extern volatile int PERFORMANCE_EVAL;

// Performance evaluation variables
extern volatile long coap_pkt_time;
extern volatile long coap_pkt_delay_tot;
extern volatile int coap_pkt_sent;
extern volatile int coap_pkt_rcv;
extern volatile long mqtt_pkt_time;
extern volatile long mqtt_pkt_delay_tot;
extern volatile int mqtt_pkt_sent;
extern volatile int mqtt_pkt_rcv;

/*
 * 
 * Utility functions declaration
 * 
 */
extern void setup_wifi();
extern void setup_MQ2(MQUnifiedsensor *MQ2);
extern void setup_DHT(DHT_Unified *dht);
extern void send_data(String data, int protocol);
extern int computeAQI(float gas);
extern void get_dht_data(DHT_Unified *dht, String *data);
extern float get_mq2_data(MQUnifiedsensor *MQ2, String *data);
extern void get_conf_eeprom();

#endif
