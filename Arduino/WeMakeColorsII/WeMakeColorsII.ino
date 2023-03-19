// tested on ESP8266 Core 3.1.1
// Flash Size: "4MB (FS:2MB OTA:~1019KB)
// lwIP Variant: "v2 Lower Memory"
// https://arduino.esp8266.com/stable/package_esp8266com_index.json


#define WMCII 0
#define IOTKIT 1

#define BOARD_TYPE WMCII  // please define one of the above boards

boolean DEBUG_MAIN = true;

#include "info.h"
#include "miniUI.h"
#include "app_status.h"
#include "WiFi.h"

#include "app.h"
#include "MQTT.h"
#include "WiFiManager.h"


void setup() {
  miniUI_Setup();
  softwareInfo_setup("");
  app_setup();


  loadParametersFromFile();

  uint8_t bc = miniUI_bootCount();
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
  WebUpdate_setup() ;
  mqtt_setup();
  //setup_time();

  ledOFF();
  if (DEBUG_MAIN) Serial.println("BOARD_TYPE:" + String(BOARD_TYPE));
  if (DEBUG_MAIN) Serial.println("starting loop");


}

void loop() {

  miniUI_loop();
  WiFi_loop();
  mqtt_loop();

  // app loop----------------

  if ((millis() - lastAppTime) > APP_INTERVAL)  {
    lastAppTime = millis();
    if (checkLight()) {
      hsvColor_t c = newRndColor();
      setLED(MY_LED, c);
      publishEvent(c);
    }
  }



#if BOARD_TYPE==IOTKIT
  if ( ((millis() - lastButtonTime) > BUTTON_INTERVAL) && !digitalRead(BUTTON) )  {
    lastButtonTime = millis();

    hsvColor c = newRndColor();
    setLED(MY_LED, c);
    publishEvent(c);
  }
#endif

  if (newSettings) {
    newSettings = false;
    saveParametersToFile();
  }
  delay(1);
}
