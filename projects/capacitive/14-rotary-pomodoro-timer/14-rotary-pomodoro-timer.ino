/*
 * CYD 2.8" Capacitive (ESP32-2432S028C) - 14: Rotary Gesture Pomodoro
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

int totalMinutes = 25;
int remainingSec = 25 * 60;
bool isRunning = false;
unsigned long lastSec = 0;

void drawTimer() {
  tft.fillRect(0, 0, 320, 240, 0x0821);

  // Outer Rotary Ring
  tft.drawCircle(160, 115, 80, 0x2126);
  tft.drawCircle(160, 115, 81, 0x2126);
  tft.drawCircle(160, 115, 82, TFT_RED);

  // Center Time Display
  tft.setTextColor(TFT_WHITE, 0x0821);
  tft.setTextSize(3);
  char buf[16];
  snprintf(buf, sizeof(buf), "%02d:%02d", remainingSec / 60, remainingSec % 60);
  tft.setCursor(115, 95);
  tft.print(buf);

  tft.setTextSize(1);
  tft.setTextColor(isRunning ? TFT_GREEN : TFT_YELLOW, 0x0821);
  tft.setCursor(130, 130);
  tft.print(isRunning ? "[ RUNNING ]" : "[ PAUSED ]");

  // Bottom Buttons: -5m, Start/Pause, +5m
  tft.fillRoundRect(30, 195, 75, 34, 6, 0x18C3);
  tft.setTextColor(TFT_WHITE, 0x18C3);
  tft.setCursor(55, 206);
  tft.print("-5m");

  tft.fillRoundRect(120, 195, 80, 34, 6, isRunning ? TFT_RED : TFT_GREEN);
  tft.setTextColor(0x0000, isRunning ? TFT_RED : TFT_GREEN);
  tft.setCursor(140, 206);
  tft.print(isRunning ? "PAUSE" : "START");

  tft.fillRoundRect(215, 195, 75, 34, 6, 0x18C3);
  tft.setTextColor(TFT_WHITE, 0x18C3);
  tft.setCursor(240, 206);
  tft.print("+5m");
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_AUDIO, OUTPUT);
  tft.init();
  tft.setRotation(1);
  touch.begin();
  drawTimer();
}

void loop() {
  if (touch.available()) {
    int sx = touch.data.y;
    int sy = 240 - touch.data.x;

    if (sy >= 195) {
      if (sx >= 30 && sx <= 105) { // -5m
        totalMinutes = max(5, totalMinutes - 5);
        remainingSec = totalMinutes * 60;
        tone(PIN_AUDIO, 1600, 20);
        drawTimer();
      } else if (sx >= 120 && sx <= 200) { // Start/Pause
        isRunning = !isRunning;
        tone(PIN_AUDIO, 2200, 25);
        drawTimer();
      } else if (sx >= 215 && sx <= 290) { // +5m
        totalMinutes = min(60, totalMinutes + 5);
        remainingSec = totalMinutes * 60;
        tone(PIN_AUDIO, 2000, 20);
        drawTimer();
      }
    }
  }

  if (isRunning && millis() - lastSec >= 1000) {
    lastSec = millis();
    if (remainingSec > 0) {
      remainingSec--;
      drawTimer();
      if (remainingSec == 0) {
        isRunning = false;
        tone(PIN_AUDIO, 2400, 500);
      }
    }
  }
}
