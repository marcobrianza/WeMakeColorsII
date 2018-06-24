
String softwareName = "WeMakeColorsII";
String softwareVersion = "1.0.2"; //
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

// name, prompt, default, length
WiFiManagerParameter wfm_thingName("thingName", "Thing Name", THING_NAME, sizeof(THING_NAME));
WiFiManagerParameter wfm_mqttServer("mqttServer", "MQTT Server", MQTT_SERVER, sizeof(MQTT_SERVER));

//OTA
#include <ESP8266mDNS.h>
#include <ArduinoOTA.h>
const char* OTA_PASSWORD = "12345678";

//http update
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>


String MD5_URL = "http://iot.marcobrianza.it/WeMakeColorsII/md5.txt";
String FW_URL = "http://iot.marcobrianza.it/WeMakeColorsII/WeMakeColorsII.ino.d1_mini.bin";

//MQTT
#include <PubSubClient.h> // version 2.6.0 //in PubSubClient.h change #define MQTT_MAX_PACKET_SIZE 512 
#include <ArduinoJson.h> // version 5.13.1
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);
int MQTT_PORT = 1883;
String mqttRoot =   "WeMakeColorsII";
char* MQTT_PASSWORD = "aieie";

String mqttRandomColor = "randomColor";
String mqttBeat = "beat";
String mqttConfig = "config";

String mqttPublishRandomColor = "";
String mqttPublishBeat = "";

String mqttSubscribeRandomColor = "";
String mqttSubscribeConfig = "";


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

// test
#define BOOT_TEST_LIGHT 2
#define BOOT_RESET 3
#define BOOT_TEST_BOARD 4

#define TEST_TIME 30000

//test connection
const char* SSID = "colors";
const char* PASSWORD = "colors01";

//beat
#define BEAT_INTERVAL 900000 //900000 60000
unsigned long last_beat = 0;

void setup() {

  FastLED.setBrightness(GLOBAL_BRIGHTNESS);
  FastLED.addLeds<WS2812B, LED_DATA_PIN, GRB>(leds, NUM_LEDS);
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

  if (c == BOOT_RESET) {
    WiFi.disconnect();
    writeAttribute("thingName", "");
    writeAttribute("mqttServer", "");
  }


  if (c == BOOT_TEST_BOARD) {
    connectWifi();
  } else {
    connectWifi_or_AP(c);
  }


  autoUpdate();

  s_thingName = readAttribute("thingName");
  if (s_thingName == "") {
    s_thingName = thingId;
  }

  Serial.println ("thingName=" + s_thingName);
  s_thingName.toCharArray(THING_NAME, MAX_PARAM);
  WiFi.hostname(s_thingName);

  s_mqttServer = readAttribute("mqttServer");
  if (s_mqttServer != "") {
    s_mqttServer.toCharArray(MQTT_SERVER, MAX_PARAM);
  }

  mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
  mqttClient.setCallback(mqtt_callback);
  mqttSubscribeRandomColor = mqttRoot + "/+/" + mqttRandomColor;
  mqttSubscribeConfig = mqttRoot + "/" + thingId + "/" + mqttConfig;

  mqttPublishRandomColor = mqttRoot + "/" + thingId + "/" + mqttRandomColor;
  mqttPublishBeat = mqttRoot + "/" + thingId + "/" + mqttBeat;

  setupOTA();

  //MDNS discovery
  MDNS.addServiceTxt("arduino", "tcp", "thingId", thingId);
  MDNS.addServiceTxt("arduino", "tcp", "thingName", s_thingName);
  MDNS.addServiceTxt("arduino", "tcp", "software", software);
  MDNS.update();

  int a = analogRead(inputPin);

  for (int i = 0; i < numReadings; i++) {
    readings[i] = a;
  }
  total = a * numReadings;
  //average = total / numReadings; average is not updated so we have  a new color at start

  for (int iGB = 0; iGB < numReadingsGB; iGB++) {
    readingsGB[iGB] = a;
  }
  totalGB = a * numReadingsGB;
  averageGB = totalGB / numReadingsGB;

  WiFi.setAutoReconnect(true);
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








