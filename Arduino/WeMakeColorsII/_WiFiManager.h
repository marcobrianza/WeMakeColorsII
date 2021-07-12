boolean DEBUG_WIFIMANAGER = true;

//Wi-FiManager
#include "FS.h"
#include <DNSServer.h> // built in ESP8266 Core
#include <ESP8266WebServer.h> // built in ESP8266 Core
#include <WiFiManager.h> // 0.16.0 

#define MAX_PARAM 40

#define CAPTIVE_TIMEOUT 300  //300s is 5 minutes
#define CAPTIVE_SIGNAL_QUALITY 10 //default is 8

// name, prompt, default, length
WiFiManagerParameter wfm_name("name", "name", name.c_str(), MAX_PARAM);
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
      if (DEBUG_WIFIMANAGER) Serial.println("readAttribute: " + attributeName + "=" + value);
    } else {
      if (DEBUG_WIFIMANAGER) Serial.println("readAttribute: " + attributeName + "= null" );
    }
    configFile.close();
    SPIFFS.end();
  } else  if (DEBUG_WIFIMANAGER) Serial.println("readAttribute ERROR: cannot open file system");
  return value;
}

void deleteAttribute(String attributeName) {
  if (SPIFFS.begin()) {
    String value = "";
    SPIFFS.remove(attributeName + ".txt");
  } else if (DEBUG_WIFIMANAGER)  Serial.println("deleteAttribute ERROR: cannot open file system");
}

void writeAttribute(String attributeName, String value) {
  if (SPIFFS.begin()) {
    File configFile = SPIFFS.open(attributeName + ".txt", "w");
    if (configFile) {
      configFile.print(value);
      if (DEBUG_WIFIMANAGER) Serial.println("writeAttribute: " + attributeName + "=" + value);
    } else if (DEBUG_WIFIMANAGER)   Serial.println("writeAttribute: ERROR cannot open file " + attributeName);
    configFile.close();
    SPIFFS.end();
  } else if (DEBUG_WIFIMANAGER) Serial.println("writeAttribute ERROR: cannot open file system");
}


void loadParametersFromFile() {
  String temp = "";

  // convert the old attributes
  temp = readAttribute("thingName");
  if (temp != "") {
    if (DEBUG_WIFIMANAGER) Serial.println ("thingName=" + name);
    writeAttribute("name", temp);
    deleteAttribute("thingName");
  }

  temp = readAttribute("friendlyName");
  if (temp != "") {
    if (DEBUG_WIFIMANAGER) Serial.println ("friendlyName=" + name);
    writeAttribute("name", temp);
    deleteAttribute("friendlyName");
  }



  temp = readAttribute("name");
  if (temp != "") name = temp;
  if (DEBUG_WIFIMANAGER) Serial.println ("name=" + name);

  temp = readAttribute("mqttServer");
  if (temp != "")  mqttServer = temp;
  if (DEBUG_WIFIMANAGER) Serial.println ("mqttServer=" + mqttServer);

  temp = readAttribute("mqttUsername");
  if (temp != "") {
    mqttUsername = temp;
  } else {
    mqttUsername = thingId; // if username is null it defaults to thingId
  }
  if (DEBUG_WIFIMANAGER) Serial.println ("mqttUsername=" + mqttUsername);

  temp = readAttribute("mqttPassword");
  if (temp != "")  mqttPassword = temp;
  if (DEBUG_WIFIMANAGER) Serial.println ("mqttPassword=" + mqttPassword);

}


void saveParametersToFile() {
  writeAttribute("mqttServer", mqttServer);
  writeAttribute("mqttUsername", mqttUsername);
  writeAttribute("mqttPassword", mqttPassword);
  writeAttribute("name", name);
}

// -------------------------------------------



// ------ Wi-Fi Manager functions-------------------------------------

void configModeCallback (WiFiManager *myWiFiManager) {
  if (DEBUG_WIFIMANAGER) {
    Serial.println("Entered config mode");
    Serial.println(WiFi.softAPIP());
    Serial.println(myWiFiManager->getConfigPortalSSID());
  }
  showState(WIFI_MANAGER_AP);
}


void saveConfigCallback () {
  if (DEBUG_WIFIMANAGER)  Serial.println("Config callback");
  if (String(wfm_name.getValue()) != "") name = wfm_name.getValue();
  if (String(wfm_mqttServer.getValue()) != "")  mqttServer = wfm_mqttServer.getValue();
  if (String(wfm_mqttUsername.getValue()) != "") mqttUsername = wfm_mqttUsername.getValue();
  if (String(wfm_mqttPassword.getValue()) != "") mqttPassword = wfm_mqttPassword.getValue();

  saveParametersToFile();
  showState(WIFI_MANAGER_SAVE);

}


void connectWiFi_Manager(bool force_config) {


  WiFiManager wifiManager;

  if (DEBUG_WIFIMANAGER)  wifiManager.setDebugOutput(true);
  else wifiManager.setDebugOutput(false);

  wifiManager.setAPStaticIPConfig(IPAddress(10, 1, 1, 1), IPAddress(10, 1, 1, 1), IPAddress(255, 255, 255, 0));
  wifiManager.setMinimumSignalQuality(CAPTIVE_SIGNAL_QUALITY);
  wifiManager.setAPCallback(configModeCallback);
  wifiManager.setSaveConfigCallback(saveConfigCallback);

  wifiManager.addParameter(&wfm_name);
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
    wifiManager.setConfigPortalTimeout(CAPTIVE_TIMEOUT);
    wifiManager.autoConnect(thingId.c_str());
    if (DEBUG_WIFIMANAGER)  Serial.println("WiFiManager end");

    if (WiFi.SSID() == "") {
      if (DEBUG_WIFIMANAGER)  Serial.println("no SSID found, connecting with previously saved credentials");
      WiFi.begin(savedSSID, savedPassword);
    } else {
      if (DEBUG_WIFIMANAGER)  Serial.println("new network credentials: " + WiFi.SSID() + " " + WiFi.psk() );
    }

  }

  if (DEBUG_WIFIMANAGER) {
    Serial.println("-------WiFi connection status:-----");
    WiFi.printDiag(Serial);
    Serial.println("-----------------------------------");
  }
}
