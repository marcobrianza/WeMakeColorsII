//MQTT
#include <PubSubClient.h> // version 2.7.0 in PubSubClient.h change #define MQTT_MAX_PACKET_SIZE 512 
#include <ArduinoJson.h> // version 5.13.5
PubSubClient mqttClient(wifiClient);

#define MQTT_MAX MQTT_MAX_PACKET_SIZE

int MQTT_PORT = 1883;

#define QOS_AT_LEAST_1 1

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

void mqttReceive(char* topic, byte* payload, unsigned int length) {
  String Topic = String(topic);
  Serial.println("MQTT message arrived: " + Topic);

  int p1 = mqttRoot.length() + 1;
  int p2 = Topic.indexOf("/", p1);

  // String topic_root = String(topic).substring(0, p1-1);
  String topic_id = String(topic).substring(p1, p2);
  String topic_leaf = String(topic).substring(p2 + 1);

  //Serial.println(topic_root);
  //  Serial.println(topic_id);
  //  Serial.println(topic_leaf);

  DynamicJsonBuffer jsonBuffer(MQTT_MAX);

  if (topic_leaf == mqtt_randomColor) {
    if (topic_id != thingId) {

      /*
        {
        "h": 0,
        "s": 128,
        "v": 255
        }
      */
      JsonObject& root = jsonBuffer.parseObject(payload);
      int h = root["h"];
      int s = root["s"];
      int v = root["v"];

      Serial.print("hsv ");
      Serial.print(h); Serial.print(" ");
      Serial.print(s); Serial.print(" ");
      Serial.print(v); Serial.print(" ");

      Serial.println();

      setRemoteLED(CHSV(h, s, v));

    } else Serial.println("My message");
  }

  if ((topic_id = thingId) && (topic_leaf == mqtt_config)) {
    Serial.println("Config message:");
    JsonObject& root = jsonBuffer.parseObject(payload);


    String command = root["command"];
    String option = root["option"];

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
      //jsonMsg["FreeHeap"] = fh;
      //jsonMsg["MD5"] = ESP.getSketchMD5();


      //String  savedSSID = WiFi.SSID();
      //String  savedPassword = WiFi.psk();
    }

  }
}

void setupMqtt() {
  mqttClient.setServer(mqttServer.c_str(), MQTT_PORT);
  mqttClient.setCallback(mqttReceive);

  mqttPublish_randomColor = mqttRoot + "/" + thingId + "/" + mqtt_randomColor;
  mqttPublish_status = mqttRoot + "/" + thingId + "/" + mqtt_status;

  mqttSubscribe_randomColor = mqttRoot + "/+/" + mqtt_randomColor;
  mqttSubscribe_config = mqttRoot + "/" + thingId + "/" + mqtt_config;

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

  Serial.println( "MQTT Status=" + String(c) + " " + s);
  return c;

}

void subscribeMQTT() {
  mqttClient.subscribe(mqttSubscribe_randomColor.c_str(), QOS_AT_LEAST_1);
  Serial.print("Subscibed to: ");
  Serial.println(mqttSubscribe_randomColor);

  mqttClient.subscribe(mqttSubscribe_config.c_str(), QOS_AT_LEAST_1);
  Serial.print("Subscibed to: ");
  Serial.println(mqttSubscribe_config);
}

void prepareStatusMessage(int ut) {

  StaticJsonBuffer<MQTT_MAX> jsonBufferMQTTStatus;
  JsonObject& jsonMsgStatus = jsonBufferMQTTStatus.createObject();

  jsonMsgStatus["softwareName"] = softwareName;
  jsonMsgStatus["softwareVersion"] = softwareVersion;
  jsonMsgStatus["friendlyName"] = friendlyName;

  jsonMsgStatus["upTime"] = ut;
  jsonMsgStatus["lightLevel"] = average;

  jsonMsgStatus.printTo(mqttDataStatus);
}

void reconnectMQTT() {

  if (checkWiFiStatus() == WL_CONNECTED) {
    if (checkMQTTStatus () != MQTT_CONNECTED) {

      prepareStatusMessage(-1);
      Serial.print("\nAttempting MQTT connection...");
      //if (mqttClient.connect( thingId.c_str(), mqttUsername.c_str(), mqttPassword.c_str())) {
      if (mqttClient.connect( thingId.c_str(), mqttUsername.c_str(), mqttPassword.c_str(), mqttPublish_status.c_str(), QOS_AT_LEAST_1, true, mqttDataStatus)) {
        Serial.println("connected\n");

        subscribeMQTT();
        publishStatus = true;

      } else {
        blink(BLINK_NO_MQTT);
        Serial.print("MQTT connect failed, rc=" + mqttClient.state());
        Serial.println(" try again in 5 seconds");
        delay(5000); // Wait 5 seconds before retrying
      }
    }
  } // no else since there is auto reconnect
  
  //else {
   // delay(5000);
  //}
}





void publishRandomColor(CHSV c) {

  if (mqttClient.connected()) {
    StaticJsonBuffer<MQTT_MAX> jsonBufferMQTT;
    JsonObject& jsonMsg = jsonBufferMQTT.createObject();

    jsonMsg["h"] = c.h;
    jsonMsg["s"] = c.s;
    jsonMsg["v"] = c.v;

    jsonMsg["friendlyName"] = friendlyName;
    jsonMsg["lightLevel"] = average;

    char mqttData[MQTT_MAX];
    jsonMsg.printTo(mqttData);

    int ret = mqttClient.publish(mqttPublish_randomColor.c_str(), mqttData);

   Serial.println("MQTT message sent: " + mqttPublish_randomColor + " " + mqttData + " result: " + ret);

  } else Serial.println("MQTT not connected");
}




void publishStatusMQTT() {

  if (mqttClient.connected()) {

    prepareStatusMessage(upTime);

    int ret = mqttClient.publish(mqttPublish_status.c_str(), mqttDataStatus, true);
    
    Serial.println("MQTT message sent: " + mqttPublish_status + " " + mqttDataStatus + " result: " + ret);

  } else Serial.println("MQTT not connected");

}
