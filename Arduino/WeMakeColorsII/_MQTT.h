//MQTT

boolean DEBUG_MQTT=true;

String mqttServer = "wmc.marcobrianza.it";
String mqttUsername = "";
String mqttPassword = "";

#include <PubSubClient.h> // version 2.7.0 in PubSubClient.h (line 26) change #define MQTT_MAX_PACKET_SIZE 512 (from 128)
#include <ArduinoJson.h> // version 6.14.0
PubSubClient mqttClient(wifiClient);

#define MQTT_MAX MQTT_MAX_PACKET_SIZE

int MQTT_PORT = 1883;

#define QOS_AT_LEAST_1 1
#define QOS_BEST_EFFORT 0

#define BLINK_NO_MQTT 3

String mqttRoot =   "WeMakeColorsII";

String mqtt_randomColor = "randomColor";
String mqtt_status = "status";
String mqtt_config = "config";

unsigned long upTime = 0;
unsigned long lastMinute = 0;

//status
#define STATUS_INTERVAL 5 //minutes
boolean publishStatus = false;

int RECONNECT_INTERVAL = 5000;
int CHECK_INTERVAL = 20000;
int reconnectInterval = RECONNECT_INTERVAL;
unsigned long lastConnectTime = 0;


void mqttReceive(char* topic, byte* payload, unsigned int length) {
  String Topic = String(topic);
  if (DEBUG_MQTT)  Serial.println("MQTT received: " + String(length) + " " + Topic);

  int p1 = mqttRoot.length() + 1;
  int p2 = Topic.indexOf("/", p1);

  // String topic_root = String(topic).substring(0, p1-1);
  String topic_id = String(topic).substring(p1, p2);
  String topic_leaf = String(topic).substring(p2 + 1);

  //Serial.println(topic_root);
  //Serial.println(topic_id);
  //Serial.println(topic_leaf);

  StaticJsonDocument<MQTT_MAX> doc;

  if (topic_leaf == mqtt_randomColor) {
    if ((topic_id == thingId) && (ECHO_MODE) || (topic_id != thingId) ) {

      /*
        {
        "h": 0,
        "s": 128,
        "v": 255
        }
      */

      DeserializationError error = deserializeJson(doc, payload);
      if (error) if (DEBUG_MQTT)  Serial.println("deserializeJson() failed with code: " + String(error.c_str()));

      int h = doc["h"];
      int s = doc["s"];
      int v = doc["v"];

      if (topic_id == thingId) if (DEBUG_MQTT)  Serial.print("my message ");
      if (DEBUG_MQTT)  Serial.println("hsv: " + String(h) + " " + String(s) + " " + String(v) );

      setRemoteLED(CHSV(h, s, v));
    }
  }

  if ((topic_id == thingId) && (topic_leaf == mqtt_config)) {
    if (DEBUG_MQTT) Serial.println("Config message:");
    DeserializationError error = deserializeJson(doc, payload);
    if (error) if (DEBUG_MQTT)  Serial.println("deserializeJson() failed with code: " + String(error.c_str()));

    String command = doc["command"];
    String option = doc["option"];

    if (DEBUG_MQTT) Serial.println(command + " " + option);


    /*
         {
         "command":"update",
         "option":"http://iot.marcobrianza.it/WeMakeColorsII/WeMakeColorsII.ino.d1_mini.bin"
         }
    */
    if (command == "update") {
      showAllLeds(64, 0, 0);
      int u = httpUpdate(option);
      if (u != HTTP_UPDATE_OK) {
        showAllLeds(64, 64, 0);
      }
    }

    /*
         {
         "command":"info"
         }
    */

    if (command == "info") {
      // send detailed information on the device


      //int fh = ESP.getFreeHeap();
      //Serial.print("FreeHeap: ");
      //Serial.println(fh);
      //doc["FreeHeap"] = fh;
      //doc["MD5"] = ESP.getSketchMD5();


      //String  savedSSID = WiFi.SSID();
      //String  savedPassword = WiFi.psk();
    }

  }
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

void subscribeMQTT() {

  String mqttTopic;

  mqttTopic = mqttRoot + "/+/" + mqtt_randomColor;
  mqttClient.subscribe(mqttTopic.c_str(), QOS_AT_LEAST_1);
  if (DEBUG_MQTT) Serial.println("Subscibed to: " + mqttTopic);


  mqttTopic = mqttRoot + "/" + thingId + "/" + mqtt_config;
  mqttClient.subscribe(mqttTopic.c_str(), QOS_AT_LEAST_1);
  if (DEBUG_MQTT) Serial.println("Subscibed to: " + mqttTopic);


  //mqttClient.subscribe(mqttPublish_status.c_str(), QOS_AT_LEAST_1); //***

}


String prepareLastWillMessage() {
  StaticJsonDocument<MQTT_MAX> doc;

  doc["friendlyName"] = friendlyName;
  doc["softwareInfo"] = softwareInfo;
  doc["softwarePlatform"] = softwarePlatform;
  doc["upTime"] = -1;

  String lastWillMessage;

  serializeJson(doc, lastWillMessage);
  return (lastWillMessage);
}

void connectMQTT() {

  if (checkWiFiStatus() == WL_CONNECTED) {
    if (checkMQTTStatus () != MQTT_CONNECTED) {
      //netStatus = 10;

      if (DEBUG_MQTT)  Serial.print("\nAttempting MQTT connection..." + String(millis()) + " ");

      mqttClient.setServer(mqttServer.c_str(), MQTT_PORT);
      mqttClient.setCallback(mqttReceive);

      //if (mqttClient.connect( thingId.c_str(), mqttUsername.c_str(), mqttPassword.c_str())) {
      String  mqttTopic = mqttRoot + "/" + thingId + "/" + mqtt_status;
      if (mqttClient.connect( thingId.c_str(), mqttUsername.c_str(), mqttPassword.c_str(), mqttTopic.c_str(), QOS_AT_LEAST_1, true, prepareLastWillMessage().c_str())) {
        if (DEBUG_MQTT) {Serial.println("connected\n"); Serial.println("FreeHeap: " + String(ESP.getFreeHeap()));}


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

void publishJSON(String topicLeaf, StaticJsonDocument<MQTT_MAX> jdoc) {
  if (mqttClient.connected()) {

    char mqttData[MQTT_MAX];
    serializeJson(jdoc, mqttData);

    String   mqttTopic = mqttRoot + "/" + thingId + "/" + topicLeaf;
    int ret = mqttClient.publish(mqttTopic.c_str(), mqttData);
    if (DEBUG_MQTT) Serial.println(String(ret) + " " +  String(String(mqttData).length()) + " MQTT sent: " + mqttTopic + " " + mqttData );

  } else if (DEBUG_MQTT)  Serial.println("publish " + topicLeaf + ": MQTT not connected");
}


void publishRandomColor(CHSV c) {
  StaticJsonDocument<MQTT_MAX> doc;
  doc["h"] = c.h;
  doc["s"] = c.s;
  doc["v"] = c.v;

  doc["friendlyName"] = friendlyName;
  doc["lightLevel"] = averageLightLevel;

  publishJSON(mqtt_randomColor, doc);

}



void publishStatusMQTT() {
  StaticJsonDocument<MQTT_MAX> doc;

  doc["friendlyName"] = friendlyName;
  doc["softwareInfo"] = softwareInfo;
  doc["softwarePlatform"] = softwarePlatform;
  doc["upTime"] = upTime;

  doc["lightLevel"] = averageLightLevel;

  if (upTime == 0) {
    doc["resetReason"] = ESP.getResetReason();
  }

  publishJSON(mqtt_status, doc);

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

    if (upTime % STATUS_INTERVAL == 0 )publishStatus = true;
  }

  if (publishStatus) {
    publishStatus = false;
    publishStatusMQTT();
  }

}
