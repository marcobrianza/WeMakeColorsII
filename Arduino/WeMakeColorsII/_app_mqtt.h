
String mqttServer = "wmc.marcobrianza.it";
int MQTT_PORT = 1883;
String mqttUsername = "";
String mqttPassword = "";

String mqttRoot =   "WeMakeColorsII";

String mqttTopicStatus = "status";
String mqttTopicSensor = "sensor";
String mqttTopicEvent = "randomColor";

String mqttTopicConfig = "config";

//sensor
unsigned long SENSOR_INTERVAL = 300; //seconds
unsigned long lastSensorSend = 0;


//-----MQTT subscribe --------------

void subscribeMQTT() {

  String mqttTopic;

  mqttTopic = mqttRoot + "/" + thingId + "/" + mqttTopicConfig;
  mqttClient.subscribe(mqttTopic.c_str(), QOS_AT_LEAST_1);
  if (DEBUG_MQTT) Serial.println("Subscibed to: " + mqttTopic);

  mqttTopic = mqttRoot + "/+/" + mqttTopicEvent;
  mqttClient.subscribe(mqttTopic.c_str(), QOS_AT_LEAST_1);
  if (DEBUG_MQTT) Serial.println("Subscibed to: " + mqttTopic);


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

  if (topic_leaf == mqttTopicEvent) {
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

  if ((topic_id == thingId) && (topic_leaf == mqttTopicConfig)) {
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


void publishSensor() {
  StaticJsonDocument<MQTT_MAX_PACKET_SIZE> doc;

  doc["name"] = name;
  doc["lightLevel"] = int(averageLightLevel);

  String   mqttTopic = mqttRoot + "/" + thingId + "/" + mqttTopicSensor;

  publishJSON(mqttTopic, doc, false);

}


void publishEvent(CHSV c) {
  StaticJsonDocument<MQTT_MAX_PACKET_SIZE> doc;

  doc["h"] = c.h;
  doc["s"] = c.s;
  doc["v"] = c.v;

  doc["name"] = name;
  doc["lightLevel"] = int(averageLightLevel);

  String   mqttTopic = mqttRoot + "/" + thingId + "/" + mqttTopicEvent;

  publishJSON(mqttTopic, doc, false);

}
