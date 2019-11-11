String softwareName = "WeMakeColorsII";
String softwareVersion = "1.9.10";
String softwareInfo = "";

bool echoMode = true; //legacy =flase

#define TEST false

#include "_userInterface.h"
#include "_light.h"
#include "_WiFi.h"
#include "_MQTT.h"
#include "_WiFiManager.h"

#if  (TEST)
bool test = false;
#include "_test.h"
#endif

void setup() {

  light_setup();

  Serial.begin(115200);  Serial.println();
  softwareInfo = softwareName + " - " + softwareVersion + " - " + ESP.getCoreVersion() + " - " + ESP.getSketchMD5() + " - " + String (ESP.getCpuFreqMHz()); // + " - " + String (__DATE__) + " - " + String(__TIME__);;
  Serial.println(softwareInfo);
  Serial.println(ESP.getFullVersion());

  IoT_setup();
  Serial.println("thingId: " + thingId);
  friendlyName = thingId;

  loadParametersFromFile();
  //mqttServer = "192.168.1.137";
  //mqttServer = "192.168.1.4";

  WiFi.hostname(friendlyName);

  byte bc = UI_setup();
  if (bc == BOOT_TEST_DEVICE)  testDevice();

  ledON();

  setWiFi() ;

  connectWiFi("PucciOffice", "Grandebellezza3");
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
  
#if  (TEST)
  //test_setup();
#endif
}

void loop() {

  mqtt_loop() ;

  if (newColor) {
    newColor = false;
    CHSV c = newRndColor();
    setMyLED(c);
    publishRandomColor(c);
  }

#if  (TEST)
  if (test) {
    test = false;
    publishStatusMQTT() ;
  }
#endif

#if  (LAN_OTA)
  ArduinoOTA.handle();
#endif

}
