boolean DEBUG_APP = true;

unsigned long lastAppTime = 0;
unsigned long  APP_INTERVAL = 100;

unsigned long lastButtonTime = 0;
unsigned long  BUTTON_INTERVAL = 2000;

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

bool AUTO_BRIGHTNESS = true;


#define abs2(x) ((x)>0?(x):-(x))

#define MY_LED 0
#define REMOTE_LED 1

#define BUTTON D6

void app_setup() {
  setupLEDs();
  showState(APP_START);
  globalLightLevel = analogRead(inputPin);
  pinMode(BUTTON, INPUT_PULLUP);
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





hsvColor_t newRndColor() {

  int h = random(0, 255);
  int s = random(128, 255);
  int v = random(128, 255);

  //Serial.print(h); Serial.print(" "); Serial.print(s); Serial.print(" "); Serial.println(v);

  return {h, s, v};
}

int mapInt(int x, int in_min, int in_max, int out_min, int out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void showLEDs() {

  if (AUTO_BRIGHTNESS) {
    //Serial.println("Setting AUTO_BRIGHTNESS");
    GLOBAL_BRIGHTNESS = mapInt(globalLightLevel, 0, 512, 64, 255);
  }
  if (GLOBAL_BRIGHTNESS > 255) GLOBAL_BRIGHTNESS = 255;

  pixels.setBrightness(GLOBAL_BRIGHTNESS);
  pixels.show();

  //  Serial.println("AUTO_BRIGHTNESS: " + String(AUTO_BRIGHTNESS));
  //  Serial.println("globalLightLevel: " + String(globalLightLevel));
  //  Serial.println("GLOBAL_BRIGHTNESS " + String(GLOBAL_BRIGHTNESS));

}


void setLED(int i, hsvColor_t newC) {
  //  Serial.println("setLED " + String(i));
  //  Serial.println(newC.h);
  //  Serial.println(newC.s);
  //  Serial.println(newC.v);

  if (i < NUM_LEDS) {
    CHSV c = CHSV(newC.h , newC.s, newC.v);
    CRGB cc = c;
    pixels.setPixelColor(i, pixels.Color(cc.r, cc.g, cc.b));
    showLEDs();
  } else if (DEBUG_APP) Serial.println("pixel index too high");
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
