

bool echoMode = true; //legacy =flase

#include "_softwareInfo.h"
#include "_miniUI.h"
#include "_light.h"
#include "_WiFi.h"
#include "_MQTT.h"
#include "_WiFiManager.h"

void setup() {

  softwareInfo_setup();
  ledOFF();
  light_setup();

  loadParametersFromFile();

  WiFi.hostname(friendlyName);

  byte bc = miniUI_bootCount();
  //byte bc = 0;
  if (bc == BOOT_TEST_DEVICE)  testDevice();

  setWiFiRadio() ;

  // test
  //connectWiFi("PucciCube24", "Grandebellezza3!");
  //mqttServer = "192.168.1.5";

  ledON();
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

  autoUpdate();

#if  (LAN_OTA)
  OTA_setup();
#endif
  setWiFiRadio() ;
  ledOFF();
  Serial.println("starting Application");
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
