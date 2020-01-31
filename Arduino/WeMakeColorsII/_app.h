boolean DEBUG_APP = true;

bool ECHO_MODE = true; //legacy =flase

int LIGHT_TRIGGER = 100;

int NEW_COLOR_TIME = 1000;

//presence
unsigned long last_color_t = 0;
int inputPin = A0;

boolean newColor = false;

int CHECK_LIGHT_TIME = 100;

unsigned long lastLightTime = 0;


float averageLightLevel = 512;
float averageLightLevelW = 0.2;

float globalLightLevel = 1023;
float globalLightLevelW = 0.004;

void setupLightLevel() {
  int a = analogRead(inputPin);

  averageLightLevel = a;
  globalLightLevel = a;
}


void app_setup() {
  setupLEDs();
  showAllLeds(64, 64, 64);
  setupLightLevel() ;
}




void checkLight() {

  float ll = analogRead(inputPin);
  int dl = ll - averageLightLevel;

  averageLightLevel = averageLightLevel * (1 - averageLightLevelW)   + ll * averageLightLevelW;
  globalLightLevel = globalLightLevel * (1 - globalLightLevelW) + ll * globalLightLevelW;

  boolean change = false;
  if (abs(dl) > LIGHT_TRIGGER) {
    //Serial.println("day trigger");
    change = true;
  }

  //  if ((abs(dl) > average / 2 ) && (average < 150)) {
  //    Serial.println("night trigger");
  //    change = true;
  //  }

  if (change && (millis() - last_color_t > NEW_COLOR_TIME)) {
    last_color_t = millis();
    newColor = true;
  }
  //
  //  Serial.print(ll);
  //  Serial.print(" ");
  //  Serial.print(averageLightLevel);
  //  Serial.print(" ");
  //  Serial.print(globalLightLevel);
  //  Serial.print(" ");
  //  Serial.print(change * 100);
  //  Serial.println();

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

//  Serial.print("globalLightLevel: ");
//  Serial.print(globalLightLevel);
//  Serial.print(" ");
//  Serial.println(mb);

  FastLED.setBrightness(mb);
  FastLED.show();
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
