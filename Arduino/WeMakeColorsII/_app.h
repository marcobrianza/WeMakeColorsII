boolean DEBUG_APP=false;

bool ECHO_MODE = true; //legacy =flase

const int numReadings = 10;
int readings[numReadings];      // the readings from the analog input
int readIndex = 0;              // the index of the current reading
int total = 0;                  // the running total
int average = 0;

//brightness
const int numReadingsGB = 25;
int readingsGB[numReadingsGB];      // the readings from the analog input
int readIndexGB = 0;              // the index of the current reading
int totalGB = 0;                  // the running total
int averageGB = 0;

int LIGHT_TRIGGER = 100;

int NEW_COLOR_TIME = 1000;


//presence
unsigned long last_color_t = 0;
int inputPin = A0;

boolean newColor = false;

int CHECK_LIGHT_TIME = 100;
int GLOBAL_BRIGHTNESS_TIME = 1000;

unsigned long lastLightTime = 0;
unsigned long lastBrightnessTime = 0;


float lightLevel = 512;
float lighightLevelSamples = 10;


void setupLightLevel() {
  int a = analogRead(inputPin);

  for (int i = 0; i < numReadings; i++) {
    readings[i] = a;
  }
  total = a * numReadings;
  //average = total / numReadings; average is not updated so we have  a new color at start

  for (int iGB = 0; iGB < numReadingsGB; iGB++) {
    readingsGB[iGB] = a;
  }
  totalGB = a * numReadingsGB;
  averageGB = totalGB / numReadingsGB;
}


void app_setup() {
  setupLEDs();
  showAllLeds(64, 64, 64);
  setupLightLevel() ;
}

void setGlobalBrightness() {

  int lGB = analogRead(inputPin);
  totalGB = totalGB - readingsGB[readIndexGB];
  readingsGB[readIndexGB] = lGB;
  totalGB = totalGB + lGB;
  averageGB = totalGB / numReadingsGB;

  readIndexGB = readIndexGB + 1;
  if (readIndexGB >= numReadingsGB)  readIndexGB = 0;

}


void checkLight() {

  int l = analogRead(inputPin);
  int dl = l - average;

  Serial.print(l);
  Serial.print(" ");
  Serial.print( average);
  Serial.print(" ");
  Serial.print(dl);


  total = total - readings[readIndex];
  readings[readIndex] = l;
  total = total + l;
  average = total / numReadings;

  readIndex = readIndex + 1;
  if (readIndex >= numReadings)  readIndex = 0;

  float ll = l;
  lightLevel = (lightLevel * (lighightLevelSamples - 1)  + ll) / lighightLevelSamples;

  float dl2 = ll - lightLevel;

  Serial.print(" ");
  Serial.print(lightLevel);
  Serial.print(" ");
  Serial.print(dl2);
  Serial.println();


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

  // return change;
}



CHSV newRndColor() {

  int h = random(0, 255);
  int s = random(128, 255);
  int v = random(128, 255);

  //Serial.print(h); Serial.print(" "); Serial.print(s); Serial.print(" "); Serial.println(v);
  return CHSV(h, s, v);
}


void showLLEDs() {

  int mb = map(averageGB, 0, 512, 64, 255);
  if (mb > 255) mb = 255;

  //  Serial.print(averageGB);
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

  Serial.println("Testing  device for 30s");
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
