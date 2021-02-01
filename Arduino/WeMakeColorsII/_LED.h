//LED
#include <FastLED.h> // version  3.4.0

// standard defines for WMCII
#define LED_PIN D1
#define LED_ORDER GRB

// defines for IoT kit
//#define LED_PIN D5
//#define LED_ORDER RGB

int GLOBAL_BRIGHTNESS = 255;

const int NUM_LEDS = 2;
CRGB leds[NUM_LEDS];

void setupLEDs() {
  FastLED.setBrightness(GLOBAL_BRIGHTNESS);
  FastLED.addLeds<WS2812B, LED_PIN, LED_ORDER>(leds, NUM_LEDS);
}

void showAllLeds(CRGB c ) {
  for ( int i = 0; i < NUM_LEDS; i++) {
    leds[i] = c;
  }
  FastLED.show();
}

void showAllLeds(int r, int g, int b ) {
  showAllLeds (CRGB(r, g, b));
}
