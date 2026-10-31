/**
 * ✍️ Project 36: Stylus Note Messenger
 * 
 * Hardware: Elecrow CrowPanel 2.8" (ESP32-2432S028R)
 * Channel Branding: IamNotIntoTech (https://youtube.com/@IamNotIntoTech)
 * Author: Tushar Aggarwal (tushar2704)
 * 
 * Features:
 * - High-precision resistive touch stylus drawing canvas with vector line interpolation.
 * - Multi-color ink palette (Cyan, Yellow, Green, Pink, White, and Eraser).
 * - 3 Tip widths: 1px Fine, 3px Medium, 5px Chisel.
 * - One-tap "SEND NOTE" webhook transmission simulation with delivery feedback modal.
 * - Multi-color transmission RGB LED on GPIOs 4/16/17 & acoustic chimes on GPIO 26 buzzer.
 * - Hardware-calibrated non-inverted resistive touch coordinates.
 */

#include <Arduino.h>
#include <SPI.h>
#include <WiFi.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>

// Hardware Pin Definitions for Elecrow 2.8" CYD
#define TFT_BL_PIN    21
#define TOUCH_MOSI    32
#define TOUCH_MISO    39
#define TOUCH_CLK     25
#define TOUCH_CS      33
#define TOUCH_IRQ     36

#define LED_RED       4
#define LED_GREEN     16
#define LED_BLUE      17
#define BUZZER_PIN    26

SPIClass touchSPI = SPIClass(VSPI);
XPT2046_Touchscreen ts(TOUCH_CS);
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
int penWidth = 2; // 1 = 1px, 2 = 3px, 3 = 5px

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
unsigned long lastBtnTouchTime = 0;

void playBeep(int freq = 2000, int ms = 30) {
  tone(BUZZER_PIN, freq, ms);
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
  tft.drawString("IamNotIntoTech", 8, 6, 2);

  tft.setTextColor(COLOR_TEXT, COLOR_TOOLBAR);
  tft.drawCentreString("STYLUS NOTE MESSENGER", 175, 6, 2);

  // Status Badge
  tft.setTextColor(COLOR_MUTED, COLOR_TOOLBAR);
  char statusBuf[32];
  snprintf(statusBuf, sizeof(statusBuf), "%d PTS", strokePointsCount);
  tft.drawRightString(statusBuf, 312, 6, 1);
}

void drawToolbar() {
  tft.fillRect(0, 27, 46, 213, COLOR_TOOLBAR);
  tft.drawFastVLine(46, 27, 213, COLOR_CARD_BRD);

  // 5 Color Swatches (y: 32, 56, 80, 104, 128)
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

  if (penWidth == 1) {
    tft.drawLine(x0, y0, x1, y1, color);
  } else if (penWidth == 2) {
    tft.drawLine(x0, y0, x1, y1, color);
    tft.drawLine(x0 + 1, y0 + 1, x1 + 1, y1 + 1, color);
    tft.drawLine(x0, y0 + 1, x1, y1 + 1, color);
  } else {
    tft.drawLine(x0 - 1, y0 - 1, x1 - 1, y1 - 1, color);
    tft.drawLine(x0, y0, x1, y1, color);
    tft.drawLine(x0 + 1, y0 + 1, x1 + 1, y1 + 1, color);
    tft.drawLine(x0, y0 - 1, x1, y1 - 1, color);
    tft.drawLine(x0, y0 + 1, x1, y1 + 1, color);
  }
}

void transmitNote() {
  tft.fillRoundRect(80, 100, 200, 60, 6, tft.color565(20, 30, 45));
  tft.drawRoundRect(80, 100, 200, 60, 6, 0x07FF);

  tft.setTextColor(0x07FF, tft.color565(20, 30, 45));
  tft.drawCentreString("TRANSMITTING NOTE...", 180, 112, 2);

  tft.setTextColor(COLOR_TEXT, tft.color565(20, 30, 45));
  char countBuf[32];
  snprintf(countBuf, sizeof(countBuf), "Payload: %d Vector Points", strokePointsCount);
  tft.drawCentreString(countBuf, 180, 134, 1);

  for (int i = 0; i < 4; i++) {
    setRGB(false, false, true);
    delay(100);
    setRGB(false, false, false);
    delay(100);
  }

  tft.fillRoundRect(80, 100, 200, 60, 6, tft.color565(15, 45, 20));
  tft.drawRoundRect(80, 100, 200, 60, 6, 0x07E0);
  tft.setTextColor(0x07E0, tft.color565(15, 45, 20));
  tft.drawCentreString("NOTE DELIVERED!", 180, 112, 2);
  tft.setTextColor(COLOR_TEXT, tft.color565(15, 45, 20));
  tft.drawCentreString("Sent via Webhook / MQTT", 180, 134, 1);

  setRGB(false, true, false);
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

  pinMode(TFT_BL_PIN, OUTPUT);
  digitalWrite(TFT_BL_PIN, HIGH);

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(COLOR_BG);

  // Initialize Touch (VSPI)
  touchSPI.begin(TOUCH_CLK, TOUCH_MISO, TOUCH_MOSI, TOUCH_CS);
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
    if (p.z > 50 || ts.touched()) {
      int screenX = map(p.x, 240, 3800, 0, 320);
      int screenY = map(p.y, 240, 3800, 0, 240);
      screenX = constrain(screenX, 0, 319);
      screenY = constrain(screenY, 0, 239);

      // Touching Toolbar (screenX <= 46 && screenY >= 27)
      if (screenX <= 46 && screenY >= 27) {
        isDrawing = false;
        if (millis() - lastBtnTouchTime > 180) {
          lastBtnTouchTime = millis();
          // Color swatches (screenY < 152)
          if (screenY < 152) {
            int cIdx = (screenY - 30) / 24;
            if (cIdx >= 0 && cIdx < NUM_COLORS) {
              selectedColorIdx = cIdx;
              eraserMode = false;
              playBeep(2000, 20);
              drawToolbar();
            }
          }
          // Eraser Toggle (154..174)
          else if (screenY >= 154 && screenY <= 174) {
            eraserMode = !eraserMode;
            playBeep(1800, 20);
            drawToolbar();
          }
          // Pen Width Stepper (178..200)
          else if (screenY >= 178 && screenY <= 200) {
            penWidth = (penWidth % 3) + 1;
            playBeep(2400, 20);
            drawToolbar();
          }
          // Send Button (204..238)
          else if (screenY >= 204 && screenY <= 238) {
            playBeep(2600, 50);
            transmitNote();
          }
        }
      }
      // Canvas Drawing Area (screenX >= CANVAS_X && screenY >= CANVAS_Y)
      else if (screenX >= CANVAS_X && screenY >= CANVAS_Y) {
        if (!isDrawing) {
          isDrawing = true;
          lastDrawX = screenX;
          lastDrawY = screenY;
        } else {
          drawInterpolatedStroke(lastDrawX, lastDrawY, screenX, screenY);
          lastDrawX = screenX;
          lastDrawY = screenY;
          strokePointsCount++;
        }
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

  delay(5);
}
