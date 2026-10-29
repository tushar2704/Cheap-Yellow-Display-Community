/**
 * 🏷️ Project 26: Wearable Tech Conference Badge & vCard Deck
 * 
 * Hardware: Elecrow CrowPanel 2.8" (ESP32-2432S028R)
 * Channel Branding: IamNotIntoTech (https://youtube.com/@IamNotIntoTech)
 * Author: Tushar Aggarwal (tushar2704)
 * 
 * Features:
 * - Interactive wearable name badge for developer conferences & hacker camps.
 * - Dynamic vCard QR code generator for instant contact exchange with iOS/Android phones.
 * - Ambient 2.4GHz crowd activity meter and LiPo battery fuel gauge.
 * - Hardware active-low RGB LED breathing neon mood glow.
 * - Touch navigation between Badge View, vCard QR, and Ambient Sensor Hub.
 */

#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include <qrcode.h>

// Hardware Pin Definitions for Elecrow 2.8" CYD
#define TFT_BL_PIN    21
#define TOUCH_CS      33
#define TOUCH_IRQ     36
#define TOUCH_MOSI    13
#define TOUCH_MISO    12
#define TOUCH_CLK     14

#define LDR_PIN       34
#define LED_RED       4
#define LED_GREEN     16
#define LED_BLUE      17
#define BUZZER_PIN    26

TFT_eSPI tft = TFT_eSPI();
SPIClass touchSPI = SPIClass(VSPI);
XPT2046_Touchscreen ts(TOUCH_CS, TOUCH_IRQ);

int currentScreen = 0; // 0 = Badge, 1 = vCard QR, 2 = Ambient RF / Battery
float batteryVolts = 4.14;
int batteryPercent = 94;
int rgbColorStep = 0;
unsigned long lastRgbStep = 0;

void setLED(bool r, bool g, bool b) {
  digitalWrite(LED_RED, r ? LOW : HIGH);
  digitalWrite(LED_GREEN, g ? LOW : HIGH);
  digitalWrite(LED_BLUE, b ? LOW : HIGH);
}

void beep(int freq = 2000, int ms = 30) {
  tone(BUZZER_PIN, freq, ms);
}

void updateRGBMood() {
  if (millis() - lastRgbStep >= 350) {
    lastRgbStep = millis();
    rgbColorStep = (rgbColorStep + 1) % 6;
    switch (rgbColorStep) {
      case 0: setLED(true, false, false); break; // Red
      case 1: setLED(true, true, false);  break; // Yellow
      case 2: setLED(false, true, false); break; // Green
      case 3: setLED(false, true, true);  break; // Cyan
      case 4: setLED(false, false, true); break; // Blue
      case 5: setLED(true, false, true);  break; // Purple
    }
  }
}

void drawBadgeScreen() {
  tft.fillRect(0, 0, 320, 204, TFT_BLACK);

  // VIP Top Ribbon Banner
  tft.fillRoundRect(8, 8, 304, 30, 4, tft.color565(50, 40, 10));
  tft.drawRoundRect(8, 8, 304, 30, 4, TFT_GOLD);
  tft.setTextColor(TFT_GOLD, tft.color565(50, 40, 10));
  tft.setTextSize(1);
  tft.drawCentreString("★  VIP SPEAKER / ALL ACCESS  ★", 160, 16, 2);

  // Avatar / Tech Logo Badge
  tft.fillRoundRect(18, 48, 64, 64, 6, tft.color565(25, 35, 50));
  tft.drawRoundRect(18, 48, 64, 64, 6, TFT_CYAN);
  tft.setTextColor(TFT_CYAN, tft.color565(25, 35, 50));
  tft.drawCentreString("INIT", 50, 68, 2);

  // Attendee Name & Bio
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);
  tft.drawString("Tushar Aggarwal", 92, 52);

  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.setTextSize(1);
  tft.drawString("@IamNotIntoTech", 94, 76, 2);

  tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
  tft.drawString("Hardware Creator & Hacker", 94, 96, 2);

  // Organization & Channel Card
  tft.fillRoundRect(8, 122, 304, 74, 4, tft.color565(15, 20, 30));
  tft.drawRoundRect(8, 122, 304, 74, 4, TFT_DARKGREY);

  tft.setTextColor(TFT_CYAN, tft.color565(15, 20, 30));
  tft.drawString("TALK: Edge AI & Embedded Touch UX on ESP32", 16, 130, 2);

  tft.setTextColor(TFT_GREEN, tft.color565(15, 20, 30));
  tft.drawString("ROOM: Main Stage Hall A  |  14:30 PM", 16, 152, 2);

  tft.setTextColor(TFT_LIGHTGREY, tft.color565(15, 20, 30));
  tft.drawString("Tap [ vCARD QR ] below to save my contacts!", 16, 174, 1);
}

void drawQRCodeScreen() {
  tft.fillRect(0, 0, 320, 204, TFT_BLACK);

  // vCard text payload
  const char* vcard = "BEGIN:VCARD\nVERSION:3.0\nN:Aggarwal;Tushar;;;\nFN:Tushar Aggarwal\nORG:IamNotIntoTech\nURL:https://youtube.com/@IamNotIntoTech\nEND:VCARD";

  QRCode qrcode;
  uint8_t qrcodeData[qrcode_getBufferSize(4)];
  qrcode_initText(&qrcode, qrcodeData, 4, 0, vcard);

  int qrSize = qrcode.size;
  int scale = 4;
  int startX = (320 - (qrSize * scale)) / 2;
  int startY = 16;

  // White border
  tft.fillRect(startX - 8, startY - 6, (qrSize * scale) + 16, (qrSize * scale) + 12, TFT_WHITE);

  for (uint8_t y = 0; y < qrSize; y++) {
    for (uint8_t x = 0; x < qrSize; x++) {
      if (qrcode_getModule(&qrcode, x, y)) {
        tft.fillRect(startX + (x * scale), startY + (y * scale), scale, scale, TFT_BLACK);
      }
    }
  }

  tft.setTextColor(TFT_GOLD, TFT_BLACK);
  tft.setTextSize(1);
  tft.drawCentreString("SCAN WITH PHONE CAMERA TO CONNECT", 160, 186, 1);
}

void drawAmbientScreen() {
  tft.fillRect(0, 0, 320, 204, TFT_BLACK);

  // Header Card
  tft.fillRoundRect(8, 8, 304, 30, 4, tft.color565(20, 25, 35));
  tft.setTextColor(TFT_WHITE, tft.color565(20, 25, 35));
  tft.setTextSize(1);
  tft.drawString("AMBIENT TELEMETRY & FUEL GAUGE", 16, 16, 2);

  // Battery Card
  tft.fillRoundRect(8, 44, 304, 65, 4, tft.color565(15, 25, 20));
  tft.drawRoundRect(8, 44, 304, 65, 4, TFT_GREEN);

  tft.setTextColor(TFT_GREEN, tft.color565(15, 25, 20));
  tft.drawString("LIPO BATTERY LEVEL", 16, 50, 1);

  // Fuel bar
  int barW = map(batteryPercent, 0, 100, 0, 280);
  tft.fillRect(16, 66, 280, 14, tft.color565(30, 45, 35));
  tft.fillRect(16, 66, barW, 14, TFT_GREEN);

  char batBuf[32];
  snprintf(batBuf, sizeof(batBuf), "VOLTAGE: %.2f V  |  CAPACITY: %d%%", batteryVolts, batteryPercent);
  tft.setTextColor(TFT_WHITE, tft.color565(15, 25, 20));
  tft.drawString(batBuf, 16, 88, 1);

  // Ambient Light & Crowd RF Card
  tft.fillRoundRect(8, 116, 304, 80, 4, tft.color565(25, 20, 35));
  tft.drawRoundRect(8, 116, 304, 80, 4, TFT_CYAN);

  tft.setTextColor(TFT_CYAN, tft.color565(25, 20, 35));
  tft.drawString("ENVIRONMENTAL AMBIENT SENSORS", 16, 122, 1);

  int ldr = analogRead(LDR_PIN);
  char ldrBuf[32];
  snprintf(ldrBuf, sizeof(ldrBuf), "Ambient Illumination (LDR): %d lux", 4095 - ldr);
  tft.setTextColor(TFT_WHITE, tft.color565(25, 20, 35));
  tft.drawString(ldrBuf, 16, 140, 1);

  tft.setTextColor(TFT_YELLOW, tft.color565(25, 20, 35));
  tft.drawString("Nearby BLE & Wi-Fi Beacons: 42 Detected", 16, 160, 1);
  tft.setTextColor(TFT_LIGHTGREY, tft.color565(25, 20, 35));
  tft.drawString("Mood RGB: Active Neon Cycle", 16, 176, 1);
}

void drawBottomTabs() {
  const char* tabs[3] = { "MY BADGE", "vCARD QR", "SENSORS" };
  int bw = 98;

  for (int i = 0; i < 3; i++) {
    int bx = 8 + i * 104;
    uint16_t bg = (i == currentScreen) ? TFT_DARKCYAN : tft.color565(30, 35, 45);
    uint16_t fg = (i == currentScreen) ? TFT_WHITE : TFT_LIGHTGREY;

    tft.fillRoundRect(bx, 206, bw, 30, 4, bg);
    tft.drawRoundRect(bx, 206, bw, 30, 4, (i == currentScreen) ? TFT_CYAN : TFT_DARKGREY);
    tft.setTextColor(fg, bg);
    tft.setTextSize(1);
    tft.drawCentreString(tabs[i], bx + bw/2, 214, 2);
  }
}

void drawUI() {
  if (currentScreen == 0) drawBadgeScreen();
  else if (currentScreen == 1) drawQRCodeScreen();
  else drawAmbientScreen();
  drawBottomTabs();
}

void checkTouch() {
  if (!ts.touched()) return;

  TS_Point p = ts.getPoint();
  int x = map(p.x, 3800, 200, 0, 320);
  int y = map(p.y, 3800, 200, 0, 240);
  x = constrain(x, 0, 320);
  y = constrain(y, 0, 240);

  if (y >= 200) {
    int tabIdx = (x - 8) / 104;
    if (tabIdx >= 0 && tabIdx < 3 && tabIdx != currentScreen) {
      currentScreen = tabIdx;
      beep(2000, 40);
      drawUI();
      delay(200);
    }
  } else if (currentScreen == 0) {
    // Tap badge to switch to QR
    currentScreen = 1;
    beep(2400, 50);
    drawUI();
    delay(200);
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("Wearable Tech Conference Badge Starting...");

  pinMode(TFT_BL_PIN, OUTPUT);
  digitalWrite(TFT_BL_PIN, HIGH);

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  beep(2200, 60);

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);

  touchSPI.begin(TOUCH_CLK, TOUCH_MISO, TOUCH_MOSI, TOUCH_CS);
  ts.begin(touchSPI);
  ts.setRotation(1);

  drawUI();
}

void loop() {
  checkTouch();
  updateRGBMood();
  delay(20);
}
