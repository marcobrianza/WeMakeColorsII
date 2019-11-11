
#include <EEPROM.h> // built in ESP8266 Core
#define COUNT_ADDR 0

//led builtin
#define LED_ON LOW
#define LED_OFF HIGH

#include <Ticker.h>
Ticker T_UI;

int blinkC = 0;
bool LED_STATE = false;


#define BOOT_TEST_DEVICE 2
#define BOOT_RESET 3
#define BOOT_DEFAULT_AP 4
#define BOOT_ESPTOUCH 5

void F_UI() {
  blinkC--;
  if (blinkC == 0) {
    T_UI.detach();
  } else {
    LED_STATE = !LED_STATE;
    digitalWrite(LED_BUILTIN, LED_STATE);
  }
}
void blink(int b) {

  blinkC = b * 2;
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


byte bootCount() {

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LED_OFF);
  delay(500);

  EEPROM.begin(512);
  byte boot_count = EEPROM.read(COUNT_ADDR);
  boot_count++;
  for (int i = 0;  i  < boot_count; i++) {
    digitalWrite(LED_BUILTIN, LED_ON);
    delay(300);
    digitalWrite(LED_BUILTIN, LED_OFF);
    delay(300);
  }

  EEPROM.write(COUNT_ADDR, boot_count);
  EEPROM.commit();

  delay(2000);

  EEPROM.write(COUNT_ADDR, 0);
  EEPROM.commit();
  return boot_count;
}


byte UI_setup() {
  byte c = bootCount();
  Serial.print("\nboot count=");
  Serial.println(c);
  return c;
}
