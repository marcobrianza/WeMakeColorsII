String softwareName = "WeMakeColorsII";
String softwareVersion = "1.9.15";
String softwareInfo = "";
String softwarePlatform = "";

bool echoMode = true; //legacy =flase

#include "_miniUI.h" 
#include "_light.h"
#include "_WiFi.h"
#include "_MQTT.h"
#include "_WiFiManager.h"

void setup() {

  light_setup();

  Serial.begin(115200);  Serial.println();
  softwareInfo = softwareName + " - " + softwareVersion +  " - " + ESP.getSketchMD5() + " - " + String (ESP.getCpuFreqMHz()); // + " - " + String (__DATE__) + " - " + String(__TIME__);;
  softwarePlatform = ESP.getFullVersion();
  Serial.println(softwareInfo);
  Serial.println(softwarePlatform);

  IoT_setup();
  Serial.println("thingId: " + thingId);
  friendlyName = thingId;

  loadParametersFromFile();
  //mqttServer = "192.168.1.137";
  // mqttServer = "192.168.1.1";

  WiFi.hostname(friendlyName);

  byte bc = miniUI_setup();
  if (bc == BOOT_TEST_DEVICE)  testDevice();

  ledON();

  // connectWiFi("PucciOffice", "Grandebellezza3");
  //connectWiFi("PucciThings", "Grandebellezza3");

  switch  (bc) {
    case BOOT_RESET:
      Serial.println("Reset parameters");
      saveParametersToFile();
      connectWiFi_Manager(true);
      break;

    case BOOT_DEFAULT_AP:
      Serial.println("Reset parameters and connecting to default AP");
      saveParametersToFile();
      connectWiFi("", "");
      break;

    case BOOT_ESPTOUCH:
      connectWiFi_Smart();
      break;
    default:
      connectWiFi_Manager(false);
  }

  ledOFF();

  autoUpdate();

  mqtt_setup();

#if  (LAN_OTA)
  OTA_setup();
#endif

}

void loop() {

  mqtt_loop() ;
  light_loop() ;
  miniUI_loop();

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
