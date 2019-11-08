String softwareName = "WeMakeColorsII";
String softwareVersion = "1.9.3";
String softwareInfo = "";

String mqttServer = "wmc.marcobrianza.it";
String mqttUsername = "";
String mqttPassword = "";

bool echoMode = true; //legacy =flase

#define LAN_OTA false

#include "_userInterface.h"
#include "_light.h"
#include "_WiFi.h"
#include "_MQTT.h"

// test device
#define BOOT_TEST_LIGHT 2
#define BOOT_RESET 3
#define BOOT_DEFAULT_AP 4
#define BOOT_ESPTOUCH 5


void setup() {

  setupLight();

  Serial.begin(115200);  Serial.println();
  softwareInfo = softwareName + " - " + softwareVersion + " - " + ESP.getCoreVersion() + " - " + ESP.getSketchMD5();// + " - " + String (__DATE__) + " - " + String(__TIME__);;
  Serial.println(softwareInfo);

  setup_IoT();
  Serial.println("thingId: " + thingId);
  friendlyName = thingId;

  setWiFi() ;

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

  setWiFi() ;

  switch  (c) {
    case BOOT_DEFAULT_AP:
      Serial.println("Reset parameters and connect to default AP");
      saveParametersToFile();
      connectWiFi(defaultSSID, defaultPassword);
      break;
    case BOOT_RESET:
      Serial.println("Reset parameters");
      saveParametersToFile();
      connectWiFi_or_AP(true);
      break;
    case BOOT_ESPTOUCH:
      Serial.println("Starting ESPTouch SmartConfig");
      WiFi.beginSmartConfig();
      break;
    default:
      connectWiFi_or_AP(false);
  }


  Serial.println("-------WiFi connection status:-----");
  WiFi.printDiag(Serial);
  Serial.println("-----------------------------------");

  digitalWrite(LED_BUILTIN, LED_OFF);

  autoUpdate();

#if  (LAN_OTA)
  setupOTA();
#endif

  setupMqtt();
  startLight();

}

void loop() {

  mqtt_loop() ;

  if (newColor) {
    newColor = false;
    CHSV c = newRndColor();
    setMyLED(c);
    publishRandomColor(c);
  }


#if  (LAN_OTA)
  ArduinoOTA.handle();
#endif

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
