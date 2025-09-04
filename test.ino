#include <Wire.h>
#include "lcd.h"   // <— renamed header

// Change 0x27 -> 0x3F if your backpack uses 0x3F
LCD1602_I2C lcd(0x27);

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

void setup() {
  // For ESP32/ESP8266 with custom I2C pins, do: Wire.begin(SDA, SCL) before lcd.begin()
  lcd.begin(16, 2);
  lcd.backlight(true);

  lcd.clear();
  lcd.setCursor(0,0); lcd.print("Hello, 1602A!");
  lcd.setCursor(0,1); lcd.print("It just works :)");
  delay(2000);

  lcd.cursor(true);
  lcd.blink(true);
  lcd.setCursor(15,1);
  delay(1200);
  lcd.cursor(false);
  lcd.blink(false);

  lcd.createChar(0, DEGREE5x8);
  lcd.clear();
  lcd.setCursor(0,0); lcd.print("Temp: 25"); lcd.write((uint8_t)0); lcd.print("C");
  lcd.setCursor(0,1); lcd.print("Status: OK");
  delay(2000);

  lcd.clear();
  lcd.log("System boot");
  delay(1000);
  lcd.log("WiFi connected");
  delay(1000);
  lcd.log("IP: 192.168.1.5");
  delay(2000);

  lcd.clear();
  lcd.setCursor(0,0); lcd.print("Scrolling demo...");
  for (int i=0; i<16; ++i) { lcd.scrollDisplayLeft(); delay(300); }
}

void loop() {
  delay(1000);
}
