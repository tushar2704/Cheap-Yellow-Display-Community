/*
 * CYD 2.8" Capacitive (ESP32-2432S028C) - 08: Swipe-to-Slice Arcade
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

struct Target { float x, y, vx, vy; bool active, isBomb, sliced; uint16_t col; };
Target target = {160, 240, 0, 0, false, false, false, TFT_RED};

int score = 0;
int lives = 3;
int lastBladeX = -1, lastBladeY = -1;
unsigned long lastFrame = 0;

void spawnTarget() {
  target.x = random(40, 280);
  target.y = 230;
  target.vx = random(-20, 20) / 10.0;
  target.vy = -random(80, 110) / 10.0;
  target.isBomb = (random(0, 4) == 0);
  target.col = target.isBomb ? 0x0000 : (random(0, 2) ? TFT_GREEN : TFT_ORANGE);
  target.active = true;
  target.sliced = false;
}

void drawGame() {
  tft.fillRect(0, 0, 320, 240, 0x0821);
  tft.setTextColor(TFT_WHITE, 0x0821);
  tft.setTextSize(1);
  tft.setCursor(10, 10);
  tft.printf("SCORE: %d", score);
  tft.setCursor(240, 10);
  tft.printf("LIVES: %d/3", lives);
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_AUDIO, OUTPUT);
  tft.init();
  tft.setRotation(1);
  touch.begin();
  drawGame();
  spawnTarget();
}

void loop() {
  unsigned long now = millis();
  if (now - lastFrame >= 33) { // ~30 FPS
    lastFrame = now;

    // Erase old target
    if (target.active) {
      tft.fillCircle((int)target.x, (int)target.y, 14, 0x0821);
    }

    // Physics Update
    target.x += target.vx;
    target.y += target.vy;
    target.vy += 0.35; // Gravity

    if (target.y > 240) {
      target.active = false;
      spawnTarget();
    }

    // Draw target
    if (target.active && !target.sliced) {
      tft.fillCircle((int)target.x, (int)target.y, 12, target.col);
      tft.drawCircle((int)target.x, (int)target.y, 12, TFT_WHITE);
    }

    // Draw HUD
    tft.fillRect(10, 10, 120, 12, 0x0821);
    tft.setTextColor(TFT_WHITE, 0x0821);
    tft.setTextSize(1);
    tft.setCursor(10, 10);
    tft.printf("SCORE: %d", score);
  }

  // Blade Swipe
  if (touch.available()) {
    int sx = touch.data.y;
    int sy = 240 - touch.data.x;

    if (lastBladeX > 0 && lastBladeY > 0) {
      tft.drawLine(lastBladeX, lastBladeY, sx, sy, TFT_CYAN);

      // Check collision with target
      if (target.active && !target.sliced) {
        float d = sq(sx - target.x) + sq(sy - target.y);
        if (d < 400) { // Sliced!
          target.sliced = true;
          if (target.isBomb) {
            lives--;
            tone(PIN_AUDIO, 150, 300);
            tft.fillScreen(TFT_RED);
            delay(100);
            drawGame();
            if (lives <= 0) { score = 0; lives = 3; }
          } else {
            score += 10;
            tone(PIN_AUDIO, 2200, 30);
          }
        }
      }
    }
    lastBladeX = sx; lastBladeY = sy;
  } else {
    lastBladeX = -1; lastBladeY = -1;
  }
}
