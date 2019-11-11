
//Wi-FiManager
#include "FS.h"
#include <DNSServer.h> // built in ESP8266 Core
#include <ESP8266WebServer.h> // built in ESP8266 Core
#include <WiFiManager.h> // 0.14 

#define MAX_PARAM 40

#define CAPTIVE_TIMEOUT 300
#define CAPTIVE_SIGNAL_QUALITY 10

// name, prompt, default, length
WiFiManagerParameter wfm_friendlyName("friendlyName", "Friendly Name", friendlyName.c_str(), MAX_PARAM);
WiFiManagerParameter wfm_mqttServer("mqttServer", "MQTT Server", mqttServer.c_str(), MAX_PARAM);
WiFiManagerParameter wfm_mqttUsername("mqttUsername", "MQTT Username", mqttUsername.c_str(), MAX_PARAM);
WiFiManagerParameter wfm_mqttPassword("mqttPassword", "MQTT Password", mqttPassword.c_str(), MAX_PARAM);


//---- file attribute functions----

String readAttribute(String attributeName) {
  String value = "";
  if (SPIFFS.begin()) {
    File configFile = SPIFFS.open(attributeName + ".txt", "r");
    if (configFile) {
      value = configFile.readString();
      Serial.println("readAttribute: " + attributeName + "=" + value);
    } else {
      Serial.println("readAttribute: " + attributeName + "= null" );
    }
    configFile.close();
    SPIFFS.end();
  } else Serial.println("readAttribute ERROR: cannot open file system");
  return value;
}

String deleteAttribute(String attributeName) {
  if (SPIFFS.begin()) {
    String value = "";
    SPIFFS.remove(attributeName + ".txt");
  } else Serial.println("deleteAttribute ERROR: cannot open file system");
}

void writeAttribute(String attributeName, String value) {
  if (SPIFFS.begin()) {
    File configFile = SPIFFS.open(attributeName + ".txt", "w");
    if (configFile) {
      configFile.print(value);
      Serial.println("writeAttribute: " + attributeName + "=" + value);
    } else  Serial.println("writeAttribute: ERROR cannot open file " + attributeName);
    configFile.close();
    SPIFFS.end();
  } else Serial.println("writeAttribute ERROR: cannot open file system");
}


void loadParametersFromFile() {
  String temp = "";

  // convert the old attribute
  temp = readAttribute("thingName");
  if (temp != "") {
    Serial.println ("thingName=" + friendlyName);
    writeAttribute("friendlyName", temp);
    deleteAttribute("thingName");
  }


  temp = readAttribute("friendlyName");
  if (temp != "") friendlyName = temp;
  Serial.println ("friendlyName=" + friendlyName);

  temp = readAttribute("mqttServer");
  if (temp != "")  mqttServer = temp;
  Serial.println ("mqttServer=" + mqttServer);

  temp = readAttribute("mqttUsername");
  if (temp != "") {
    mqttUsername = temp;
  } else {
    mqttUsername = thingId; // if username is null it defaults to thingId
  }
  Serial.println ("mqttUsername=" + mqttUsername);

  temp = readAttribute("mqttPassword");
  if (temp != "")  mqttPassword = temp;
  Serial.println ("mqttPassword=" + mqttPassword);

}


void saveParametersToFile() {
  writeAttribute("mqttServer", mqttServer);
  writeAttribute("mqttUsername", mqttUsername);
  writeAttribute("mqttPassword", mqttPassword);
  writeAttribute("friendlyName", friendlyName);
}

// -------------------------------------------



// ------ Wi-Fi Manager functions-------------------------------------

void configModeCallback (WiFiManager *myWiFiManager) {
  Serial.println("Entered config mode");
  Serial.println(WiFi.softAPIP());
  Serial.println(myWiFiManager->getConfigPortalSSID());
  showAllLeds(0, 0, 255);
}


void saveConfigCallback () {
  Serial.println("Config callback");
  if (String(wfm_friendlyName.getValue()) != "") friendlyName = wfm_friendlyName.getValue();
  if (String(wfm_mqttServer.getValue()) != "")  mqttServer = wfm_mqttServer.getValue();
  if (String(wfm_mqttUsername.getValue()) != "") mqttUsername = wfm_mqttUsername.getValue();
  if (String(wfm_mqttPassword.getValue()) != "") mqttPassword = wfm_mqttPassword.getValue();

  saveParametersToFile();
  showAllLeds(0, 255, 255);
}


void connectWiFi_Manager(bool force_config) {

  WiFiManager wifiManager;
  wifiManager.setDebugOutput(true);
  wifiManager.setAPStaticIPConfig(IPAddress(1, 1, 1, 1), IPAddress(1, 1, 1, 1), IPAddress(255, 255, 255, 0));
  wifiManager.setMinimumSignalQuality(CAPTIVE_SIGNAL_QUALITY); //default is 8
  wifiManager.setAPCallback(configModeCallback);
  wifiManager.setSaveConfigCallback(saveConfigCallback);

  wifiManager.addParameter(&wfm_friendlyName);
  wifiManager.addParameter(&wfm_mqttServer);
  wifiManager.addParameter(&wfm_mqttUsername);
  wifiManager.addParameter(&wfm_mqttPassword);


  if ( force_config == true) { //config must be done
    WiFi.disconnect();
    wifiManager.resetSettings(); //reset saved settings
    wifiManager.setConfigPortalTimeout(0);
    wifiManager.startConfigPortal(thingId.c_str());
  } else
  {
    wifiManager.setConfigPortalTimeout(CAPTIVE_TIMEOUT); //300s is 5 minutes
    wifiManager.autoConnect(thingId.c_str());
    Serial.println("Captive portal timeout");
  }
}
