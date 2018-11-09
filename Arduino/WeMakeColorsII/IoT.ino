#include <EEPROM.h>

#define COUNT_ADDR 0


void setupParameters() {
  s_thingName = readAttribute("thingName");
  if (s_thingName == "") {
    s_thingName = thingId;
  }
  s_thingName.toCharArray(THING_NAME, MAX_PARAM);
  Serial.println ("thingName=" + String(THING_NAME));


  s_mqttServer = readAttribute("mqttServer");
  if (s_mqttServer != "") {
    s_mqttServer.toCharArray(MQTT_SERVER, MAX_PARAM);
  }
  Serial.println ("mqttServer=" + String(MQTT_SERVER));


  s_mqttUsername = readAttribute("mqttUsername");
  if (s_mqttUsername != "") {
    s_mqttUsername.toCharArray(MQTT_USERNAME, MAX_PARAM);
  } else {
    thingId.toCharArray(MQTT_USERNAME, MAX_PARAM);// if username is null it defaults to thingName
  }
  Serial.println ("mqttUsername=" + String(MQTT_USERNAME));


  s_mqttPassword = readAttribute("mqttPassword");
  if (s_mqttPassword != "") {
    s_mqttPassword.toCharArray(MQTT_PASSWORD, MAX_PARAM);
  }
  Serial.println ("mqttPassword=" + String(MQTT_PASSWORD));

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

// ------ Wi-Fi Manager functions-------------------------------------



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
  WiFi.setAutoReconnect(true);
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

  s_mqttServer = wfm_mqttServer.getValue();
  writeAttribute("mqttServer", s_mqttServer);

  s_mqttUsername = wfm_mqttUsername.getValue();
  writeAttribute("mqttUsername", s_mqttUsername);

  s_mqttPassword = wfm_mqttPassword.getValue();
  writeAttribute("mqttPassword", s_mqttPassword);

  showAllLeds(0, 255, 255);

}


String readAttribute(String attribute) {
  String value = "";
  SPIFFS.begin();
  File configFile = SPIFFS.open(attribute + ".txt", "r");
  if (configFile) {
    value = configFile.readString();
    Serial.println("readAttribute: " + attribute + "=" + value);
  } else {
    Serial.println("readAttribute: " + attribute + "= null" );
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


void setupMdns() {
  //MDNS discovery
  MDNS.addServiceTxt("arduino", "tcp", "thingId", thingId);
  MDNS.addServiceTxt("arduino", "tcp", "thingName", s_thingName);
  MDNS.addServiceTxt("arduino", "tcp", "software", software);
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
