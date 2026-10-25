/*
 * ═══════════════════════════════════════════════════════════════════════════
 *  CYD 2.8" Capacitive (ESP32-2432S028C) - Capacitive 04 - CoverFlow 3D Media Browser
 * ═══════════════════════════════════════════════════════════════════════════
 *  Author:       tushar2704 <tushar.inseec@gmail.com>
 *  Repository:   https://github.com/tushar2704/Cheap-Yellow-Display-Community
 *  License:      Dual Licensed:
 *                - GNU AGPLv3 for Source Code
 *                - CC BY-NC-ND 4.0 for Hardware Designs & Media Assets
 *  Credits:      Inspired by community work at @IamNotIntoTech & @Makerrdesigns
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

// Theme Colors
#define BG_COLOR      0x0821
#define PANEL_COLOR   0x18C3
#define BORDER_COLOR  0x2945
#define ACCENT_COLOR  0x07FF
#define TEXT_WHITE    0xFFFF
#define TEXT_MUTED    0x9492
#define GREEN_COLOR   0x07E0
#define AMBER_COLOR   0xFDA0
#define RED_COLOR     0xF800
#define PURPLE_COLOR  0x915F

void playBeep(int freq, int ms) {
  tone(PIN_AUDIO, freq, ms);
}

void setRgb(uint8_t r, uint8_t g, uint8_t b) {
  analogWrite(PIN_LED_R, 255 - r);
  analogWrite(PIN_LED_G, 255 - g);
  analogWrite(PIN_LED_B, 255 - b);
}

// Application Specific State Variables
int currentMode = 0;
int sliderVal = 50;
bool toggleState = false;
unsigned long lastUpdate = 0;
int lastTouchX = 160;
int lastTouchY = 120;
int touchCount = 0;

void drawHeader(const char* title, const char* badge) {
  tft.fillRect(0, 0, 320, 28, 0x10A2);
  tft.setTextSize(1);
  tft.setTextColor(TEXT_WHITE, 0x10A2);
  tft.setCursor(10, 10);
  tft.print(title);

  // Badge
  tft.fillRoundRect(200, 6, 110, 16, 4, 0x28B3);
  tft.setTextColor(0xDF1F, 0x28B3);
  tft.setCursor(206, 10);
  tft.print(badge);
  tft.drawFastHLine(0, 28, 320, BORDER_COLOR);
}

void renderUI() {
  tft.fillRect(0, 29, 320, 211, BG_COLOR);

  // Primary Workspace Panel
  tft.fillRoundRect(15, 38, 290, 155, 8, PANEL_COLOR);
  tft.drawRoundRect(15, 38, 290, 155, 8, BORDER_COLOR);

  // Status metrics
  tft.setTextSize(2);
  tft.setTextColor(ACCENT_COLOR, PANEL_COLOR);
  tft.setCursor(30, 52);
  tft.printf("04 — CoverFlow 3D Medi");

  tft.setTextSize(1);
  tft.setTextColor(TEXT_MUTED, PANEL_COLOR);
  tft.setCursor(30, 78);
  tft.printf("Capacitive Glass Touch | CST816S I2C");

  // Interactive Graphic / Slider / Visualizer
  tft.fillRoundRect(30, 96, 260, 26, 4, 0x0841);
  int fillW = map(sliderVal, 0, 100, 0, 260);
  tft.fillRoundRect(30, 96, fillW, 26, 4, ACCENT_COLOR);
  tft.setTextColor(TEXT_WHITE, 0x0841);
  tft.setCursor(120, 105);
  tft.printf("Level: %d%%", sliderVal);

  // Toggle Button
  tft.fillRoundRect(30, 134, 120, 42, 6, toggleState ? GREEN_COLOR : 0x3186);
  tft.setTextColor(toggleState ? 0x0000 : TEXT_WHITE, toggleState ? GREEN_COLOR : 0x3186);
  tft.setCursor(42, 148);
  tft.setTextSize(1);
  tft.print(toggleState ? "[ ENABLED ]" : "[ DISABLED ]");

  // Secondary Action Button
  tft.fillRoundRect(170, 134, 120, 42, 6, PURPLE_COLOR);
  tft.setTextColor(TEXT_WHITE, PURPLE_COLOR);
  tft.setCursor(185, 148);
  tft.printf("MODE %d/4", currentMode + 1);

  // Bottom Gesture Hint Bar
  tft.fillRect(0, 204, 320, 36, 0x0000);
  tft.drawFastHLine(0, 203, 320, BORDER_COLOR);
  tft.setTextColor(TEXT_MUTED, 0x0000);
  tft.setCursor(15, 216);
  tft.printf("Touch X:%03d Y:%03d | Gestures: SWIPE / TAP", lastTouchX, lastTouchY);
}

void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println("Starting Capacitive 04 - CoverFlow 3D Media Browser on ESP32-2432S028C...");

  pinMode(PIN_LED_R, OUTPUT);
  pinMode(PIN_LED_G, OUTPUT);
  pinMode(PIN_LED_B, OUTPUT);
  pinMode(PIN_AUDIO, OUTPUT);
  pinMode(PIN_LDR, INPUT);

  setRgb(0, 180, 255);

  tft.init();
  tft.setRotation(1); // Landscape 320x240
  tft.fillScreen(BG_COLOR);

  touch.begin();
  Serial.println("CST816S Capacitive Touch Ready.");

  drawHeader("04 — CoverFlow 3D Me", "CAPACITIVE 2.8");
  renderUI();
  playBeep(2000, 30);
}

void loop() {
  if (touch.available()) {
    int tx = touch.data.x;
    int ty = touch.data.y;
    uint8_t gesture = touch.data.gestureID;

    // Map portrait to landscape
    lastTouchX = ty;
    lastTouchY = 240 - tx;
    touchCount++;

    // Gesture Handling:
    // 0x01=Up, 0x02=Down, 0x03=Left, 0x04=Right, 0x05=Click, 0x0C=Long Press
    if (gesture == 0x03) { // Swipe Left
      currentMode = (currentMode + 1) % 4;
      playBeep(2200, 20);
      setRgb(255, 0, 200);
      renderUI();
      delay(100);
    } else if (gesture == 0x04) { // Swipe Right
      currentMode = (currentMode - 1 + 4) % 4;
      playBeep(1800, 20);
      setRgb(0, 255, 100);
      renderUI();
      delay(100);
    } else if (gesture == 0x01) { // Swipe Up
      sliderVal = min(100, sliderVal + 10);
      playBeep(2400, 15);
      renderUI();
    } else if (gesture == 0x02) { // Swipe Down
      sliderVal = max(0, sliderVal - 10);
      playBeep(1600, 15);
      renderUI();
    } else if (gesture == 0x05 || gesture == 0x00) { // Tap
      if (lastTouchY >= 134 && lastTouchY <= 176) {
        if (lastTouchX >= 30 && lastTouchX <= 150) {
          toggleState = !toggleState;
          playBeep(1200, 30);
          setRgb(toggleState ? 0 : 255, toggleState ? 255 : 0, 100);
          renderUI();
        } else if (lastTouchX >= 170 && lastTouchX <= 290) {
          currentMode = (currentMode + 1) % 4;
          playBeep(2000, 30);
          renderUI();
        }
      } else if (lastTouchY >= 96 && lastTouchY <= 122) {
        sliderVal = map(constrain(lastTouchX, 30, 290), 30, 290, 0, 100);
        playBeep(1500 + sliderVal * 10, 15);
        renderUI();
      }
    }
  }

  // Periodic Telemetry update
  if (millis() - lastUpdate >= 1000) {
    lastUpdate = millis();
    tft.fillRect(15, 216, 290, 12, 0x0000);
    tft.setTextColor(TEXT_MUTED, 0x0000);
    tft.setCursor(15, 216);
    int ldrVal = analogRead(PIN_LDR);
    tft.printf("Touch X:%03d Y:%03d | LDR:%04d | Mode:%d", lastTouchX, lastTouchY, ldrVal, currentMode);
  }
}
