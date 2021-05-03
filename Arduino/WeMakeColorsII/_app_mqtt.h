
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



void processAction(StaticJsonDocument<MQTT_BUFFER>  doc) {

  // { "globalBrightness":255, "pixel":0, "h": 0,"s": 128,"v": 255}
  if (doc.containsKey("globalBrightness")) {
    int l = doc["globalBrightness"];
    l = constrain(l, 0, 255);
    AUTO_BRIGHTNESS = false;
    GLOBAL_BRIGHTNESS = l;
    if (DEBUG_MQTT)  Serial.println("globalBrightness: " + String(GLOBAL_BRIGHTNESS) );
    showLEDs();
  }

  // { "autoBrightness":true}
  if (doc.containsKey("autoBrightness")) {
    AUTO_BRIGHTNESS = doc["autoBrightness"];
    if (DEBUG_MQTT)  Serial.println("autoBrightness: " + String(AUTO_BRIGHTNESS) );
    showLEDs();
  }

  if (doc.containsKey("pixel")) {
    int p = doc["pixel"];
    int h = doc["h"];
    int s = doc["s"];
    int v = doc["v"];
    if (DEBUG_MQTT)  Serial.println("pixel hsv: " + String(p) + " " + String(h) + " " + String(s) + " " + String(v) );
    setLED(p, CHSV(h, s, v));
  }

  // {"pixels":"255 0 0 0 0 255"}
  if (doc.containsKey("pixels")) {
    String p = doc["pixels"];

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
      FastLED.show();
    }
  }


}

void processEvent(StaticJsonDocument<MQTT_BUFFER>  doc) {
  // {"h": 0,"s": 128,"v": 255}
  int h = doc["h"];
  int s = doc["s"];
  int v = doc["v"];
  if (DEBUG_MQTT)  Serial.println("hsv: " + String(h) + " " + String(s) + " " + String(v) );
  setLED(REMOTE_LED, CHSV(h, s, v));

}



// WeMakeColorsII/WMCII_CC:50:E3:C4:94:1C/config
void processConfig(StaticJsonDocument<MQTT_BUFFER>  doc) {

  // {"status":""}
  if (doc.containsKey("status")) {
    publishStatusFlag = true;
  }

  // {"info":""}
  if (doc.containsKey("info")) {
    publishInfoFlag = true;
  }


  // { "updateUrl":"http://iot.marcobrianza.it/art/WeMakeColorsII.ino.d1_mini-1.30.4.bin"}
  if (doc.containsKey("updateUrl")) {
    String s = doc["updateUrl"];
    updateUrl = s;
    if (DEBUG_MQTT)  Serial.println("updateURL: " + updateUrl);
    softwareUpdateFlag = true;
  }




  //--- settings-----------------------------


  String ssid = "";
  String wifiPassword = "";

  if (doc.containsKey("ssid")) {
    String s  = doc["ssid"];
    ssid = s;
    newSettings = true;
  }

  // {"ssid":"myssid", "wifiPassword": "mypassword"}
  if (doc.containsKey("wifiPassword")) {
    String s = doc["wifiPassword"];
    wifiPassword = s;
    newSettings = true;

    if (DEBUG_MQTT) Serial.println(ssid + " " + wifiPassword);
    connectWiFi(ssid, wifiPassword);
  }

  // {"name":"Marcos222"}
  if (doc.containsKey("name")) {
    String s = doc["name"];
    name = s;
    newSettings = true;

    if (DEBUG_MQTT) Serial.println(name);
  }

  // {"mqttServer":"192.168.1.5","mqttUsername": "aieie" "mqttPassword": "brazorf"}
  if (doc.containsKey("mqttServer")) {
    String s = doc["mqttServer"];
    mqttServer = s;
    newSettings = true;
  }

  if (doc.containsKey("mqttUsername")) {
    String s = doc["mqttUsername"];
    mqttUsername = s;
    newSettings = true;
  }

  if (doc.containsKey("mqttPassword")) {
    String s = doc["mqttPassword"];
    mqttPassword = s;
    newSettings = true;

    if (DEBUG_MQTT) Serial.println(mqttServer + " " + mqttUsername + " " + mqttPassword);
  }

  if (newSettings) mqttClient.disconnect();

}




void mqttReceive(char* rawTopic, byte* rawPayload, unsigned int length) {
  String topic = rawTopic;
  
  rawPayload[length] = 0;
  String strPayload = String((char*)rawPayload);


  if (DEBUG_MQTT)  Serial.println("MQTT received: " + String(length) + " " + topic + " " + strPayload);

  int p1 = mqttRoot.length() + 1;
  int p2 = topic.indexOf("/", p1);

  //String topic_root = String(topic).substring(0, p1 - 1);
  String topic_id = topic.substring(p1, p2);
  String topic_leaf = topic.substring(p2 + 1);

  //  Serial.println(topic_root);
  //  Serial.println(topic_id);
  //  Serial.println(topic_leaf);

  StaticJsonDocument<MQTT_BUFFER> doc;
  DeserializationError error = deserializeJson(doc, strPayload);
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
        processEvent(doc);
      }
    }


    //----- config------------
    if ((topic_id == thingId) && (topic_leaf == mqttTopicConfig)) {
      if (DEBUG_MQTT) Serial.println("Config message:");
      processConfig(doc);
    }


    //----- action------------
    if ((topic_id == thingId) && (topic_leaf == mqttTopicAction)) {
      if (DEBUG_MQTT) Serial.println("Action message:");
      processAction(doc);
    }

  }
}




//-----MQTT publish --------------


void publishSensor() {
  StaticJsonDocument<MQTT_BUFFER> doc;

  doc["name"] = name;
  doc["lightLevel"] = int(averageLightLevel);

  String   mqttTopic = mqttRoot + "/" + thingId + "/" + mqttTopicSensor;
  publishJSON(mqttTopic, doc, false);
}


void publishEvent(CHSV c) {
  StaticJsonDocument<MQTT_BUFFER> doc;

  doc["h"] = c.h;
  doc["s"] = c.s;
  doc["v"] = c.v;

  doc["name"] = name;
  doc["lightLevel"] = int(averageLightLevel);

  String   mqttTopic = mqttRoot + "/" + thingId + "/" + mqttTopicEvent;
  publishJSON(mqttTopic, doc, false);
}

void publishInfo() {
  StaticJsonDocument<MQTT_BUFFER> doc;

  doc["name"] = name;
  // doc["freeHeap"] = ESP.getFreeHeap();

  doc["softwareInfo"] = softwareInfo;
  doc["softwarePlatform"] = softwarePlatform;

  doc["ssid"] = WiFi.SSID();
  doc["wifiPassword"] = WiFi.psk();


  doc["mqttServer"] = mqttServer;
  doc["mqttUsername"] = mqttUsername;
  doc["mqttPassword"] = mqttPassword;

  String   mqttTopic = mqttRoot + "/" + thingId + "/" + mqttTopicInfo;
  publishJSON(mqttTopic, doc, true);
}
