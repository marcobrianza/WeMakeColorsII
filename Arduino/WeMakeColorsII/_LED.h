//LED
//#define FASTLED_ALLOW_INTERRUPTS 0
//#define FASTLED_INTERRUPT_RETRY_COUNT 1

#include <FastLED.h> // FastLED - by Daniel Garcia - 3.5.0 http://librarymanager/all#fastled


#if BOARD_TYPE==IOTKIT// defines for IoT kit
  #define LED_PIN D5
  #define LED_ORDER RGB
#elif BOARD_TYPE==WMCII //standard defines for WMCII
  #define LED_PIN D1
  #define LED_ORDER GRB
# else 
  #error("please define a compatible BOARD_TYPE");
#endif

int GLOBAL_BRIGHTNESS = 255;

const int NUM_LEDS = 2;
CRGB leds[NUM_LEDS];

bool streamLEDs =false;

void setupLEDs() {
  FastLED.setBrightness(GLOBAL_BRIGHTNESS);
  FastLED.addLeds<WS2812B, LED_PIN, LED_ORDER>(leds, NUM_LEDS);
}

void showAllLeds(CRGB c ) {
  for ( int i = 0; i < NUM_LEDS; i++) {
    leds[i] = c;
  }
  streamLEDs=true;
  //FastLED.show();
}

void showAllLeds(int r, int g, int b ) {
  showAllLeds (CRGB(r, g, b));
}
