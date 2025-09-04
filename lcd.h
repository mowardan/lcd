#pragma once
#include <Arduino.h>
#include <Wire.h>
#include <Print.h>

// 1602A / HD44780 over I2C (PCF8574 backpack).
// Common mapping (adjust if your backpack differs):
// P0=RS, P1=RW, P2=EN, P3=BACKLIGHT, P4=D4, P5=D5, P6=D6, P7=D7
class LCD1602_I2C : public Print {
public:
  explicit LCD1602_I2C(uint8_t i2c_addr = 0x27, TwoWire& wire = Wire);

  // Setup
  void begin(uint8_t cols = 16, uint8_t rows = 2);

  // Screen control
  void clear();             // ~2ms
  void home();              // ~2ms
  void setCursor(uint8_t col, uint8_t row);

  // Print API (from Print)
  virtual size_t write(uint8_t ch) override;
  virtual size_t write(const uint8_t* buffer, size_t size) override {
    size_t n=0; while (size--) n += write(*buffer++); return n;
  }
  using Print::write; // pull in other Print::write overloads

  // Display flags
  void display(bool on);    // display on/off (content kept)
  void cursor(bool on);     // underline cursor
  void blink(bool on);      // blinking block

  // Shifts
  void scrollDisplayLeft();
  void scrollDisplayRight();

  // Custom glyphs (5x8), index 0..7
  void createChar(uint8_t index, const uint8_t bitmap[8]);

  // Backlight LED
  void backlight(bool on);

  // Simple 2-line logger helper
  void autoscroll(bool on);
  void log(const String& line);

private:
  // Command/data low-level
  void command(uint8_t value);
  void send(uint8_t value, bool isData);
  void write4bits(uint8_t nibble);
  void pulseEnable(uint8_t out);
  void writeI2C(uint8_t out);

  // Helpers
  uint8_t rowAddress(uint8_t row) const;
  void    applyDisplayCtl(); // push _dispCtl shadow to LCD

  // Bus/config
  TwoWire* _wire;
  uint8_t  _addr;
  uint8_t  _cols, _rows;

  // Bit masks for PCF8574 mapping
  static constexpr uint8_t MASK_RS = 0x01; // P0
  static constexpr uint8_t MASK_RW = 0x02; // P1 (we keep RW=0)
  static constexpr uint8_t MASK_EN = 0x04; // P2
  static constexpr uint8_t MASK_BL = 0x08; // P3
  // P4..P7 are D4..D7, we place nibble in high 4 bits.

  // State
  uint8_t _blState = MASK_BL; // backlight on by default
  bool    _autoscroll = false;

  // Shadow of "display control" register bits (D=0x04, C=0x02, B=0x01)
  // We always send 0x08 | _dispCtl (0x08 is base "display control" command)
  uint8_t _dispCtl = 0x04; // start with D=1, C=0, B=0

  // Tiny 2-line ring for logger
  String  _buf[2] = {"", ""};
};
