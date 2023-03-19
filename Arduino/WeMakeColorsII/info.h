boolean DEBUG_INFO = true;

#include <ESP8266WiFi.h>  // built in ESP8266 Core

String softwareName = "WeMakeColorsII";
String softwareVersion = "1.91.0";
String appId = "WMCII";

String softwareInfo = "";
String softwarePlatform = "";

String thingId = "";
String name = "";

void softwareInfo_setup(String n) {
  Serial.begin(115200);
  
  if (n == "") name = thingId;
  else   name = n;

  softwareInfo = softwareName + " - " + softwareVersion +  " - " + ESP.getSketchMD5() + " - " + String (ESP.getCpuFreqMHz()); // + " - " + String (__DATE__) + " - " + String(__TIME__);;
  softwarePlatform = ESP.getFullVersion();
  thingId = appId + "_" +  WiFi.macAddress().c_str();
  name = thingId;



  if (DEBUG_INFO) {
    Serial.println("\n");
    Serial.println(softwareInfo);
    Serial.println(softwarePlatform);
    Serial.println("resetReason=" + ESP.getResetReason());
    Serial.println("thingId: " + thingId);
    Serial.println("name: " + name);
    Serial.flush();
  }

}
