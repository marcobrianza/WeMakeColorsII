String softwareName = "WeMakeColorsII";
String softwareVersion = "1.12.2";
String softwareInfo = "";
String softwarePlatform = "";

String appId = "WMCII";
String thingId = "";
String friendlyName = "";

#include <ESP8266WiFi.h>  // built in ESP8266 Core

void softwareInfo_setup() {

  Serial.begin(115200);  Serial.println();
  softwareInfo = softwareName + " - " + softwareVersion +  " - " + ESP.getSketchMD5() + " - " + String (ESP.getCpuFreqMHz()); // + " - " + String (__DATE__) + " - " + String(__TIME__);;
  softwarePlatform = ESP.getFullVersion();
  Serial.println(softwareInfo);
  Serial.println(softwarePlatform);

  Serial.println("resetReason=" + ESP.getResetReason());

  pinMode(LED_BUILTIN, OUTPUT);

  thingId = appId + "_" +  WiFi.macAddress().c_str();
  Serial.println("thingId: " + thingId);
  friendlyName = thingId;
}
