//NTP
#include <TimeLib.h> // Time by Michael Margolis 1.6.1
#include <NtpClientLib.h> // NtpCliebtLib by German Martin 3.0.2-beta from https://github.com/marcobrianza/NtpClient

bool validNtpTime = false;

String getDateTimeStr() {
  String Summer = NTP.isSummerTime() ? " *" :  "";
  String fullTime = String(year()) + "/" + String(month()) + "/" + String(day()) + " " + NTP.getTimeStr().substring(0, 5) + Summer;
  return (fullTime);
}

void Setup_Time() {

  NTP.begin("time.ien.it", 1, true);
  NTP.setInterval(601);

  unsigned long t0 = millis();
  while (millis() - t0 < 5000) {
    Serial.print("NTP...");
    if (year() != 1970) {
      validNtpTime = true;
      Serial.println(millis()-t0);
      break;
    }
    delay(500);
  }

  ///delay(5000); //let some time to sync to ntp
}
