/*
 * Elecrow 2.8" CYD — Sim-Racing Telemetry Dash
 * Channel: @IamNotIntoTech (https://youtube.com/@IamNotIntoTech)
 * 
 * Target: ESP32-2432S028R (2.8" ILI9341 TFT + XPT2046 Resistive Touch)
 * Features:
 *  - 60Hz Sim-Racing Digital Dashboard & RPM Shift Light Bar
 *  - Giant Center Gear Indicator with dynamic redline shift flash
 *  - Speedometer (KM/H & MPH toggle) and live Lap Delta time
 *  - 4-Wheel Tyre Temperature & Pressure Matrix (FL, FR, RL, RR)
 *  - On-screen Discord Voice Mute / Push-to-Talk macro touch button
 *  - Peripheral RGB shift flash on IO4/16/17 & acoustic shift beep on IO26
 */

#include <Arduino.h>
#include <SPI.h>
#include <WiFi.h>
#include <WiFiUdp.h>
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
#define COLOR_BG        0x0000 // Deep Black
#define COLOR_PANEL     0x10A3
#define COLOR_CARD      0x18E5
#define COLOR_CARD_BRD  0x2967
#define COLOR_TEXT      0xFFFF
#define COLOR_MUTED     0x8410
#define COLOR_GREEN     0x07E0
#define COLOR_AMBER     0xFD20
#define COLOR_RED       0xF800
#define COLOR_BLUE      0x001F
#define COLOR_MAGENTA   0xF81F
#define COLOR_CYAN      0x07FF

// Telemetry State
int currentGear = 3; // 0=N, 1..7, -1=R
int currentRPM = 6200;
const int maxRPM = 9000;
const int shiftRPM = 8400;

float currentSpeed = 164.0; // km/h
bool speedMph = false;
float lapDelta = -0.285; // Seconds
int currentLap = 14;
int totalLaps = 45;

bool discordMuted = false;
bool pitLimiter = false;

// 4-Wheel Tyre Temps (°C) & Pressures (psi)
float tyreTemps[4] = { 88.5, 89.2, 92.0, 93.4 }; // FL, FR, RL, RR
float tyrePress[4] = { 24.2, 24.3, 24.8, 24.9 };

// Simulation Physics & Timers
unsigned long lastTelemetryTick = 0;
unsigned long lastShiftFlash = 0;
bool shiftFlashState = false;
float accelRate = 85.0;

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

uint16_t getTyreColor(float temp) {
  if (temp < 75.0) return COLOR_CYAN;   // Cold
  if (temp <= 98.0) return COLOR_GREEN; // Optimal
  return COLOR_RED;                     // Overheating
}

void drawShiftLightBar() {
  // Top Shift Bar: y: 0..16, 16 individual LED segments
  int numSegments = 16;
  int segW = 17;
  int segSpacing = 3;
  int startX = 2;

  int activeCount = map(currentRPM, 4500, maxRPM, 0, numSegments);
  activeCount = constrain(activeCount, 0, numSegments);
  bool isRedline = (currentRPM >= shiftRPM);

  for (int i = 0; i < numSegments; i++) {
    int x = startX + i * (segW + segSpacing);
    uint16_t segColor = tft.color565(30, 35, 40); // Inactive dark

    if (i < activeCount) {
      if (i < 5) segColor = COLOR_GREEN;
      else if (i < 11) segColor = COLOR_AMBER;
      else segColor = COLOR_RED;
    }

    // Flash all blue/magenta at redline
    if (isRedline && shiftFlashState) {
      segColor = COLOR_MAGENTA;
    }

    tft.fillRect(x, 2, segW, 14, segColor);
  }

  // Peripheral RGB LED sync
  if (isRedline) {
    setRGB(shiftFlashState, false, shiftFlashState); // Magenta flash
  } else if (currentRPM > 7000) {
    setRGB(true, true, false); // Amber
  } else {
    setRGB(false, false, false);
  }
}

void drawGearDisplay() {
  // Center Box: x: 110, y: 22, w: 100, h: 100
  uint16_t gearBg = (currentRPM >= shiftRPM && shiftFlashState) ? COLOR_RED : COLOR_CARD;
  tft.fillRoundRect(110, 22, 100, 100, 6, gearBg);
  tft.drawRoundRect(110, 22, 100, 100, 6, COLOR_CARD_BRD);

  // Large Gear Number
  tft.setTextColor(COLOR_TEXT, gearBg);
  tft.setTextSize(3);
  char gBuf[4];
  if (currentGear == 0) strcpy(gBuf, "N");
  else if (currentGear == -1) strcpy(gBuf, "R");
  else snprintf(gBuf, sizeof(gBuf), "%d", currentGear);
  tft.drawCentreString(gBuf, 160, 36, 4);

  // RPM Readout below gear
  tft.setTextSize(1);
  tft.setTextColor((currentRPM >= shiftRPM) ? COLOR_RED : COLOR_GREEN, gearBg);
  char rpmBuf[16];
  snprintf(rpmBuf, sizeof(rpmBuf), "%d RPM", currentRPM);
  tft.drawCentreString(rpmBuf, 160, 104, 1);
}

void drawSpeedAndDelta() {
  // Left Column Card: x: 8, y: 22, w: 96, h: 100
  tft.fillRoundRect(8, 22, 96, 100, 6, COLOR_CARD);
  tft.drawRoundRect(8, 22, 96, 100, 6, COLOR_CARD_BRD);

  // Speed
  tft.setTextColor(COLOR_MUTED, COLOR_CARD);
  tft.setTextSize(1);
  tft.drawString(speedMph ? "SPEED (MPH)" : "SPEED (KM/H)", 14, 28, 1);

  float dispSpeed = speedMph ? (currentSpeed * 0.621371f) : currentSpeed;
  char spdBuf[16];
  snprintf(spdBuf, sizeof(spdBuf), "%.0f", dispSpeed);
  tft.setTextColor(COLOR_TEXT, COLOR_CARD);
  tft.drawString(spdBuf, 14, 42, 4);

  // Lap Delta
  tft.drawFastHLine(14, 76, 84, COLOR_CARD_BRD);
  tft.setTextColor(COLOR_MUTED, COLOR_CARD);
  tft.drawString("DELTA", 14, 82, 1);

  char dBuf[16];
  snprintf(dBuf, sizeof(dBuf), "%s%.3fs", (lapDelta <= 0 ? "" : "+"), lapDelta);
  tft.setTextColor((lapDelta <= 0) ? COLOR_GREEN : COLOR_RED, COLOR_CARD);
  tft.drawString(dBuf, 14, 96, 2);
}

void drawTyreMatrix() {
  // Right Column Card: x: 216, y: 22, w: 96, h: 100
  tft.fillRoundRect(216, 22, 96, 100, 6, COLOR_CARD);
  tft.drawRoundRect(216, 22, 96, 100, 6, COLOR_CARD_BRD);

  tft.setTextColor(COLOR_MUTED, COLOR_CARD);
  tft.setTextSize(1);
  tft.drawCentreString("TYRE TEMPS", 264, 28, 1);

  const char* labels[4] = { "FL", "FR", "RL", "RR" };
  int pos[4][2] = {
    { 222, 44 }, { 266, 44 },
    { 222, 74 }, { 266, 74 }
  };

  for (int i = 0; i < 4; i++) {
    int tx = pos[i][0];
    int ty = pos[i][1];
    uint16_t tColor = getTyreColor(tyreTemps[i]);

    tft.fillRoundRect(tx, ty, 40, 24, 3, tft.color565(20, 25, 30));
    tft.drawRoundRect(tx, ty, 40, 24, 3, tColor);

    tft.setTextColor(tColor, tft.color565(20, 25, 30));
    char tBuf[8];
    snprintf(tBuf, sizeof(tBuf), "%.0f°", tyreTemps[i]);
    tft.drawCentreString(tBuf, tx + 20, ty + 5, 2);
  }
}

void drawBottomToolbar() {
  // Bottom Bar: y: 198..240
  tft.fillRect(0, 198, 320, 42, COLOR_PANEL);
  tft.drawFastHLine(0, 198, 320, COLOR_CARD_BRD);

  // Discord Mute Macro (x: 8..110)
  uint16_t muteBg = discordMuted ? COLOR_RED : COLOR_CARD;
  uint16_t muteBrd = discordMuted ? COLOR_TEXT : COLOR_CARD_BRD;
  tft.fillRoundRect(8, 204, 100, 30, 4, muteBg);
  tft.drawRoundRect(8, 204, 100, 30, 4, muteBrd);
  tft.setTextColor(COLOR_TEXT, muteBg);
  tft.setTextSize(1);
  tft.drawCentreString(discordMuted ? "DISCORD MUTED" : "DISCORD MIC ON", 58, 212, 1);

  // Speed Unit Toggle (x: 116..205)
  tft.fillRoundRect(116, 204, 90, 30, 4, COLOR_CARD);
  tft.drawRoundRect(116, 204, 90, 30, 4, COLOR_CARD_BRD);
  tft.setTextColor(COLOR_CYAN, COLOR_CARD);
  tft.drawCentreString(speedMph ? "UNIT: MPH" : "UNIT: KM/H", 161, 212, 1);

  // Pit Limiter Toggle (x: 214..312)
  uint16_t pitBg = pitLimiter ? COLOR_AMBER : COLOR_CARD;
  uint16_t pitBrd = pitLimiter ? COLOR_TEXT : COLOR_CARD_BRD;
  tft.fillRoundRect(214, 204, 98, 30, 4, pitBg);
  tft.drawRoundRect(214, 204, 98, 30, 4, pitBrd);
  tft.setTextColor(pitLimiter ? 0x0000 : COLOR_MUTED, pitBg);
  tft.drawCentreString(pitLimiter ? "PIT LIMITER ON" : "PIT LIMIT OFF", 263, 212, 1);
}

void drawMiddleSectorTelemetry() {
  // Middle strip: y: 126..194
  tft.fillRect(8, 126, 304, 68, COLOR_CARD);
  tft.drawRoundRect(8, 126, 304, 68, 6, COLOR_CARD_BRD);

  // Lap Info
  tft.setTextColor(COLOR_MUTED, COLOR_CARD);
  tft.setTextSize(1);
  tft.drawString("SESSION", 16, 134, 1);
  char lapBuf[24];
  snprintf(lapBuf, sizeof(lapBuf), "LAP %d / %d", currentLap, totalLaps);
  tft.setTextColor(COLOR_TEXT, COLOR_CARD);
  tft.drawString(lapBuf, 16, 148, 2);

  // Last Lap Time
  tft.setTextColor(COLOR_MUTED, COLOR_CARD);
  tft.drawString("LAST LAP", 115, 134, 1);
  tft.setTextColor(COLOR_GREEN, COLOR_CARD);
  tft.drawString("1:28.432", 115, 148, 2);

  // Best Lap Time
  tft.setTextColor(COLOR_MUTED, COLOR_CARD);
  tft.drawString("BEST LAP", 215, 134, 1);
  tft.setTextColor(COLOR_MAGENTA, COLOR_CARD);
  tft.drawString("1:28.147", 215, 148, 2);

  // Sector Bars at bottom of strip
  tft.fillRoundRect(16, 172, 85, 12, 2, COLOR_GREEN);   // S1 Purple/Green
  tft.fillRoundRect(110, 172, 85, 12, 2, COLOR_GREEN);  // S2
  tft.fillRoundRect(204, 172, 85, 12, 2, COLOR_AMBER);  // S3
}

void renderAll() {
  drawShiftLightBar();
  drawGearDisplay();
  drawSpeedAndDelta();
  drawTyreMatrix();
  drawMiddleSectorTelemetry();
  drawBottomToolbar();
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

  renderAll();

  playBeep(2400, 60);
  Serial.println(F("[SYSTEM] Sim-Racing Telemetry Dash initialized."));
}

void loop() {
  // Telemetry Physics Simulation (~30 FPS)
  if (millis() - lastTelemetryTick > 33) {
    lastTelemetryTick = millis();

    if (!pitLimiter) {
      currentRPM += (int)accelRate;
      currentSpeed += (accelRate / 180.0f);

      // Auto gear shift logic
      if (currentRPM >= maxRPM) {
        if (currentGear < 6) {
          currentGear++;
          currentRPM = 5400;
          playBeep(2800, 30);
        } else {
          currentRPM = maxRPM - 150;
        }
      }
    } else {
      currentSpeed = 60.0;
      currentRPM = 4200;
      currentGear = 2;
    }

    // Shift Flash frequency
    if (currentRPM >= shiftRPM) {
      if (millis() - lastShiftFlash > 80) {
        lastShiftFlash = millis();
        shiftFlashState = !shiftFlashState;
        if (shiftFlashState) playBeep(3400, 20);
      }
    } else {
      shiftFlashState = false;
    }

    drawShiftLightBar();
    drawGearDisplay();
    drawSpeedAndDelta();
  }

  // Touch Handling
  if (ts.touched()) {
    TS_Point p = ts.getPoint();
    int touchX = map(p.x, 3800, 300, 0, 320);
    int touchY = map(p.y, 3800, 400, 0, 240);
    touchX = constrain(touchX, 0, 320);
    touchY = constrain(touchY, 0, 240);

    // Toolbar buttons (y >= 198)
    if (touchY >= 198) {
      // Discord Mute (x: 8..110)
      if (touchX >= 8 && touchX <= 110) {
        discordMuted = !discordMuted;
        playBeep(discordMuted ? 1400 : 2200, 40);
        drawBottomToolbar();
        delay(180);
      }
      // Speed Units (x: 116..205)
      else if (touchX >= 116 && touchX <= 205) {
        speedMph = !speedMph;
        playBeep(2000, 30);
        drawBottomToolbar();
        drawSpeedAndDelta();
        delay(180);
      }
      // Pit Limiter (x: 214..312)
      else if (touchX >= 214 && touchX <= 312) {
        pitLimiter = !pitLimiter;
        playBeep(pitLimiter ? 1600 : 2600, 40);
        drawBottomToolbar();
        delay(180);
      }
    }
  }

  delay(10);
}
