
String mqttServer = "wmc.marcobrianza.it";
int MQTT_PORT = 1883;
String mqttUsername = "";
String mqttPassword = "";

String mqttRoot =   "WeMakeColorsII";

String mqtt_randomColor = "randomColor";
String mqtt_status = "status";
String mqtt_config = "config";


//-----MQTT subscribe --------------

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


void mqttReceive(char* topic, byte* payload, unsigned int length) {
  String Topic = String(topic);
  payload[length] = 0;
  String Payload = String((char*)payload);

  if (DEBUG_MQTT)  Serial.println("MQTT received: " + String(length) + " " + Topic + " " + Payload);

  int p1 = mqttRoot.length() + 1;
  int p2 = Topic.indexOf("/", p1);

  // String topic_root = String(topic).substring(0, p1-1);
  String topic_id = String(topic).substring(p1, p2);
  String topic_leaf = String(topic).substring(p2 + 1);

  //Serial.println(topic_root);
  //Serial.println(topic_id);
  //Serial.println(topic_leaf);

  StaticJsonDocument<MQTT_MAX_PACKET_SIZE> doc;

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
      showState(UPDATE);

      int u = httpUpdate(option);
      if (u != HTTP_UPDATE_OK) {
        showState(UPDATE_OK);
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

//-----MQTT publish --------------

void publishStatusMQTT() {
  StaticJsonDocument<MQTT_MAX_PACKET_SIZE> doc;

  doc["friendlyName"] = friendlyName;
  doc["softwareInfo"] = softwareInfo;
  doc["softwarePlatform"] = softwarePlatform;
  doc["upTime"] = upTime;

  doc["lightLevel"] = int(averageLightLevel);

  if (upTime == 0) {
    doc["resetReason"] = ESP.getResetReason();
  }

  String   mqttTopic = mqttRoot + "/" + thingId + "/" + mqtt_status;
  publishJSON(mqttTopic, doc);

}


void publishRandomColor(CHSV c) {
  StaticJsonDocument<MQTT_MAX_PACKET_SIZE> doc;
  doc["h"] = c.h;
  doc["s"] = c.s;
  doc["v"] = c.v;

  doc["friendlyName"] = friendlyName;
  doc["lightLevel"] = int(averageLightLevel);

  String   mqttTopic = mqttRoot + "/" + thingId + "/" + mqtt_randomColor;

  publishJSON(mqttTopic, doc);

}
