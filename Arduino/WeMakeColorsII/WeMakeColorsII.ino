

bool echoMode = true; //legacy =flase

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

  WiFi.hostname(friendlyName);

  byte bc = miniUI_bootCount();
  //byte bc = 0;
  if (bc == BOOT_TEST_DEVICE)  testDevice();

  WiFi_setup() ;

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


  ledOFF();
  Serial.println("starting Application");
}

void loop() {

  miniUI_loop();
  WiFi_loop();
  mqtt_loop() ;

// app loop----------------
  unsigned long m = millis();
  if ((m - lastLightTime) > CHECK_LIGHT_TIME)  {
    lastLightTime = m;
    checkLight();
  }

  if ((m - lastBrightnessTime) > GLOBAL_BRIGHTNESS_TIME) {
    lastBrightnessTime = m;
    setGlobalBrightness();
  }

  if (newColor) {
    newColor = false;
    CHSV c = newRndColor();
    setMyLED(c);
    publishRandomColor(c);
  }

}
