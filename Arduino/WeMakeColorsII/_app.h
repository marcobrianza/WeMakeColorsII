boolean DEBUG_APP = true;

unsigned long lastAppTime = 0;
int APP_INTERVAL = 100;

//------


bool ECHO_MODE = true; //legacy =flase

int inputPin = A0;
unsigned long lastColorTime = 0;

int NEW_COLOR_TIME = 2000;
float LIGHT_AVERAGE_DIFFERENCE = 0.3;
float LIGHT_TRIGGER_MINIMUM = 15;

float averageLightLevel = 0;
float averageLightLevelW = 0.2;

float globalLightLevel = 1023;
float globalLightLevelW = 0.004;


#define abs2(x) ((x)>0?(x):-(x))

void app_setup() {
  setupLEDs();
  showState(APP_START);
  globalLightLevel = analogRead(inputPin);
}



boolean checkLight() {

  float ll = analogRead(inputPin);
  int dl = ll - averageLightLevel;

  averageLightLevel = averageLightLevel * (1 - averageLightLevelW)   + ll * averageLightLevelW;
  globalLightLevel = globalLightLevel * (1 - globalLightLevelW) + ll * globalLightLevelW;


  //trigger 1.0
  //  if (abs(dl) > LIGHT_TRIGGER) {
  //    //Serial.println("light trigger");
  //    change = true;
  //  }

  //trigger 2.0
  boolean trig_difference = false;
  float pa = abs2( dl / averageLightLevel);
  if (pa > LIGHT_AVERAGE_DIFFERENCE)  {
    trig_difference = true;
    // Serial.println("trig_difference:" + String(pa));
  }

  boolean trig_level = false;
  if (averageLightLevel > LIGHT_TRIGGER_MINIMUM)  {
    trig_level = true;
    //Serial.println("trig_level");
  }

  boolean trig_time = false;
  if (millis() - lastColorTime > NEW_COLOR_TIME) {
    trig_time = true;
    // Serial.println("trig_time");
  }


  boolean newColor = false;
  if (trig_difference && trig_level && trig_time) {
    lastColorTime = millis();
    newColor = true;
  }



  //  Serial.print(pa, 4);
  //
  //  Serial.print(" ");
  //  Serial.print(change);
  //
  //
  // Serial.println();

  //
  //  Serial.print(ll);
  //  Serial.print(" ");
  //  Serial.print(averageLightLevel);
  //  Serial.print(" ");
  //  Serial.print(globalLightLevel);
  //  Serial.print(" ");
  //  Serial.print(change * 100);
  //  Serial.println();


  return newColor;
}



CHSV newRndColor() {

  int h = random(0, 255);
  int s = random(128, 255);
  int v = random(128, 255);

  //Serial.print(h); Serial.print(" "); Serial.print(s); Serial.print(" "); Serial.println(v);

  return CHSV(h, s, v);
}


void showLLEDs() {

  int mb = map(globalLightLevel, 0, 512, 64, 255);
  if (mb > 255) mb = 255;
  FastLED.setBrightness(mb);
  FastLED.show();

  //  Serial.print("globalLightLevel: ");
  //  Serial.print(globalLightLevel);
  //  Serial.print(" ");
  //  Serial.println(mb);
}


void setMyLED(CHSV newC) {
  leds[0] = newC;
  showLLEDs();
}

void setRemoteLED(CHSV newC) {
  leds[1] = newC;
  showLLEDs();
}

void  app_testDevice() {

  if (DEBUG_APP) Serial.println("Testing  device for 30s");
  int TEST_TIME = 30000; // 30 seconds

  while (millis() < TEST_TIME) {
    int a = analogRead(inputPin);
    int v = a / 4;
    if (v > 255) v = 255;
    Serial.print(millis());
    Serial.print(" ");
    Serial.println(v);
    showAllLeds(v, v, v);
    delay(40);
  }
}
