# 👆 Capacitive Touch Compatibility Pack for Cheap Yellow Display

> Complete I2C touch controller integration for all capacitive touch CYD variants (`ESP32-2432S028C`, `ESP32-3248S035C`, `ESP32-4048S040`, `ESP32-8048S050`, and `ESP32-8048S070`).

---

## 🔍 Resistive vs Capacitive CYD Overview

| Feature | Resistive (`S028R`, `S035R`) | Capacitive (`S028C`, `S035C`) |
|---|---|---|
| **Touch Surface** | Flexible PET film (requires pressure/stylus) | Scratch-resistant optical glass (smooth finger touch) |
| **Touch Controller** | XPT2046 | CST816T / FT6236 / GT911 |
| **Communication Bus** | SPI (shares MISO/MOSI/SCK with TFT) | I2C (dedicated SDA: 33, SCL: 32) |
| **Calibration** | Requires 4-point coordinate mapping | Factory calibrated (true absolute pixels) |

---

## ⚡ How to Use in Your Sketches

Include [`CYD_Capacitive_Touch.h`](CYD_Capacitive_Touch.h):

```cpp
#include "CYD_Capacitive_Touch.h"

CYD_CapacitiveTouch touch(33, 32, 25, 21); // SDA, SCL, RST, IRQ

void setup() {
  touch.begin(320, 240, 1); // 320x240 landscape
}

void loop() {
  uint16_t x, y;
  if (touch.getTouch(&x, &y)) {
    Serial.printf("Capacitive Touch at: X=%d, Y=%d\n", x, y);
  }
}
```
