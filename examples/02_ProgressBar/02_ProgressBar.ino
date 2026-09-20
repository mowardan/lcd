#include <Wire.h>
#include "lcd.h"

LCD1602_I2C lcd(0x27);

void setup() {
  lcd.begin(16, 2);
  lcd.backlight(true);

  // Initialize progress bar custom glyphs in CGRAM
  lcd.initProgressBar();

  lcd.setCursor(0, 0);
  lcd.print("Battery Charging");
}

void loop() {
  for (int p = 0; p <= 100; p += 2) {
    lcd.setCursor(0, 0);
    lcd.printf("Progress: %3d%%", p);

    // Draw a 16-character wide progress bar on row 1
    lcd.drawProgressBar(1, p, 0, 16);
    delay(60);
  }
  delay(1500);

  // Clear and restart
  lcd.clearLine(1);
  delay(500);
}
