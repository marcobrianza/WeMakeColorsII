#include <EEPROM.h> //boot Count

//Wi-Fi
#include <ESP8266WiFi.h>  // ESP8266 core 2.5.2
WiFiClient wifiClient;

//default ssid password
String defaultSSID = "colors";
String defaultPassword = "colors01";

//Wi-FiManager
#include "FS.h"
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h> // 0.14 
#define COUNT_ADDR 0

String thingId = "";
String appId = "WMCII";
String friendlyName = "";

#define MAX_PARAM 40

// name, prompt, default, length
WiFiManagerParameter wfm_friendlyName("friendlyName", "Friendly Name", friendlyName.c_str(), MAX_PARAM);
WiFiManagerParameter wfm_mqttServer("mqttServer", "MQTT Server", mqttServer.c_str(), MAX_PARAM);
WiFiManagerParameter wfm_mqttUsername("mqttUsername", "MQTT Username", mqttUsername.c_str(), MAX_PARAM);
WiFiManagerParameter wfm_mqttPassword("mqttPassword", "MQTT Password", mqttPassword.c_str(), MAX_PARAM);


#define CAPTIVE_TIMEOUT 300
#define CAPTIVE_SIGNAL_QUALITY 10

#define BLINK_NO_SSID 1
#define BLINK_CONNECTION_ERROR 2

int netStatus = 0; //0=undefined 1=WiFi connected 2=MQTT connected -1 WiFi not Connected

unsigned int upTime = 0;
//status
#define STATUS_INTERVAL 5 //minutes
boolean publishStatus = false;

//OTA
#include <ESP8266mDNS.h>
#include <ArduinoOTA.h>
String OTA_PASSWORD = "12345678";

//http update
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>

String MD5_URL = "http://iot.marcobrianza.it/art/WeMakeColorsII.md5.txt";
String FW_URL = "http://iot.marcobrianza.it/art/WeMakeColorsII.ino.d1_mini.bin";



void setWiFi() {
  WiFi.setOutputPower(20.5); // 20.5 is maximum power
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);

  //Serial.println(wifi_get_sleep_type());
  wifi_set_sleep_type(NONE_SLEEP_T);

  //Serial.println("PhyMode:" + String( WiFi.getPhyMode()));
  WiFi.setPhyMode(WIFI_PHY_MODE_11N);   // WIFI_PHY_MODE_11B = 1, WIFI_PHY_MODE_11G = 2, WIFI_PHY_MODE_11N = 3
  //Serial.println("PhyMode:" + String( WiFi.getPhyMode()));
}


void setup_IoT() {
  thingId = appId + "_" +  WiFi.macAddress().c_str();

}



byte bootCount() {

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LED_OFF);
  delay(500);

  EEPROM.begin(512);
  byte boot_count = EEPROM.read(COUNT_ADDR);
  boot_count++;
  for (int i = 0;  i  < boot_count; i++) {
    digitalWrite(LED_BUILTIN, LED_ON);
    delay(300);
    digitalWrite(LED_BUILTIN, LED_OFF);
    delay(300);
  }

  EEPROM.write(COUNT_ADDR, boot_count);
  EEPROM.commit();

  delay(2000);

  EEPROM.write(COUNT_ADDR, 0);
  EEPROM.commit();
  return boot_count;
}



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





void connectWiFi(String ssid, String password) {
  WiFi.disconnect();
  delay(100);
  // We start by connecting to a WiFi network
  Serial.print("Connecting to: "); Serial.println(ssid);

  WiFi.begin(ssid.c_str(), password.c_str());

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  IPAddress ip = WiFi.localIP();
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(ip);
  WiFi.setAutoReconnect(true);
}


int checkWiFiStatus() {

  String s = "";
  int c = WiFi.status();
  int b = 0;

  switch (c) {
    case WL_CONNECTED:
      s = "WL_CONNECTED";
      break;
    case WL_NO_SSID_AVAIL:
      s = "WL_NO_SSID_AVAIL";
      b = BLINK_NO_SSID;
      break;
    case WL_CONNECT_FAILED:
      s = "WL_CONNECT_FAILED";
      b = BLINK_CONNECTION_ERROR;
      break;
    case WL_IDLE_STATUS:
      s = "WL_IDLE_STATUS";
      break;
    case WL_DISCONNECTED:
      s = "WL_DISCONNECTED";
      b = BLINK_NO_SSID;
      break;
    case WL_CONNECTION_LOST:
      s = "WL_CONNECTION_LOST";
      break;
    default:
      s = "UNKNOWN";
      break;
  }

  if (c != WL_CONNECTED) Serial.println( "WiFi Status=" + String(c) + " " + s);
  if (b > 0) blink(b);
  return c;
}


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


void connectWiFi_or_AP(bool force_config) {

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



// --------------- OTA Lan ---------------


void setupOTA() {


  ArduinoOTA.setPort(8266); // Port defaults to 8266
  ArduinoOTA.setHostname(friendlyName.c_str());

  //ArduinoOTA.setPassword((const char *) OTA_PASSWORD.c_str());
  ArduinoOTA.setPassword(OTA_PASSWORD.c_str());

  ArduinoOTA.onStart([]() {
    Serial.println("\nStart OTA");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd OTA");
    //delay(100);
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });

  ArduinoOTA.begin();

  //MDNS discovery additiona info
  MDNS.addServiceTxt("arduino", "tcp", "thingId", thingId);
  MDNS.addServiceTxt("arduino", "tcp", "friendlyName", friendlyName);
  MDNS.addServiceTxt("arduino", "tcp", "softwareInfo", softwareInfo);
  MDNS.update();

  Serial.println("OTA Ready");
}



// -----  http update ----------------

int httpUpdate(String url) {
  t_httpUpdate_return ret = ESPhttpUpdate.update(url);

  switch (ret) {
    case HTTP_UPDATE_FAILED:
      Serial.printf("HTTP_UPDATE_FAILD Error (%d): %s", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
      break;
    case HTTP_UPDATE_NO_UPDATES:
      Serial.println("HTTP_UPDATE_NO_UPDATES");
      break;
    case HTTP_UPDATE_OK:
      Serial.println("HTTP_UPDATE_OK");
      break;
  }
}


void autoUpdate() {
  Serial.println("Autoupdate...");
  HTTPClient http;
  http.begin(MD5_URL);
  int httpCode = http.GET();

  if (httpCode > 0) {
    // HTTP header has been send and Server response header has been handled
    Serial.print("[HTTP] GET " + MD5_URL);
    Serial.println("... code:" + String(httpCode));

    // file found at server
    if (httpCode == HTTP_CODE_OK) {
      String md5 = http.getString();
      Serial.println("Server md5: " + md5);
      Serial.println(" Board md5: " + ESP.getSketchMD5());
      if ((ESP.getSketchMD5() != md5) && (md5.length() == 32) ) {
        Serial.println("Trying update...");
        showAllLeds(64, 0, 0);
        blink(10);

        int u = httpUpdate(FW_URL);
        if (u != HTTP_UPDATE_OK) showAllLeds(64, 64, 0);
      }
      else {
        Serial.println("will not update");
      }

    }
    else {
      Serial.println( "[HTTP] GET... failed, error:" + String(http.errorToString(httpCode).c_str()));
    }
    http.end();
  }
  Serial.println();
}
