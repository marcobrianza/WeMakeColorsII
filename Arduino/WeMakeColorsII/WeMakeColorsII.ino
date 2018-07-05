
String softwareName = "WeMakeColorsII";
String softwareVersion = "1.1.0"; //
String software = "";

//boot Count
#include <EEPROM.h>

//Wi-Fi
#include <ESP8266WiFi.h>  // ESP8266 core 2.4.1
WiFiClient  wifi;

//Wi-FiManager
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h> // Version 0.12.0
#include "FS.h"

#define MAX_PARAM 40
#define MQTT_MAX MQTT_MAX_PACKET_SIZE

char THING_ID[MAX_PARAM];
String thingId = "";
String appId = "WMCII_";

#define THING_NAME_DEFAULT ""
char THING_NAME[MAX_PARAM] = THING_NAME_DEFAULT;
String s_thingName = "";

char MQTT_SERVER[MAX_PARAM] = "wmc.marcobrianza.it";
String s_mqttServer = "";

char MQTT_USERNAME[MAX_PARAM] = "";
String s_mqttUsername = "";

char MQTT_PASSWORD[MAX_PARAM] = "";
String s_mqttPassword = "";


// name, prompt, default, length
WiFiManagerParameter wfm_thingName("thingName", "Thing Name", THING_NAME, sizeof(THING_NAME));
WiFiManagerParameter wfm_mqttServer("mqttServer", "MQTT Server", MQTT_SERVER, sizeof(MQTT_SERVER));
WiFiManagerParameter wfm_mqttUsername("mqttUsername", "MQTT Username", MQTT_USERNAME, sizeof(MQTT_USERNAME));
WiFiManagerParameter wfm_mqttPassword("mqttPassword", "MQTT Password", MQTT_PASSWORD, sizeof(MQTT_PASSWORD));

//OTA
#include <ESP8266mDNS.h>
#include <ArduinoOTA.h>
const char* OTA_PASSWORD = "12345678";

//http update
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>


String MD5_URL = "http://iot.marcobrianza.it/art/WeMakeColorsII.md5.txt";
String FW_URL = "http://iot.marcobrianza.it/art/WeMakeColorsII.ino.d1_mini.bin";

//MQTT
#include <PubSubClient.h> // version 2.6.0 //in PubSubClient.h change #define MQTT_MAX_PACKET_SIZE 512 
#include <ArduinoJson.h> // version 5.13.1
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);
int MQTT_PORT = 1883;
String mqttRoot =   "WeMakeColorsII";
//char* MQTT_PASSWORD = "aieie";

String mqtt_randomColor = "randomColor";
String mqtt_beat = "beat";
String mqtt_config = "config";

String mqttPublish_randomColor = "";
String mqttPublish_beat = "";

String mqttSubscribe_randomColor = "";
String mqttSubscribe_config = "";


//LED
#include <FastLED.h> // version  3.1.6
#define LED_DATA_PIN D1 //D1 is GPIO5
const int NUM_LEDS = 2;
int GLOBAL_BRIGHTNESS = 255;
CRGB leds[NUM_LEDS];

//presence
unsigned long last_color_t = 0;

const int numReadings = 25;
int readings[numReadings];      // the readings from the analog input
int readIndex = 0;              // the index of the current reading
int total = 0;                  // the running total
int average = 0;

int inputPin = A0;

//brightness
const int numReadingsGB = 25;
int readingsGB[numReadingsGB];      // the readings from the analog input
int readIndexGB = 0;              // the index of the current reading
int totalGB = 0;                  // the running total
int averageGB = 0;


int NEW_COLOR_TIME = 1000;
int LOOP_DELAY = 40;
#define  MAX_C  16

// test device
#define BOOT_TEST_LIGHT 2
#define BOOT_RESET 3
#define BOOT_DEFAULT_AP 4

#define TEST_TIME 30000

//default ssid password
const char* SSID = "colors";
const char* PASSWORD = "colors01";

//beat
#define BEAT_INTERVAL 900000 //900000 60000
unsigned long last_beat = 0;

//led builtin
#define LED_ON LOW
#define LED_OFF HIGH

void setup() {

  setupLEDs();
  showAllLeds(64, 64, 64);

  Serial.begin(115200);  Serial.println();
  software = softwareName + " - " + softwareVersion + " - " + ESP.getCoreVersion() + " - " + ESP.getSketchMD5();// + " - " + String (__DATE__) + " - " + String(__TIME__);;
  Serial.println(software);
  getTHING_ID();

  byte c = bootCount();
  Serial.print("\nboot count=");
  Serial.println(c);

  if (c == BOOT_TEST_LIGHT) {
    Serial.println("Test mode");
    testDevice();
  }

  switch  (c) {
    case BOOT_DEFAULT_AP:
      connectWifi();
      break;
    case BOOT_RESET:
      writeAttribute("thingName", "");
      writeAttribute("mqttServer", "");
      writeAttribute("mqttUsername", "");
      writeAttribute("mqttPassword", "");
      connectWifi_or_AP(true);
      break;
    default:
      connectWifi_or_AP(false);
  }

  autoUpdate();
  setupParameters();
  setupMqtt();
  setupOTA();
  setupMdns();
  setupLightLevel();


  WiFi.hostname(s_thingName);
}

void loop() {

  unsigned long now = millis();

  if (!mqttClient.connected())  reconnectMQTT();
  mqttClient.loop();

  if (lightChange() && (now - last_color_t > NEW_COLOR_TIME)) {
    CHSV c = newRndColor();
    leds[0] = c;
    publishRandomColor(c);
    applyColor();
    last_color_t = now;
  }

  if (now - last_beat > BEAT_INTERVAL) {
    last_beat = now;
    publishBeat();
  }

  setGlobalBrightness();

  ArduinoOTA.handle();
  delay(LOOP_DELAY);

}

void testDevice() {

  while (millis() < TEST_TIME) {
    int a = analogRead(inputPin);
    int v = a / 4;
    if (v > 255) v = 255;
    Serial.println(v);
    showAllLeds(v, v, v);
    delay(LOOP_DELAY);
  }
}








