boolean DEBUG_MQTT = true;

#include <PubSubClient.h> // PubSubClient - by Nick O'Leary - 2.8.0 http://librarymanager/all#PubSubClient
#include <ArduinoJson.h> //  ArduinoJson  - by Benoit Blanchon - 7.4.2 http://librarymanager/all#ArduinoJson
PubSubClient mqttClient(wifiClient);

#define QOS_BEST_EFFORT 0
#define QOS_AT_LEAST_1 1
#define QOS_EXACTLY_ONE 2;

#define BLINK_NO_MQTT 3

#define MQTT_BUFFER 512
#define MQTT_KEEP_ALIVE 15  // default 15
#define SOCKET_KEEP_ALIVE 15 // default 15


unsigned long upTime = 0;
unsigned long lastMinute = 0;

//status
int STATUS_INTERVAL = 15; //minutes
boolean publishStatusFlag = false;
boolean publishInfoFlag = false;
boolean softwareUpdateFlag = false;

//
int RECONNECT_INTERVAL = 5000; //ms
int CHECK_INTERVAL = 20000; //ms
int reconnectInterval = RECONNECT_INTERVAL;
unsigned long lastConnectTime = 0;

void disconnectMQTT() {
  mqttClient.disconnect();
}

void publishJSON(String mqttTopic, StaticJsonDocument<MQTT_BUFFER> jPayload, bool retained) {
  if (mqttClient.connected()) {

    jPayload["date"] = getDateTimeStr();

    char mqttPayload[MQTT_BUFFER];
    serializeJson(jPayload, mqttPayload);

    int ret = mqttClient.publish(mqttTopic.c_str(), mqttPayload, retained);
    if (DEBUG_MQTT) Serial.println(String(ret) + "   MQTT sent: " +  String(String(mqttPayload).length()) + " " + mqttTopic + " " + mqttPayload );

  } else if (DEBUG_MQTT)  Serial.println("publish " + mqttTopic + ": MQTT not connected");
}




//----------------------

#include "app_mqtt.h"


//-------------------------

String prepareLastWillMessage() {
  StaticJsonDocument<MQTT_BUFFER> jPayload;

  jPayload["name"] = name;
  jPayload["upTime"] = -1;
  jPayload["RSSI"] = WiFi.RSSI();

  jPayload["softwareInfo"] = softwareInfo;
  jPayload["softwarePlatform"] = softwarePlatform;

  String lastWillMessage;

  serializeJson(jPayload, lastWillMessage);
  return (lastWillMessage);
}


void publishStatus(String extraAttribute = "", String extraValue = "") {
  StaticJsonDocument<MQTT_BUFFER> jPayload;

  jPayload["name"] = name;
  jPayload["upTime"] = upTime;
  jPayload["RSSI"] = WiFi.RSSI();
  jPayload["BSSID"] = WiFi.BSSIDstr();
  jPayload["IP"] = WiFi.localIP().toString();

  //  jPayload["softwareInfo"] = softwareInfo;
  //  jPayload["softwarePlatform"] = softwarePlatform;

  static bool firstStatus = true;
  if (firstStatus) {
    firstStatus = false;
    jPayload["resetReason"] = ESP.getResetReason();
  }


  if (extraAttribute != "") jPayload[extraAttribute] = extraValue;

  String   mqttTopic = mqttRoot + "/" + thingId + "/" + mqttTopicStatus;
  publishJSON(mqttTopic, jPayload, true);

}


int checkMQTTStatus () {
  int c = mqttClient.state();
  String s = "";

  switch (c) {
    case MQTT_CONNECTION_TIMEOUT:
      s = "MQTT_CONNECTION_TIMEOUT";
      break;
    case MQTT_CONNECTION_LOST:
      s = "MQTT_CONNECTION_LOST";
      break;
    case MQTT_CONNECT_FAILED:
      s = "MQTT_CONNECT_FAILED";
      break;
    case MQTT_DISCONNECTED:
      s = "MQTT_DISCONNECTED";
      break;
    case MQTT_CONNECTED:
      s = "MQTT_CONNECTED";
      break;
    case MQTT_CONNECT_BAD_PROTOCOL:
      s = "MQTT_CONNECT_BAD_PROTOCOL";
      break;
    case MQTT_CONNECT_BAD_CLIENT_ID:
      s = "MQTT_CONNECT_BAD_CLIENT_ID";
      break;
    case MQTT_CONNECT_UNAVAILABLE:
      s = "MQTT_CONNECT_UNAVAILABLE";
      break;
    case MQTT_CONNECT_BAD_CREDENTIALS:
      s = "MQTT_CONNECT_BAD_CREDENTIALS";
      break;
    case MQTT_CONNECT_UNAUTHORIZED:
      s = "MQTT_CONNECT_UNAUTHORIZED";
      break;
    default:
      s = "UNKNOWN";
  }

  if (c != MQTT_CONNECTED) if (DEBUG_MQTT)  Serial.println( "MQTT Status=" + String(c) + " " + s);
  return c;

}


void connectMQTT() {

  if (checkWiFiStatus() == WL_CONNECTED) {
    if (checkMQTTStatus () != MQTT_CONNECTED) {
      //netStatus = 10;


      if (DEBUG_MQTT)  Serial.print(String(millis()) + " MQTT connecting: " + mqttUsername + ":" + mqttPassword + "@" + mqttServer + ":" + MQTT_PORT + " ... " );

      mqttClient.setServer(mqttServer.c_str(), MQTT_PORT);
      mqttClient.setCallback(mqttReceive);
      mqttClient.setBufferSize(MQTT_BUFFER);
      mqttClient.setKeepAlive(MQTT_KEEP_ALIVE);
      mqttClient.setSocketTimeout(SOCKET_KEEP_ALIVE);

      String  mqttTopic = mqttRoot + "/" + thingId + "/" + mqttTopicStatus;
      if (mqttClient.connect( thingId.c_str(), mqttUsername.c_str(), mqttPassword.c_str(), mqttTopic.c_str(), QOS_AT_LEAST_1, true, prepareLastWillMessage().c_str())) {
        if (DEBUG_MQTT) {
          Serial.println("connected");
          //Serial.println("FreeHeap: " + String(ESP.getFreeHeap()));
        }

        subscribeMQTT();
        publishStatusFlag = true;
        publishInfoFlag = true;
        // netStatus = 2;
        reconnectInterval = CHECK_INTERVAL;

      } else {
        blink(BLINK_NO_MQTT);
        reconnectInterval = RECONNECT_INTERVAL;

        if (DEBUG_MQTT) Serial.print("MQTT connect failed, rc=" + mqttClient.state()); Serial.println(" will try again..." + String (millis()));

      }
    }
    //netStatus = 1;
  }
  else {
    reconnectInterval = RECONNECT_INTERVAL;
  }

}


void mqtt_setup() {
  mqttClient.setBufferSize(MQTT_BUFFER);
}

void mqtt_loop() {
  mqttClient.loop();

  if ((millis() - lastConnectTime) > reconnectInterval ) {
    lastConnectTime = millis();
    connectMQTT() ;
  }

  unsigned long m = millis() / 60000;
  if (m != lastMinute) {
    lastMinute = m;
    upTime++;

    if (upTime % STATUS_INTERVAL == 0 && STATUS_INTERVAL > 0 )publishStatusFlag = true;
  }

  if (publishStatusFlag) {
    publishStatusFlag = false;
    publishStatus();
  }

  if (publishInfoFlag) {
    publishInfoFlag = false;
    publishInfo();
  }

  if ((millis() - lastSensorSend > SENSOR_INTERVAL * 1000) && (SENSOR_INTERVAL > 0) ) {
    lastSensorSend = millis();
    publishSensor();
  }

  if (softwareUpdateFlag) {
    softwareUpdateFlag = false;
    Serial.println("...softwareUpdate...");

    //publishInfo();
    publishStatus("softwareUpdate", updateUrl );
    delay(100);
    disconnectMQTT();

    showState(UPDATE);
    String u = httpUpdate(updateUrl);

    if (u == "HTTP_UPDATE_OK")  {
      showState(UPDATE_OK);
    } else {
      showState(UPDATE_ERROR);
      publishStatus("softwareUpdate", "http:error:" + u);
    }
  }


}
