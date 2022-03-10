// tested on ESP8266 Core 3.0.2 (not working) use 2.7.4
// Flash Size: "4MB (FS:1MB OTA:~1019KB)
// lwIP Variant: "v2 Lower Memory"
// https://arduino.esp8266.com/stable/package_esp8266com_index.json


#define WMCII 0
#define IOTKIT 1

#define BOARD_TYPE WMCII  // please define one of the above boards

boolean DEBUG_MAIN = true;


#include "_info.h"
#include "_miniUI.h"
#include "_app_status.h"
#include "_WiFi.h"

#include "_app.h"
#include "_MQTT.h"
#include "_WiFiManager.h"


void setup() {
  miniUI_Setup();
  softwareInfo_setup("");
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
  WebUpdate_setup() ;
  mqtt_setup();

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
      CHSV c = newRndColor();
      setLED(MY_LED, c);
      publishEvent(c);
    }
  }

  if (streamLEDs) {
    streamLEDs = false;
    Serial.flush();
    //yield();
    //noInterrupts();
    FastLED.show();
    //interrupts();
  }


#if BOARD_TYPE==IOTKIT
  if ( ((millis() - lastButtonTime) > BUTTON_INTERVAL) && !digitalRead(BUTTON) )  {
    lastButtonTime = millis();

    CHSV c = newRndColor();
    setLED(MY_LED, c);
    publishEvent(c);
  }
#endif

  if (newSettings) {
    newSettings = false;
    saveParametersToFile();
  }

}
