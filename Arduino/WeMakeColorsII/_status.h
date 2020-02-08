
#include "_LED.h"

int const APP_START = 0;
int const UPDATE = 1;
int const AUTO_UPDATE = 2;
int const UPDATE_ERROR = -3;
int const UPDATE_OK = 3;
int const WIFI_MANAGER_AP = 4;
int const WIFI_MANAGER_SAVE = 5;


void showState(int s) {
  CRGB c;
  //String ss;
  switch (s) {
    case UPDATE:
      c = CRGB( 64, 0, 0);
      break;
    case AUTO_UPDATE:
      c = CRGB(64, 0, 0);
      blink(10);
      break;
    case UPDATE_OK:
      c = CRGB(0, 64, 0);
      blink(10);
      break;
    case UPDATE_ERROR:
      c = CRGB(64, 64, 0);
      break;
    case WIFI_MANAGER_AP:
      //ss = "wifi manager ap";
      c =  CRGB(0, 0, 255);
      break;
    case WIFI_MANAGER_SAVE:
      c =  CRGB(0, 255, 255);
      break;
    case APP_START:
      //ss = "app start";
      c =  CRGB(255, 255, 255);
      break;

  }
  //if (ss != "") Serial.println(ss);
  showAllLeds(c);
}
