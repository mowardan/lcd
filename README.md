# LCD1602_I2C Library for Arduino

A lightweight, high-performance, and feature-packed library for **HD44780 / 1602A / 2004A LCD displays** using **PCF8574 I2C backpacks**.

---

## ✨ Features

- 🚀 **Full HD44780 & PCF8574 Support**: Fast 4-bit nibble transmission over I2C with minimal bus overhead.
- 📊 **Smooth Progress Bars**: High-resolution horizontal bar rendering using custom 5-level partial character glyphs.
- 🔢 **2-Line Big Digits**: Render large 3x2 font digits (0–9) and colons spanning 2 rows.
- 📝 **`printf` Support**: Formatted printing directly to LCD (e.g. `lcd.printf("Temp: %.1f C", temp)`).
- 🔍 **I2C Auto-Detection (`beginAuto()`)**: Automatically detects display address (`0x27`, `0x3F`, etc.).
- 📜 **Smooth Marquee**: Horizontal ticker text for strings longer than screen width.
- 🪵 **Rolling Event Logger (`log()`)**: Multi-line terminal-like log buffer.
- 💡 **Backlight Control**: Turn backlight ON/OFF, query state, or toggle easily.
- 🔄 **Display & Cursor Controls**: Blink, underline cursor, display on/off, scroll left/right, and text flow direction.

---

## 🔌 Wiring & Pinout

Connect the 4 pins of the I2C backpack to your microcontroller:

| I2C Backpack Pin | Arduino Uno / Nano | Arduino Mega | ESP8266 (NodeMCU/D1) | ESP32 (Default) | Raspberry Pi Pico |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **GND** | GND | GND | GND | GND | GND |
| **VCC** | 5V | 5V | 5V / Vin | 5V / VIN | 5V (VBUS) |
| **SDA** | A4 | Pin 20 | D2 (GPIO4) | GPIO 21 | GP4 / Pin 6 |
| **SCL** | A5 | Pin 21 | D1 (GPIO5) | GPIO 22 | GP5 / Pin 7 |

> [!TIP]
> **Contrast Adjustment**: If the display lights up but shows no text or only solid blue/black rectangles, turn the small blue potentiometer on the back of the I2C backpack with a screwdriver to adjust contrast.

---

## 📦 Installation

1. Download or clone this repository into your Arduino `libraries` folder:
   ```bash
   cd ~/Documents/Arduino/libraries/
   git clone https://github.com/mowardan/lcd.git LCD1602_I2C
   ```
2. Restart the Arduino IDE.
3. Open any example from **File > Examples > LCD1602_I2C**.

---

## 🚀 Quick Start

```cpp
#include <Wire.h>
#include "lcd.h"

// Initialize with I2C address 0x27 (or 0x3F)
LCD1602_I2C lcd(0x27);

void setup() {
  // Initialize LCD (16 columns, 2 rows)
  lcd.begin(16, 2);
  lcd.backlight(true);

  lcd.setCursor(0, 0);
  lcd.print("Hello World!");

  // Formatted printing
  float voltage = 3.3;
  lcd.setCursor(0, 1);
  lcd.printf("Voltage: %.1fV", voltage);
}

void loop() {
}
```

---

## 📖 API Reference

### Setup & Control
- `lcd.begin(cols, rows)`: Initialize display dimensions (default: 16 cols, 2 rows).
- `lcd.beginAuto(cols, rows)`: Automatically scan I2C bus and initialize.
- `lcd.isConnected()`: Returns `true` if LCD acknowledges on I2C.
- `lcd.clear()`: Clears entire screen.
- `lcd.clearLine(row)`: Clears a single line and resets cursor to column 0.
- `lcd.home()`: Resets cursor position to `(0, 0)`.
- `lcd.setCursor(col, row)`: Sets cursor position (0-indexed).

### Printing & Output
- `lcd.print(...)`: Standard Arduino `Print` library support.
- `lcd.println(...)`: Print with newline.
- `lcd.printf(const char* format, ...)`: Standard `printf` string formatting.
- `lcd.write(uint8_t ch)`: Write raw byte or custom character index.

### Cursor & Display Modes
- `lcd.display(bool on)` / `lcd.noDisplay()`: Toggle LCD visibility without losing memory.
- `lcd.cursor(bool on)` / `lcd.noCursor()`: Toggle underline cursor.
- `lcd.blink(bool on)` / `lcd.noBlink()`: Toggle blinking block cursor.
- `lcd.scrollDisplayLeft()` / `lcd.scrollDisplayRight()`: Shift display content horizontally.
- `lcd.leftToRight()` / `lcd.rightToLeft()`: Set text writing direction.

### Backlight
- `lcd.backlight(bool on)` / `lcd.noBacklight()`: Turn backlight on or off.
- `lcd.toggleBacklight()`: Invert current backlight state.
- `lcd.isBacklightOn()`: Returns boolean status of backlight.

### Custom Characters & Visual Helpers
- `lcd.createChar(index, bitmap)`: Load custom 5x8 glyph into CGRAM (indexes 0–7).
- `lcd.initProgressBar()`: Preloads custom characters for smooth progress bars.
- `lcd.drawProgressBar(row, percent, startCol, width)`: Draws a smooth progress bar (0–100%).
- `lcd.initBigDigits()`: Preloads font segments for 2-line big numbers.
- `lcd.drawBigDigit(digit, col, startRow)`: Draws a single 3x2 big digit (0–9).
- `lcd.drawBigNumber(number, col, minDigits, startRow)`: Draws a multi-digit big number.
- `lcd.drawBigColon(col, startRow)`: Draws a clock colon (`:`).
- `lcd.marquee(text, row, delayMs)`: Smooth scrolling text banner.
- `lcd.log(message)`: Appends message to multi-line rolling terminal logger.

---

## 📂 Included Examples

- **`01_HelloWorld`**: Basic text display and `printf` demonstration.
- **`02_ProgressBar`**: Smooth animated 0–100% progress bar.
- **`03_BigDigitsClock`**: 2-line big digit digital clock display.
- **`04_I2CScanner`**: Automatic address discovery (`0x27` vs `0x3F`).
- **`05_RollingLogger`**: Multi-line terminal-style rolling log.
- **`test.ino`**: Comprehensive test suite cycling through all features.

---

## 📄 License
MIT License. Free for personal, academic, and commercial use.
