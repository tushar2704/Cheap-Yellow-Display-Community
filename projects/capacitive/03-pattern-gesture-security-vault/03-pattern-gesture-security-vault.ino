/*
 * ═══════════════════════════════════════════════════════════════════════════
 *  CYD 2.8" Capacitive (ESP32-2432S028C) - 03: Android 9-Dot Pattern Lock
 * ═══════════════════════════════════════════════════════════════════════════
 *  Author:       tushar2704 <tushar.inseec@gmail.com>
 *  Repository:   https://github.com/tushar2704/Cheap-Yellow-Display-Community
 *  License:      Dual Licensed (AGPLv3 / CC BY-NC-ND 4.0)
 *  Credits:      Inspired by @IamNotIntoTech & @Makerrdesigns
 *  Year:         2026
 * ═══════════════════════════════════════════════════════════════════════════
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
#define PIN_LDR   34

TFT_eSPI tft = TFT_eSPI();
CST816S touch(I2C_SDA_PIN, I2C_SCL_PIN, TOUCH_RST_PIN, TOUCH_INT_PIN);

// 3x3 Dot Grid Geometry
const int DOT_RADIUS = 8;
const int HIT_RADIUS = 26;
struct Dot { int x, y; bool active; };
Dot dots[9] = {
  {70, 70, false},  {160, 70, false},  {250, 70, false},
  {70, 130, false}, {160, 130, false}, {250, 130, false},
  {70, 190, false}, {160, 190, false}, {250, 190, false}
};

int pattern[9];
int patternLen = 0;
bool vaultUnlocked = false;
unsigned long resetAt = 0;

void setRgb(uint8_t r, uint8_t g, uint8_t b) {
  analogWrite(PIN_LED_R, 255 - r);
  analogWrite(PIN_LED_G, 255 - g);
  analogWrite(PIN_LED_B, 255 - b);
}

void playToneFeedback(int f, int ms) {
  tone(PIN_AUDIO, f, ms);
}

void drawPatternLock() {
  tft.fillRect(0, 0, 320, 240, 0x0821);
  
  // Header
  tft.fillRect(0, 0, 320, 34, 0x10A2);
  tft.setTextColor(TFT_WHITE, 0x10A2);
  tft.setTextSize(1);
  tft.setCursor(15, 12);
  tft.print("SECURITY TERMINAL");
  
  tft.setTextColor(TFT_YELLOW, 0x10A2);
  tft.setCursor(160, 12);
  tft.print("[ CONNECT PATTERN ]");
  tft.drawFastHLine(0, 34, 320, 0x2945);

  // Draw 9 Dots
  for (int i = 0; i < 9; i++) {
    tft.drawCircle(dots[i].x, dots[i].y, DOT_RADIUS + 6, 0x3186);
    tft.fillCircle(dots[i].x, dots[i].y, DOT_RADIUS, TFT_WHITE);
  }

  tft.setTextColor(0x9492, 0x0821);
  tft.setCursor(45, 222);
  tft.print("Correct: Z-Shape (0 -> 1 -> 2 -> 4 -> 6 -> 7 -> 8)");
}

void drawVaultUnlocked() {
  tft.fillRect(0, 0, 320, 240, 0x0841);
  
  tft.fillRoundRect(20, 20, 280, 200, 10, 0x18C3);
  tft.drawRoundRect(20, 20, 280, 200, 10, TFT_GREEN);

  tft.setTextColor(TFT_GREEN, 0x18C3);
  tft.setTextSize(2);
  tft.setCursor(40, 40);
  tft.print("VAULT UNLOCKED!");

  tft.setTextSize(1);
  tft.setTextColor(TFT_WHITE, 0x18C3);
  tft.setCursor(40, 75);
  tft.print("AES-256 Key: 0x9F4B...72A1");
  tft.setCursor(40, 95);
  tft.print("WiFi Secret: CYD_IoT_Admin_2026");
  tft.setCursor(40, 115);
  tft.print("Relay Pin: GPIO 27 -> ACTIVATED");
  tft.setCursor(40, 135);
  tft.print("Master Token: valid for 60s");

  // Lock Button
  tft.fillRoundRect(70, 170, 180, 35, 6, TFT_RED);
  tft.setTextColor(TFT_WHITE, TFT_RED);
  tft.setCursor(105, 182);
  tft.print("LOCK VAULT NOW");
}

void resetPattern() {
  for (int i = 0; i < 9; i++) dots[i].active = false;
  patternLen = 0;
  drawPatternLock();
  setRgb(0, 0, 120);
}

void verifyPattern() {
  // Correct pattern: Z-shape: 0, 1, 2, 4, 6, 7, 8
  int correct[7] = {0, 1, 2, 4, 6, 7, 8};
  bool match = (patternLen == 7);
  if (match) {
    for (int i = 0; i < 7; i++) {
      if (pattern[i] != correct[i]) { match = false; break; }
    }
  }

  if (match) {
    vaultUnlocked = true;
    setRgb(0, 255, 0);
    playToneFeedback(1800, 60);
    delay(70);
    playToneFeedback(2400, 100);
    drawVaultUnlocked();
  } else {
    setRgb(255, 0, 0);
    playToneFeedback(400, 200);
    tft.setTextColor(TFT_RED, 0x0821);
    tft.setCursor(90, 222);
    tft.print("PATTERN INCORRECT!  ");
    delay(1000);
    resetPattern();
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_LED_R, OUTPUT);
  pinMode(PIN_LED_G, OUTPUT);
  pinMode(PIN_LED_B, OUTPUT);
  pinMode(PIN_AUDIO, OUTPUT);
  setRgb(0, 0, 120);

  tft.init();
  tft.setRotation(1);
  touch.begin();

  drawPatternLock();
}

void loop() {
  if (touch.available()) {
    int tx = touch.data.x;
    int ty = touch.data.y;
    uint8_t gesture = touch.data.gestureID;
    int sx = ty;
    int sy = 240 - tx;

    if (vaultUnlocked) {
      // Tap Lock Vault Button
      if (sx >= 70 && sx <= 250 && sy >= 170 && sy <= 205) {
        vaultUnlocked = false;
        playToneFeedback(1200, 40);
        resetPattern();
      }
      return;
    }

    // Check hit on any of the 9 dots
    for (int i = 0; i < 9; i++) {
      int dist = sq(sx - dots[i].x) + sq(sy - dots[i].y);
      if (dist <= sq(HIT_RADIUS) && !dots[i].active) {
        dots[i].active = true;
        pattern[patternLen++] = i;
        
        // Visual green highlight
        tft.fillCircle(dots[i].x, dots[i].y, DOT_RADIUS + 3, TFT_GREEN);
        
        // Connect to previous dot with a line
        if (patternLen > 1) {
          int prev = pattern[patternLen - 2];
          tft.drawLine(dots[prev].x, dots[prev].y, dots[i].x, dots[i].y, TFT_GREEN);
          tft.drawLine(dots[prev].x + 1, dots[prev].y, dots[i].x + 1, dots[i].y, TFT_GREEN);
        }

        playToneFeedback(1600 + patternLen * 150, 25);
        setRgb(0, 100, 255);
        break;
      }
    }

    // Touch release / Gesture complete
    if (gesture == 0x05 || patternLen >= 7) {
      if (patternLen > 0) {
        verifyPattern();
      }
    }
  }
}
