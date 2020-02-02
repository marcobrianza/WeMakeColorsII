boolean DEBUG_MAIN = true;

#include "_LED.h"
#include "_softwareInfo.h"
#include "_miniUI.h"
#include "_WiFi.h"
#include "_app.h"
#include "_MQTT.h"
#include "_WiFiManager.h"


void setup() {

  softwareInfo_setup();
  ledOFF();
  app_setup();

  loadParametersFromFile();

  byte bc = miniUI_bootCount();
  if (bc == BOOT_TEST_DEVICE)  app_testDevice();

  ledON();
  WiFi_setup();

  // test parameters
  //connectWiFi("PucciCube24", "Grandebellezza3!");
  //mqttServer = "192.168.1.5";

  switch  (bc) {
    case BOOT_RESET:
      if (DEBUG_MAIN) Serial.println("Reset parameters");
      saveParametersToFile();
      connectWiFi_Manager(true);
      break;

    case BOOT_DEFAULT_AP:
      if (DEBUG_MAIN) Serial.println("Reset parameters and connecting to default AP");
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
  OTA_setup();

  ledOFF();
  if (DEBUG_MAIN) Serial.println("starting loop");
}

void loop() {

  miniUI_loop();
  WiFi_loop();
  mqtt_loop();

  // app loop----------------

  if ((millis() - lastLightTime) > CHECK_LIGHT_TIME)  {
    lastLightTime = millis();
    if (checkLight()) {
      CHSV c = newRndColor();
      setMyLED(c);
      publishRandomColor(c);
    }
  }

}
