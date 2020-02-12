boolean DEBUG_MQTT = true;


#include <PubSubClient.h> // version 2.7.0 in PubSubClient.h (line 26) change #define MQTT_MAX_PACKET_SIZE 512 (from 128)
#include <ArduinoJson.h> // version 6.14.1
PubSubClient mqttClient(wifiClient);


#define QOS_AT_LEAST_1 1
#define QOS_BEST_EFFORT 0

#define BLINK_NO_MQTT 3

unsigned long upTime = 0;
unsigned long lastMinute = 0;

//status
#define STATUS_INTERVAL 15 //minutes
boolean publishStatus = false;




//
int RECONNECT_INTERVAL = 5000; //ms
int CHECK_INTERVAL = 20000; //ms
int reconnectInterval = RECONNECT_INTERVAL;
unsigned long lastConnectTime = 0;

void publishJSON(String mqttTopic, StaticJsonDocument<MQTT_MAX_PACKET_SIZE> jdoc, bool retained) {
  if (mqttClient.connected()) {

    char mqttData[MQTT_MAX_PACKET_SIZE];
    serializeJson(jdoc, mqttData);


    int ret = mqttClient.publish(mqttTopic.c_str(), mqttData, retained);
    if (DEBUG_MQTT) Serial.println(String(ret) + " " +  String(String(mqttData).length()) + " MQTT sent: " + mqttTopic + " " + mqttData );

  } else if (DEBUG_MQTT)  Serial.println("publish " + mqttTopic + ": MQTT not connected");
}

String prepareLastWillMessage() {
  StaticJsonDocument<MQTT_MAX_PACKET_SIZE> doc;

  doc["friendlyName"] = friendlyName;
  doc["upTime"] = -1;

  doc["softwareInfo"] = softwareInfo;
  doc["softwarePlatform"] = softwarePlatform;

  String lastWillMessage;

  serializeJson(doc, lastWillMessage);
  return (lastWillMessage);
}




//----------------------

#include "_app_mqtt.h"


//-------------------------

void publishStatusMQTT() {
  StaticJsonDocument<MQTT_MAX_PACKET_SIZE> doc;

  doc["friendlyName"] = friendlyName;
  doc["upTime"] = upTime;
  if (upTime == 0) {
    doc["resetReason"] = ESP.getResetReason();
  }
  doc["softwareInfo"] = softwareInfo;
  doc["softwarePlatform"] = softwarePlatform;


  String   mqttTopic = mqttRoot + "/" + thingId + "/" + mqttTopicStatus;
  publishJSON(mqttTopic, doc, true);

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

      String  mqttTopic = mqttRoot + "/" + thingId + "/" + mqttTopicStatus;
      if (mqttClient.connect( thingId.c_str(), mqttUsername.c_str(), mqttPassword.c_str(), mqttTopic.c_str(), QOS_AT_LEAST_1, true, prepareLastWillMessage().c_str())) {
        if (DEBUG_MQTT) {
          Serial.println("connected");
          Serial.println("FreeHeap: " + String(ESP.getFreeHeap()));
        }

        subscribeMQTT();
        publishStatus = true;
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

    if (upTime % STATUS_INTERVAL == 0 && STATUS_INTERVAL > 0 )publishStatus = true;
  }

  if (publishStatus) {
    publishStatus = false;
    publishStatusMQTT();
  }

  if ((millis() - lastSensorSend > SENSOR_INTERVAL * 1000) && (SENSOR_INTERVAL > 0) ) {
    lastSensorSend = millis();
    publishSensor();
  }




}
