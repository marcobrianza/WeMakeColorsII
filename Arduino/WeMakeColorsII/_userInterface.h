
//led builtin
#define LED_ON LOW
#define LED_OFF HIGH

#include <Ticker.h>
Ticker T_UI;

int c = 0;
bool LED_STATE = false;


void F_UI() {
  c--;
  if (c == 0) {
    T_UI.detach();
  } else {
    LED_STATE = !LED_STATE;
    digitalWrite(LED_BUILTIN, LED_STATE);
  }
}
void blink(int b) {

  c = b * 2;
  LED_STATE = LED_ON;
  digitalWrite(LED_BUILTIN, LED_STATE);

  T_UI.attach_ms(200, F_UI);
}

void ledON() {
  digitalWrite(LED_BUILTIN, LED_ON);
}

void ledOFF() {
  digitalWrite(LED_BUILTIN, LED_OFF);
}


//void setup_UI() {
//
//
//}
