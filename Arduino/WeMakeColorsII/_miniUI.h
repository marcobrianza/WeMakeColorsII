boolean DEBUG_MINIUI = true;

#include <EEPROM.h> // built in ESP8266 Core
#define COUNT_ADDR 0

//led builtin
#define LED_ON LOW
#define LED_OFF HIGH

int blinkC = 0;
bool LED_STATE = LED_OFF;

bool blinkActive = false;
int blinkTime = 200;
int lastBlinkTime = 0;

#define BOOT_TEST_DEVICE 2
#define BOOT_RESET 3
#define BOOT_DEFAULT_AP 4
#define BOOT_ESPTOUCH 5


void ledON() {
  LED_STATE = LED_ON;
  digitalWrite(LED_BUILTIN, LED_STATE);
  //Serial.println("LED_STATE " + String(LED_STATE));
}

void ledOFF() {
  LED_STATE = LED_OFF;
  digitalWrite(LED_BUILTIN, LED_STATE);
  //Serial.println("LED_STATE " + String(LED_STATE));
}

void ledInvert() {
  LED_STATE = !LED_STATE;
  digitalWrite(LED_BUILTIN, LED_STATE);
  //Serial.println("LED_STATE " + String(LED_STATE));
}

void blink(int b) {

  blinkC = b * 2;
  ledON();

  blinkActive = true;
  lastBlinkTime = millis();
}


byte bootCount() {

  void ledOFF() ;
  delay(500);

  EEPROM.begin(512);
  byte boot_count = EEPROM.read(COUNT_ADDR);
  boot_count++;
  for (int i = 0;  i  < boot_count; i++) {
    ledON();
    delay(300);
    ledOFF();
    delay(300);
  }

  EEPROM.write(COUNT_ADDR, boot_count);
  EEPROM.commit();

  delay(2000);

  EEPROM.write(COUNT_ADDR, 0);
  EEPROM.commit();
  return boot_count;
}


void miniUI_loop() {

  if ((blinkActive) && ((millis() - lastBlinkTime) > blinkTime)) {
    lastBlinkTime = millis();
    blinkC--;
    if (blinkC == 0) {
      blinkActive = false;
    } else {
      ledInvert();
    }
  }

}

byte miniUI_bootCount() {
  byte c = bootCount();
  if (DEBUG_MINIUI)Serial.println("boot count=" + String(c));
  return c;
}
