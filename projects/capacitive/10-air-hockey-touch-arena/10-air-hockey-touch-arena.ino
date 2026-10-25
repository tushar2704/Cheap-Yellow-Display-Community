/*
 * CYD 2.8" Capacitive (ESP32-2432S028C) - 10: Air Hockey Touch Arena
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

float puckX = 160, puckY = 120;
float puckVx = 3.5, puckVy = 2.0;
float playerX = 60, playerY = 120;
float aiX = 260, aiY = 120;
int scorePlayer = 0, scoreAi = 0;
unsigned long lastTick = 0;

void resetPuck() {
  puckX = 160; puckY = 120;
  puckVx = (random(0, 2) ? 3.0 : -3.0);
  puckVy = (random(0, 2) ? 2.5 : -2.5);
  tone(PIN_AUDIO, 800, 100);
}

void drawRink() {
  tft.fillRect(0, 0, 320, 240, 0x03E0); // Ice Blue/Green Rink
  
  // Center Line & Circle
  tft.drawFastVLine(160, 0, 240, TFT_RED);
  tft.drawCircle(160, 120, 35, TFT_RED);
  tft.fillCircle(160, 120, 4, TFT_RED);

  // Goal Creases
  tft.drawRect(0, 70, 25, 100, TFT_BLUE);
  tft.drawRect(295, 70, 25, 100, TFT_BLUE);
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_AUDIO, OUTPUT);
  tft.init();
  tft.setRotation(1);
  touch.begin();
  drawRink();
  resetPuck();
}

void loop() {
  if (touch.available()) {
    int sx = touch.data.y;
    int sy = 240 - touch.data.x;
    // Player controls left paddle
    if (sx < 150) {
      playerX = sx;
      playerY = sy;
    }
  }

  unsigned long now = millis();
  if (now - lastTick >= 25) { // 40 FPS
    lastTick = now;

    // Erase Old Positions
    tft.fillCircle((int)puckX, (int)puckY, 8, 0x03E0);
    tft.fillCircle((int)playerX, (int)playerY, 14, 0x03E0);
    tft.fillCircle((int)aiX, (int)aiY, 14, 0x03E0);

    // AI Paddle Logic (Tracks puck with dampening)
    if (aiY < puckY - 5) aiY += 2.8;
    else if (aiY > puckY + 5) aiY -= 2.8;
    aiY = constrain(aiY, 20, 220);

    // Move Puck
    puckX += puckVx;
    puckY += puckVy;

    // Top/Bottom Wall Collisions
    if (puckY <= 10 || puckY >= 230) {
      puckVy = -puckVy;
      tone(PIN_AUDIO, 1200, 10);
    }

    // Player Paddle Collision
    if (sq(puckX - playerX) + sq(puckY - playerY) < sq(22)) {
      puckVx = fabs(puckVx) + 0.3; // Bounce forward
      puckVy += (puckY - playerY) * 0.15;
      tone(PIN_AUDIO, 1800, 15);
    }

    // AI Paddle Collision
    if (sq(puckX - aiX) + sq(puckY - aiY) < sq(22)) {
      puckVx = -fabs(puckVx) - 0.3; // Bounce back
      puckVy += (puckY - aiY) * 0.15;
      tone(PIN_AUDIO, 1500, 15);
    }

    puckVx = constrain(puckVx, -7.0, 7.0);
    puckVy = constrain(puckVy, -5.0, 5.0);

    // Goal Checks
    if (puckX <= 5) {
      if (puckY >= 70 && puckY <= 170) {
        scoreAi++;
        tone(PIN_AUDIO, 400, 200);
        resetPuck();
        drawRink();
      } else {
        puckVx = -puckVx;
      }
    } else if (puckX >= 315) {
      if (puckY >= 70 && puckY <= 170) {
        scorePlayer++;
        tone(PIN_AUDIO, 2400, 200);
        resetPuck();
        drawRink();
      } else {
        puckVx = -puckVx;
      }
    }

    // Redraw Elements
    tft.fillCircle((int)playerX, (int)playerY, 14, TFT_BLUE);
    tft.drawCircle((int)playerX, (int)playerY, 14, TFT_WHITE);

    tft.fillCircle((int)aiX, (int)aiY, 14, TFT_RED);
    tft.drawCircle((int)aiX, (int)aiY, 14, TFT_WHITE);

    tft.fillCircle((int)puckX, (int)puckY, 8, TFT_BLACK);
    tft.drawCircle((int)puckX, (int)puckY, 8, TFT_YELLOW);

    // Scoreboard in Center Top
    tft.fillRect(130, 2, 60, 16, 0x10A2);
    tft.setTextColor(TFT_WHITE, 0x10A2);
    tft.setTextSize(1);
    tft.setCursor(140, 6);
    tft.printf("%d - %d", scorePlayer, scoreAi);
  }
}
