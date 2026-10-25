/*
 * CYD 2.8" Capacitive (ESP32-2432S028C) - 17: Multi-Channel Light Mixer
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

int faders[4] = {80, 50, 70, 30};
const char* faderNames[4] = {"WARM", "COLD", "AMBER", "RGB FX"};
uint16_t faderCols[4] = {0xFDA0, TFT_CYAN, 0xF940, TFT_MAGENTA};
bool blackout = false;

void drawMixer() {
  tft.fillRect(0, 0, 320, 240, 0x0821);

  // Header
  tft.fillRect(0, 0, 320, 24, 0x10A2);
  tft.setTextColor(TFT_WHITE, 0x10A2);
  tft.setTextSize(1);
  tft.setCursor(10, 8);
  tft.print("DMX 4-CHANNEL LIGHTING DESK");

  // Blackout Button
  tft.fillRoundRect(240, 4, 70, 18, 4, blackout ? TFT_RED : 0x2126);
  tft.setTextColor(TFT_WHITE, blackout ? TFT_RED : 0x2126);
  tft.setCursor(252, 9);
  tft.print(blackout ? "B.O. ON" : "B.O. OFF");

  // 4 Vertical Faders
  for (int i = 0; i < 4; i++) {
    int fx = 25 + i * 72;
    tft.fillRoundRect(fx, 34, 48, 160, 6, 0x18C3);
    
    // Fill level
    int val = blackout ? 0 : faders[i];
    int fillH = map(val, 0, 100, 0, 140);
    tft.fillRoundRect(fx + 6, 174 - fillH, 36, fillH + 6, 4, faderCols[i]);

    // Handle
    tft.fillRoundRect(fx + 2, 170 - fillH, 44, 8, 2, TFT_WHITE);

    // Label & Level %
    tft.setTextColor(TFT_WHITE, 0x0821);
    tft.setTextSize(1);
    tft.setCursor(fx + 4, 202);
    tft.print(faderNames[i]);
    tft.setCursor(fx + 12, 218);
    tft.printf("%d%%", val);
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_AUDIO, OUTPUT);
  tft.init();
  tft.setRotation(1);
  touch.begin();
  drawMixer();
}

void loop() {
  if (touch.available()) {
    int sx = touch.data.y;
    int sy = 240 - touch.data.x;

    if (sy < 24 && sx >= 240) {
      blackout = !blackout;
      tone(PIN_AUDIO, blackout ? 400 : 1800, 40);
      drawMixer();
      delay(100);
    } else if (sy >= 34 && sy <= 194) {
      int fIdx = (sx - 20) / 72;
      if (fIdx >= 0 && fIdx < 4) {
        faders[fIdx] = constrain(map(sy, 174, 34, 0, 100), 0, 100);
        tone(PIN_AUDIO, 1200 + faders[fIdx] * 10, 10);
        drawMixer();
      }
    }
  }
}
