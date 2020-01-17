String softwareName = "WeMakeColorsII";
String softwareVersion = "1.9.18";
String softwareInfo = "";
String softwarePlatform = "";

String appId = "IPI";
String thingId = "";
String friendlyName = "";

#include <EEPROM.h> // built in ESP8266 Core
#define COUNT_ADDR 0

//led builtin
#define LED_ON LOW
#define LED_OFF HIGH

int blinkC = 0;
bool LED_STATE = false;

bool blinkActive = false;
int blinkTime = 200;
int lastBlinkTime = 0;

#define BOOT_TEST_DEVICE 2
#define BOOT_RESET 3
#define BOOT_DEFAULT_AP 4
#define BOOT_ESPTOUCH 5

#include <ESP8266WiFi.h>  // built in ESP8266 Core

void software_setup() {

  Serial.begin(115200);  Serial.println();
  softwareInfo = softwareName + " - " + softwareVersion +  " - " + ESP.getSketchMD5() + " - " + String (ESP.getCpuFreqMHz()); // + " - " + String (__DATE__) + " - " + String(__TIME__);;
  softwarePlatform = ESP.getFullVersion();
  Serial.println(softwareInfo);
  Serial.println(softwarePlatform);

  Serial.println("ResetReason=" + ESP.getResetReason());

  pinMode(LED_BUILTIN, OUTPUT);

  thingId = appId + "_" +  WiFi.macAddress().c_str();
  Serial.println("thingId: " + thingId);
  friendlyName = thingId;
}



void blink(int b) {

  blinkC = b * 2;
  LED_STATE = LED_ON;
  digitalWrite(LED_BUILTIN, LED_STATE);

  blinkActive = true;
  lastBlinkTime = millis();
}

void ledON() {
  digitalWrite(LED_BUILTIN, LED_ON);
  LED_STATE = LED_ON;
}

void ledOFF() {
  digitalWrite(LED_BUILTIN, LED_OFF);
  LED_STATE = LED_OFF;
}

void ledInvert() {
  LED_STATE = !LED_STATE;
  digitalWrite(LED_BUILTIN, LED_STATE);
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


void miniUI_loop() {

  if ((blinkActive) && ((millis() - lastBlinkTime) > blinkTime)) {
    lastBlinkTime = millis();
    blinkC--;
    if (blinkC == 0) {
      blinkActive = false;
    } else {
      LED_STATE = !LED_STATE;
      digitalWrite(LED_BUILTIN, LED_STATE);
    }
  }

}

byte miniUI_bootCount() {
  byte c = bootCount();
  Serial.print("\nboot count=");
  Serial.println(c);
  return c;
}
