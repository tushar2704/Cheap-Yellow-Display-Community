/**
 * @file 06-touch-buttons.ino
 * @brief Touchscreen Button Matrix & Toggle Switches for CYD 2.8" (ESP32-2432S028R)
 * @author Tushar Aggarwal (tushar2704)
 * @repository https://github.com/tushar2704/Cheap-Yellow-Display-Community
 * 
 * 📺 Follow on YouTube:
 * - Tech & Coding: https://youtube.com/@IamNotIntoTech
 * - 3D Printing & Enclosure: https://youtube.com/@Makerrdesigns
 * 
 * Hardware:
 * - ESP32-2432S028R (2.8" Resistive Touch CYD)
 */

#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>

#define XPT2046_IRQ 36
#define XPT2046_MOSI 13
#define XPT2046_MISO 12
#define XPT2046_CLK 14
#define XPT2046_CS 33

#define LED_RED 4
#define LED_GREEN 16
#define LED_BLUE 17
#define SPEAKER_PIN 26

#define TS_MINX 300
#define TS_MAXX 3800
#define TS_MINY 200
#define TS_MAXY 3700

SPIClass touchSPI(VSPI);
XPT2046_Touchscreen ts(XPT2046_CS, XPT2046_IRQ);
TFT_eSPI tft = TFT_eSPI();

// Button States
bool toggleLight = false;
bool toggleFan   = false;
bool toggleAlarm = false;
int counterValue = 0;

void beep(int freq, int ms) {
  tone(SPEAKER_PIN, freq, ms);
}

void setRgb(bool r, bool g, bool b) {
  digitalWrite(LED_RED, r ? LOW : HIGH);
  digitalWrite(LED_GREEN, g ? LOW : HIGH);
  digitalWrite(LED_BLUE, b ? LOW : HIGH);
}

void drawToggleButton(int x, int y, int w, int h, const char* label, bool state, uint16_t onColor) {
  uint16_t btnColor = state ? onColor : 0x2104;
  uint16_t borderColor = state ? TFT_WHITE : TFT_DARKGREY;

  tft.fillRoundRect(x, y, w, h, 8, btnColor);
  tft.drawRoundRect(x, y, w, h, 8, borderColor);

  tft.setTextColor(state ? TFT_BLACK : TFT_WHITE, btnColor);
  tft.setTextSize(2);
  tft.setCursor(x + 14, y + 14);
  tft.print(label);

  tft.setTextSize(1);
  tft.setCursor(x + 14, y + 38);
  tft.print(state ? "STATUS: [ON]" : "STATUS: [OFF]");
}

void drawCounterCard() {
  tft.fillRoundRect(165, 126, 140, 84, 8, 0x18C3);
  tft.drawRoundRect(165, 126, 140, 84, 8, 0x39C7);

  tft.setTextColor(TFT_WHITE, 0x18C3);
  tft.setTextSize(1);
  tft.setCursor(180, 136);
  tft.print("TAP COUNTER");

  tft.setTextColor(0xFDC0, 0x18C3);
  tft.setTextSize(3);
  tft.setCursor(205, 156);
  tft.printf("%03d", counterValue);

  tft.setTextColor(TFT_LIGHTGREY, 0x18C3);
  tft.setTextSize(1);
  tft.setCursor(176, 190);
  tft.print("Tap to increment +1");
}

void renderAllButtons() {
  drawToggleButton(15, 48, 140, 64, "LIGHT", toggleLight, 0xFDC0); // CYD Yellow
  drawToggleButton(165, 48, 140, 64, "FAN", toggleFan, 0x07FF);     // Cyan
  drawToggleButton(15, 126, 140, 64, "ALARM", toggleAlarm, 0xF800); // Red
  drawCounterCard();
}

void setup() {
  Serial.begin(115200);

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  pinMode(SPEAKER_PIN, OUTPUT);
  setRgb(false, false, false);

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(0x0821);

  touchSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  ts.begin(touchSPI);
  ts.setRotation(1);

  // Header Bar
  tft.fillRect(0, 0, 320, 36, 0x18C3);
  tft.setTextColor(TFT_WHITE, 0x18C3);
  tft.setTextSize(2);
  tft.setCursor(12, 10);
  tft.print("CYD 2.8\" Button Matrix");

  renderAllButtons();

  // Footer
  tft.fillRect(0, 222, 320, 18, TFT_BLACK);
  tft.setTextColor(0x07E0, TFT_BLACK);
  tft.setTextSize(1);
  tft.setCursor(6, 226);
  tft.print("YT: @IamNotIntoTech  |  3D: @Makerrdesigns");

  beep(1200, 80);
}

void loop() {
  if (ts.touched()) {
    TS_Point p = ts.getPoint();
    int x = map(p.x, TS_MINX, TS_MAXX, 0, 320);
    int y = map(p.y, TS_MINY, TS_MAXY, 0, 240);

    // Button 1: Light (15, 48, 140, 64)
    if (x >= 15 && x <= 155 && y >= 48 && y <= 112) {
      toggleLight = !toggleLight;
      setRgb(toggleLight, toggleLight, false); // Yellow LED
      beep(1800, 30);
      drawToggleButton(15, 48, 140, 64, "LIGHT", toggleLight, 0xFDC0);
      delay(200);
    }
    // Button 2: Fan (165, 48, 140, 64)
    else if (x >= 165 && x <= 305 && y >= 48 && y <= 112) {
      toggleFan = !toggleFan;
      setRgb(false, false, toggleFan); // Blue LED
      beep(1500, 30);
      drawToggleButton(165, 48, 140, 64, "FAN", toggleFan, 0x07FF);
      delay(200);
    }
    // Button 3: Alarm (15, 126, 140, 64)
    else if (x >= 15 && x <= 155 && y >= 126 && y <= 190) {
      toggleAlarm = !toggleAlarm;
      setRgb(toggleAlarm, false, false); // Red LED
      beep(toggleAlarm ? 2400 : 800, 50);
      drawToggleButton(15, 126, 140, 64, "ALARM", toggleAlarm, 0xF800);
      delay(200);
    }
    // Counter Card (165, 126, 140, 64)
    else if (x >= 165 && x <= 305 && y >= 126 && y <= 190) {
      counterValue = (counterValue + 1) % 1000;
      beep(2200, 20);
      drawCounterCard();
      delay(150);
    }
  }

  delay(20);
}
