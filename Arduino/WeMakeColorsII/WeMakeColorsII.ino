String softwareName = "WeMakeColorsII";
String softwareVersion = "1.7.9";
String softwareInfo = "";

String mqttServer = "wmc.marcobrianza.it";
String mqttUsername = "";
String mqttPassword = "";


#include "_userInterface.h"
#include "_light.h"
#include "_IoT.h"
#include "_MQTT.h"


int NEW_COLOR_TIME = 1000;
int LOOP_DELAY = 5;

// test device
#define BOOT_TEST_LIGHT 2
#define BOOT_RESET 3
#define BOOT_DEFAULT_AP 4
#define BOOT_ESPTOUCH 5


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

  setup_IoT();

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

  T_upTime.attach(60, upTimeInc); //fires every minute
  T_globalBrighness.attach(1, setGlobalBrightness);
  //T_mqttConnect.attach(5, reconnectMQTT);

}

void loop() {

  unsigned long now = millis();

  mqttClient.loop();
  if (!mqttClient.connected())  {
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
  int TEST_TIME = 30000; // 30 seconds

  while (millis() < TEST_TIME) {
    int a = analogRead(inputPin);
    int v = a / 4;
    if (v > 255) v = 255;
    Serial.println(v);
    showAllLeds(v, v, v);
    delay(40);
  }
}
