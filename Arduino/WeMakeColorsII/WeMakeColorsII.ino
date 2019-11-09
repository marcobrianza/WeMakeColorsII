String softwareName = "WeMakeColorsII";
String softwareVersion = "1.9.6";
String softwareInfo = "";

bool echoMode = true; //legacy =flase

#include "_userInterface.h"
#include "_light.h"
#include "_WiFi.h"
#include "_MQTT.h"
#include "_WiFiManager.h"


void setup() {

  light_setup();

  Serial.begin(115200);  Serial.println();
  softwareInfo = softwareName + " - " + softwareVersion + " - " + ESP.getCoreVersion() + " - " + ESP.getSketchMD5();// + " - " + String (__DATE__) + " - " + String(__TIME__);;
  Serial.println(softwareInfo + +" @" + String (ESP.getCpuFreqMHz()) + "MHz" );

  IoT_setup();
  Serial.println("thingId: " + thingId);
  friendlyName = thingId;

  loadParametersFromFile();
  //mqttServer = "192.168.1.138";

  WiFi.hostname(friendlyName);

  byte c = bootCount();
  Serial.print("\nboot count=");
  Serial.println(c);

  if (c == BOOT_TEST_LIGHT) {
    Serial.println("Test mode");
    testDevice();
  }

  ledON();

  setWiFi() ;

  //connectWiFi("PucciThings", "Grandebellezza3");

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

  ledOFF();

  autoUpdate();

#if  (LAN_OTA)
  OTA_setup();
#endif

  mqtt_setup();
  light_start();

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
