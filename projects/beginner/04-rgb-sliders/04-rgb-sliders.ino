/**
 * @file 04-rgb-sliders.ino
 * @brief Interactive RGB Touch Sliders for Cheap Yellow Display 2.8" (ESP32-2432S028R)
 * @author Tushar Aggarwal (tushar2704)
 * @repository https://github.com/tushar2704/Cheap-Yellow-Display-Community
 * 
 * 📺 Follow on YouTube:
 * - Tech & Coding: https://youtube.com/@IamNotIntoTech
 * - 3D Printing & Enclosure: https://youtube.com/@Makerrdesigns
 * 
 * Hardware:
 * - 2.8" Resistive Touch Cheap Yellow Display (ESP32-2432S028R)
 * - Onboard RGB LED (GPIO 4: Red, GPIO 16: Green, GPIO 17: Blue)
 */

#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>

#define XPT2046_IRQ 36
#define XPT2046_MOSI 13
#define XPT2046_MISO 12
#define XPT2046_CLK 14
#define XPT2046_CS 33

#define LED_RED 4     // Active LOW PWM
#define LED_GREEN 16  // Active LOW PWM
#define LED_BLUE 17   // Active LOW PWM

#define TS_MINX 300
#define TS_MAXX 3800
#define TS_MINY 200
#define TS_MAXY 3700

SPIClass touchSPI(VSPI);
XPT2046_Touchscreen ts(XPT2046_CS, XPT2046_IRQ);
TFT_eSPI tft = TFT_eSPI();

// Slider channels
int valR = 255;
int valG = 120;
int valB = 40;

const int SLIDER_X = 50;
const int SLIDER_W = 220;
const int SLIDER_H = 22;

const int SLIDER_Y_R = 60;
const int SLIDER_Y_G = 105;
const int SLIDER_Y_B = 150;

void updateLedPwm() {
  // Invert values because onboard RGB LED is Active LOW
  ledcWrite(0, 255 - valR);
  ledcWrite(1, 255 - valG);
  ledcWrite(2, 255 - valB);
}

void drawSlider(int y, int val, uint16_t color, const char* label) {
  // Label
  tft.setTextColor(color, 0x0821);
  tft.setTextSize(2);
  tft.setCursor(14, y + 4);
  tft.print(label);

  // Background track
  tft.fillRoundRect(SLIDER_X, y, SLIDER_W, SLIDER_H, 6, 0x2104);
  tft.drawRoundRect(SLIDER_X, y, SLIDER_W, SLIDER_H, 6, TFT_DARKGREY);

  // Active filled bar
  int fillW = map(val, 0, 255, 0, SLIDER_W);
  if (fillW > 0) {
    tft.fillRoundRect(SLIDER_X, y, fillW, SLIDER_H, 6, color);
  }

  // Thumb handle
  int thumbX = SLIDER_X + fillW;
  thumbX = constrain(thumbX, SLIDER_X + 6, SLIDER_X + SLIDER_W - 6);
  tft.fillCircle(thumbX, y + (SLIDER_H / 2), 10, TFT_WHITE);
  tft.drawCircle(thumbX, y + (SLIDER_H / 2), 10, TFT_BLACK);

  // Value readout
  tft.fillRect(280, y + 4, 38, 16, 0x0821);
  tft.setTextColor(TFT_WHITE, 0x0821);
  tft.setTextSize(1);
  tft.setCursor(282, y + 7);
  tft.printf("%3d", val);
}

void drawColorPreview() {
  // 16-bit 565 color preview box
  uint16_t previewColor = tft.color565(valR, valG, valB);
  tft.fillRoundRect(50, 186, 220, 28, 4, previewColor);
  tft.drawRoundRect(50, 186, 220, 28, 4, TFT_WHITE);

  tft.setTextColor((valR + valG + valB > 380) ? TFT_BLACK : TFT_WHITE, previewColor);
  tft.setTextSize(1);
  tft.setCursor(95, 196);
  tft.printf("HEX: #%02X%02X%02X", valR, valG, valB);
}

void setup() {
  Serial.begin(115200);

  // Setup PWM channels for RGB LED (5kHz, 8-bit)
  ledcSetup(0, 5000, 8);
  ledcSetup(1, 5000, 8);
  ledcSetup(2, 5000, 8);
  ledcAttachPin(LED_RED, 0);
  ledcAttachPin(LED_GREEN, 1);
  ledcAttachPin(LED_BLUE, 2);
  updateLedPwm();

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(0x0821);

  touchSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  ts.begin(touchSPI);
  ts.setRotation(1);

  // Header Banner
  tft.fillRect(0, 0, 320, 34, 0x18C3);
  tft.setTextColor(TFT_WHITE, 0x18C3);
  tft.setTextSize(2);
  tft.setCursor(8, 9);
  tft.print("CYD 2.8\" RGB Sliders");

  // Render initial sliders
  drawSlider(SLIDER_Y_R, valR, TFT_RED, "R");
  drawSlider(SLIDER_Y_G, valG, TFT_GREEN, "G");
  drawSlider(SLIDER_Y_B, valB, TFT_CYAN, "B");
  drawColorPreview();

  // Footer Bar
  tft.fillRect(0, 224, 320, 16, TFT_BLACK);
  tft.setTextColor(0x07E0, TFT_BLACK);
  tft.setTextSize(1);
  tft.setCursor(6, 228);
  tft.print("YT: @IamNotIntoTech  |  3D: @Makerrdesigns");
}

void loop() {
  if (ts.touched()) {
    TS_Point p = ts.getPoint();
    int x = map(p.x, TS_MINX, TS_MAXX, 0, 320);
    int y = map(p.y, TS_MINY, TS_MAXY, 0, 240);

    bool changed = false;

    // Check Red Slider
    if (y >= SLIDER_Y_R - 10 && y <= SLIDER_Y_R + SLIDER_H + 10) {
      valR = constrain(map(x, SLIDER_X, SLIDER_X + SLIDER_W, 0, 255), 0, 255);
      drawSlider(SLIDER_Y_R, valR, TFT_RED, "R");
      changed = true;
    }
    // Check Green Slider
    else if (y >= SLIDER_Y_G - 10 && y <= SLIDER_Y_G + SLIDER_H + 10) {
      valG = constrain(map(x, SLIDER_X, SLIDER_X + SLIDER_W, 0, 255), 0, 255);
      drawSlider(SLIDER_Y_G, valG, TFT_GREEN, "G");
      changed = true;
    }
    // Check Blue Slider
    else if (y >= SLIDER_Y_B - 10 && y <= SLIDER_Y_B + SLIDER_H + 10) {
      valB = constrain(map(x, SLIDER_X, SLIDER_X + SLIDER_W, 0, 255), 0, 255);
      drawSlider(SLIDER_Y_B, valB, TFT_CYAN, "B");
      changed = true;
    }

    if (changed) {
      updateLedPwm();
      drawColorPreview();
    }
  }

  delay(20);
}
