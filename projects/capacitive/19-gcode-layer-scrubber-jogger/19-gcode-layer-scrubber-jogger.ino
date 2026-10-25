/*
 * CYD 2.8" Capacitive (ESP32-2432S028C) - 19: G-Code Layer Scrubber
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

int activeLayer = 85;
const int TOTAL_LAYERS = 240;

void drawPendant() {
  tft.fillRect(0, 0, 320, 240, 0x0821);

  // Header
  tft.fillRect(0, 0, 320, 24, 0x10A2);
  tft.setTextColor(TFT_WHITE, 0x10A2);
  tft.setTextSize(1);
  tft.setCursor(10, 8);
  tft.printf("3D PENDANT | LAYER %d/%d", activeLayer, TOTAL_LAYERS);

  // Left G-Code Toolpath Canvas Preview
  tft.fillRoundRect(15, 32, 160, 160, 6, 0x18C3);
  tft.drawRoundRect(15, 32, 160, 160, 6, TFT_CYAN);

  // Draw simulated layer toolpaths
  int radius = map(activeLayer, 1, TOTAL_LAYERS, 10, 60);
  tft.drawCircle(95, 112, radius, TFT_GREEN);
  tft.drawRect(95 - radius / 2, 112 - radius / 2, radius, radius, TFT_YELLOW);

  // Layer Scrubber Slider
  tft.fillRoundRect(185, 32, 22, 160, 4, 0x2126);
  int scrubY = map(activeLayer, TOTAL_LAYERS, 1, 32, 178);
  tft.fillRoundRect(183, scrubY, 26, 12, 3, TFT_YELLOW);

  // Right Motion Jog D-Pad
  tft.fillRoundRect(220, 32, 40, 40, 6, 0x18C3); // Y+
  tft.fillRoundRect(220, 120, 40, 40, 6, 0x18C3); // Y-
  tft.fillRoundRect(265, 76, 40, 40, 6, 0x18C3); // X+
  tft.fillRoundRect(220 - 45, 76, 40, 40, 6, 0x18C3); // X- (Wait, keep in bounds)

  tft.setTextColor(TFT_WHITE, 0x18C3);
  tft.setCursor(235, 48); tft.print("Y+");
  tft.setCursor(235, 136); tft.print("Y-");
  tft.setCursor(280, 92); tft.print("X+");

  // Bottom Status
  tft.fillRect(0, 204, 320, 36, 0x0000);
  tft.setTextColor(0x9492, 0x0000);
  tft.setCursor(15, 216);
  tft.print("Scrub vertical slider to inspect toolpath layer slices");
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_AUDIO, OUTPUT);
  tft.init();
  tft.setRotation(1);
  touch.begin();
  drawPendant();
}

void loop() {
  if (touch.available()) {
    int sx = touch.data.y;
    int sy = 240 - touch.data.x;

    // Layer Scrubber hit
    if (sx >= 175 && sx <= 215 && sy >= 32 && sy <= 192) {
      activeLayer = map(sy, 192, 32, 1, TOTAL_LAYERS);
      tone(PIN_AUDIO, 1800 + activeLayer * 5, 10);
      drawPendant();
    }
  }
}
