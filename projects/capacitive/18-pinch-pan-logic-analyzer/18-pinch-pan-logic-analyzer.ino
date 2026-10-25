/*
 * CYD 2.8" Capacitive (ESP32-2432S028C) - 18: Logic Waveform Analyzer
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

const int SAMPLES = 500;
uint8_t ch1[SAMPLES], ch2[SAMPLES], ch3[SAMPLES], ch4[SAMPLES];
int panOffset = 50;

void generateSignals() {
  for (int i = 0; i < SAMPLES; i++) {
    ch1[i] = (i / 4) % 2; // CLK
    ch2[i] = ((i / 8) % 3 == 0); // MOSI
    ch3[i] = ((i / 16) % 2 == 1); // MISO
    ch4[i] = (i < 20 || i > 400); // CS (Active Low)
  }
}

void drawAnalyzer() {
  tft.fillRect(0, 0, 320, 240, 0x0000);

  // Header
  tft.fillRect(0, 0, 320, 22, 0x10A2);
  tft.setTextColor(TFT_WHITE, 0x10A2);
  tft.setTextSize(1);
  tft.setCursor(10, 7);
  tft.printf("SPI LOGIC ANALYZER | PAN:%03d", panOffset);

  const char* labels[4] = {"CH1: CLK", "CH2: MOSI", "CH3: MISO", "CH4: CS"};
  uint16_t cols[4] = {TFT_YELLOW, TFT_CYAN, TFT_GREEN, TFT_RED};

  for (int c = 0; c < 4; c++) {
    int baseLine = 55 + c * 42;
    tft.setTextColor(cols[c], 0x0000);
    tft.setCursor(10, baseLine - 18);
    tft.print(labels[c]);

    uint8_t* sig = (c == 0 ? ch1 : (c == 1 ? ch2 : (c == 2 ? ch3 : ch4)));
    for (int x = 0; x < 240; x++) {
      int sIdx = constrain(panOffset + x, 0, SAMPLES - 1);
      int y = sig[sIdx] ? (baseLine - 14) : baseLine;
      tft.drawPixel(75 + x, y, cols[c]);
      if (x > 0) {
        int prevIdx = constrain(panOffset + x - 1, 0, SAMPLES - 1);
        int prevY = sig[prevIdx] ? (baseLine - 14) : baseLine;
        if (prevY != y) tft.drawFastVLine(75 + x, min(y, prevY), abs(y - prevY), cols[c]);
      }
    }
  }

  // Footer
  tft.fillRect(0, 218, 320, 22, 0x10A2);
  tft.setTextColor(0x9492, 0x10A2);
  tft.setCursor(10, 224);
  tft.print("Drag horizontally to pan through captured sample timeline");
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_AUDIO, OUTPUT);
  tft.init();
  tft.setRotation(1);
  touch.begin();
  generateSignals();
  drawAnalyzer();
}

void loop() {
  if (touch.available()) {
    int sx = touch.data.y;
    panOffset = constrain(map(sx, 0, 320, 0, SAMPLES - 240), 0, SAMPLES - 240);
    tone(PIN_AUDIO, 2000, 10);
    drawAnalyzer();
  }
}
