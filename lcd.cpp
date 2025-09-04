#include "lcd.h"

// HD44780 commands
static constexpr uint8_t CMD_CLEAR      = 0x01; // ~2ms
static constexpr uint8_t CMD_HOME       = 0x02; // ~2ms
static constexpr uint8_t CMD_ENTRYMODE  = 0x04; // +1: I/D, +2: S
static constexpr uint8_t CMD_DISPLAYCTL = 0x08; // +4: D, +2: C, +1: B
static constexpr uint8_t CMD_SHIFT      = 0x10; // +8: S/C, +4: R/L (1=R,0=L)
static constexpr uint8_t CMD_FUNCSET    = 0x20; // +16: DL, +8: N, +4: F
static constexpr uint8_t CMD_SETCGRAM   = 0x40;
static constexpr uint8_t CMD_SETDDRAM   = 0x80;

LCD1602_I2C::LCD1602_I2C(uint8_t i2c_addr, TwoWire& wire)
: _wire(&wire), _addr(i2c_addr), _cols(16), _rows(2) {}

void LCD1602_I2C::begin(uint8_t cols, uint8_t rows) {
  _cols = cols; _rows = rows;

  // If you need custom SDA/SCL (ESP32/ESP8266), call Wire.begin(SDA,SCL)
  // BEFORE this function
  _wire->begin();

  delay(50); // power-on settle (per datasheet)

  // Force 4-bit mode init (send 0x03 three times, then 0x02) as nibbles
  write4bits(0x03); delayMicroseconds(4500);
  write4bits(0x03); delayMicroseconds(4500);
  write4bits(0x03); delayMicroseconds(150);
  write4bits(0x02); // now in 4-bit mode

  // Function set: DL=0(4-bit), N=1(2 lines if available), F=0(5x8)
  command(CMD_FUNCSET | 0x08); // 0x28
  // Display off while configuring
  _dispCtl = 0x00; applyDisplayCtl(); // 0x08 | 0 -> display off
  clear(); // also homes; needs ~2ms
  // Entry mode: I/D=1 (cursor increment), S=0 (no shift)
  command(CMD_ENTRYMODE | 0x02); // 0x06
  // Display on, cursor/blink off
  _dispCtl = 0x04; applyDisplayCtl(); // D=1
}

void LCD1602_I2C::clear() {
  command(CMD_CLEAR);
  delayMicroseconds(2000);
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

void LCD1602_I2C::createChar(uint8_t index, const uint8_t bitmap[8]) {
  index &= 0x07; // 0..7
  command(CMD_SETCGRAM | (index << 3));
  for (uint8_t i = 0; i < 8; ++i) {
    // Only lower 5 bits used per row
    send(bitmap[i] & 0x1F, true);
  }
}

void LCD1602_I2C::backlight(bool on) {
  _blState = on ? MASK_BL : 0x00;
  // Touch bus so the state takes effect immediately
  writeI2C(_blState);
}

void LCD1602_I2C::autoscroll(bool on) { _autoscroll = on; }

void LCD1602_I2C::log(const String& line) {
  // Shift ring (newest at top row)
  _buf[1] = _buf[0];
  _buf[0] = line;

  // Render row 0
  setCursor(0,0);
  for (uint8_t i=0; i<_cols; ++i)
    write(i < _buf[0].length() ? (uint8_t)_buf[0][i] : (uint8_t)' ');

  // Render row 1 (if present)
  if (_rows > 1) {
    setCursor(0,1);
    for (uint8_t i=0; i<_cols; ++i)
      write(i < _buf[1].length() ? (uint8_t)_buf[1][i] : (uint8_t)' ');
  }

  // Optional horizontal marquee for long lines
  if (_autoscroll && _buf[0].length() > _cols) {
    for (uint8_t k = _cols; k < _buf[0].length(); ++k) {
      scrollDisplayLeft();
      write((uint8_t)_buf[0][k]);
      delay(80);
    }
  }
}

void LCD1602_I2C::command(uint8_t value) { send(value, false); }

void LCD1602_I2C::send(uint8_t value, bool isData) {
  // Nibble in high 4 bits (P4..P7)
  uint8_t rs = isData ? MASK_RS : 0x00;
  uint8_t high = (value & 0xF0) | _blState | rs;
  uint8_t low  = ((value << 4) & 0xF0) | _blState | rs;

  // RW=0 (write), we never set MASK_RW
  writeI2C(high);
  pulseEnable(high);
  writeI2C(low);
  pulseEnable(low);
}

void LCD1602_I2C::write4bits(uint8_t nibble) {
  uint8_t out = ((nibble & 0x0F) << 4) | _blState; // nibble -> D4..D7
  writeI2C(out);
  pulseEnable(out);
}

void LCD1602_I2C::pulseEnable(uint8_t out) {
  writeI2C(out | MASK_EN);
  delayMicroseconds(1);          // enable pulse width >= 450ns
  writeI2C(out & ~MASK_EN);
  delayMicroseconds(50);         // command settle time ~37µs
}

void LCD1602_I2C::writeI2C(uint8_t out) {
  _wire->beginTransmission(_addr);
  _wire->write(out);
  _wire->endTransmission();
}

uint8_t LCD1602_I2C::rowAddress(uint8_t row) const {
  // Handle 16x2 / 20x2 / 20x4 common mappings
  if (_rows == 4) {
    static const uint8_t base20x4[] = { 0x00, 0x40, 0x14, 0x54 };
    return base20x4[row % 4];
  } else {
    // 1x.. or 2x..: second line starts at 0x40
    static const uint8_t base2[] = { 0x00, 0x40, 0x00, 0x40 };
    return base2[row % 4];
  }
}

void LCD1602_I2C::applyDisplayCtl() {
  command(CMD_DISPLAYCTL | (_dispCtl & 0x07));
}
