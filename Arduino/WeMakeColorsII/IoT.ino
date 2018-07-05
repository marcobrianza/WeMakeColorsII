#include <EEPROM.h>

#define COUNT_ADDR 0

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

void getTHING_ID() {
  byte ma[6];
  char* MAC = "11:22:33:44:55:66";

  WiFi.macAddress(ma);
  sprintf(MAC, "%02X:%02X:%02X:%02X:%02X:%02X", ma[0], ma[1], ma[2], ma[3], ma[4], ma[5]);

  thingId = appId + String(MAC);
  thingId.toCharArray(THING_ID, thingId.length() + 1);

  Serial.print("THING_ID: ");
  Serial.println(THING_ID);
}

// ------ Wi-Fi Manager -------------------------------------

void connectWifi_or_AP(bool force_config) {
  digitalWrite(LED_BUILTIN, LOW);

  //  WiFi.disconnect();
  //  delay(100);

  WiFiManager wifiManager;
  wifiManager.setDebugOutput(true);
  wifiManager.setAPStaticIPConfig(IPAddress(1, 1, 1, 1), IPAddress(1, 1, 1, 1), IPAddress(255, 255, 255, 0));
  wifiManager.setMinimumSignalQuality(50); //default is 8
  wifiManager.setAPCallback(configModeCallback);
  wifiManager.setSaveConfigCallback(saveConfigCallback);

  wifiManager.addParameter(&wfm_thingName);
  wifiManager.addParameter(&wfm_mqttServer);


  if ( force_config == true) { //config must be done
    WiFi.disconnect();
    wifiManager.resetSettings(); //reset saved settings
    wifiManager.setConfigPortalTimeout(0);
    wifiManager.startConfigPortal(THING_ID);
  } else
  {
    wifiManager.setConfigPortalTimeout(300); //5 minutes
    wifiManager.autoConnect(THING_ID);
  }

  boolean led = false;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    digitalWrite(LED_BUILTIN, led);
    led = !led;
  }

  //if you get here you have connected to the WiFi
  Serial.print("connected to network: ");
  Serial.println(WiFi.SSID());

  digitalWrite(LED_BUILTIN, HIGH);
  WiFi.mode(WIFI_STA);

}


void connectWifi() {
  WiFi.disconnect();
  delay(100);
  // We start by connecting to a WiFi network
  Serial.print("Connecting to: "); Serial.println(SSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID, PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  IPAddress ip = WiFi.localIP();
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(ip);
}

//---- config functions----

void configModeCallback (WiFiManager *myWiFiManager) {
  Serial.println("Entered config mode");
  Serial.println(WiFi.softAPIP());
  Serial.println(myWiFiManager->getConfigPortalSSID());
  showAllLeds(0, 0, 255);
}


void saveConfigCallback () {
  s_thingName = wfm_thingName.getValue();
  writeAttribute("thingName", s_thingName);
  //s_thingName.toCharArray(thingName, MAX_PARAM);

  s_mqttServer = wfm_mqttServer.getValue();
  writeAttribute("mqttServer", s_mqttServer);
  //s_mqttServer.toCharArray(mqttServer, MAX_PARAM);
  showAllLeds(0, 255, 255);

}


String readAttribute(String attribute) {
  String value;
  SPIFFS.begin();
  File configFile = SPIFFS.open(attribute + ".txt", "r");
  if (configFile) {
    value = configFile.readString();
    Serial.println("readAttribute: " + attribute + "=" + value);
  }
  configFile.close();
  SPIFFS.end();
  return value;
}

void writeAttribute(String attribute, String value) {
  SPIFFS.begin();
  File configFile = SPIFFS.open(attribute + ".txt", "w");
  if (configFile) {
    configFile.print(value);
    Serial.println("writeAttribute: " + attribute + "=" + value);
  }
  configFile.close();
  SPIFFS.end();

}


// --------------- OTA Lan ---------------
void setupOTA() {

  ArduinoOTA.setPort(8266); // Port defaults to 8266
  if (THING_NAME != THING_NAME_DEFAULT) {
    ArduinoOTA.setHostname(THING_NAME);   // Hostname defaults to esp8266-[ChipID]
  } else {
    ArduinoOTA.setHostname(THING_ID);
  }
  ArduinoOTA.setPassword((const char *) OTA_PASSWORD);   // No authentication by default

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



// http update

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


void blink(int b) {
  for (int i = 0; i < b; i++) {
    digitalWrite(LED_BUILTIN, LED_ON);
    delay(100);
    digitalWrite(LED_BUILTIN, LED_OFF);
    delay(100);
  }
}

