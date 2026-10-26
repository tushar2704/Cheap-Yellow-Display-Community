/*
 * CYD_Easy — Example 02: Touch & Draw Canvas
 * Board: ESP32-2432S028R / ESP32-2432S028C
 * Author: tushar2704 <tushar.inseec@gmail.com>
 */

#include <CYD_Easy.h>

uint16_t palette[6] = {TFT_WHITE, TFT_RED, TFT_GREEN, TFT_BLUE, TFT_YELLOW, TFT_MAGENTA};
int activeColorIdx = 0;
int lastX = -1, lastY = -1;

void drawPalette() {
  cyd.tft.fillRect(0, 0, 320, 30, 0x18C3);
  for (int i = 0; i < 6; i++) {
    cyd.tft.fillCircle(25 + i * 35, 15, 10, palette[i]);
    if (i == activeColorIdx) {
      cyd.tft.drawCircle(25 + i * 35, 15, 13, TFT_WHITE);
    }
  }

  // Clear button
  cyd.tft.fillRoundRect(245, 4, 65, 22, 4, TFT_RED);
  cyd.tft.setTextColor(TFT_WHITE, TFT_RED);
  cyd.tft.setTextSize(1);
  cyd.tft.setCursor(258, 11);
  cyd.tft.print("CLEAR");
}

void setup() {
  cyd.begin(1); // Landscape
  cyd.setBacklight(90);
  cyd.setRgb(255, 255, 255);
  cyd.beep(2000, 30);

  cyd.tft.fillScreen(TFT_BLACK);
  drawPalette();
}

void loop() {
  if (cyd.touched()) {
    CYD_Point p = cyd.getTouch();
    
    // Tap on top palette bar
    if (p.y <= 30) {
      if (p.x >= 240) { // Clear
        cyd.tft.fillRect(0, 31, 320, 209, TFT_BLACK);
        cyd.playClick();
        delay(100);
      } else {
        int idx = (p.x - 10) / 35;
        if (idx >= 0 && idx < 6) {
          activeColorIdx = idx;
          drawPalette();
          cyd.playClick();
          delay(100);
        }
      }
      lastX = -1; lastY = -1;
    } 
    // Drawing area
    else if (p.y > 32) {
      if (lastX > 0 && lastY > 0) {
        cyd.tft.drawLine(lastX, lastY, p.x, p.y, palette[activeColorIdx]);
        cyd.tft.drawLine(lastX + 1, lastY, p.x + 1, p.y, palette[activeColorIdx]);
      } else {
        cyd.tft.fillCircle(p.x, p.y, 2, palette[activeColorIdx]);
      }
      lastX = p.x;
      lastY = p.y;
    }
  } else {
    lastX = -1;
    lastY = -1;
  }
}
