#include <Wire.h>
#include "lcd.h"

// Initialize with default or blank
LCD1602_I2C lcd;

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("Auto-detecting LCD I2C address...");

  // beginAuto will scan 0x27, 0x3F and other PCF8574 addresses
  if (lcd.beginAuto(16, 2)) {
    lcd.backlight(true);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("LCD Found!");
    lcd.setCursor(0, 1);
    lcd.printf("I2C Addr: 0x%02X", lcd.address());
    Serial.printf("LCD found at 0x%02X\n", lcd.address());
  } else {
    Serial.println("No LCD found on I2C bus. Check wiring & contrast.");
  }
}

void loop() {
  delay(1000);
}
