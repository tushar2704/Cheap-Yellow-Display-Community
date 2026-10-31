/**
 * 🏎️ Project 38: Sim-Racing Telemetry Dash
 * 
 * Hardware: Elecrow CrowPanel 2.8" (ESP32-2432S028R)
 * Channel Branding: IamNotIntoTech (https://youtube.com/@IamNotIntoTech)
 * Author: Tushar Aggarwal (tushar2704)
 * 
 * Features:
 * - High-speed digital telemetry dashboard with 16-segment RPM shift light bar.
 * - Giant center gear indicator with dynamic redline flash and acoustic shift beep.
 * - Digital speedometer (KM/H & MPH toggle) with millisecond-precision lap delta.
 * - 4-Wheel tyre temperature thermal matrix (FL, FR, RL, RR).
 * - Interactive touch deck: Discord voice mute macro, speed unit toggle, pit limiter.
 * - Active-low RGB peripheral shift light synchronizing to redline rev limit.
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
#define COLOR_BG        0x0000 // Deep Black
#define COLOR_PANEL     0x10A3
#define COLOR_CARD      0x18E5
#define COLOR_CARD_BRD  0x2967
#define COLOR_TEXT      0xFFFF
#define COLOR_MUTED     0x8410
#define COLOR_GREEN     0x07E0
#define COLOR_AMBER     0xFD20
#define COLOR_RED       0xF800
#define COLOR_CYAN      0x07FF
#define COLOR_BLUE      0x001F

// Telemetry State
int currentRPM = 6200;
int maxRPM = 8500;
int shiftRPM = 8000;
int currentGear = 4; // -1: R, 0: N, 1..7
float currentSpeed = 184.0; // km/h
float lapDelta = -0.342;    // Negative = faster, Positive = slower
float tyreTemps[4] = { 88.5, 91.2, 84.0, 86.5 }; // FL, FR, RL, RR

bool speedMph = false;
bool discordMuted = false;
bool pitLimiter = false;
bool shiftFlashState = false;

unsigned long lastTick = 0;
unsigned long lastShiftFlash = 0;
unsigned long lastTouchTime = 0;

void playBeep(int freq = 2000, int ms = 30) {
  tone(BUZZER_PIN, freq, ms);
}

void setRGB(bool r, bool g, bool b) {
  digitalWrite(LED_RED, r ? LOW : HIGH);
  digitalWrite(LED_GREEN, g ? LOW : HIGH);
  digitalWrite(LED_BLUE, b ? LOW : HIGH);
}

uint16_t getTyreColor(float temp) {
  if (temp < 75.0) return COLOR_BLUE;       // Cold
  if (temp <= 95.0) return COLOR_GREEN;     // Optimal
  if (temp <= 105.0) return COLOR_AMBER;    // Warm
  return COLOR_RED;                         // Overheating
}

void drawRPMBar() {
  int numSegments = 16;
  int segW = 17;
  int segSpacing = 3;
  int startX = 2;

  int activeCount = map(currentRPM, 4500, maxRPM, 0, numSegments);
  activeCount = constrain(activeCount, 0, numSegments);
  bool isRedline = (currentRPM >= shiftRPM);

  for (int i = 0; i < numSegments; i++) {
    int x = startX + i * (segW + segSpacing);
    uint16_t segColor;

    if (i >= activeCount) {
      segColor = tft.color565(25, 25, 30);
    } else {
      if (isRedline && shiftFlashState) {
        segColor = COLOR_RED;
      } else {
        if (i < 6) segColor = COLOR_GREEN;
        else if (i < 12) segColor = COLOR_AMBER;
        else segColor = COLOR_RED;
      }
    }

    tft.fillRect(x, 2, segW, 14, segColor);
  }

  // Peripheral RGB LED sync
  if (isRedline) {
    setRGB(shiftFlashState, false, shiftFlashState);
  } else if (currentRPM > 7000) {
    setRGB(true, true, false);
  } else {
    setRGB(false, false, false);
  }
}

void drawGearDisplay() {
  uint16_t gearBg = (currentRPM >= shiftRPM && shiftFlashState) ? COLOR_RED : COLOR_CARD;
  tft.fillRoundRect(110, 22, 100, 100, 6, gearBg);
  tft.drawRoundRect(110, 22, 100, 100, 6, COLOR_CARD_BRD);

  tft.setTextColor(COLOR_TEXT, gearBg);
  char gBuf[4];
  if (currentGear == 0) strcpy(gBuf, "N");
  else if (currentGear == -1) strcpy(gBuf, "R");
  else snprintf(gBuf, sizeof(gBuf), "%d", currentGear);
  tft.drawCentreString(gBuf, 160, 36, 6);

  tft.setTextColor((currentRPM >= shiftRPM) ? COLOR_RED : COLOR_GREEN, gearBg);
  char rpmBuf[16];
  snprintf(rpmBuf, sizeof(rpmBuf), "%d RPM", currentRPM);
  tft.drawCentreString(rpmBuf, 160, 104, 1);
}

void drawSpeedAndDelta() {
  tft.fillRoundRect(8, 22, 96, 100, 6, COLOR_CARD);
  tft.drawRoundRect(8, 22, 96, 100, 6, COLOR_CARD_BRD);

  tft.setTextColor(COLOR_MUTED, COLOR_CARD);
  tft.drawString(speedMph ? "SPEED (MPH)" : "SPEED (KM/H)", 14, 28, 1);

  float dispSpeed = speedMph ? (currentSpeed * 0.621371f) : currentSpeed;
  char spdBuf[16];
  snprintf(spdBuf, sizeof(spdBuf), "%.0f", dispSpeed);
  tft.setTextColor(COLOR_TEXT, COLOR_CARD);
  tft.drawString(spdBuf, 14, 42, 4);

  tft.drawFastHLine(14, 76, 84, COLOR_CARD_BRD);
  tft.setTextColor(COLOR_MUTED, COLOR_CARD);
  tft.drawString("DELTA", 14, 82, 1);

  char dBuf[16];
  snprintf(dBuf, sizeof(dBuf), "%s%.3fs", (lapDelta <= 0 ? "" : "+"), lapDelta);
  tft.setTextColor((lapDelta <= 0) ? COLOR_GREEN : COLOR_RED, COLOR_CARD);
  tft.drawString(dBuf, 14, 96, 2);
}

void drawTyreMatrix() {
  tft.fillRoundRect(216, 22, 96, 100, 6, COLOR_CARD);
  tft.drawRoundRect(216, 22, 96, 100, 6, COLOR_CARD_BRD);

  tft.setTextColor(COLOR_MUTED, COLOR_CARD);
  tft.drawCentreString("TYRE TEMPS", 264, 28, 1);

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
    snprintf(tBuf, sizeof(tBuf), "%.0f", tyreTemps[i]);
    tft.drawCentreString(tBuf, tx + 20, ty + 5, 2);
  }
}

void drawTelemetryMidRow() {
  // Center status row between dashboard and toolbar
  tft.fillRoundRect(8, 130, 304, 62, 6, COLOR_CARD);
  tft.drawRoundRect(8, 130, 304, 62, 6, COLOR_CARD_BRD);

  tft.setTextColor(COLOR_CYAN, COLOR_CARD);
  tft.drawString("IamNotIntoTech", 18, 138, 2);

  tft.setTextColor(COLOR_MUTED, COLOR_CARD);
  tft.drawRightString("SIM-RACING TELEMETRY", 300, 138, 1);

  // Status metrics
  char lapBuf[32];
  snprintf(lapBuf, sizeof(lapBuf), "LAP 14/52 | POS P2");
  tft.setTextColor(COLOR_TEXT, COLOR_CARD);
  tft.drawString(lapBuf, 18, 162, 2);

  char timeBuf[32];
  snprintf(timeBuf, sizeof(timeBuf), "LAST: 1:28.412");
  tft.setTextColor(COLOR_GREEN, COLOR_CARD);
  tft.drawRightString(timeBuf, 300, 162, 2);
}

void drawBottomToolbar() {
  tft.fillRect(0, 198, 320, 42, COLOR_PANEL);
  tft.drawFastHLine(0, 198, 320, COLOR_CARD_BRD);

  // Discord Mute Macro (x: 8..110)
  uint16_t muteBg = discordMuted ? COLOR_RED : COLOR_CARD;
  uint16_t muteBrd = discordMuted ? COLOR_TEXT : COLOR_CARD_BRD;
  tft.fillRoundRect(8, 204, 100, 30, 4, muteBg);
  tft.drawRoundRect(8, 204, 100, 30, 4, muteBrd);
  tft.setTextColor(COLOR_TEXT, muteBg);
  tft.drawCentreString(discordMuted ? "DISCORD MUTED" : "DISCORD ON", 58, 212, 1);

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
  tft.drawCentreString(pitLimiter ? "PIT LIMITER ON" : "PIT LIMITER OFF", 263, 212, 1);
}

void renderAll() {
  drawRPMBar();
  drawGearDisplay();
  drawSpeedAndDelta();
  drawTyreMatrix();
  drawTelemetryMidRow();
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

  pinMode(TFT_BL_PIN, OUTPUT);
  digitalWrite(TFT_BL_PIN, HIGH);

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(COLOR_BG);

  // Initialize Touch (VSPI)
  touchSPI.begin(TOUCH_CLK, TOUCH_MISO, TOUCH_MOSI, TOUCH_CS);
  ts.begin(touchSPI);
  ts.setRotation(1);

  renderAll();

  playBeep(2400, 60);
  Serial.println(F("[SYSTEM] Sim-Racing Telemetry Dash Ready."));
}

void loop() {
  // Update physics simulation (every 50ms)
  if (millis() - lastTick > 50) {
    lastTick = millis();

    if (!pitLimiter) {
      currentRPM += 120;
      if (currentRPM > maxRPM) {
        currentRPM = 5200;
        currentGear = (currentGear >= 6) ? 1 : currentGear + 1;
        playBeep(2800, 40); // Shift chirp
      }
      currentSpeed = 60.0 + (currentGear * 32.0) + (currentRPM - 5000) * 0.015;
    } else {
      currentRPM = 4800 + random(-80, 80);
      currentSpeed = 60.0;
      currentGear = 2;
    }

    // Shift Flash cadence
    if (currentRPM >= shiftRPM) {
      if (millis() - lastShiftFlash > 80) {
        lastShiftFlash = millis();
        shiftFlashState = !shiftFlashState;
      }
    } else {
      shiftFlashState = false;
    }

    drawRPMBar();
    drawGearDisplay();
    drawSpeedAndDelta();
  }

  // Touch Handling
  if (ts.touched()) {
    TS_Point p = ts.getPoint();
    if (p.z > 50 || ts.touched()) {
      if (millis() - lastTouchTime > 180) {
        lastTouchTime = millis();
        int screenX = map(p.x, 240, 3800, 0, 320);
        int screenY = map(p.y, 240, 3800, 0, 240);
        screenX = constrain(screenX, 0, 319);
        screenY = constrain(screenY, 0, 239);

        // Toolbar buttons (screenY >= 198)
        if (screenY >= 198) {
          // Discord Mute (x: 8..110)
          if (screenX >= 8 && screenX <= 110) {
            discordMuted = !discordMuted;
            playBeep(discordMuted ? 1400 : 2200, 40);
            drawBottomToolbar();
          }
          // Speed Units (x: 116..205)
          else if (screenX >= 116 && screenX <= 205) {
            speedMph = !speedMph;
            playBeep(2000, 30);
            drawBottomToolbar();
            drawSpeedAndDelta();
          }
          // Pit Limiter (x: 214..312)
          else if (screenX >= 214 && screenX <= 312) {
            pitLimiter = !pitLimiter;
            playBeep(pitLimiter ? 1600 : 2600, 40);
            drawBottomToolbar();
          }
        }
      }
    }
  }

  delay(10);
}
