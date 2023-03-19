//LED
//#define FASTLED_ALLOW_INTERRUPTS 0
//#define FASTLED_INTERRUPT_RETRY_COUNT 1

#include <FastLED.h> // FastLED - by Daniel Garcia - 3.5.0 http://librarymanager/all#fastled //we need this for HSV to RGB conversion
#include <Adafruit_NeoPixel.h> //Adafruit NeoPixel by Adafruit 1.11.0 

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
//CRGB leds[NUM_LEDS];

bool streamLEDs = false;

typedef struct {
  byte h;
  byte s;
  byte v;
} hsvColor_t;

typedef struct {
  byte r;
  byte g;
  byte b;
} rgbColor_t;


Adafruit_NeoPixel pixels(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

void setupLEDs() {
  pixels.setBrightness(GLOBAL_BRIGHTNESS);
  pixels.begin();
  pixels.clear();
  pixels.show();
}

rgbColor_t rgbColor ( byte r, byte g, byte b) {
  rgbColor_t c = {r, g, b};
  return (c);
}

void showAllLeds(rgbColor_t c ) {
  for ( int i = 0; i < NUM_LEDS; i++) {
    pixels.setPixelColor(i, c.r, c.g, c.b);
  }
  pixels.show();
}

void showAllLeds(int r, int g, int b ) {
  for ( int i = 0; i < NUM_LEDS; i++) {
    pixels.setPixelColor(i, r, g, b);
  }
  pixels.show();
}


void testRGB(){
    showAllLeds({255, 0, 0});
  pixels.show();
  delay(1000);

  showAllLeds({0, 255, 0});
  pixels.show();
  delay(1000);

  showAllLeds({0, 0, 255});
  pixels.show();
  delay(1000);
}
