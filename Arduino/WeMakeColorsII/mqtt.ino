

//--------------MQTT--------------
void setupMqtt() {
  mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
  mqttClient.setCallback(mqtt_callback);
}

void reconnectMQTT() {
  // Loop until we're reconnected
  while (!mqttClient.connected()) {
    digitalWrite(LED_BUILTIN, LOW);
    Serial.print("\nAttempting MQTT connection...");
    // Attempt to connect
    wifiClient = WiFiClient(); // workaround to fix reconnection?
    if (mqttClient.connect(THING_ID, MQTT_USERNAME, MQTT_PASSWORD)) {
      digitalWrite(LED_BUILTIN, HIGH);
      Serial.println("connected\n");
      // ... and resubscribe
      char MQTT_SUBSCRIBE[MQTT_MAX_PACKET_SIZE];
#define QOS_AT_LEAST_1 1


      mqttSubscribe_randomColor = mqttRoot + "/+/" + mqtt_randomColor;
      mqttSubscribe_randomColor.toCharArray(MQTT_SUBSCRIBE, mqttSubscribe_randomColor.length() + 1);
      mqttClient.subscribe(MQTT_SUBSCRIBE, QOS_AT_LEAST_1);
      Serial.print("Subscibed to: ");
      Serial.println(MQTT_SUBSCRIBE);

      mqttSubscribe_config = mqttRoot + "/" + thingId + "/" + mqtt_config;
      mqttSubscribe_config.toCharArray(MQTT_SUBSCRIBE, mqttSubscribe_config.length() + 1);
      mqttClient.subscribe(MQTT_SUBSCRIBE, QOS_AT_LEAST_1);
      Serial.print("Subscibed to: ");
      Serial.println(MQTT_SUBSCRIBE);

      publishBeat();

    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}


void publishRandomColor(CHSV c) {
  StaticJsonBuffer<MQTT_MAX> jsonBufferMQTT;
  JsonObject& jsonMsg = jsonBufferMQTT.createObject();

  jsonMsg["h"] = c.h;
  jsonMsg["s"] = c.s;
  jsonMsg["v"] = c.v;

  String jsonU;

  char mqttData[MQTT_MAX];
  jsonMsg.printTo(mqttData);

  char mqttTopicPub[MQTT_MAX];
  mqttPublish_randomColor = mqttRoot + "/" + thingId + "/" + mqtt_randomColor;
  mqttPublish_randomColor.toCharArray(mqttTopicPub, mqttPublish_randomColor.length() + 1);

  int ret = mqttClient.publish(mqttTopicPub, mqttData);

  Serial.print("MQTT message sent: ");
  Serial.print(mqttTopicPub);
  Serial.print(" ");
  Serial.print(mqttData);
  Serial.print(" result: ");
  Serial.println(ret);
}

void publishBeat() {
  static unsigned long b = 0;
  String bb = String(b);
  b++;

  StaticJsonBuffer<MQTT_MAX> jsonBufferMQTT;
  JsonObject& jsonMsg = jsonBufferMQTT.createObject();

  jsonMsg["count"] = bb;
  jsonMsg["softwareName"] = softwareName;
  jsonMsg["softwareVersion"] = softwareVersion;
  jsonMsg["thingName"] = THING_NAME;

  //jsonMsg["MD5"] = ESP.getSketchMD5();


  jsonMsg["lightLevel"] = average;

  String jsonU;

  char mqttData[MQTT_MAX];
  jsonMsg.printTo(mqttData);

  char mqttTopicPub[MQTT_MAX];
  mqttPublish_beat = mqttRoot + "/" + thingId + "/" + mqtt_beat;
  mqttPublish_beat.toCharArray(mqttTopicPub, mqttPublish_beat.length() + 1);
  int ret = mqttClient.publish(mqttTopicPub, mqttData);

  Serial.print("MQTT message sent: ");
  Serial.print(mqttTopicPub);
  Serial.print(" ");
  Serial.print(mqttData);
  Serial.print(" result: ");
  Serial.println(ret);
}

void mqtt_callback(char* topic, byte* payload, unsigned int length) {
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

      leds[1] = CHSV(h, s, v);
      applyColor();
    } else Serial.println("My message");
  }

  if ((topic_id = thingId) && (topic_leaf == mqtt_config)) {
    Serial.println("Config message:");
    JsonObject& root = jsonBuffer.parseObject(payload);
    /*
      {
      "command":"update",
      "option":"http://iot.marcobrianza.it/WeMakeColorsII/WeMakeColorsII.ino.d1_mini.bin"
      }
    */

    String command = root["command"];
    String option = root["option"];

    Serial.println(command);
    Serial.println(option);

    if (command = "update") {
      showAllLeds(64, 0, 0);
      int u = httpUpdate(option);
      if (u != HTTP_UPDATE_OK) showAllLeds(64, 64, 0);
    }

    Serial.println(command);
    Serial.println(option);

  }
}

