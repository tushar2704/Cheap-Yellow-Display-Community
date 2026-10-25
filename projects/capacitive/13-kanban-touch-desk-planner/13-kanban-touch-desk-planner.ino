/*
 * CYD 2.8" Capacitive (ESP32-2432S028C) - 13: Kanban Card Desk Planner
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

struct Card { const char* title; int col; uint16_t tagColor; };
Card cards[3] = {
  {"PCB Routing", 0, TFT_RED},
  {"Firmware SPI", 1, TFT_YELLOW},
  {"3D Housing", 2, TFT_GREEN}
};

const char* colNames[3] = {"TO DO", "IN PROGRESS", "DONE"};

void drawKanban() {
  tft.fillRect(0, 0, 320, 240, 0x0821);
  
  // Header
  tft.fillRect(0, 0, 320, 26, 0x10A2);
  tft.setTextColor(TFT_WHITE, 0x10A2);
  tft.setTextSize(1);
  tft.setCursor(10, 8);
  tft.print("KANBAN DESK PLANNER");
  tft.setTextColor(TFT_CYAN, 0x10A2);
  tft.setCursor(185, 8);
  tft.print("[ TAP CARD TO MOVE ]");

  // 3 Columns
  for (int c = 0; c < 3; c++) {
    int cx = 10 + c * 102;
    tft.fillRoundRect(cx, 34, 96, 170, 6, 0x18C3);
    tft.drawRoundRect(cx, 34, 96, 170, 6, 0x2945);
    
    tft.setTextColor(TFT_WHITE, 0x18C3);
    tft.setTextSize(1);
    tft.setCursor(cx + 8, 42);
    tft.print(colNames[c]);
  }

  // Cards
  for (int i = 0; i < 3; i++) {
    int cx = 16 + cards[i].col * 102;
    int cy = 65 + (i % 2) * 60;
    tft.fillRoundRect(cx, cy, 84, 50, 4, 0x2126);
    tft.drawRoundRect(cx, cy, 84, 50, 4, cards[i].tagColor);
    
    tft.setTextColor(TFT_WHITE, 0x2126);
    tft.setTextSize(1);
    tft.setCursor(cx + 6, cy + 12);
    tft.print(cards[i].title);
    
    tft.fillRect(cx + 6, cy + 34, 16, 4, cards[i].tagColor);
  }

  // Bottom Status
  tft.fillRect(0, 212, 320, 28, 0x0000);
  tft.setTextColor(0x9492, 0x0000);
  tft.setCursor(20, 222);
  tft.print("Tap any card to advance it to the next column");
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_AUDIO, OUTPUT);
  tft.init();
  tft.setRotation(1);
  touch.begin();
  drawKanban();
}

void loop() {
  if (touch.available()) {
    int sx = touch.data.y;
    int sy = 240 - touch.data.x;

    for (int i = 0; i < 3; i++) {
      int cx = 16 + cards[i].col * 102;
      int cy = 65 + (i % 2) * 60;
      if (sx >= cx && sx <= cx + 84 && sy >= cy && sy <= cy + 50) {
        cards[i].col = (cards[i].col + 1) % 3;
        tone(PIN_AUDIO, 1800, 25);
        drawKanban();
        delay(150);
        break;
      }
    }
  }
}
