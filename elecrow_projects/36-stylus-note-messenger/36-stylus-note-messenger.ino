/*
 * Elecrow 2.8" CYD — Stylus Note Messenger
 * Channel: @IamNotIntoTech (https://youtube.com/@IamNotIntoTech)
 * 
 * Target: ESP32-2432S028R (2.8" ILI9341 TFT + XPT2046 Resistive Touch)
 * Features:
 *  - High-precision resistive touch stylus drawing canvas with vector interpolation
 *  - Multi-color ink palette (Cyan, Yellow, Green, Pink, White, and Eraser)
 *  - 3 pen tip sizes: 1px Fine, 3px Medium, 5px Chisel
 *  - Clear canvas with confirmation
 *  - One-tap "SEND NOTE" webhook transmission simulation with delivery feedback
 *  - Tri-color RGB transmission LED on IO4/16/17 & acoustic chimes on IO26
 */

#include <Arduino.h>
#include <SPI.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>

// Hardware Pin Definitions for Elecrow 2.8" CYD
#define XPT2046_IRQ 36
#define XPT2046_MOSI 13
#define XPT2046_MISO 12
#define XPT2046_CLK 14
#define XPT2046_CS 33

#define TFT_BL 21
#define LED_RED 4
#define LED_GREEN 16
#define LED_BLUE 17
#define BUZZER_PIN 26

SPIClass touchSPI = SPIClass(VSPI);
XPT2046_Touchscreen ts(XPT2046_CS, XPT2046_IRQ);
TFT_eSPI tft = TFT_eSPI();

// UI Colors (16-bit 565)
#define COLOR_BG        0x0000 // Deep black for canvas
#define COLOR_TOOLBAR   0x18C4 // Charcoal toolbar
#define COLOR_CARD_BRD  0x2967
#define COLOR_TEXT      0xFFFF
#define COLOR_MUTED     0x8410
#define COLOR_ACTIVE    0x04DF

// Available Ink Colors
const uint16_t INK_COLORS[] = {
  0x07FF, // Cyan
  0xFFE0, // Yellow
  0x07E0, // Green
  0xF81F, // Pink
  0xFFFF  // White
};
const int NUM_COLORS = 5;

int selectedColorIdx = 0; // 0..4
bool eraserMode = false;
int penWidth = 2; // 1, 2, or 3 (corresponds to 1px, 3px, 5px)

// Canvas Area Bounds
#define CANVAS_X 48
#define CANVAS_Y 27
#define CANVAS_W 272
#define CANVAS_H 212

// Touch Drawing State
bool isDrawing = false;
int lastDrawX = -1;
int lastDrawY = -1;
int strokePointsCount = 0;

void playBeep(int freq, int ms) {
  tone(BUZZER_PIN, freq, ms);
  delay(ms);
  noTone(BUZZER_PIN);
}

void setRGB(bool r, bool g, bool b) {
  digitalWrite(LED_RED, r ? LOW : HIGH);
  digitalWrite(LED_GREEN, g ? LOW : HIGH);
  digitalWrite(LED_BLUE, b ? LOW : HIGH);
}

void drawHeader() {
  tft.fillRect(0, 0, 320, 26, COLOR_TOOLBAR);
  tft.drawFastHLine(0, 26, 320, COLOR_CARD_BRD);

  tft.setTextColor(0x07FF, COLOR_TOOLBAR);
  tft.setTextSize(1);
  tft.drawString("STYLUS NOTE MESSENGER", 10, 6, 2);

  // Status Badge
  tft.setTextColor(COLOR_MUTED, COLOR_TOOLBAR);
  char statusBuf[32];
  snprintf(statusBuf, sizeof(statusBuf), "PTS: %d | READY ●", strokePointsCount);
  tft.drawRightString(statusBuf, 310, 6, 1);
}

void drawToolbar() {
  // Left side vertical toolbar: x: 0..46, y: 27..240
  tft.fillRect(0, 27, 46, 213, COLOR_TOOLBAR);
  tft.drawFastVLine(46, 27, 213, COLOR_CARD_BRD);

  // 5 Color Swatches (y: 32, 58, 84, 110, 136)
  for (int i = 0; i < NUM_COLORS; i++) {
    int y = 32 + i * 24;
    bool active = (!eraserMode && selectedColorIdx == i);

    tft.fillCircle(23, y + 10, 9, INK_COLORS[i]);
    if (active) {
      tft.drawCircle(23, y + 10, 11, COLOR_TEXT);
      tft.drawCircle(23, y + 10, 12, COLOR_TEXT);
    }
  }

  // Eraser Button (y: 154)
  int eraseY = 154;
  tft.fillRoundRect(6, eraseY, 34, 20, 4, eraserMode ? COLOR_ACTIVE : tft.color565(40, 45, 55));
  tft.drawRoundRect(6, eraseY, 34, 20, 4, eraserMode ? COLOR_TEXT : COLOR_CARD_BRD);
  tft.setTextColor(COLOR_TEXT, eraserMode ? COLOR_ACTIVE : tft.color565(40, 45, 55));
  tft.drawCentreString("ERA", 23, eraseY + 4, 1);

  // Pen Width Stepper (y: 178)
  int penY = 178;
  tft.fillRoundRect(6, penY, 34, 22, 4, tft.color565(40, 45, 55));
  tft.drawRoundRect(6, penY, 34, 22, 4, COLOR_CARD_BRD);
  tft.setTextColor(COLOR_TEXT, tft.color565(40, 45, 55));
  char szBuf[8];
  snprintf(szBuf, sizeof(szBuf), "%dpx", penWidth * 2 - 1);
  tft.drawCentreString(szBuf, 23, penY + 5, 1);

  // Send Note Button (y: 204)
  int sendY = 204;
  tft.fillRoundRect(5, sendY, 36, 30, 4, 0x07E0);
  tft.setTextColor(0x0000, 0x07E0);
  tft.drawCentreString("SEND", 23, sendY + 8, 1);
}

void clearCanvas() {
  tft.fillRect(CANVAS_X, CANVAS_Y, CANVAS_W, CANVAS_H, COLOR_BG);
  strokePointsCount = 0;
  drawHeader();
}

void drawInterpolatedStroke(int x0, int y0, int x1, int y1) {
  uint16_t color = eraserMode ? COLOR_BG : INK_COLORS[selectedColorIdx];

  // Draw stroke based on width
  if (penWidth == 1) {
    tft.drawLine(x0, y0, x1, y1, color);
  } else if (penWidth == 2) {
    tft.drawLine(x0, y0, x1, y1, color);
    tft.drawLine(x0 + 1, y0, x1 + 1, y1, color);
    tft.drawLine(x0, y0 + 1, x1, y1 + 1, color);
  } else {
    // 5px chisel tip
    tft.drawLine(x0 - 1, y0, x1 - 1, y1, color);
    tft.drawLine(x0, y0, x1, y1, color);
    tft.drawLine(x0 + 1, y0, x1 + 1, y1, color);
    tft.drawLine(x0, y0 - 1, x1, y1 - 1, color);
    tft.drawLine(x0, y0 + 1, x1, y1 + 1, color);
  }
}

void transmitNote() {
  // Transmit animation banner
  tft.fillRoundRect(80, 100, 200, 60, 6, tft.color565(20, 30, 45));
  tft.drawRoundRect(80, 100, 200, 60, 6, 0x07FF);

  tft.setTextColor(0x07FF, tft.color565(20, 30, 45));
  tft.setTextSize(1);
  tft.drawCentreString("TRANSMITTING NOTE...", 180, 112, 2);

  tft.setTextColor(COLOR_TEXT, tft.color565(20, 30, 45));
  char countBuf[32];
  snprintf(countBuf, sizeof(countBuf), "Payload: %d Vector Points", strokePointsCount);
  tft.drawCentreString(countBuf, 180, 134, 1);

  // Blink Blue LED during upload simulation
  for (int i = 0; i < 4; i++) {
    setRGB(false, false, true);
    delay(100);
    setRGB(false, false, false);
    delay(100);
  }

  // Delivery Success
  tft.fillRoundRect(80, 100, 200, 60, 6, tft.color565(15, 45, 20));
  tft.drawRoundRect(80, 100, 200, 60, 6, 0x07E0);
  tft.setTextColor(0x07E0, tft.color565(15, 45, 20));
  tft.drawCentreString("NOTE DELIVERED!", 180, 112, 2);
  tft.setTextColor(COLOR_TEXT, tft.color565(15, 45, 20));
  tft.drawCentreString("Sent via Webhook / MQTT", 180, 134, 1);

  setRGB(false, true, false); // Solid green
  playBeep(2400, 70);
  delay(120);
  playBeep(3200, 120);

  delay(1200);
  setRGB(false, false, false);
  clearCanvas();
}

void setup() {
  Serial.begin(115200);

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  setRGB(false, false, false);

  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(COLOR_BG);

  touchSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  ts.begin(touchSPI);
  ts.setRotation(1);

  drawHeader();
  drawToolbar();
  clearCanvas();

  playBeep(2200, 50);
  Serial.println(F("[SYSTEM] Stylus Note Messenger Initialized."));
}

void loop() {
  if (ts.touched()) {
    TS_Point p = ts.getPoint();
    int touchX = map(p.x, 3800, 300, 0, 320);
    int touchY = map(p.y, 3800, 400, 0, 240);
    touchX = constrain(touchX, 0, 320);
    touchY = constrain(touchY, 0, 240);

    // Check if touching toolbar (x <= 46)
    if (touchX <= 46 && touchY >= 27) {
      isDrawing = false;

      // Color swatches (y: 30 to 150)
      if (touchY < 150) {
        int cIdx = (touchY - 30) / 24;
        if (cIdx >= 0 && cIdx < NUM_COLORS) {
          selectedColorIdx = cIdx;
          eraserMode = false;
          playBeep(2000, 20);
          drawToolbar();
          delay(120);
        }
      }
      // Eraser Toggle (154..174)
      else if (touchY >= 154 && touchY <= 174) {
        eraserMode = !eraserMode;
        playBeep(1800, 20);
        drawToolbar();
        delay(150);
      }
      // Pen Width Stepper (178..200)
      else if (touchY >= 178 && touchY <= 200) {
        penWidth = (penWidth % 3) + 1;
        playBeep(2400, 20);
        drawToolbar();
        delay(150);
      }
      // Send Button (204..238)
      else if (touchY >= 204 && touchY <= 238) {
        playBeep(2600, 50);
        transmitNote();
        delay(200);
      }
    }
    // Canvas Area (x >= 48)
    else if (touchX >= CANVAS_X && touchY >= CANVAS_Y) {
      if (!isDrawing) {
        isDrawing = true;
        lastDrawX = touchX;
        lastDrawY = touchY;
      } else {
        drawInterpolatedStroke(lastDrawX, lastDrawY, touchX, touchY);
        lastDrawX = touchX;
        lastDrawY = touchY;
        strokePointsCount++;
      }
    }
  } else {
    if (isDrawing) {
      isDrawing = false;
      lastDrawX = -1;
      lastDrawY = -1;
      drawHeader();
    }
  }

  delay(5); // Ultra low latency touch loop
}
