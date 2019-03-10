
String softwareName = "WeMakeColorsII";
String softwareVersion = "1.4.1"; // wifi manager paramenters all strings
String softwareInfo = "";

//Wi-Fi
#include <ESP8266WiFi.h>  // ESP8266 core 2.5.0
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
String thingName = "";

// name, prompt, default, length
WiFiManagerParameter wfm_thingName("thingName", "Thing Name", thingName.c_str(), MAX_PARAM);
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
#include <FastLED.h> // version  3.2.6

//presence
unsigned long last_color_t = 0;
int inputPin = A0;

int NEW_COLOR_TIME = 1000;
int LOOP_DELAY = 40;

// test device
#define BOOT_TEST_LIGHT 2
#define BOOT_RESET 3
#define BOOT_DEFAULT_AP 4

#define TEST_TIME 30000 // 30 seconds

//default ssid password
String defaultSSID = "colors";
String defaultPassword = "colors01";

String savedSSID = "";
String savedPassword = "";

//beat
#define BEAT_INTERVAL 900 // 900 seconds is 15 minutes

//led builtin
#define LED_ON LOW
#define LED_OFF HIGH

#include <Ticker.h>
Ticker T_mqtt_beat;
Ticker T_globalBrighness;

void setup() {

  setupLEDs();
  showAllLeds(64, 64, 64);

  Serial.begin(115200);  Serial.println();
  softwareInfo = softwareName + " - " + softwareVersion + " - " + ESP.getCoreVersion() + " - " + ESP.getSketchMD5();// + " - " + String (__DATE__) + " - " + String(__TIME__);;
  Serial.println(softwareInfo);
  thingId = getTHING_ID(appId);
  Serial.println("thingId: " + thingId);
  thingName = thingId;

  byte c = bootCount();
  Serial.print("\nboot count=");
  Serial.println(c);

  if (c == BOOT_TEST_LIGHT) {
    Serial.println("Test mode");
    testDevice();
  }

  loadParametersFromFile();
  WiFi.hostname(thingName);

  savedSSID = WiFi.SSID();
  savedPassword = WiFi.psk();

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
    default:
      connectWifi_or_AP(false);
      if (WiFi.status() != WL_CONNECTED) ESP.restart();

  }

  WiFi.setAutoReconnect(true);
  Serial.println("connected to network: " + WiFi.SSID());
  digitalWrite(LED_BUILTIN, LED_OFF);

  autoUpdate();
  setupMqtt();
  setupOTA();
  setupMdns();
  setupLightLevel();

  T_mqtt_beat.attach(BEAT_INTERVAL, publishBeat);
  T_globalBrighness.attach(1, setGlobalBrightness);

}

void loop() {

  unsigned long now = millis();

  if (!mqttClient.connected())  reconnectMQTT();
  mqttClient.loop();

  if (lightChange() && (now - last_color_t > NEW_COLOR_TIME)) {
    CHSV c = newRndColor();
    setMyLED(c);
    publishRandomColor(c);
    last_color_t = now;
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
