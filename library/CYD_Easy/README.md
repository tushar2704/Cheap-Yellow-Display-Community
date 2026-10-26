# 📦 CYD_Easy — The All-in-One Cheap Yellow Display Library

A unified, lightweight Arduino & PlatformIO library designed specifically for the **ESP32 Cheap Yellow Display (CYD)** family (`ESP32-2432S028R` Resistive and `ESP32-2432S028C` Capacitive).

---

## ⚡ Quickstart in 3 Lines

```cpp
#include <CYD_Easy.h>

void setup() {
    cyd.begin(1);                 // 1 line initializes display, touch, audio, RGB & backlight!
    cyd.setRgb(0, 255, 255);      // Active-LOW tri-color LED to Cyan
    cyd.beep(2000, 50);           // Audio chime on GPIO 26
    cyd.tft.println("Hello CYD!"); // Direct TFT_eSPI graphics access
}

void loop() {
    if (cyd.touched()) {
        CYD_Point p = cyd.getTouch(); // Auto-calibrated screen pixels (0-320, 0-240)
        cyd.tft.fillCircle(p.x, p.y, 4, TFT_YELLOW);
    }
}
```

---

## 🚀 Key Features

- **Zero-Friction Setup**: No need to fiddle with `User_Setup.h` or secondary SPI touch bus pins.
- **Dual Hardware Support**: Supports both **2.8" Resistive (XPT2046)** and **2.8" Capacitive (CST816S)** with `#define CYD_CAPACITIVE`.
- **Auto-Calibrated Touch**: `cyd.getTouch()` returns pixel coordinates mapped directly to the screen dimensions in any orientation.
- **PWM Backlight**: `cyd.setBacklight(percent)` (0-100%).
- **Active-LOW RGB LED**: `cyd.setRgb(r, g, b)` and `cyd.setLedHex(0xFF00FF)`.
- **Audio Feedback**: `cyd.beep(freq, duration)` and `cyd.playClick()`.
- **Light Sensor**: `cyd.readLdr()` (raw ADC) and `cyd.readLux()` (estimated lux).

---

## 📖 API Reference

### Display & Lifecycle
- `cyd.begin(uint8_t rotation = 1)`: Initializes all hardware. Rotation `0` = Portrait, `1` = Landscape (Default), `2` = Inverted Portrait, `3` = Inverted Landscape.
- `cyd.setBacklight(uint8_t percent)`: Sets screen brightness (0-100%).
- `cyd.tft`: Direct access to the `TFT_eSPI` graphics instance.

### Touch Interface
- `bool cyd.touched()`: Returns `true` if screen is currently pressed.
- `CYD_Point cyd.getTouch()`: Returns `{ x, y, z, gesture }` calibrated to display coordinates.

### Audio & Sound
- `cyd.beep(int freqHz = 2000, int durationMs = 30)`: Generates tone on onboard audio amp (GPIO 26).
- `cyd.playClick()`: Short tactile feedback click sound.

### RGB LED
- `cyd.setRgb(uint8_t r, uint8_t g, uint8_t b)`: Active-LOW PWM control (0-255 each).
- `cyd.setLedHex(uint32_t hexColor)`: Sets color by 24-bit hex (e.g. `0xFFD700`).
- `cyd.ledOff()`: Turns off RGB LED.

### Light Sensor (LDR)
- `int cyd.readLdr()`: Raw ADC reading from GPIO 34 (0-4095).
- `int cyd.readLux()`: Ambient light estimate in Lux.

---

## 📁 Examples
- **`01_QuickStart`**: Initialize display, audio, RGB, and touch button.
- **`02_TouchAndDraw`**: Full-screen drawing canvas with 6-color palette and clear function.
- **`03_RGB_Audio_LDR`**: Interactive hardware diagnostics testing all onboard peripherals.
