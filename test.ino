#include <Wire.h>
#include "lcd.h"

// Initialize with standard 0x27 address (or use lcd.beginAuto(16, 2))
LCD1602_I2C lcd(0x27);

// Custom degree symbol glyph (5x8)
const uint8_t DEGREE5x8[8] = {
  0b00110,
  0b01001,
  0b01001,
  0b00110,
  0b00000,
  0b00000,
  0b00000,
  0b00000
};

// Custom heart glyph (5x8)
const uint8_t HEART5x8[8] = {
  0b00000,
  0b01010,
  0b11111,
  0b11111,
  0b01110,
  0b00100,
  0b00000,
  0b00000
};

void setup() {
  Serial.begin(115200);

  // Auto-probe I2C address if 0x27 is not responding
  if (!lcd.beginAuto(16, 2)) {
    lcd.begin(16, 2);
  }
  lcd.backlight(true);

  // 1. Welcome Screen with printf
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("LCD1602 I2C Pro");
  lcd.setCursor(0, 1);
  lcd.printf("Addr: 0x%02X  v1.2", lcd.address());
  delay(2500);

  // 2. Custom Glyphs Demo
  lcd.createChar(0, DEGREE5x8);
  lcd.createChar(1, HEART5x8);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.printf("Temp: 24.5");
  lcd.write((uint8_t)0); // Degree symbol
  lcd.print("C");
  lcd.setCursor(0, 1);
  lcd.print("Made with ");
  lcd.write((uint8_t)1); // Heart symbol
  delay(2500);

  // 3. Smooth Progress Bar Demo
  lcd.initProgressBar();
  lcd.clear();
  for (int p = 0; p <= 100; p += 4) {
    lcd.setCursor(0, 0);
    lcd.printf("Charging: %3d%%", p);
    lcd.drawProgressBar(1, p, 0, 16);
    delay(50);
  }
  delay(1500);

  // 4. Big Digits Counter Demo (spanning 2 lines)
  lcd.initBigDigits();
  lcd.clear();
  for (int num = 0; num <= 25; ++num) {
    lcd.drawBigNumber(num, 4, 2);
    delay(150);
  }
  delay(1500);

  // 5. Rolling Logger Demo
  lcd.clear();
  lcd.log("System init OK");
  delay(800);
  lcd.log("WiFi connected");
  delay(800);
  lcd.log("IP: 192.168.1.5");
  delay(800);
  lcd.log("All systems GO!");
  delay(2000);

  // 6. Marquee Ticker Demo
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Marquee Demo:");
  lcd.marquee("Welcome to Arduino LCD1602 with smooth scrolling!", 1, 150);
  delay(1000);
}

void loop() {
  // Live Dashboard demo in loop
  static int counter = 0;
  counter++;

  lcd.setCursor(0, 0);
  lcd.printf("Live Uptime: %ds ", counter);

  lcd.setCursor(0, 1);
  lcd.printf("FreeRAM: %4u B", (unsigned int)(1024 + (counter % 512)));

  delay(1000);
}
