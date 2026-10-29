/*
 * Elecrow 2.8" CYD — Solar & Battery Flow Dashboard
 * Channel: @IamNotIntoTech (https://youtube.com/@IamNotIntoTech)
 * 
 * Target: ESP32-2432S028R (2.8" ILI9341 TFT + XPT2046 Resistive Touch)
 * Features:
 *  - Animated energy flow topology (Solar PV -> Inverter -> Battery / Home / Grid)
 *  - Real-time animated flow particles with directional power arrows
 *  - Two interactive views: Animated Flow Topology & 24H Energy Balance History
 *  - Daily cumulative metrics: Solar Yield, Battery SoC %, Home Draw, Grid Net
 *  - Self-Sufficiency status RGB LED (Green: Off-grid/Exporting, Amber: Battery, Red: Grid import)
 */

#include <Arduino.h>
#include <SPI.h>
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

// UI Palette (16-bit 565)
#define COLOR_BG        0x0842 // Charcoal
#define COLOR_PANEL     0x10A3
#define COLOR_CARD      0x18E5
#define COLOR_CARD_BRD  0x2967
#define COLOR_TEXT      0xFFFF
#define COLOR_MUTED     0x8410
#define COLOR_SOLAR     0xFFE0 // Bright Yellow / Sun
#define COLOR_BATTERY   0x07E0 // Bright Green
#define COLOR_HOME      0x7DDF // Sky Blue
#define COLOR_GRID      0xF81F // Magenta / Purple
#define COLOR_INVERTER  0x07FF // Cyan
#define COLOR_ORANGE    0xFD20
#define COLOR_RED       0xF800

// Energy Telemetry Values (Watts & kWh)
float solarPowerW = 3450.0;    // PV production
float homeLoadW = 1820.0;      // House consumption
float batteryPowerW = 1200.0;  // Positive = Charging, Negative = Discharging
int batterySoC = 84;           // State of charge %
float gridPowerW = 430.0;      // Positive = Exporting, Negative = Importing

float dailySolarKWh = 18.6;
float dailyHomeKWh = 12.4;
float dailyExportKWh = 6.2;
int selfSufficiency = 94; // %

// Navigation & Animation State
int currentView = 0; // 0: Live Flow Diagram, 1: 24H History
int animStep = 0;
unsigned long lastAnimTick = 0;
unsigned long lastSimTick = 0;

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

void updateStatusRGB() {
  if (gridPowerW > 100.0) {
    setRGB(false, true, false); // Green: 100% clean export
  } else if (batteryPowerW < 0.0) {
    setRGB(true, true, false);  // Yellow/Amber: running off battery storage
  } else {
    setRGB(true, false, false); // Red: consuming from utility grid
  }
}

void drawHeader() {
  tft.fillRect(0, 0, 320, 24, COLOR_PANEL);
  tft.drawFastHLine(0, 24, 320, COLOR_CARD_BRD);

  tft.setTextColor(COLOR_SOLAR, COLOR_PANEL);
  tft.setTextSize(1);
  tft.drawString("SOLAR POWER FLOW", 10, 5, 2);

  // Self sufficiency badge
  char selfBuf[24];
  snprintf(selfBuf, sizeof(selfBuf), "SELF-SUFF: %d%%", selfSufficiency);
  tft.setTextColor(COLOR_BATTERY, COLOR_PANEL);
  tft.drawRightString(selfBuf, 310, 5, 2);
}

void drawBottomToolbar() {
  tft.fillRect(0, 206, 320, 34, COLOR_PANEL);
  tft.drawFastHLine(0, 206, 320, COLOR_CARD_BRD);

  // View toggle button (x: 10, w: 90)
  tft.fillRoundRect(8, 210, 94, 26, 4, COLOR_CARD);
  tft.drawRoundRect(8, 210, 94, 26, 4, COLOR_INVERTER);
  tft.setTextColor(COLOR_INVERTER, COLOR_CARD);
  tft.drawCentreString(currentView == 0 ? "24H CHART" : "LIVE FLOW", 55, 215, 1);

  // Daily Solar yield stat
  char solBuf[24];
  snprintf(solBuf, sizeof(solBuf), "YIELD: %.1fkWh", dailySolarKWh);
  tft.setTextColor(COLOR_SOLAR, COLOR_PANEL);
  tft.drawString(solBuf, 114, 215, 1);

  // Net Grid stat
  char gridBuf[24];
  snprintf(gridBuf, sizeof(gridBuf), "GRID: +%.1fkWh", dailyExportKWh);
  tft.setTextColor(COLOR_GRID, COLOR_PANEL);
  tft.drawRightString(gridBuf, 312, 215, 1);
}

void drawFlowNode(int cx, int cy, int w, int h, const char* title, const char* valStr, uint16_t color) {
  int x = cx - w / 2;
  int y = cy - h / 2;
  tft.fillRoundRect(x, y, w, h, 6, COLOR_CARD);
  tft.drawRoundRect(x, y, w, h, 6, color);

  tft.setTextColor(COLOR_MUTED, COLOR_CARD);
  tft.setTextSize(1);
  tft.drawCentreString(title, cx, y + 6, 1);

  tft.setTextColor(color, COLOR_CARD);
  tft.drawCentreString(valStr, cx, y + 20, 2);
}

void drawFlowLine(int x1, int y1, int x2, int y2, uint16_t color, bool forward, int step) {
  tft.drawLine(x1, y1, x2, y2, tft.color565(35, 45, 55));

  // Draw 3 moving packet particles
  for (int p = 0; p < 3; p++) {
    float progress = fmod(((step * 0.05f) + p * 0.33f), 1.0f);
    if (!forward) progress = 1.0f - progress;

    int px = x1 + (int)((x2 - x1) * progress);
    int py = y1 + (int)((y2 - y1) * progress);
    tft.fillCircle(px, py, 3, color);
  }
}

void drawFlowDiagram() {
  tft.fillRect(0, 25, 320, 180, COLOR_BG);

  // Inverter Center: (160, 115)
  int invX = 160;
  int invY = 115;

  // Node Centers
  int solX = 54,  solY = 62;
  int batX = 54,  batY = 168;
  int homeX = 266, homeY = 62;
  int gridX = 266, gridY = 168;

  // Draw animated connecting lines
  // Solar -> Inverter (forward)
  drawFlowLine(solX + 38, solY, invX - 34, invY - 10, COLOR_SOLAR, true, animStep);

  // Inverter <-> Battery
  bool batCharging = (batteryPowerW >= 0);
  drawFlowLine(invX - 34, invY + 10, batX + 38, batY, COLOR_BATTERY, !batCharging, animStep);

  // Inverter -> Home
  drawFlowLine(invX + 34, invY - 10, homeX - 38, homeY, COLOR_HOME, true, animStep);

  // Inverter <-> Grid
  bool exporting = (gridPowerW >= 0);
  drawFlowLine(invX + 34, invY + 10, gridX - 38, gridY, COLOR_GRID, exporting, animStep);

  // Draw Central Hub: Inverter
  char invBuf[16];
  snprintf(invBuf, sizeof(invBuf), "%.1f kW", (solarPowerW + abs(batteryPowerW)) / 1000.0);
  drawFlowNode(invX, invY, 68, 44, "INVERTER", invBuf, COLOR_INVERTER);

  // Draw Solar Node
  char solBuf[16];
  snprintf(solBuf, sizeof(solBuf), "%.1f kW", solarPowerW / 1000.0);
  drawFlowNode(solX, solY, 76, 42, "SOLAR PV", solBuf, COLOR_SOLAR);

  // Draw Battery Node
  char batBuf[24];
  snprintf(batBuf, sizeof(batBuf), "%d%% (%.1fkW)", batterySoC, abs(batteryPowerW) / 1000.0);
  drawFlowNode(batX, batY, 76, 42, "BATTERY", batBuf, COLOR_BATTERY);

  // Draw Home Load Node
  char homeBuf[16];
  snprintf(homeBuf, sizeof(homeBuf), "%.1f kW", homeLoadW / 1000.0);
  drawFlowNode(homeX, homeY, 76, 42, "HOME LOAD", homeBuf, COLOR_HOME);

  // Draw Grid Node
  char gridBuf[16];
  snprintf(gridBuf, sizeof(gridBuf), "%s%.1f kW", (gridPowerW >= 0 ? "+" : "-"), abs(gridPowerW) / 1000.0);
  drawFlowNode(gridX, gridY, 76, 42, "GRID NET", gridBuf, COLOR_GRID);
}

void drawHistoryChart() {
  tft.fillRect(0, 25, 320, 180, COLOR_BG);

  tft.setTextColor(COLOR_TEXT, COLOR_BG);
  tft.setTextSize(1);
  tft.drawString("24-HOUR ENERGY BALANCE (KW)", 14, 32, 2);

  // Bar Graph area: x: 20, y: 55, w: 280, h: 115
  int gx = 20;
  int gy = 55;
  int gw = 280;
  int gh = 115;

  tft.drawFastHLine(gx, gy + gh - 1, gw, COLOR_CARD_BRD);

  // 12 2-hour interval bars
  const float solarHist[12] = { 0, 0, 0.4, 1.8, 3.5, 4.2, 4.0, 3.1, 1.6, 0.3, 0, 0 };
  const float homeHist[12]  = { 0.8, 0.6, 0.9, 1.4, 1.2, 1.5, 1.3, 1.7, 2.1, 2.4, 1.8, 1.1 };

  int barW = 10;
  int groupW = 23;

  for (int i = 0; i < 12; i++) {
    int bx = gx + 6 + i * groupW;

    // Solar bar (Yellow)
    int sH = map(solarHist[i] * 10, 0, 50, 0, gh - 15);
    tft.fillRect(bx, gy + gh - sH, barW, sH, COLOR_SOLAR);

    // Home Load bar (Blue)
    int hH = map(homeHist[i] * 10, 0, 50, 0, gh - 15);
    tft.fillRect(bx + barW + 2, gy + gh - hH, barW, hH, COLOR_HOME);

    // Time Label
    if (i % 2 == 0) {
      char timeBuf[8];
      snprintf(timeBuf, sizeof(timeBuf), "%02d", i * 2);
      tft.setTextColor(COLOR_MUTED, COLOR_BG);
      tft.drawCentreString(timeBuf, bx + barW, gy + gh + 4, 1);
    }
  }

  // Legend
  tft.fillRect(180, 32, 8, 8, COLOR_SOLAR);
  tft.setTextColor(COLOR_MUTED, COLOR_BG);
  tft.drawString("Solar", 192, 32, 1);

  tft.fillRect(240, 32, 8, 8, COLOR_HOME);
  tft.drawString("Home", 252, 32, 1);
}

void renderCurrentView() {
  if (currentView == 0) {
    drawFlowDiagram();
  } else {
    drawHistoryChart();
  }
  drawBottomToolbar();
  updateStatusRGB();
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

  drawHeader();
  renderCurrentView();

  playBeep(2200, 60);
  Serial.println(F("[SYSTEM] Solar & Battery Flow Dashboard Initialized."));
}

void loop() {
  // Animate flow packets (~15 FPS)
  if (millis() - lastAnimTick > 65) {
    lastAnimTick = millis();
    animStep = (animStep + 1) % 100;
    if (currentView == 0) {
      drawFlowDiagram();
    }
  }

  // Simulation drift cycle (every 3 seconds)
  if (millis() - lastSimTick > 3000) {
    lastSimTick = millis();
    solarPowerW = constrain(solarPowerW + random(-150, 160), 1200.0f, 4800.0f);
    homeLoadW = constrain(homeLoadW + random(-100, 120), 800.0f, 3200.0f);
    float excess = solarPowerW - homeLoadW;
    if (excess > 0) {
      batteryPowerW = min(excess * 0.7f, 2500.0f);
      gridPowerW = excess - batteryPowerW;
    } else {
      batteryPowerW = max(excess, -2800.0f);
      gridPowerW = excess - batteryPowerW;
    }
    updateStatusRGB();
    drawBottomToolbar();
  }

  // Touch Handling
  if (ts.touched()) {
    TS_Point p = ts.getPoint();
    int touchX = map(p.x, 3800, 300, 0, 320);
    int touchY = map(p.y, 3800, 400, 0, 240);
    touchX = constrain(touchX, 0, 320);
    touchY = constrain(touchY, 0, 240);

    // Bottom left view toggle button
    if (touchY >= 205 && touchX <= 110) {
      currentView = (currentView == 0) ? 1 : 0;
      playBeep(2000, 40);
      renderCurrentView();
      delay(200);
    }
  }

  delay(15);
}
