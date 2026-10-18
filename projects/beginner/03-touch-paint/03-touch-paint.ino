/**
 * @file 03-touch-paint.ino
 * @brief Touchscreen Drawing Pad & Paint App for Cheap Yellow Display (CYD)
 * @author Tushar Aggarwal (tushar2704)
 * @repository https://github.com/tushar2704/Cheap-Yellow-Display-Community
 * 
 * 📺 Follow on YouTube:
 * - Tech & Coding: https://youtube.com/@IamNotIntoTech
 * - 3D Printing & Enclosure: https://youtube.com/@Makerrdesigns
 */

#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>

// Touch Pins
#define XPT2046_IRQ 36
#define XPT2046_MOSI 13
#define XPT2046_MISO 12
#define XPT2046_CLK 14
#define XPT2046_CS 33

// Indicator & Audio Pins
#define LED_RED 4
#define LED_GREEN 16
#define LED_BLUE 17
#define SPEAKER_PIN 26

// Calibration constants
#define TS_MINX 300
#define TS_MAXX 3800
#define TS_MINY 200
#define TS_MAXY 3700

SPIClass touchSPI(VSPI);
XPT2046_Touchscreen ts(XPT2046_CS, XPT2046_IRQ);
TFT_eSPI tft = TFT_eSPI();

// Color Palette
const uint16_t PALETTE[] = {
  TFT_RED,
  TFT_GREEN,
  TFT_BLUE,
  TFT_YELLOW,
  TFT_CYAN,
  TFT_MAGENTA,
  TFT_WHITE,
  TFT_BLACK // Eraser
};
const int NUM_COLORS = 8;
int currentColorIdx = 0;
uint16_t currentColor = TFT_RED;
int brushRadius = 2; // 1 = Fine, 2 = Medium, 4 = Thick

int lastX = -1;
int lastY = -1;

void setRgbLedColor(uint16_t color) {
  bool r = false, g = false, b = false;
  if (color == TFT_RED) r = true;
  else if (color == TFT_GREEN) g = true;
  else if (color == TFT_BLUE) b = true;
  else if (color == TFT_YELLOW) { r = true; g = true; }
  else if (color == TFT_CYAN) { g = true; b = true; }
  else if (color == TFT_MAGENTA) { r = true; b = true; }
  else if (color == TFT_WHITE) { r = true; g = true; b = true; }

  digitalWrite(LED_RED, r ? LOW : HIGH);
  digitalWrite(LED_GREEN, g ? LOW : HIGH);
  digitalWrite(LED_BLUE, b ? LOW : HIGH);
}

void beep(int freq, int ms) {
  tone(SPEAKER_PIN, freq, ms);
}

void clearCanvas() {
  tft.fillRect(0, 42, 320, 198, TFT_BLACK);
  beep(800, 80);
}

void drawToolBar() {
  // Top Header Bar
  tft.fillRect(0, 0, 320, 40, 0x18C3);

  // Palette Color Swatches
  for (int i = 0; i < NUM_COLORS; ++i) {
    int x = 6 + (i * 30);
    int y = 5;
    tft.fillRect(x, y, 24, 24, PALETTE[i]);
    if (i == currentColorIdx) {
      tft.drawRect(x - 2, y - 2, 28, 28, TFT_WHITE);
    } else {
      tft.drawRect(x, y, 24, 24, TFT_DARKGREY);
    }
  }

  // Clear Canvas Button
  tft.fillRoundRect(250, 6, 64, 26, 4, 0xD800); // Red clear button
  tft.setTextColor(TFT_WHITE, 0xD800);
  tft.setTextSize(1);
  tft.setCursor(266, 15);
  tft.print("CLEAR");

  // Bottom Canvas divider
  tft.drawLine(0, 41, 320, 41, TFT_LIGHTGREY);
}

void setup() {
  Serial.begin(115200);

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  pinMode(SPEAKER_PIN, OUTPUT);

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);

  touchSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  ts.begin(touchSPI);
  ts.setRotation(1);

  drawToolBar();
  setRgbLedColor(currentColor);
  beep(1400, 100);
}

void loop() {
  if (ts.touched()) {
    TS_Point p = ts.getPoint();
    int x = map(p.x, TS_MINX, TS_MAXX, 0, 320);
    int y = map(p.y, TS_MINY, TS_MAXY, 0, 240);
    x = constrain(x, 0, 319);
    y = constrain(y, 0, 239);

    // Check Toolbar interactions
    if (y < 40) {
      lastX = -1;
      lastY = -1;

      // Check Clear Button
      if (x >= 250) {
        clearCanvas();
        delay(150);
        return;
      }

      // Check Color Swatches
      for (int i = 0; i < NUM_COLORS; ++i) {
        int swatchX = 6 + (i * 30);
        if (x >= swatchX && x <= swatchX + 28) {
          currentColorIdx = i;
          currentColor = PALETTE[i];
          drawToolBar();
          setRgbLedColor(currentColor);
          beep(2000, 40);
          delay(120);
          break;
        }
      }
      return;
    }

    // Inside Drawing Canvas (y >= 42)
    if (y >= 43) {
      if (lastX != -1 && lastY != -1) {
        // Continuous smooth stroke interpolation
        tft.drawLine(lastX, lastY, x, y, currentColor);
        if (brushRadius > 1) {
          tft.drawLine(lastX + 1, lastY, x + 1, y, currentColor);
          tft.drawLine(lastX, lastY + 1, x, y + 1, currentColor);
        }
      } else {
        tft.fillCircle(x, y, brushRadius, currentColor);
      }
      lastX = x;
      lastY = y;
    }
  } else {
    // Pen lifted
    lastX = -1;
    lastY = -1;
  }

  delay(10);
}
