//LED
#include <FastLED.h> // version  3.3.2

int GLOBAL_BRIGHTNESS = 255;

const int NUM_LEDS = 2;
CRGB leds[NUM_LEDS];

void setupLEDs() {
  FastLED.setBrightness(GLOBAL_BRIGHTNESS);
  FastLED.addLeds<WS2812B, D1, GRB>(leds, NUM_LEDS); //D1 is GPIO5
}


void showAllLeds(int r, int g, int b ) {
  for ( int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB(r, g, b);
  }
  FastLED.show();
}
