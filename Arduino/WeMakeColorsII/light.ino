
int LIGHT_TRIGGER = 100;

void setupLEDs() {
  FastLED.setBrightness(GLOBAL_BRIGHTNESS);
  FastLED.addLeds<WS2812B, LED_DATA_PIN, GRB>(leds, NUM_LEDS);
}


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

void setGlobalBrightness() {
  static unsigned long lastTimeGB = 0;

  if (millis() - lastTimeGB > 1000) {
    int lGB = analogRead(inputPin);
    totalGB = totalGB - readingsGB[readIndexGB];
    readingsGB[readIndexGB] = lGB;
    totalGB = totalGB + lGB;
    averageGB = totalGB / numReadingsGB;

    readIndexGB = readIndexGB + 1;
    if (readIndexGB >= numReadingsGB)  readIndexGB = 0;

    lastTimeGB = millis();
  }
}

void applyColor() {

  int mb = map(averageGB, 0, 512, 64, 255);
  if (mb > 255) mb = 255;

  //  Serial.print(averageGB);
  //  Serial.print(" ");
  //  Serial.println(mb);

  FastLED.setBrightness(mb);
  FastLED.show();
}


boolean lightChange() {

  int l = analogRead(inputPin);
  int dl = l - average;

  //  Serial.print(l);
  //  Serial.print(" ");
  //  Serial.print( average);
  //  Serial.print(" ");
  //  Serial.println(dl);

  total = total - readings[readIndex];
  readings[readIndex] = l;
  total = total + l;
  average = total / numReadings;

  readIndex = readIndex + 1;
  if (readIndex >= numReadings)  readIndex = 0;

  boolean change = false;

  if (abs(dl) > LIGHT_TRIGGER) {
    //Serial.println("day trigger");
    change = true;
  }

  //  if ((abs(dl) > average / 2 ) && (average < 150)) {
  //    Serial.println("night trigger");
  //    change = true;
  //  }

  return change;
}


CHSV newRndColor() {

  int h = random(0, 255);
  int s = random(128, 255);
  int v = random(128, 255);

  //Serial.print(h); Serial.print(" "); Serial.print(s); Serial.print(" "); Serial.println(v);
  return CHSV(h, s, v);
}



void showAllLeds(int r, int g, int b ) {
  for ( int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB(r, g, b);
  }
  FastLED.show();
}


