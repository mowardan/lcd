#pragma once
#include <Arduino.h>
#include <Wire.h>
#include <Print.h>
#include <stdarg.h>

// 1602A / HD44780 over I2C (PCF8574 backpack).
// Common pinout mapping on PCF8574:
// P0=RS, P1=RW, P2=EN, P3=BACKLIGHT, P4=D4, P5=D5, P6=D6, P7=D7
class LCD1602_I2C : public Print {
public:
  explicit LCD1602_I2C(uint8_t i2c_addr = 0x27, TwoWire& wire = Wire);

  // Setup & Connection
  void begin(uint8_t cols = 16, uint8_t rows = 2);
  bool beginAuto(uint8_t cols = 16, uint8_t rows = 2); // auto-probes 0x27, 0x3F, etc.
  bool isConnected();                                   // checks if LCD responds on I2C bus

  // Screen control
  void clear();             // clears entire display (~2ms)
  void clearLine(uint8_t row); // clears a single line and resets cursor to start of line
  void home();              // returns cursor to (0, 0) (~2ms)
  void setCursor(uint8_t col, uint8_t row);

  // Print API (overriding Print base class)
  virtual size_t write(uint8_t ch) override;
  virtual size_t write(const uint8_t* buffer, size_t size) override {
    size_t n = 0; while (size--) n += write(*buffer++); return n;
  }
  using Print::write; // bring in other Print::write overloads

  // Formatted printing
  size_t printf(const char* format, ...) __attribute__((format(printf, 2, 3)));

  // Display flags
  void display(bool on);       // display on/off (content preserved in DDRAM)
  void noDisplay() { display(false); }
  void cursor(bool on);        // underline cursor on/off
  void noCursor() { cursor(false); }
  void blink(bool on);         // blinking block cursor on/off
  void noBlink() { blink(false); }

  // Shifts & Text Flow
  void scrollDisplayLeft();
  void scrollDisplayRight();
  void leftToRight();          // text flows left-to-right (default)
  void rightToLeft();          // text flows right-to-left
  void autoscroll(bool on);    // right-justify text from cursor
  void noAutoscroll() { autoscroll(false); }

  // Custom glyphs (5x8), index 0..7
  void createChar(uint8_t index, const uint8_t bitmap[8]);

  // Backlight LED
  void backlight(bool on);
  void noBacklight() { backlight(false); }
  void toggleBacklight();
  bool isBacklightOn() const { return (_blState & MASK_BL) != 0; }

  // Visual Helpers: Progress Bar
  void initProgressBar();      // loads progress bar glyphs into CGRAM (indexes 0..4)
  void drawProgressBar(uint8_t row, uint8_t percent, uint8_t startCol = 0, uint8_t width = 16);

  // Visual Helpers: Big Digits (spans 2 rows x 3 cols per digit)
  void initBigDigits();        // loads big digit glyph segments into CGRAM (indexes 0..7)
  void drawBigDigit(uint8_t digit, uint8_t col, uint8_t startRow = 0);
  void drawBigNumber(int num, uint8_t col = 0, uint8_t minDigits = 1, uint8_t startRow = 0);
  void drawBigColon(uint8_t col, uint8_t startRow = 0);

  // Marquee & Rolling Logger
  void marquee(const String& text, uint8_t row = 0, uint16_t delayMs = 250);
  void log(const String& line);

  // Dimensions & Address
  uint8_t cols() const { return _cols; }
  uint8_t rows() const { return _rows; }
  uint8_t address() const { return _addr; }
  void setAddress(uint8_t addr) { _addr = addr; }

private:
  // Command/data low-level
  void command(uint8_t value);
  void send(uint8_t value, bool isData);
  void write4bits(uint8_t nibble);
  void pulseEnable(uint8_t out);
  void writeI2C(uint8_t out);

  // Helpers
  uint8_t rowAddress(uint8_t row) const;
  void    applyDisplayCtl();
  void    applyEntryMode();

  // Bus/config
  TwoWire* _wire;
  uint8_t  _addr;
  uint8_t  _cols, _rows;

  // Bit masks for PCF8574 mapping
  static constexpr uint8_t MASK_RS = 0x01; // P0
  static constexpr uint8_t MASK_RW = 0x02; // P1 (we keep RW=0)
  static constexpr uint8_t MASK_EN = 0x04; // P2
  static constexpr uint8_t MASK_BL = 0x08; // P3
  // P4..P7 are D4..D7 (placed in high nibble)

  // State
  uint8_t _blState    = MASK_BL; // backlight on by default
  uint8_t _dispCtl    = 0x04;    // D=1, C=0, B=0
  uint8_t _entryMode  = 0x02;    // I/D=1 (increment), S=0 (no shift)
  bool    _autoScroll = false;

  // Tiny 4-line ring buffer for logger
  String  _buf[4]     = {"", "", "", ""};
};
