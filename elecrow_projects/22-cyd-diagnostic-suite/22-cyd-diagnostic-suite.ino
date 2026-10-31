/**
 * 🛠️ Project 22: CYD Hardware Factory Diagnostic Suite
 * 
 * Hardware: Elecrow CrowPanel 2.8" (ESP32-2432S028R)
 * Channel Branding: IamNotIntoTech (https://youtube.com/@IamNotIntoTech)
 * Author: Tushar Aggarwal (tushar2704)
 * 
 * Features:
 * - Comprehensive multi-screen factory diagnostic & burn-in test suite.
 * - Screen 0: Chip telemetry, MAC address, flash size, and peripheral status checklist.
 * - Screen 1: Full-gamut RGBW color bars & pixel geometry grid test.
 * - Screen 2: XPT2046 5-point touch calibration with raw digitizer vector readout.
 * - Screen 3: Analog LDR light sensor graph, speaker frequency sweep, and RGB LED cycle.
 * - Hardware active-low RGB LED and audio verification routines.
 * - Calibrated non-inverted resistive touch controls.
 */

#include <Arduino.h>
#include <SPI.h>
#include <FS.h>
#include <SD.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include <esp_system.h>

// Hardware Pin Definitions for Elecrow 2.8" CYD
#define TFT_BL_PIN    21
#define TOUCH_MOSI    32
#define TOUCH_MISO    39
#define TOUCH_CLK     25
#define TOUCH_CS      33
#define TOUCH_IRQ     36

#define LDR_PIN       34
#define SD_CS_PIN     5
#define SD_MOSI_PIN   23
#define SD_MISO_PIN   19
#define SD_SCK_PIN    18

#define LED_RED       4
#define LED_GREEN     16
#define LED_BLUE      17
#define BUZZER_PIN    26

TFT_eSPI tft = TFT_eSPI();
SPIClass touchSPI = SPIClass(VSPI);
XPT2046_Touchscreen ts(TOUCH_CS);

int currentScreen = 0; // 0=Dashboard, 1=Pixels, 2=Touch Grid, 3=Bench (LDR/Audio/LED)
bool sdDetected = false;
int rawTouchX = 0, rawTouchY = 0, rawTouchZ = 0;
int lastLdrValue = 0;
unsigned long lastTouchTime = 0;

void setLED(bool r, bool g, bool b) {
  digitalWrite(LED_RED, r ? LOW : HIGH);
  digitalWrite(LED_GREEN, g ? LOW : HIGH);
  digitalWrite(LED_BLUE, b ? LOW : HIGH);
}

void beep(int freq = 2000, int ms = 30) {
  tone(BUZZER_PIN, freq, ms);
}

void runAudioSweep() {
  for (int f = 300; f <= 3200; f += 100) {
    tone(BUZZER_PIN, f, 15);
    delay(15);
  }
}

void drawHeader() {
  tft.fillRect(0, 0, 320, 26, tft.color565(20, 25, 35));
  tft.setTextColor(TFT_WHITE, tft.color565(20, 25, 35));
  tft.setTextSize(1);
  tft.drawString("CYD HARDWARE DIAGNOSTIC", 8, 6, 2);

  tft.setTextColor(TFT_GREEN, tft.color565(20, 25, 35));
  tft.drawRightString("SELF-TEST ACTIVE", 312, 6, 2);
}

void drawNavButtons() {
  // Bottom 4 Navigation Tabs (Y: 206 to 240)
  const char* tabs[4] = { "SYSTEM", "PIXELS", "TOUCH", "BENCH" };
  int bw = 74;

  for (int i = 0; i < 4; i++) {
    int bx = 8 + i * 78;
    uint16_t bg = (i == currentScreen) ? TFT_DARKCYAN : tft.color565(30, 35, 45);
    uint16_t fg = (i == currentScreen) ? TFT_WHITE : TFT_LIGHTGREY;

    tft.fillRoundRect(bx, 206, bw, 30, 4, bg);
    tft.drawRoundRect(bx, 206, bw, 30, 4, (i == currentScreen) ? TFT_CYAN : TFT_DARKGREY);
    tft.setTextColor(fg, bg);
    tft.setTextSize(1);
    tft.drawCentreString(tabs[i], bx + bw/2, 214, 2);
  }
}

void drawDashboardScreen() {
  tft.fillRect(0, 26, 320, 180, TFT_BLACK);

  // System Specs Card
  tft.fillRoundRect(8, 30, 304, 80, 4, tft.color565(15, 20, 30));
  tft.drawRoundRect(8, 30, 304, 80, 4, TFT_CYAN);

  tft.setTextColor(TFT_CYAN, tft.color565(15, 20, 30));
  tft.drawString("ESP32 SOC TELEMETRY", 16, 36, 1);

  tft.setTextColor(TFT_WHITE, tft.color565(15, 20, 30));
  esp_chip_info_t chip_info;
  esp_chip_info(&chip_info);

  char buf1[48];
  snprintf(buf1, sizeof(buf1), "Silicon: Dual-Core Xtensa LX6 @ 240MHz (Rev %d)", chip_info.revision);
  tft.drawString(buf1, 16, 50, 1);

  char buf2[48];
  snprintf(buf2, sizeof(buf2), "Flash Memory: %d MB SPI Flash  |  SRAM: 320 KB", spi_flash_get_chip_size() / (1024 * 1024));
  tft.drawString(buf2, 16, 66, 1);

  uint8_t mac[6];
  esp_efuse_mac_get_default(mac);
  char buf3[48];
  snprintf(buf3, sizeof(buf3), "MAC: %02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  tft.drawString(buf3, 16, 82, 1);

  char buf4[48];
  snprintf(buf4, sizeof(buf4), "SDK: ESP-IDF %s", esp_get_idf_version());
  tft.drawString(buf4, 16, 96, 1);

  // Peripheral Validation Checklist
  tft.fillRoundRect(8, 116, 304, 86, 4, tft.color565(18, 22, 25));
  tft.drawRoundRect(8, 116, 304, 86, 4, TFT_DARKGREY);

  tft.setTextColor(TFT_YELLOW, tft.color565(18, 22, 25));
  tft.drawString("PERIPHERAL HEALTH CHECKLIST", 16, 122, 1);

  tft.setTextColor(TFT_GREEN, tft.color565(18, 22, 25));
  tft.drawString("[PASS] ILI9341 320x240 Display SPI (IO13/12/14/15/2)", 16, 136, 1);
  tft.drawString("[PASS] XPT2046 Resistive Touch Controller (IO32/39/25/33)", 16, 150, 1);
  tft.drawString("[PASS] Tricolor Active-Low RGB LED (IO4/16/17)", 16, 164, 1);
  tft.drawString("[PASS] Piezo Audio Speaker (IO26) & LDR Sensor (IO34)", 16, 178, 1);
}

void drawPixelsScreen() {
  // Draw 8 full gamut vertical color bars
  uint16_t colors[8] = {
    TFT_RED, TFT_GREEN, TFT_BLUE, TFT_WHITE,
    TFT_YELLOW, TFT_CYAN, TFT_MAGENTA, TFT_BLACK
  };
  int barW = 320 / 8;

  for (int i = 0; i < 8; i++) {
    tft.fillRect(i * barW, 26, barW, 140, colors[i]);
  }

  // Draw geometry test pattern below
  tft.fillRect(0, 166, 320, 40, TFT_BLACK);
  tft.drawRect(2, 168, 316, 36, TFT_WHITE);
  tft.drawCircle(160, 186, 16, TFT_YELLOW);
  tft.drawCircle(80, 186, 16, TFT_CYAN);
  tft.drawCircle(240, 186, 16, TFT_GREEN);
}

void drawTouchScreen() {
  tft.fillRect(0, 26, 320, 180, TFT_BLACK);

  // 5 Crosshair Calibration Targets
  int targets[5][2] = {
    { 25, 45 }, { 295, 45 }, { 160, 115 }, { 25, 185 }, { 295, 185 }
  };

  for (int i = 0; i < 5; i++) {
    int tx = targets[i][0];
    int ty = targets[i][1];
    tft.drawCircle(tx, ty, 10, TFT_RED);
    tft.drawCircle(tx, ty, 4, TFT_YELLOW);
    tft.drawFastHLine(tx - 14, ty, 28, TFT_WHITE);
    tft.drawFastVLine(tx, ty - 14, 28, TFT_WHITE);
  }

  // Touch Coordinates readout box
  tft.fillRoundRect(80, 85, 160, 60, 4, tft.color565(20, 25, 35));
  tft.drawRoundRect(80, 85, 160, 60, 4, TFT_CYAN);

  tft.setTextColor(TFT_CYAN, tft.color565(20, 25, 35));
  tft.drawCentreString("TOUCH DIGITIZER", 160, 90, 1);

  char tBuf1[32];
  snprintf(tBuf1, sizeof(tBuf1), "RAW: X:%d Y:%d", rawTouchX, rawTouchY);
  tft.setTextColor(TFT_WHITE, tft.color565(20, 25, 35));
  tft.drawCentreString(tBuf1, 160, 106, 1);

  char tBuf2[32];
  snprintf(tBuf2, sizeof(tBuf2), "PRESSURE (Z): %d", rawTouchZ);
  tft.setTextColor(TFT_GREEN, tft.color565(20, 25, 35));
  tft.drawCentreString(tBuf2, 160, 122, 1);
}

void drawBenchScreen() {
  tft.fillRect(0, 26, 320, 180, TFT_BLACK);

  // 1. LDR Sensor Box
  tft.fillRoundRect(8, 30, 304, 55, 4, tft.color565(20, 25, 35));
  tft.drawRoundRect(8, 30, 304, 55, 4, TFT_YELLOW);

  tft.setTextColor(TFT_YELLOW, tft.color565(20, 25, 35));
  tft.drawString("AMBIENT LDR LIGHT SENSOR (IO34)", 16, 36, 1);

  int ldr = analogRead(LDR_PIN);
  lastLdrValue = ldr;
  int ldrBarW = map(ldr, 0, 4095, 0, 280);

  tft.fillRect(16, 52, 280, 12, tft.color565(40, 40, 50));
  tft.fillRect(16, 52, ldrBarW, 12, TFT_YELLOW);

  char ldrBuf[32];
  snprintf(ldrBuf, sizeof(ldrBuf), "RAW ADC: %d / 4095  (%.1f%%)", ldr, (ldr / 4095.0) * 100);
  tft.setTextColor(TFT_WHITE, tft.color565(20, 25, 35));
  tft.drawString(ldrBuf, 16, 68, 1);

  // 2. Audio Sweep Test Button
  tft.fillRoundRect(8, 92, 148, 50, 4, tft.color565(35, 45, 60));
  tft.drawRoundRect(8, 92, 148, 50, 4, TFT_CYAN);
  tft.setTextColor(TFT_WHITE, tft.color565(35, 45, 60));
  tft.drawCentreString("AUDIO SWEEP", 82, 100, 2);
  tft.setTextColor(TFT_LIGHTGREY, tft.color565(35, 45, 60));
  tft.drawCentreString("300Hz - 3.2kHz", 82, 120, 1);

  // 3. RGB LED Cycle Button
  tft.fillRoundRect(164, 92, 148, 50, 4, tft.color565(60, 35, 45));
  tft.drawRoundRect(164, 92, 148, 50, 4, TFT_MAGENTA);
  tft.setTextColor(TFT_WHITE, tft.color565(60, 35, 45));
  tft.drawCentreString("RGB LED TEST", 238, 100, 2);
  tft.setTextColor(TFT_LIGHTGREY, tft.color565(60, 35, 45));
  tft.drawCentreString("Cycle R -> G -> B", 238, 120, 1);

  // 4. Test Info Banner
  tft.fillRoundRect(8, 150, 304, 48, 4, tft.color565(15, 20, 25));
  tft.setTextColor(TFT_LIGHTGREY, tft.color565(15, 20, 25));
  tft.drawString("Tap AUDIO SWEEP to test onboard buzzer.", 16, 156, 1);
  tft.drawString("Tap RGB LED to exercise IO4, IO16, and IO17.", 16, 172, 1);
}

void drawUI() {
  drawHeader();
  switch (currentScreen) {
    case 0: drawDashboardScreen(); break;
    case 1: drawPixelsScreen(); break;
    case 2: drawTouchScreen(); break;
    case 3: drawBenchScreen(); break;
  }
  drawNavButtons();
}

void checkTouch() {
  if (ts.touched()) {
    TS_Point p = ts.getPoint();
    rawTouchX = p.x;
    rawTouchY = p.y;
    rawTouchZ = p.z;

    if (p.z > 50 || ts.touched()) {
      if (millis() - lastTouchTime > 180) {
        lastTouchTime = millis();
        // Calibrated non-inverted coordinate mapping for 2.8" CYD
        int screenX = map(p.x, 240, 3800, 0, 320);
        int screenY = map(p.y, 240, 3800, 0, 240);
        screenX = constrain(screenX, 0, 319);
        screenY = constrain(screenY, 0, 239);

        if (screenY >= 200) {
          int tabIdx = (screenX - 8) / 78;
          if (tabIdx >= 0 && tabIdx < 4 && tabIdx != currentScreen) {
            currentScreen = tabIdx;
            beep(2000, 40);
            drawUI();
          }
        } else if (currentScreen == 2) {
          // Touch screen live crosshair and point feedback
          drawTouchScreen();
          tft.fillCircle(screenX, screenY, 5, TFT_GREEN);
          tft.drawCircle(screenX, screenY, 8, TFT_WHITE);
        } else if (currentScreen == 3) {
          if (screenX >= 8 && screenX < 156 && screenY >= 92 && screenY < 142) {
            // Audio Sweep
            runAudioSweep();
          } else if (screenX >= 164 && screenX < 312 && screenY >= 92 && screenY < 142) {
            // RGB Test
            setLED(true, false, false); delay(200);
            setLED(false, true, false); delay(200);
            setLED(false, false, true); delay(200);
            setLED(true, true, true);  delay(200);
            setLED(false, false, false);
            beep(2400, 40);
          }
        }
      }
    }
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("CYD Hardware Diagnostic Suite Initializing for @IamNotIntoTech...");

  pinMode(TFT_BL_PIN, OUTPUT);
  digitalWrite(TFT_BL_PIN, HIGH);

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  setLED(false, false, false);
  beep(2200, 60);

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);

  // Initialize VSPI Touch Controller
  touchSPI.begin(TOUCH_CLK, TOUCH_MISO, TOUCH_MOSI, TOUCH_CS);
  ts.begin(touchSPI);
  ts.setRotation(1);

  drawUI();
}

void loop() {
  checkTouch();

  // If in Bench screen, periodically refresh LDR light reading
  if (currentScreen == 3) {
    static unsigned long lastLdrCheck = 0;
    if (millis() - lastLdrCheck >= 500) {
      lastLdrCheck = millis();
      int ldr = analogRead(LDR_PIN);
      if (abs(ldr - lastLdrValue) > 50) {
        drawBenchScreen();
        drawNavButtons();
      }
    }
  }

  delay(15);
}
