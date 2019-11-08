
//led builtin
#define LED_ON LOW
#define LED_OFF HIGH

void blink(int b) {
  for (int i = 0; i < b; i++) {
    digitalWrite(LED_BUILTIN, LED_ON);
    delay(200);
    digitalWrite(LED_BUILTIN, LED_OFF);
    delay(200);
  }
}

void setup_UI(){
  
}
