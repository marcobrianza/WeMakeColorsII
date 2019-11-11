int tc;

Ticker T_test;

void F_test() {
  test = true;
}

void test_setup() {
  T_test.attach(10, F_test);
}

void publishTest() {

  if (mqttClient.connected()) {
    String s = String(tc);
    int ret = mqttClient.publish("test", s.c_str());
    Serial.println(String(ret) + " MQTT sent: test "  + s );
  } else Serial.println("test: MQTT not connected");
  tc++;
}
