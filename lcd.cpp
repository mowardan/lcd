#include "lcd.h"

// HD44780 base commands
static constexpr uint8_t CMD_CLEAR      = 0x01; // ~2ms
static constexpr uint8_t CMD_HOME       = 0x02; // ~2ms
static constexpr uint8_t CMD_ENTRYMODE  = 0x04; // +2: I/D (1=inc,0=dec), +1: S (1=shift)
static constexpr uint8_t CMD_DISPLAYCTL = 0x08; // +4: D (display), +2: C (cursor), +1: B (blink)
static constexpr uint8_t CMD_SHIFT      = 0x10; // +8: S/C, +4: R/L (1=R, 0=L)
static constexpr uint8_t CMD_FUNCSET    = 0x20; // +16: DL, +8: N, +4: F
static constexpr uint8_t CMD_SETCGRAM   = 0x40; // +6-bit CGRAM addr
static constexpr uint8_t CMD_SETDDRAM   = 0x80; // +7-bit DDRAM addr

// Custom glyph bitmaps for Progress Bar (0..4 vertical sub-columns)
static const uint8_t PROG_BAR_GLYPHS[5][8] = {
  {0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10}, // 1 bar
  {0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18}, // 2 bars
  {0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C}, // 3 bars
  {0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E}, // 4 bars
  {0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F}  // 5 bars (full block)
};

// Custom glyph bitmaps for Big Digits (3x2 font)
// 0: Left block, 1: Right block, 2: Top bar, 3: Bottom bar,
// 4: Top+Bottom bar, 5: Full block, 6: Middle bar, 7: Top half block
static const uint8_t BIG_DIGIT_GLYPHS[8][8] = {
  {0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C}, // 0: Left bar
  {0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07}, // 1: Right bar
  {0x1F, 0x1F, 0x1F, 0x00, 0x00, 0x00, 0x00, 0x00}, // 2: Top bar
  {0x00, 0x00, 0x00, 0x00, 0x00, 0x1F, 0x1F, 0x1F}, // 3: Bottom bar
  {0x1F, 0x1F, 0x00, 0x00, 0x00, 0x00, 0x1F, 0x1F}, // 4: Top+Bottom bar
  {0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F}, // 5: Full block
  {0x00, 0x00, 0x1F, 0x1F, 0x1F, 0x00, 0x00, 0x00}, // 6: Middle bar
  {0x1F, 0x1F, 0x1F, 0x1F, 0x00, 0x00, 0x00, 0x00}  // 7: Top half block
};

// Big Digit pattern table [digit 0..9][row 0..1][col 0..2]
// Values: 0..7 are custom char indices, 32 = ' ' (space)
static const uint8_t BIG_FONT[10][2][3] = {
  { { 5, 2, 5 }, { 5, 3, 5 } }, // 0
  { { 2, 5, 32}, { 3, 5, 3 } }, // 1
  { { 2, 4, 5 }, { 5, 4, 3 } }, // 2
  { { 2, 4, 5 }, { 3, 4, 5 } }, // 3
  { { 5, 3, 5 }, { 32, 32, 5 } }, // 4
  { { 5, 4, 2 }, { 3, 4, 5 } }, // 5
  { { 5, 4, 2 }, { 5, 4, 5 } }, // 6
  { { 2, 2, 5 }, { 32, 32, 5 } }, // 7
  { { 5, 4, 5 }, { 5, 4, 5 } }, // 8
  { { 5, 4, 5 }, { 3, 4, 5 } }  // 9
};

LCD1602_I2C::LCD1602_I2C(uint8_t i2c_addr, TwoWire& wire)
: _wire(&wire), _addr(i2c_addr), _cols(16), _rows(2) {}

void LCD1602_I2C::begin(uint8_t cols, uint8_t rows) {
  _cols = cols;
  _rows = rows;

  _wire->begin();
  delay(50); // Power-on settle time per HD44780 datasheet (>40ms)

  // 4-bit initialization sequence
  write4bits(0x03); delayMicroseconds(4500);
  write4bits(0x03); delayMicroseconds(4500);
  write4bits(0x03); delayMicroseconds(150);
  write4bits(0x02); // switch to 4-bit mode

  // Function set: 4-bit mode, 2+ lines, 5x8 dots
  command(CMD_FUNCSET | 0x08); // 0x28

  // Display off during setup
  _dispCtl = 0x00;
  applyDisplayCtl();

  clear(); // Clear display and home

  // Entry mode: Left-to-Right increment, no display shift
  _entryMode = 0x02;
  applyEntryMode();

  // Display ON, cursor off, blink off
  _dispCtl = 0x04;
  applyDisplayCtl();
}

bool LCD1602_I2C::beginAuto(uint8_t cols, uint8_t rows) {
  _wire->begin();
  static const uint8_t commonAddrs[] = {
    0x27, 0x3F, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26,
    0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E
  };
  for (uint8_t a : commonAddrs) {
    _wire->beginTransmission(a);
    if (_wire->endTransmission() == 0) {
      _addr = a;
      begin(cols, rows);
      return true;
    }
  }
  return false;
}

bool LCD1602_I2C::isConnected() {
  _wire->beginTransmission(_addr);
  return (_wire->endTransmission() == 0);
}

void LCD1602_I2C::clear() {
  command(CMD_CLEAR);
  delayMicroseconds(2000);
}

void LCD1602_I2C::clearLine(uint8_t row) {
  if (row >= _rows) return;
  setCursor(0, row);
  for (uint8_t i = 0; i < _cols; ++i) {
    write((uint8_t)' ');
  }
  setCursor(0, row);
}

void LCD1602_I2C::home() {
  command(CMD_HOME);
  delayMicroseconds(2000);
}

void LCD1602_I2C::setCursor(uint8_t col, uint8_t row) {
  if (row >= _rows) row = _rows - 1;
  uint8_t addr = rowAddress(row) + (col % _cols);
  command(CMD_SETDDRAM | addr);
}

size_t LCD1602_I2C::write(uint8_t ch) {
  send(ch, true);
  return 1;
}

size_t LCD1602_I2C::printf(const char* format, ...) {
  char buf[64];
  va_list args;
  va_start(args, format);
  int len = vsnprintf(buf, sizeof(buf), format, args);
  va_end(args);
  if (len > 0) {
    size_t writeLen = (size_t)min((int)sizeof(buf) - 1, len);
    return write((const uint8_t*)buf, writeLen);
  }
  return 0;
}

void LCD1602_I2C::display(bool on) {
  if (on) _dispCtl |= 0x04; else _dispCtl &= ~0x04;
  applyDisplayCtl();
}

void LCD1602_I2C::cursor(bool on) {
  if (on) _dispCtl |= 0x02; else _dispCtl &= ~0x02;
  applyDisplayCtl();
}

void LCD1602_I2C::blink(bool on) {
  if (on) _dispCtl |= 0x01; else _dispCtl &= ~0x01;
  applyDisplayCtl();
}

void LCD1602_I2C::scrollDisplayLeft()  { command(CMD_SHIFT | 0x08 | 0x00); }
void LCD1602_I2C::scrollDisplayRight() { command(CMD_SHIFT | 0x08 | 0x04); }

void LCD1602_I2C::leftToRight() {
  _entryMode |= 0x02;
  applyEntryMode();
}

void LCD1602_I2C::rightToLeft() {
  _entryMode &= ~0x02;
  applyEntryMode();
}

void LCD1602_I2C::autoscroll(bool on) {
  if (on) _entryMode |= 0x01; else _entryMode &= ~0x01;
  _autoScroll = on;
  applyEntryMode();
}

void LCD1602_I2C::createChar(uint8_t index, const uint8_t bitmap[8]) {
  index &= 0x07; // 0..7
  command(CMD_SETCGRAM | (index << 3));
  for (uint8_t i = 0; i < 8; ++i) {
    send(bitmap[i] & 0x1F, true);
  }
}

void LCD1602_I2C::backlight(bool on) {
  _blState = on ? MASK_BL : 0x00;
  writeI2C(_blState);
}

void LCD1602_I2C::toggleBacklight() {
  backlight(!isBacklightOn());
}

// Progress Bar
void LCD1602_I2C::initProgressBar() {
  for (uint8_t i = 0; i < 5; ++i) {
    createChar(i, PROG_BAR_GLYPHS[i]);
  }
}

void LCD1602_I2C::drawProgressBar(uint8_t row, uint8_t percent, uint8_t startCol, uint8_t width) {
  if (percent > 100) percent = 100;
  if (startCol >= _cols) return;
  if (startCol + width > _cols) width = _cols - startCol;

  uint16_t totalPixels = width * 5;
  uint16_t filledPixels = ((uint32_t)percent * totalPixels + 50) / 100;

  setCursor(startCol, row);
  for (uint8_t i = 0; i < width; ++i) {
    uint16_t cellStart = i * 5;
    if (filledPixels >= cellStart + 5) {
      write((uint8_t)4); // full 5-bar block
    } else if (filledPixels > cellStart) {
      uint8_t sub = (filledPixels - cellStart) - 1; // 0..3
      write((uint8_t)sub);
    } else {
      write((uint8_t)' ');
    }
  }
}

// Big Digits
void LCD1602_I2C::initBigDigits() {
  for (uint8_t i = 0; i < 8; ++i) {
    createChar(i, BIG_DIGIT_GLYPHS[i]);
  }
}

void LCD1602_I2C::drawBigDigit(uint8_t digit, uint8_t col, uint8_t startRow) {
  if (digit > 9 || col + 3 > _cols || startRow + 1 >= _rows) return;
  for (uint8_t r = 0; r < 2; ++r) {
    setCursor(col, startRow + r);
    for (uint8_t c = 0; c < 3; ++c) {
      write(BIG_FONT[digit][r][c]);
    }
  }
}

void LCD1602_I2C::drawBigColon(uint8_t col, uint8_t startRow) {
  if (col >= _cols || startRow + 1 >= _rows) return;
  setCursor(col, startRow);
  write((uint8_t)'.');
  setCursor(col, startRow + 1);
  write((uint8_t)'.');
}

void LCD1602_I2C::drawBigNumber(int num, uint8_t col, uint8_t minDigits, uint8_t startRow) {
  char str[8];
  snprintf(str, sizeof(str), "%d", num);
  uint8_t len = strlen(str);

  // Pad with leading spaces or zeros if needed
  uint8_t curCol = col;
  while (len < minDigits && curCol + 3 <= _cols) {
    drawBigDigit(0, curCol, startRow);
    curCol += 4;
    minDigits--;
  }

  for (uint8_t i = 0; i < len; ++i) {
    if (str[i] >= '0' && str[i] <= '9') {
      if (curCol + 3 <= _cols) {
        drawBigDigit(str[i] - '0', curCol, startRow);
        curCol += 4; // 3 width + 1 spacing
      }
    }
  }
}

void LCD1602_I2C::marquee(const String& text, uint8_t row, uint16_t delayMs) {
  if (text.length() <= _cols) {
    setCursor(0, row);
    print(text);
    for (uint8_t i = text.length(); i < _cols; ++i) write((uint8_t)' ');
    return;
  }
  String padded = text + "    " + text;
  for (int i = 0; i <= (int)text.length() + 4; ++i) {
    setCursor(0, row);
    for (uint8_t c = 0; c < _cols; ++c) {
      write((uint8_t)padded[i + c]);
    }
    delay(delayMs);
  }
}

void LCD1602_I2C::log(const String& line) {
  // Shift buffer down
  for (int r = _rows - 1; r > 0; --r) {
    _buf[r] = _buf[r - 1];
  }
  _buf[0] = line;

  // Render all available rows
  for (uint8_t r = 0; r < _rows; ++r) {
    setCursor(0, r);
    for (uint8_t c = 0; c < _cols; ++c) {
      write(c < _buf[r].length() ? (uint8_t)_buf[r][c] : (uint8_t)' ');
    }
  }
}

void LCD1602_I2C::command(uint8_t value) {
  send(value, false);
}

void LCD1602_I2C::send(uint8_t value, bool isData) {
  uint8_t rs = isData ? MASK_RS : 0x00;
  uint8_t high = (value & 0xF0) | _blState | rs;
  uint8_t low  = ((value << 4) & 0xF0) | _blState | rs;

  writeI2C(high);
  pulseEnable(high);
  writeI2C(low);
  pulseEnable(low);
}

void LCD1602_I2C::write4bits(uint8_t nibble) {
  uint8_t out = ((nibble & 0x0F) << 4) | _blState;
  writeI2C(out);
  pulseEnable(out);
}

void LCD1602_I2C::pulseEnable(uint8_t out) {
  writeI2C(out | MASK_EN);
  delayMicroseconds(1);          // Enable pulse width >= 450ns
  writeI2C(out & ~MASK_EN);
  delayMicroseconds(50);         // Execution settle time ~37µs
}

void LCD1602_I2C::writeI2C(uint8_t out) {
  _wire->beginTransmission(_addr);
  _wire->write(out);
  _wire->endTransmission();
}

uint8_t LCD1602_I2C::rowAddress(uint8_t row) const {
  if (_rows == 4) {
    static const uint8_t base20x4[] = { 0x00, 0x40, 0x14, 0x54 };
    return base20x4[row % 4];
  } else {
    static const uint8_t base2[] = { 0x00, 0x40, 0x00, 0x40 };
    return base2[row % 4];
  }
}

void LCD1602_I2C::applyDisplayCtl() {
  command(CMD_DISPLAYCTL | (_dispCtl & 0x07));
}

void LCD1602_I2C::applyEntryMode() {
  command(CMD_ENTRYMODE | (_entryMode & 0x03));
}
