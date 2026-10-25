/*
 * CYD 2.8" Capacitive (ESP32-2432S028C) - 04: CoverFlow 3D Media Browser
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
#define PIN_LED_R 4
#define PIN_LED_G 16
#define PIN_LED_B 17
#define PIN_AUDIO 26

TFT_eSPI tft = TFT_eSPI();
CST816S touch(I2C_SDA_PIN, I2C_SCL_PIN, TOUCH_RST_PIN, TOUCH_INT_PIN);

struct Album { const char* title; const char* artist; uint16_t color; uint16_t accent; };
const int NUM_ALBUMS = 5;
Album albums[NUM_ALBUMS] = {
  {"Cyberpunk 2077", "Marcin Przybylowicz", 0xFD20, 0x07FF},
  {"Dark Side Moon", "Pink Floyd", 0x780F, 0x07E0},
  {"Discovery", "Daft Punk", 0x001F, 0xF81F},
  {"Random Access", "Daft Punk", 0xFFFF, 0xFBE0},
  {"Tron Legacy", "OST Collection", 0x07FF, 0x0010}
};

int activeIdx = 2;
bool showDetails = false;

void playClick() { tone(PIN_AUDIO, 2000, 15); }

void drawCoverFlow() {
  tft.fillRect(0, 0, 320, 240, 0x0000);
  
  // Header
  tft.fillRect(0, 0, 320, 24, 0x10A2);
  tft.setTextColor(TFT_WHITE, 0x10A2);
  tft.setTextSize(1);
  tft.setCursor(10, 8);
  tft.print("COVERFLOW 3D BROWSER");
  tft.setTextColor(TFT_CYAN, 0x10A2);
  tft.setCursor(200, 8);
  tft.print("< SWIPE L / R >");
  tft.drawFastHLine(0, 24, 320, 0x2126);

  // Left card (Perspective preview)
  int leftIdx = (activeIdx - 1 + NUM_ALBUMS) % NUM_ALBUMS;
  tft.fillRoundRect(20, 60, 50, 90, 4, 0x2126);
  tft.fillRoundRect(22, 62, 46, 86, 4, albums[leftIdx].color);

  // Right card (Perspective preview)
  int rightIdx = (activeIdx + 1) % NUM_ALBUMS;
  tft.fillRoundRect(250, 60, 50, 90, 4, 0x2126);
  tft.fillRoundRect(252, 62, 46, 86, 4, albums[rightIdx].color);

  // Center Active Card (Dominant)
  Album &cur = albums[activeIdx];
  tft.fillRoundRect(100, 45, 120, 120, 8, TFT_WHITE);
  tft.fillRoundRect(102, 47, 116, 116, 6, cur.color);
  tft.drawCircle(160, 105, 30, cur.accent);
  tft.fillCircle(160, 105, 10, 0x0000);

  // Simulated Mirror Reflection Below
  for (int y = 0; y < 25; y++) {
    uint16_t fadeCol = 0x1082;
    tft.drawFastHLine(105 + y, 168 + y, 110 - y * 2, fadeCol);
  }

  // Title & Artist
  tft.setTextColor(TFT_WHITE, 0x0000);
  tft.setTextSize(2);
  int tLen = strlen(cur.title) * 12;
  tft.setCursor(max(10, (320 - tLen) / 2), 195);
  tft.print(cur.title);

  tft.setTextSize(1);
  tft.setTextColor(0x9492, 0x0000);
  int aLen = strlen(cur.artist) * 6;
  tft.setCursor(max(10, (320 - aLen) / 2), 220);
  tft.print(cur.artist);
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_AUDIO, OUTPUT);
  tft.init();
  tft.setRotation(1);
  touch.begin();
  drawCoverFlow();
  playClick();
}

void loop() {
  if (touch.available()) {
    uint8_t g = touch.data.gestureID;
    if (g == 0x03) { // Swipe Left -> Next
      activeIdx = (activeIdx + 1) % NUM_ALBUMS;
      playClick();
      drawCoverFlow();
      delay(120);
    } else if (g == 0x04) { // Swipe Right -> Prev
      activeIdx = (activeIdx - 1 + NUM_ALBUMS) % NUM_ALBUMS;
      playClick();
      drawCoverFlow();
      delay(120);
    } else if (g == 0x05) { // Tap
      playClick();
    }
  }
}
