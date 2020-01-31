boolean DEBUG_SOFTWAREINFO = true;

String softwareName = "WeMakeColorsII";
String softwareVersion = "1.13.0";
String softwareInfo = "";
String softwarePlatform = "";

String appId = "WMCII";
String thingId = "";
String friendlyName = "";

#include <ESP8266WiFi.h>  // built in ESP8266 Core

void softwareInfo_setup() {

  softwareInfo = softwareName + " - " + softwareVersion +  " - " + ESP.getSketchMD5() + " - " + String (ESP.getCpuFreqMHz()); // + " - " + String (__DATE__) + " - " + String(__TIME__);;
  softwarePlatform = ESP.getFullVersion();
  thingId = appId + "_" +  WiFi.macAddress().c_str();
  friendlyName = thingId;

  pinMode(LED_BUILTIN, OUTPUT);

  Serial.begin(115200);

  if (DEBUG_SOFTWAREINFO) {
    Serial.println("");
    Serial.println(softwareInfo);
    Serial.println(softwarePlatform);
    Serial.println("resetReason=" + ESP.getResetReason());
    Serial.println("thingId: " + thingId);
  }

}
