#include <Wire.h>
#include "lcd.h"

// Initialize with default 0x27 address (or use lcd.beginAuto() in setup)
LCD1602_I2C lcd(0x27);

void setup() {
  // Initialize LCD with 16 columns and 2 rows
  lcd.begin(16, 2);
  lcd.backlight(true);

  // Print text using standard Print methods
  lcd.setCursor(0, 0);
  lcd.print("Hello, World!");

  // Formatted printing with printf!
  int count = 42;
  float voltage = 3.3;
  lcd.setCursor(0, 1);
  lcd.printf("Val:%d V:%.1fV", count, voltage);
}

void loop() {
  // Blink cursor demo
  lcd.setCursor(15, 1);
  lcd.blink(true);
  delay(1000);
  lcd.blink(false);
  delay(1000);
}
