/*
 * CYD 2.8" Capacitive (ESP32-2432S028C) - 06: DJ Beat Launchpad & XY Filter
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

const char* padNames[8] = {"KICK", "SNARE", "HI-HAT", "CLAP", "808 BASS", "SYNTH", "CHOP", "RISER"};
int padFreqs[8] = {120, 450, 1200, 800, 65, 950, 1400, 2200};
uint16_t padColors[8] = {TFT_RED, TFT_ORANGE, TFT_YELLOW, TFT_GREEN, TFT_CYAN, TFT_BLUE, TFT_MAGENTA, TFT_PURPLE};

int filterCutoff = 50;
int filterRes = 50;

void drawLaunchpad() {
  tft.fillRect(0, 0, 320, 240, 0x0821);
  
  // Header
  tft.fillRect(0, 0, 320, 24, 0x10A2);
  tft.setTextColor(TFT_WHITE, 0x10A2);
  tft.setTextSize(1);
  tft.setCursor(10, 8);
  tft.print("DJ BEAT LAUNCHPAD & KAOSS XY");
  tft.drawFastHLine(0, 24, 320, 0x2126);

  // Left 8 Beat Pads (2 cols x 4 rows)
  for (int i = 0; i < 8; i++) {
    int col = i % 2;
    int row = i / 2;
    int px = 10 + col * 80;
    int py = 32 + row * 48;
    tft.fillRoundRect(px, py, 74, 42, 6, padColors[i]);
    tft.setTextColor(0x0000, padColors[i]);
    tft.setTextSize(1);
    tft.setCursor(px + 8, py + 16);
    tft.print(padNames[i]);
  }

  // Right XY Filter Pad
  tft.fillRoundRect(175, 32, 135, 185, 8, 0x18C3);
  tft.drawRoundRect(175, 32, 135, 185, 8, TFT_CYAN);
  
  // Crosshairs in XY Pad
  tft.drawFastHLine(175, 124, 135, 0x2945);
  tft.drawFastVLine(242, 32, 185, 0x2945);

  tft.setTextColor(TFT_CYAN, 0x18C3);
  tft.setTextSize(1);
  tft.setCursor(185, 42);
  tft.print("XY FILTER PAD");
  tft.setTextColor(0x9492, 0x18C3);
  tft.setCursor(185, 195);
  tft.print("X:Cutoff  Y:Res");

  // Bottom Status
  tft.fillRect(0, 222, 320, 18, 0x0000);
  tft.setTextColor(0x9492, 0x0000);
  tft.setCursor(10, 226);
  tft.printf("Cutoff: %d%% | Resonance: %d%%", filterCutoff, filterRes);
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_AUDIO, OUTPUT);
  tft.init();
  tft.setRotation(1);
  touch.begin();
  drawLaunchpad();
}

void loop() {
  if (touch.available()) {
    int sx = touch.data.y;
    int sy = 240 - touch.data.x;

    // Beat Pads (Left Area)
    if (sx >= 10 && sx <= 165 && sy >= 32 && sy <= 220) {
      int col = (sx - 10) / 80;
      int row = (sy - 32) / 48;
      int pad = constrain(row * 2 + col, 0, 7);
      tone(PIN_AUDIO, padFreqs[pad], 60);
    }
    // XY Filter Pad (Right Area)
    else if (sx >= 175 && sx <= 310 && sy >= 32 && sy <= 217) {
      filterCutoff = map(sx, 175, 310, 0, 100);
      filterRes = map(sy, 32, 217, 0, 100);
      int synthFreq = 200 + filterCutoff * 15 + filterRes * 8;
      tone(PIN_AUDIO, synthFreq, 30);

      tft.fillRect(10, 226, 250, 12, 0x0000);
      tft.setTextColor(TFT_CYAN, 0x0000);
      tft.setCursor(10, 226);
      tft.printf("Cutoff: %d%% | Res: %d%% (%dHz)", filterCutoff, filterRes, synthFreq);
    }
  }
}
