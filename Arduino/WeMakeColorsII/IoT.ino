#include <EEPROM.h> //boot Count

#define COUNT_ADDR 0

void loadParametersFromFile() {
  String temp = "";

  temp = readAttribute("thingName");
  if (temp != "") thingName = temp;
  Serial.println ("thingName=" + thingName);

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
  writeAttribute("thingName", thingName);
}

// -------------------------------------------
byte bootCount() {

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(500);

  EEPROM.begin(512);
  byte boot_count = EEPROM.read(COUNT_ADDR);
  boot_count++;
  for (int i = 0;  i  < boot_count; i++) {
    digitalWrite(LED_BUILTIN, LOW);
    delay(300);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(300);
  }

  EEPROM.write(COUNT_ADDR, boot_count);
  EEPROM.commit();

  delay(2000);
  digitalWrite(LED_BUILTIN, LOW);
  delay(100);
  digitalWrite(LED_BUILTIN, HIGH);

  EEPROM.write(COUNT_ADDR, 0);
  EEPROM.commit();
  return boot_count;
}


// ------THING_ID----------

String getTHING_ID(String appId) {
  byte ma[6];
  char* MAC = "00:11:22:33:44:55";
  String id;

  WiFi.macAddress(ma);
  sprintf(MAC, "%02X:%02X:%02X:%02X:%02X:%02X", ma[0], ma[1], ma[2], ma[3], ma[4], ma[5]);

  id = appId + "_" +  String(MAC);;
  return (id);
}


// ------ Wi-Fi Manager functions-------------------------------------



void connectWifi(String ssid, String password) {
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


void connectWifi_or_AP(bool force_config) {
  digitalWrite(LED_BUILTIN, LOW);

  WiFiManager wifiManager;
  wifiManager.setDebugOutput(true);
  wifiManager.setAPStaticIPConfig(IPAddress(1, 1, 1, 1), IPAddress(1, 1, 1, 1), IPAddress(255, 255, 255, 0));
  wifiManager.setMinimumSignalQuality(50); //default is 8
  wifiManager.setAPCallback(configModeCallback);
  wifiManager.setSaveConfigCallback(saveConfigCallback);

  wifiManager.addParameter(&wfm_thingName);
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
    wifiManager.setConfigPortalTimeout(300); //300s is 5 minutes
    wifiManager.autoConnect(thingId.c_str());
    Serial.println("Captive portal timeout");
  }

}



void configModeCallback (WiFiManager *myWiFiManager) {
  Serial.println("Entered config mode");
  Serial.println(WiFi.softAPIP());
  Serial.println(myWiFiManager->getConfigPortalSSID());
  showAllLeds(0, 0, 255);
}


void saveConfigCallback () {
  Serial.println("Config callback");
  if (String(wfm_thingName.getValue()) != "") thingName = wfm_thingName.getValue();
  if (String(wfm_mqttServer.getValue()) != "")  mqttServer = wfm_mqttServer.getValue();
  if (String(wfm_mqttUsername.getValue()) != "") mqttUsername = wfm_mqttUsername.getValue();
  if (String(wfm_mqttPassword.getValue()) != "") mqttPassword = wfm_mqttPassword.getValue();

  saveParametersToFile();
  showAllLeds(0, 255, 255);
}


//---- file attribute functions----

String readAttribute(String attributeName) {
  String value = "";
  SPIFFS.begin();
  File configFile = SPIFFS.open(attributeName + ".txt", "r");
  if (configFile) {
    value = configFile.readString();
    Serial.println("readAttribute: " + attributeName + "=" + value);
  } else {
    Serial.println("readAttribute: " + attributeName + "= null" );
  }
  configFile.close();
  SPIFFS.end();
  return value;
}

void writeAttribute(String attributeName, String value) {
  SPIFFS.begin();
  File configFile = SPIFFS.open(attributeName + ".txt", "w");
  if (configFile) {
    configFile.print(value);
    Serial.println("writeAttribute: " + attributeName + "=" + value);
  }
  configFile.close();
  SPIFFS.end();

}


// --------------- OTA Lan ---------------
void setupOTA() {
  ArduinoOTA.setPort(8266); // Port defaults to 8266
  ArduinoOTA.setHostname(thingName.c_str());

  //ArduinoOTA.setPassword((const char *) OTA_PASSWORD.c_str());
  ArduinoOTA.setPassword(OTA_PASSWORD.c_str());

  ArduinoOTA.onStart([]() {
    Serial.println("\nStart OTA");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd OTA");
    delay(100);
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


void setupMdns() {
  //MDNS discovery
  MDNS.addServiceTxt("arduino", "tcp", "thingId", thingId);
  MDNS.addServiceTxt("arduino", "tcp", "thingName", thingName);
  MDNS.addServiceTxt("arduino", "tcp", "softwareInfo", softwareInfo);
  MDNS.update();
}

void blink(int b) {
  for (int i = 0; i < b; i++) {
    digitalWrite(LED_BUILTIN, LED_ON);
    delay(100);
    digitalWrite(LED_BUILTIN, LED_OFF);
    delay(100);
  }
}
