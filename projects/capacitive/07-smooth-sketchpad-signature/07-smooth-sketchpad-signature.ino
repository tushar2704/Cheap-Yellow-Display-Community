/*
 * CYD 2.8" Capacitive (ESP32-2432S028C) - 07: Smooth Glass Sketchpad
 * Author: tushar2704 <tushar.inseec@gmail.com> | Dual Licensed (AGPLv3 / CC BY-NC-ND 4.0)
 */
#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <TFT_eSPI.h>
#include <CST816S.h>

#define I2C_SDA_PIN   33
#define I2C_SCL_PIN   32
#define TOUCH_RST_PIN 25
#define TOUCH_INT_PIN 21
#define PIN_AUDIO 26

TFT_eSPI tft = TFT_eSPI();
CST816S touch(I2C_SDA_PIN, I2C_SCL_PIN, TOUCH_RST_PIN, TOUCH_INT_PIN);

uint16_t colors[6] = {TFT_WHITE, TFT_CYAN, TFT_YELLOW, TFT_GREEN, TFT_RED, TFT_MAGENTA};
int activeColorIdx = 0;
int brushRadius = 2;
int lastX = -1, lastY = -1;

void drawCanvasUI() {
  tft.fillRect(0, 0, 320, 240, 0x0000);

  // Top Palette Bar
  tft.fillRect(0, 0, 320, 32, 0x10A2);
  for (int i = 0; i < 6; i++) {
    tft.fillCircle(20 + i * 36, 16, 10, colors[i]);
    if (i == activeColorIdx) tft.drawCircle(20 + i * 36, 16, 13, TFT_WHITE);
  }

  // Clear Button
  tft.fillRoundRect(240, 4, 70, 24, 4, TFT_RED);
  tft.setTextColor(TFT_WHITE, TFT_RED);
  tft.setTextSize(1);
  tft.setCursor(252, 11);
  tft.print("CLEAR");

  // Drawing Canvas Border
  tft.drawRect(0, 33, 320, 207, 0x2126);
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_AUDIO, OUTPUT);
  tft.init();
  tft.setRotation(1);
  touch.begin();
  drawCanvasUI();
}

void loop() {
  if (touch.available()) {
    int sx = touch.data.y;
    int sy = 240 - touch.data.x;
    uint8_t g = touch.data.gestureID;

    // Palette Bar Touch
    if (sy < 32) {
      if (sx >= 240) { // Clear
        drawCanvasUI();
        tone(PIN_AUDIO, 800, 40);
        delay(50);
      } else {
        int cIdx = (sx - 5) / 36;
        if (cIdx >= 0 && cIdx < 6) {
          activeColorIdx = cIdx;
          drawCanvasUI();
          tone(PIN_AUDIO, 1800, 20);
        }
      }
      lastX = -1; lastY = -1;
    }
    // Drawing Area
    else if (sy >= 34) {
      if (lastX > 0 && lastY > 0) {
        tft.drawLine(lastX, lastY, sx, sy, colors[activeColorIdx]);
        tft.drawLine(lastX + 1, lastY, sx + 1, sy, colors[activeColorIdx]);
      } else {
        tft.fillCircle(sx, sy, brushRadius, colors[activeColorIdx]);
      }
      lastX = sx; lastY = sy;
    }
  } else {
    lastX = -1; lastY = -1;
  }
}
