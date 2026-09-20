#include <Wire.h>
#include "lcd.h"

LCD1602_I2C lcd(0x27);

int seconds = 45;
int minutes = 12;

void setup() {
  lcd.begin(16, 2);
  lcd.backlight(true);

  // Initialize 2-line big digit custom characters
  lcd.initBigDigits();
}

void loop() {
  // Draw MM:SS using 2-row big numbers
  // Format: [M1][M2] : [S1][S2]
  // Positions: M1 at col 0, M2 at col 4, Colon at col 7, S1 at col 9, S2 at col 13
  lcd.drawBigDigit(minutes / 10, 0);
  lcd.drawBigDigit(minutes % 10, 4);

  lcd.drawBigColon(7);

  lcd.drawBigDigit(seconds / 10, 9);
  lcd.drawBigDigit(seconds % 10, 13);

  delay(1000);

  seconds++;
  if (seconds >= 60) {
    seconds = 0;
    minutes++;
    if (minutes >= 60) {
      minutes = 0;
    }
  }
}
