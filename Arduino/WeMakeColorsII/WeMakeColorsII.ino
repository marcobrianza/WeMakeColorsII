
String softwareName = "WeMakeColorsII";
String softwareVersion = "1.6.0";
String softwareInfo = "";

//Wi-Fi
#include <ESP8266WiFi.h>  // ESP8266 core 2.5.2
WiFiClient wifiClient;

//MQTT
#include <PubSubClient.h> // version 2.7.0 in PubSubClient.h change #define MQTT_MAX_PACKET_SIZE 512 
#include <ArduinoJson.h> // version 5.13.5
PubSubClient mqttClient(wifiClient);

String mqttServer = "wmc.marcobrianza.it";
String mqttUsername = "";
String mqttPassword = "";
int MQTT_PORT = 1883;

//Wi-FiManager
#include "FS.h"
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h> // 0.14 

#define MAX_PARAM 40
#define MQTT_MAX MQTT_MAX_PACKET_SIZE

String thingId = "";
String appId = "WMCII";
String friendlyName = "";

// name, prompt, default, length
WiFiManagerParameter wfm_friendlyName("friendlyName", "Friendly Name", friendlyName.c_str(), MAX_PARAM);
WiFiManagerParameter wfm_mqttServer("mqttServer", "MQTT Server", mqttServer.c_str(), MAX_PARAM);
WiFiManagerParameter wfm_mqttUsername("mqttUsername", "MQTT Username", mqttUsername.c_str(), MAX_PARAM);
WiFiManagerParameter wfm_mqttPassword("mqttPassword", "MQTT Password", mqttPassword.c_str(), MAX_PARAM);

//OTA
#include <ESP8266mDNS.h>
#include <ArduinoOTA.h>
String OTA_PASSWORD = "12345678";

//http update
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>

String MD5_URL = "http://iot.marcobrianza.it/art/WeMakeColorsII.md5.txt";
String FW_URL = "http://iot.marcobrianza.it/art/WeMakeColorsII.ino.d1_mini.bin";

//LED
#include <FastLED.h> // version  3.2.10

//presence
unsigned long last_color_t = 0;
int inputPin = A0;

int NEW_COLOR_TIME = 1000;
int LOOP_DELAY = 40;

// test device
#define BOOT_TEST_LIGHT 2
#define BOOT_RESET 3
#define BOOT_DEFAULT_AP 4
#define BOOT_ESPTOUCH 5

#define TEST_TIME 30000 // 30 seconds

//default ssid password
String defaultSSID = "colors";
String defaultPassword = "colors01";

//status
#define STATUS_INTERVAL 15 //minutes
boolean publishStatus = false;

//led builtin
#define LED_ON LOW
#define LED_OFF HIGH

#include <Ticker.h>
Ticker T_upTime;
Ticker T_globalBrighness;
//Ticker T_mqttConnect;

void setup() {

  setupLEDs();
  showAllLeds(64, 64, 64);

  Serial.begin(115200);  Serial.println();
  softwareInfo = softwareName + " - " + softwareVersion + " - " + ESP.getCoreVersion() + " - " + ESP.getSketchMD5();// + " - " + String (__DATE__) + " - " + String(__TIME__);;
  Serial.println(softwareInfo);
  //thingId = getTHING_ID(appId);

  thingId = appId + "_" +  WiFi.macAddress().c_str();

  Serial.println("thingId: " + thingId);
  friendlyName = thingId;

  loadParametersFromFile();
  WiFi.hostname(friendlyName);

  byte c = bootCount();
  Serial.print("\nboot count=");
  Serial.println(c);

  if (c == BOOT_TEST_LIGHT) {
    Serial.println("Test mode");
    testDevice();
  }

  digitalWrite(LED_BUILTIN, LED_ON);
  switch  (c) {
    case BOOT_DEFAULT_AP:
      Serial.println("Reset parameters and connect to default AP");
      saveParametersToFile();
      connectWifi(defaultSSID, defaultPassword);
      break;
    case BOOT_RESET:
      Serial.println("Reset parameters");
      saveParametersToFile();
      connectWifi_or_AP(true);
      break;
    case BOOT_ESPTOUCH:
      Serial.println("Starting ESPTouch SmartConfig");
      WiFi.beginSmartConfig();
      break;
    default:
      connectWifi_or_AP(false);

  }

  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  Serial.println("WiFi connection status: ");
  WiFi.printDiag(Serial);
  digitalWrite(LED_BUILTIN, LED_OFF);

  autoUpdate();
  setupMqtt();
  setupOTA();
  setupMdns();
  setupLightLevel();

  T_upTime.attach(60, upTimeInc);
  T_globalBrighness.attach(1, setGlobalBrightness);
  //T_mqttConnect.attach(5, reconnectMQTT);

}

void loop() {

  unsigned long now = millis();

  if (mqttClient.connected())  {
    mqttClient.loop();
  } else {
    reconnectMQTT();
  }

  if (lightChange() && (now - last_color_t > NEW_COLOR_TIME)) {
    CHSV c = newRndColor();
    setMyLED(c);
    publishRandomColor(c);
    last_color_t = now;
  }


  if (publishStatus) {
    publishStatus = false;
    publishStatusMQTT();
  }

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
