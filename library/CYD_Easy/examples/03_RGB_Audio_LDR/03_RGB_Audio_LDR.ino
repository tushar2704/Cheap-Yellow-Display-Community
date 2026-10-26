/*
 * CYD_Easy — Example 03: RGB LED, Audio & LDR Hardware Suite
 * Board: ESP32-2432S028R / ESP32-2432S028C
 * Author: tushar2704 <tushar.inseec@gmail.com>
 */

#include <CYD_Easy.h>

unsigned long lastTick = 0;
int noteIndex = 0;
int scale[8] = {261, 293, 329, 349, 392, 440, 493, 523}; // C Major Scale

void drawDashboard() {
  cyd.tft.fillRect(0, 0, 320, 240, 0x0821);

  // Header
  cyd.tft.fillRect(0, 0, 320, 28, 0x10A2);
  cyd.tft.setTextColor(TFT_WHITE, 0x10A2);
  cyd.tft.setTextSize(1);
  cyd.tft.setCursor(10, 10);
  cyd.tft.print("CYD HARDWARE SUITE (CYD_Easy)");

  // RGB Control Buttons (Red, Green, Blue, Rainbow)
  struct Btn { int x, y; const char* label; uint16_t color; };
  Btn btns[4] = {
    {20, 45, "RED", TFT_RED},
    {95, 45, "GREEN", TFT_GREEN},
    {170, 45, "BLUE", TFT_BLUE},
    {245, 45, "RAINBOW", TFT_YELLOW}
  };

  for (int i = 0; i < 4; i++) {
    cyd.tft.fillRoundRect(btns[i].x, btns[i].y, 65, 36, 6, btns[i].color);
    cyd.tft.setTextColor(0x0000, btns[i].color);
    cyd.tft.setTextSize(1);
    cyd.tft.setCursor(btns[i].x + 12, btns[i].y + 14);
    cyd.tft.print(btns[i].label);
  }

  // Play Scale Sound Button
  cyd.tft.fillRoundRect(20, 95, 280, 40, 6, TFT_CYAN);
  cyd.tft.setTextColor(0x0000, TFT_CYAN);
  cyd.tft.setTextSize(2);
  cyd.tft.setCursor(55, 107);
  cyd.tft.print("PLAY AUDIO SCALE");

  // LDR Light Sensor Meter Box
  cyd.tft.fillRoundRect(20, 150, 280, 75, 6, 0x18C3);
  cyd.tft.drawRoundRect(20, 150, 280, 75, 6, 0x2945);
}

void setup() {
  cyd.begin(1);
  cyd.setBacklight(100);
  cyd.setRgb(0, 255, 255);
  cyd.beep(2000, 40);

  drawDashboard();
}

void loop() {
  // Touch Handling
  if (cyd.touched()) {
    CYD_Point p = cyd.getTouch();
    // Buttons Row
    if (p.y >= 45 && p.y <= 81) {
      if (p.x >= 20 && p.x <= 85) { cyd.setRgb(255, 0, 0); cyd.playClick(); }
      else if (p.x >= 95 && p.x <= 160) { cyd.setRgb(0, 255, 0); cyd.playClick(); }
      else if (p.x >= 170 && p.x <= 235) { cyd.setRgb(0, 0, 255); cyd.playClick(); }
      else if (p.x >= 245 && p.x <= 310) { cyd.setRgb(255, 180, 0); cyd.playClick(); }
      delay(120);
    }
    // Audio Scale Button
    else if (p.y >= 95 && p.y <= 135 && p.x >= 20 && p.x <= 300) {
      cyd.beep(scale[noteIndex], 60);
      noteIndex = (noteIndex + 1) % 8;
      delay(100);
    }
  }

  // LDR Update (every 500ms)
  if (millis() - lastTick >= 500) {
    lastTick = millis();
    int raw = cyd.readLdr();
    int lux = cyd.readLux();

    cyd.tft.fillRect(30, 160, 260, 55, 0x18C3);
    cyd.tft.setTextColor(TFT_WHITE, 0x18C3);
    cyd.tft.setTextSize(1);
    cyd.tft.setCursor(35, 165);
    cyd.tft.printf("LDR Raw ADC: %d / 4095", raw);

    cyd.tft.setCursor(35, 182);
    cyd.tft.setTextColor(TFT_YELLOW, 0x18C3);
    cyd.tft.printf("Estimated Ambient Light: %d Lux", lux);

    // Lux Progress Bar
    cyd.tft.fillRoundRect(35, 198, 240, 10, 4, 0x0821);
    int barW = map(constrain(lux, 0, 1200), 0, 1200, 0, 240);
    cyd.tft.fillRoundRect(35, 198, barW, 10, 4, TFT_YELLOW);
  }
}
