/*
 * CYD_Easy — Example 01: QuickStart
 * Board: ESP32-2432S028R / ESP32-2432S028C
 * Author: tushar2704 <tushar.inseec@gmail.com>
 */

#include <CYD_Easy.h>

void setup() {
  Serial.begin(115200);

  // 1. Initialize all CYD hardware in 1 line (rotation 1 = Landscape)
  cyd.begin(1);

  // 2. Audible chime on boot
  cyd.beep(1800, 40);
  delay(50);
  cyd.beep(2400, 60);

  // 3. Set LED to Cyan
  cyd.setRgb(0, 255, 255);

  // 4. Draw Welcome Screen
  cyd.tft.fillScreen(0x0821);
  cyd.tft.setTextColor(TFT_WHITE, 0x0821);
  cyd.tft.setTextSize(2);
  cyd.tft.setCursor(20, 30);
  cyd.tft.print("CYD Easy QuickStart");

  cyd.tft.setTextSize(1);
  cyd.tft.setTextColor(TFT_GREEN, 0x0821);
  cyd.tft.setCursor(20, 65);
  cyd.tft.print("Display, Touch, Audio & RGB Ready!");

  // Draw interactive button
  cyd.tft.fillRoundRect(20, 100, 280, 50, 8, TFT_CYAN);
  cyd.tft.setTextColor(0x0000, TFT_CYAN);
  cyd.tft.setTextSize(2);
  cyd.tft.setCursor(65, 115);
  cyd.tft.print("TAP THIS BUTTON");
}

void loop() {
  // Check touch
  if (cyd.touched()) {
    CYD_Point p = cyd.getTouch();
    if (p.x >= 20 && p.x <= 300 && p.y >= 100 && p.y <= 150) {
      cyd.playClick();
      cyd.setRgb(random(0, 256), random(0, 256), random(0, 256));

      // Visual feedback
      cyd.tft.fillRoundRect(20, 170, 280, 40, 6, 0x18C3);
      cyd.tft.setTextColor(TFT_YELLOW, 0x18C3);
      cyd.tft.setTextSize(1);
      cyd.tft.setCursor(35, 184);
      cyd.tft.printf("Touched at X:%03d Y:%03d | LDR: %d Lux", p.x, p.y, cyd.readLux());
      delay(150);
    }
  }
}
