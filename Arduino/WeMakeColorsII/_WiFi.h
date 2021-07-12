boolean DEBUG_WIFI = true;

#define LAN_OTA true

//Wi-Fi
#include <ESP8266WiFi.h>  // built in ESP8266 Core
WiFiClient wifiClient;

//default ssid password
String defaultSSID = "colors";
String defaultPassword = "colors01";

//optional parameterf for static ip
String network = "192.168.3.";
String gateway = "254";
String dns = "8.8.8.8";

//saved ssid password from previous connection
String  savedSSID;
String  savedPassword;

#define BLINK_NO_SSID 1
#define BLINK_DISCONNECTED 1
#define BLINK_CONNECTION_ERROR 2
#define BLINK_IDLE_STATUS 1

//int netStatus = 0; //0=undefined 1=WiFi connected 2=MQTT connected -1 WiFi not Connected


#if  (LAN_OTA)
//OTA
#include <ESP8266mDNS.h> // built in ESP8266 Core
#include <ArduinoOTA.h> // built in ESP8266 Core
String OTA_PASSWORD = "12345678";
#endif

//http update
#include <ESP8266HTTPClient.h> // built in ESP8266 Core
#include <ESP8266httpUpdate.h> // built in ESP8266 Core

WiFiClient client;

String MD5_URL = "http://iot.marcobrianza.it/art/WeMakeColorsII.md5.txt";
String FW_URL = "http://iot.marcobrianza.it/art/WeMakeColorsII.ino.d1_mini.bin";


void disableWiFi() {
  WiFi.mode(WIFI_OFF);
}




void WiFi_setup(String ip = "") {

  savedSSID = WiFi.SSID();
  savedPassword = WiFi.psk();

  if (DEBUG_WIFI) Serial.println("Saved credentials: " + savedSSID + " " + savedPassword);

  WiFi.setOutputPower(20.5); // 20.5 is maximum power
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);

  //Serial.println(wifi_get_sleep_type());
  wifi_set_sleep_type(NONE_SLEEP_T);

  //Serial.println("PhyMode:" + String( WiFi.getPhyMode()));
  WiFi.setPhyMode(WIFI_PHY_MODE_11N);   // WIFI_PHY_MODE_11B = 1, WIFI_PHY_MODE_11G = 2, WIFI_PHY_MODE_11N = 3
  //Serial.println("PhyMode:" + String( WiFi.getPhyMode()));

  WiFi.hostname(name);

  if (ip != "") {
    IPAddress IP, GW, DNS ;

    IP.fromString(network + ip);
    GW.fromString(network + gateway);
    DNS.fromString(dns);

    WiFi.config(IP, DNS, GW );

  }
}

IPAddress AP_local_IP(192, 168, 42, 254);
IPAddress AP_gateway(192, 168, 42, 254);
IPAddress AP_subnet(255, 255, 255, 0);

void startAP(String APssid , String APpassword) {
  WiFi.mode(WIFI_AP);

  Serial.println();
  Serial.print("Configuring access point: " + APssid + " ");
  /* You can remove the password parameter if you want the AP to be open. */
  WiFi.softAPConfig(AP_local_IP, AP_gateway, AP_subnet);
  if (WiFi.softAP(APssid.c_str(), APpassword.c_str(), 1, false, 8)) { //ssid,pass,channel,hidden, clients
    Serial.println("OK");
  } else {
    Serial.println("ERROR");
  }
  delay(200);
  Serial.println("AP IP address: " + WiFi.softAPIP().toString());

}

void connectWiFi_Smart() {
  if (DEBUG_WIFI) Serial.println("Starting ESPTouch SmartConfig");
  WiFi.beginSmartConfig();
}

void connectWiFi(String ssid, String password) {

  if (ssid == "") {
    if (DEBUG_WIFI)Serial.println("Connecting to default AP");
    ssid = defaultSSID;
    password = defaultPassword;
  }

  WiFi.disconnect();
  delay(100);
  // We start by connecting to a WiFi network
  if (DEBUG_WIFI)Serial.print("Connecting to: "); Serial.println(ssid);

  WiFi.begin(ssid.c_str(), password.c_str());

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    if (DEBUG_WIFI) Serial.print(".");
  }

  IPAddress ip = WiFi.localIP();
  if (DEBUG_WIFI) {
    Serial.println("WiFi connected");
    Serial.print("IP address: ");
    Serial.println(ip);


    Serial.println("-------WiFi connection status:-----");
    WiFi.printDiag(Serial);
    Serial.println("-----------------------------------");
  }
}


int checkWiFiStatus() {

  int c = WiFi.status();
  String s = "";
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
      b = BLINK_IDLE_STATUS;
      break;
    case WL_DISCONNECTED:
      s = "WL_DISCONNECTED";
      b = BLINK_DISCONNECTED;
      break;
    case WL_CONNECTION_LOST:
      s = "WL_CONNECTION_LOST";
      break;
    default:
      s = "UNKNOWN";
      break;
  }

  if (c != WL_CONNECTED) {
    if (DEBUG_WIFI)Serial.println( "WiFi Status=" + String(c) + " " + s);
    if (b > 0) blink(b);
  }

  return c;
}


void WiFi_loop() {

#if  (LAN_OTA)
  ArduinoOTA.handle();
#endif
}



// --------------- OTA Lan ---------------


void OTA_setup() {
#if  (LAN_OTA)

  ArduinoOTA.setPort(8266); // Port defaults to 8266
  ArduinoOTA.setHostname(name.c_str());

  ArduinoOTA.setPassword(OTA_PASSWORD.c_str());

  ArduinoOTA.onStart([]() {
    Serial.println("\nStart OTA");
    //send MQTT message or at least close connection
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
  MDNS.addServiceTxt("arduino", "tcp", "name", name);
  MDNS.addServiceTxt("arduino", "tcp", "softwareInfo", softwareInfo);
  MDNS.addServiceTxt("arduino", "tcp", "softwarePlatform", softwarePlatform);
  MDNS.update();

  Serial.println("OTA Ready");
#endif
}



// -----  http update ----------------

String httpUpdate(String url) {
  t_httpUpdate_return ret = ESPhttpUpdate.update(client, url);
  String result;

  switch (ret) {
    case HTTP_UPDATE_FAILED:
      result = "HTTP_UPDATE_FAILD Error " + String(ESPhttpUpdate.getLastError()) + " " + ESPhttpUpdate.getLastErrorString().c_str();
      break;
    case HTTP_UPDATE_NO_UPDATES:
      result = "HTTP_UPDATE_NO_UPDATES";
      break;
    case HTTP_UPDATE_OK:
      result = "HTTP_UPDATE_OK";
      break;
  }

  return result;
  if (DEBUG_WIFI) Serial.println("httpUpdate:" + result);
}


void autoUpdate() {
  if (DEBUG_WIFI) Serial.println("Autoupdate...");
  HTTPClient http;
  http.begin(client, MD5_URL);
  int httpCode = http.GET();

  if (httpCode > 0) {
    // HTTP header has been send and Server response header has been handled
    if (DEBUG_WIFI)Serial.println(MD5_URL + " " + String(httpCode));

    // file found at server
    if (httpCode == HTTP_CODE_OK) {
      String md5 = http.getString();
      if (DEBUG_WIFI)Serial.println("Server md5: " + md5 + " " + md5.length());
      if (DEBUG_WIFI)Serial.println("Board md5:  " + ESP.getSketchMD5());
      if ((ESP.getSketchMD5() != md5) && (md5.length() == 32) ) {
        if (DEBUG_WIFI) Serial.println("Trying update...");
        showState(AUTO_UPDATE);

        String u = httpUpdate(FW_URL);
        if (u == "HTTP_UPDATE_OK") showState(OK) ;
        else showState(UPDATE_ERROR) ;
      }
      else {
        if (DEBUG_WIFI) Serial.println("will not update");
      }

    }
    else {
      if ( DEBUG_WIFI) Serial.println( "[HTTP] GET... failed, error:" + String(http.errorToString(httpCode).c_str()));
    }
    http.end();
  }
  if (DEBUG_WIFI) Serial.println();
}
