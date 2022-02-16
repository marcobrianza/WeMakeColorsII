
String mqttServer = "wmc.marcobrianza.it";
int MQTT_PORT = 1883;
String mqttUsername = "";
String mqttPassword = "";

String mqttRoot =   "WeMakeColorsII";

String mqttTopicStatus = "status";
String mqttTopicSensor = "sensor";

String mqttTopicEvent = "randomColor";

String mqttTopicConfig = "config";
String mqttTopicInfo = "info";

String mqttTopicAction = "action";

//sensor
unsigned long SENSOR_INTERVAL = 300; //seconds
unsigned long lastSensorSend = 0;

bool newSettings = false;

String updateUrl = "";

//-----MQTT subscribe --------------

void subscribeMQTT() {

  String mqttTopic;

  mqttTopic = mqttRoot + "/" + thingId + "/" + mqttTopicConfig;
  mqttClient.subscribe(mqttTopic.c_str(), QOS_AT_LEAST_1);
  if (DEBUG_MQTT) Serial.println("Subscibed to: " + mqttTopic);

  mqttTopic = mqttRoot + "/+/" + mqttTopicEvent;
  mqttClient.subscribe(mqttTopic.c_str(), QOS_AT_LEAST_1);
  if (DEBUG_MQTT) Serial.println("Subscibed to: " + mqttTopic);

  mqttTopic = mqttRoot + "/+/" + mqttTopicAction;
  mqttClient.subscribe(mqttTopic.c_str(), QOS_AT_LEAST_1);
  if (DEBUG_MQTT) Serial.println("Subscibed to: " + mqttTopic);

}

// ---- MQTT receive-------------------



void processAction(StaticJsonDocument<MQTT_BUFFER>  jPayload) {

  // { "globalBrightness":255, "pixel":0, "h": 0,"s": 128,"v": 255}
  if (jPayload.containsKey("globalBrightness")) {
    int l = jPayload["globalBrightness"];
    l = constrain(l, 0, 255);
    AUTO_BRIGHTNESS = false;
    GLOBAL_BRIGHTNESS = l;
    if (DEBUG_MQTT)  Serial.println("globalBrightness: " + String(GLOBAL_BRIGHTNESS) );
    showLEDs();
  }

  // { "autoBrightness":true}
  if (jPayload.containsKey("autoBrightness")) {
    AUTO_BRIGHTNESS = jPayload["autoBrightness"];
    if (DEBUG_MQTT)  Serial.println("autoBrightness: " + String(AUTO_BRIGHTNESS) );
    showLEDs();
  }

  if (jPayload.containsKey("pixel")) {
    int p = jPayload["pixel"];
    int h = jPayload["h"];
    int s = jPayload["s"];
    int v = jPayload["v"];
    if (DEBUG_MQTT)  Serial.println("pixel hsv: " + String(p) + " " + String(h) + " " + String(s) + " " + String(v) );
    setLED(p, CHSV(h, s, v));
  }

  // {"pixels":"255 0 0 0 0 255"}
  if (jPayload.containsKey("pixels")) {
    String p = jPayload["pixels"];

    if (DEBUG_MQTT)  Serial.println("pixels: " + p);

    char sep = ' ';
    p = p + sep;
    int l = 0;
    String s = "";
    int b =  NUM_LEDS * 3;
    byte newPixels[b];

    for (int i = 0; i < p.length() ; i++) {
      char c = p.charAt(i);
      if (c == sep) {
        byte v = s.toInt();
        newPixels[l] = v;
        Serial.println (v);
        s = "";
        l++;
        if (l == b) break;
      } else s = s + c;
    }
    Serial.println(l);
    if (l == b) {
      memmove(&leds[0], &newPixels[0], b);
      streamLEDs = true;
      //FastLED.show();
    }
  }


}

void processEvent(StaticJsonDocument<MQTT_BUFFER>  jPayload) {
  // {"h": 0,"s": 128,"v": 255}
  int h = jPayload["h"];
  int s = jPayload["s"];
  int v = jPayload["v"];
  if (DEBUG_MQTT)  Serial.println("hsv: " + String(h) + " " + String(s) + " " + String(v) );
  setLED(REMOTE_LED, CHSV(h, s, v));

}



// WeMakeColorsII/WMCII_CC:50:E3:C4:94:1C/config
void processConfig(StaticJsonDocument<MQTT_BUFFER>  jPayload) {

  // {"status":""}
  if (jPayload.containsKey("status")) {
    publishStatusFlag = true;
  }

  // {"info":""}
  if (jPayload.containsKey("info")) {
    publishInfoFlag = true;
  }


  // { "updateUrl":"http://iot.marcobrianza.it/art/WeMakeColorsII.ino.d1_mini-1.30.4.bin"}
  if (jPayload.containsKey("updateUrl")) {
    String s = jPayload["updateUrl"];
    updateUrl = s;
    if (DEBUG_MQTT)  Serial.println("updateURL: " + updateUrl);
    softwareUpdateFlag = true;
  }




  //--- settings-----------------------------


  String ssid = "";
  String wifiPassword = "";

  if (jPayload.containsKey("ssid")) {
    String s  = jPayload["ssid"];
    ssid = s;
    newSettings = true;
  }

  // {"ssid":"myssid", "wifiPassword": "mypassword"}
  if (jPayload.containsKey("wifiPassword")) {
    String s = jPayload["wifiPassword"];
    wifiPassword = s;
    newSettings = true;

    if (DEBUG_MQTT) Serial.println(ssid + " " + wifiPassword);
    connectWiFi(ssid, wifiPassword);
  }

  // {"name":"Marcos222"}
  if (jPayload.containsKey("name")) {
    String s = jPayload["name"];
    name = s;
    newSettings = true;

    if (DEBUG_MQTT) Serial.println(name);
  }

  // {"mqttServer":"192.168.1.5","mqttUsername": "aieie" "mqttPassword": "brazorf"}
  if (jPayload.containsKey("mqttServer")) {
    String s = jPayload["mqttServer"];
    mqttServer = s;
    newSettings = true;
  }

  if (jPayload.containsKey("mqttUsername")) {
    String s = jPayload["mqttUsername"];
    mqttUsername = s;
    newSettings = true;
  }

  if (jPayload.containsKey("mqttPassword")) {
    String s = jPayload["mqttPassword"];
    mqttPassword = s;
    newSettings = true;

    if (DEBUG_MQTT) Serial.println(mqttServer + " " + mqttUsername + " " + mqttPassword);
  }

  if (newSettings) mqttClient.disconnect();

}




void mqttReceive(char* rawTopic, byte* rawPayload, unsigned int length) {
  String topic = rawTopic;

  String strPayload = "";
  for (int i = 0; i < length; i++) {
    strPayload = strPayload + (char)rawPayload[i];
  }


  if (DEBUG_MQTT)  Serial.println("MQTT received: " + String(length) + " " + topic + " " + strPayload);

  int p1 = mqttRoot.length() + 1;
  int p2 = topic.indexOf("/", p1);

  //String topic_root = String(topic).substring(0, p1 - 1);
  String topic_id = topic.substring(p1, p2);
  String topic_leaf = topic.substring(p2 + 1);

  //  Serial.println(topic_root);
  //  Serial.println(topic_id);
  //  Serial.println(topic_leaf);

  StaticJsonDocument<MQTT_BUFFER> jPayload;
  DeserializationError error = deserializeJson(jPayload, strPayload);
  if (error) {
    if (DEBUG_MQTT)  Serial.println("deserializeJson() failed with code: " + String(error.c_str()));
  }
  else {

    //Serial.println("deserializeJson():ok");
    //--------------event---------------------
    if (topic_leaf == mqttTopicEvent) {

      if (DEBUG_MQTT) {
        String my = ""; if (topic_id == thingId)  my = " (my message)";
        Serial.println("Event message: " + my);
      }
      if ((topic_id == thingId) && (ECHO_MODE) || (topic_id != thingId) ) {
        processEvent(jPayload);
      }
    }


    //----- config------------
    if ((topic_id == thingId) && (topic_leaf == mqttTopicConfig)) {
      if (DEBUG_MQTT) Serial.println("Config message:");
      processConfig(jPayload);
    }


    //----- action------------
    if ((topic_id == thingId) && (topic_leaf == mqttTopicAction)) {
      if (DEBUG_MQTT) Serial.println("Action message:");
      processAction(jPayload);
    }

  }
}




//-----MQTT publish --------------


void publishSensor() {
  StaticJsonDocument<MQTT_BUFFER> jPayload;

  jPayload["name"] = name;
  jPayload["lightLevel"] = int(averageLightLevel);

  String   mqttTopic = mqttRoot + "/" + thingId + "/" + mqttTopicSensor;
  publishJSON(mqttTopic, jPayload, false);
}


void publishEvent(CHSV c) {
  StaticJsonDocument<MQTT_BUFFER> jPayload;

  jPayload["h"] = c.h;
  jPayload["s"] = c.s;
  jPayload["v"] = c.v;

  jPayload["name"] = name;
  jPayload["lightLevel"] = int(averageLightLevel);

  String   mqttTopic = mqttRoot + "/" + thingId + "/" + mqttTopicEvent;
  publishJSON(mqttTopic, jPayload, false);
}

void publishInfo() {
  StaticJsonDocument<MQTT_BUFFER> jPayload;

  jPayload["name"] = name;
  // jPayload["freeHeap"] = ESP.getFreeHeap();

  jPayload["softwareInfo"] = softwareInfo;
  jPayload["softwarePlatform"] = softwarePlatform;

  jPayload["ssid"] = WiFi.SSID();
  jPayload["wifiPassword"] = WiFi.psk();


  jPayload["mqttServer"] = mqttServer;
  jPayload["mqttUsername"] = mqttUsername;
  jPayload["mqttPassword"] = mqttPassword;

  String   mqttTopic = mqttRoot + "/" + thingId + "/" + mqttTopicInfo;
  publishJSON(mqttTopic, jPayload, true);
}
