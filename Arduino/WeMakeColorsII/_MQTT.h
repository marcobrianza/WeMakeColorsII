//MQTT

String mqttServer = "wmc.marcobrianza.it";
String mqttUsername = "";
String mqttPassword = "";

#include <PubSubClient.h> // version 2.7.0 in PubSubClient.h change #define MQTT_MAX_PACKET_SIZE 512 
#include <ArduinoJson.h> // version 6.13.0
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

String mqttPublish_randomColor = "";
String mqttPublish_status = "";

String mqttSubscribe_randomColor = "";
String mqttSubscribe_config = "";

char mqttDataStatus[MQTT_MAX];

unsigned int upTime = 0;
//status
#define STATUS_INTERVAL 5 //minutes
boolean publishStatus = false;

#include <Ticker.h>
Ticker T_connectMQTT;
Ticker T_upTime;

boolean B_connectMQTT = true;

void F_connectMQTT() {
  T_connectMQTT.detach();
  B_connectMQTT = true;
}

void upTimeInc() {
  upTime++;
  //publishStatus = true;
  if (upTime % STATUS_INTERVAL == 0 )publishStatus = true;
}


void mqttReceive(char* topic, byte* payload, unsigned int length) {
  String Topic = String(topic);
  Serial.println("MQTT received: " + String(length) + " " + Topic);

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
    if ((topic_id == thingId) && (echoMode) || (topic_id != thingId) ) {

      /*
        {
        "h": 0,
        "s": 128,
        "v": 255
        }
      */

      DeserializationError error = deserializeJson(doc, payload);
      if (error)  Serial.println("deserializeJson() failed with code: " + String(error.c_str()));

      int h = doc["h"];
      int s = doc["s"];
      int v = doc["v"];

      if (topic_id == thingId) Serial.print("my message ");
      Serial.println("hsv: " + String(h) + " " + String(s) + " " + String(v) );

      setRemoteLED(CHSV(h, s, v));
    }
  }

  if ((topic_id = thingId) && (topic_leaf == mqtt_config)) {
    Serial.println("Config message:");
    DeserializationError error = deserializeJson(doc, payload);
    if (error)  Serial.println("deserializeJson() failed with code: " + String(error.c_str()));

    String command = doc["command"];
    String option = doc["option"];

    Serial.println(command);
    Serial.println(option);

    /*
         {
         "command":"update",
         "option":"http://iot.marcobrianza.it/WeMakeColorsII/WeMakeColorsII.ino.d1_mini.bin"
         }
    */
    if (command = "update") {
      showAllLeds(64, 0, 0);
      int u = httpUpdate(option);
      if (u != HTTP_UPDATE_OK) showAllLeds(64, 64, 0);
    }

    /*
         {
         "command":"info"
         }
    */

    if (command = "info") {
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

  if (c != MQTT_CONNECTED) Serial.println( "MQTT Status=" + String(c) + " " + s);
  return c;

}

void subscribeMQTT() {
  mqttClient.subscribe(mqttSubscribe_randomColor.c_str(), QOS_AT_LEAST_1);
  Serial.print("Subscibed to: ");
  Serial.println(mqttSubscribe_randomColor);

  mqttClient.subscribe(mqttSubscribe_config.c_str(), QOS_AT_LEAST_1);
  Serial.print("Subscibed to: ");
  Serial.println(mqttSubscribe_config);

  mqttClient.subscribe(mqttPublish_status.c_str(), QOS_AT_LEAST_1); //***
  
}

void prepareStatusMessage(int ut) {

  StaticJsonDocument<MQTT_MAX> doc;

  doc["softwareName"] = softwareName;
  doc["softwareVersion"] = softwareVersion;
  doc["friendlyName"] = friendlyName;

  doc["upTime"] = ut;
  doc["lightLevel"] = average;

  //doc["freeHeap"] = ESP.getFreeHeap();

  serializeJson(doc, mqttDataStatus);

}

void connectMQTT() {

  if (checkWiFiStatus() == WL_CONNECTED) {
    if (checkMQTTStatus () != MQTT_CONNECTED) {
      //netStatus = 10;
      prepareStatusMessage(-1);
      Serial.print("\nAttempting MQTT connection..." + String(millis())+" ");

      //if (mqttClient.connect( thingId.c_str(), mqttUsername.c_str(), mqttPassword.c_str())) {
      if (mqttClient.connect( thingId.c_str(), mqttUsername.c_str(), mqttPassword.c_str(), mqttPublish_status.c_str(), QOS_AT_LEAST_1, true, mqttDataStatus)) {
        Serial.println("connected\n");
        Serial.println("FreeHeap: " + String(ESP.getFreeHeap()));

        subscribeMQTT();
        publishStatus = true;
        // netStatus = 2;

      } else {
        blink(BLINK_NO_MQTT);
        Serial.print("MQTT connect failed, rc=" + mqttClient.state());
        Serial.println(" will try again..." + String (millis()));
      }
    }
    //netStatus = 1;
  }
  //else {
  // netStatus = -1;
  //}

}



void publishRandomColor(CHSV c) {
  if (mqttClient.connected()) {
    StaticJsonDocument<MQTT_MAX> doc;
    doc["h"] = c.h;
    doc["s"] = c.s;
    doc["v"] = c.v;

    doc["friendlyName"] = friendlyName;
    doc["lightLevel"] = average;

    char mqttData[MQTT_MAX];
    serializeJson(doc, mqttData);

    int ret = mqttClient.publish(mqttPublish_randomColor.c_str(), mqttData);
    Serial.println(String(ret) + " " + String( String(mqttData).length() ) + " MQTT sent: " + mqttPublish_randomColor + " " + mqttData );

  } else Serial.println("publishRandomColor: MQTT not connected");
}




void publishStatusMQTT() {
  if (mqttClient.connected()) {
    prepareStatusMessage(upTime);
    int ret = mqttClient.publish(mqttPublish_status.c_str(), mqttDataStatus, true);
    Serial.println(String(ret) + + " " + String( String(mqttDataStatus).length() ) + " MQTT sent: " + mqttPublish_status + " " + mqttDataStatus );
  } else Serial.println("publishStatusMQTT: MQTT not connected");

}

void mqtt_setup() {
  mqttClient.setServer(mqttServer.c_str(), MQTT_PORT);
  mqttClient.setCallback(mqttReceive);

  mqttPublish_randomColor = mqttRoot + "/" + thingId + "/" + mqtt_randomColor;
  mqttPublish_status = mqttRoot + "/" + thingId + "/" + mqtt_status;

  mqttSubscribe_randomColor = mqttRoot + "/+/" + mqtt_randomColor;
  mqttSubscribe_config = mqttRoot + "/" + thingId + "/" + mqtt_config;

  thingId = appId + "_" +  WiFi.macAddress().c_str();
  T_upTime.attach(60, upTimeInc); //fires every minute

}

void mqtt_loop() {
  mqttClient.loop();

  if (publishStatus) {
    publishStatus = false;
    publishStatusMQTT();
  }

  if (B_connectMQTT) {
    B_connectMQTT = false;
    connectMQTT() ;
    T_connectMQTT.attach(5, F_connectMQTT);
  }
}
