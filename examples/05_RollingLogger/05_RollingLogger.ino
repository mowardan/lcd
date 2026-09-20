#include <Wire.h>
#include "lcd.h"

LCD1602_I2C lcd(0x27);

int eventCount = 0;

void setup() {
  lcd.begin(16, 2);
  lcd.backlight(true);

  lcd.log("Logger Started");
  delay(1000);
}

void loop() {
  eventCount++;
  String msg = "Event #" + String(eventCount) + " OK";
  lcd.log(msg);
  delay(1500);
}
