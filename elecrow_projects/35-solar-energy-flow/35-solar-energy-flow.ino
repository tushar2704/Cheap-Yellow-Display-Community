/**
 * ☀️ Project 35: Solar & Battery Flow Dashboard
 * 
 * Hardware: Elecrow CrowPanel 2.8" (ESP32-2432S028R)
 * Channel Branding: IamNotIntoTech (https://youtube.com/@IamNotIntoTech)
 * Author: Tushar Aggarwal (tushar2704)
 * 
 * Features:
 * - Real-time animated energy flow topology (Solar PV -> Inverter -> Battery / Home Load / Grid).
 * - Continuous moving energy packet physics showing directional power routing.
 * - Dual view modes: Animated Topology Network and 24-Hour Energy Balance bar chart.
 * - Daily cumulative analytics: Solar Yield, Battery SoC %, Home Draw, and Grid Net.
 * - Active-low RGB status LED displaying grid self-sufficiency level in real-time.
 * - Calibrated non-inverted resistive touch controls with acoustic feedback clicks.
 */

#include <Arduino.h>
#include <SPI.h>
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
#define COLOR_GRID      0xFD20 // Orange
#define COLOR_INVERTER  0xCD3E // Mauve / Violet
#define COLOR_CYAN      0x07FF

// Real-Time Energy State (Simulated / MQTT Bridge ready)
int solarPowerW = 3850;
int homeLoadW = 1450;
int batteryPowerW = -1200; // Negative = Charging, Positive = Discharging
int batterySoC = 84;       // State of charge %
int gridPowerW = 1200;     // Positive = Exporting, Negative = Importing

float dailySolarKWh = 21.4;
float dailyExportKWh = 8.2;
float dailyHomeKWh = 12.6;

// 12-Interval 24H History (Every 2 hours)
float solarHist[12] = { 0.0, 0.0, 0.0, 0.8, 2.5, 4.2, 4.8, 4.1, 2.8, 1.2, 0.2, 0.0 };
float homeHist[12]  = { 0.6, 0.5, 0.5, 0.9, 1.8, 1.4, 1.2, 1.6, 2.4, 2.8, 1.9, 0.8 };

int currentView = 0; // 0 = Flow Diagram, 1 = 24H History
int animStep = 0;
unsigned long lastAnimTick = 0;
unsigned long lastSimTick = 0;
unsigned long lastTouchTime = 0;

void playBeep(int freq = 2000, int ms = 30) {
  tone(BUZZER_PIN, freq, ms);
}

void setRGB(bool r, bool g, bool b) {
  digitalWrite(LED_RED, r ? LOW : HIGH);
  digitalWrite(LED_GREEN, g ? LOW : HIGH);
  digitalWrite(LED_BLUE, b ? LOW : HIGH);
}

void updateSelfSufficiencyLED() {
  if (gridPowerW >= 0) {
    setRGB(false, true, false); // Green: 100% self-sufficient / exporting
  } else if (batteryPowerW > 0) {
    setRGB(true, true, false);  // Yellow/Amber: Supported by battery
  } else {
    setRGB(true, false, false); // Red: Dependent on grid imports
  }
}

void drawHeader() {
  tft.fillRect(0, 0, 320, 24, COLOR_PANEL);
  tft.drawFastHLine(0, 24, 320, COLOR_CARD_BRD);

  tft.setTextColor(COLOR_CYAN, COLOR_PANEL);
  tft.drawString("IamNotIntoTech", 8, 5, 2);

  tft.setTextColor(COLOR_TEXT, COLOR_PANEL);
  tft.drawCentreString("SOLAR & BATTERY FLOW", 175, 5, 2);

  // Self sufficiency percentage badge
  int sufficiency = constrain((int)(((float)(solarPowerW + max(0, batteryPowerW)) / homeLoadW) * 100.0), 0, 100);
  char sufBuf[16];
  snprintf(sufBuf, sizeof(sufBuf), "%d%% AUT", sufficiency);
  tft.setTextColor(COLOR_BATTERY, COLOR_PANEL);
  tft.drawRightString(sufBuf, 312, 5, 2);
}

void drawBottomBar() {
  tft.fillRect(0, 206, 320, 34, COLOR_PANEL);
  tft.drawFastHLine(0, 206, 320, COLOR_CARD_BRD);

  // Toggle View Button
  tft.fillRoundRect(8, 209, 102, 28, 4, COLOR_CARD);
  tft.drawRoundRect(8, 209, 102, 28, 4, COLOR_CARD_BRD);
  tft.setTextColor(COLOR_TEXT, COLOR_CARD);
  tft.drawCentreString(currentView == 0 ? "VIEW: HISTORY" : "VIEW: FLOW", 59, 215, 1);

  // Daily Solar stat
  char solBuf[24];
  snprintf(solBuf, sizeof(solBuf), "YIELD: %.1fkWh", dailySolarKWh);
  tft.setTextColor(COLOR_SOLAR, COLOR_PANEL);
  tft.drawString(solBuf, 118, 215, 1);

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

  int invX = 160;
  int invY = 115;

  int solX = 54,  solY = 62;
  int batX = 54,  batY = 168;
  int homeX = 266, homeY = 62;
  int gridX = 266, gridY = 168;

  // Flow Lines
  drawFlowLine(solX + 38, solY, invX - 34, invY - 10, COLOR_SOLAR, true, animStep);

  bool batCharging = (batteryPowerW <= 0);
  drawFlowLine(invX - 34, invY + 10, batX + 38, batY, COLOR_BATTERY, batCharging, animStep);

  drawFlowLine(invX + 34, invY - 10, homeX - 38, homeY, COLOR_HOME, true, animStep);

  bool exporting = (gridPowerW >= 0);
  drawFlowLine(invX + 34, invY + 10, gridX - 38, gridY, COLOR_GRID, exporting, animStep);

  // Central Inverter Node
  char invBuf[16];
  snprintf(invBuf, sizeof(invBuf), "%.1f kW", (solarPowerW + abs(batteryPowerW)) / 1000.0);
  drawFlowNode(invX, invY, 68, 44, "INVERTER", invBuf, COLOR_INVERTER);

  // Solar Node
  char solBuf[16];
  snprintf(solBuf, sizeof(solBuf), "%.1f kW", solarPowerW / 1000.0);
  drawFlowNode(solX, solY, 76, 42, "SOLAR PV", solBuf, COLOR_SOLAR);

  // Battery Node
  char batBuf[24];
  snprintf(batBuf, sizeof(batBuf), "%d%% (%.1fkW)", batterySoC, abs(batteryPowerW) / 1000.0);
  drawFlowNode(batX, batY, 76, 42, "BATTERY", batBuf, COLOR_BATTERY);

  // Home Load Node
  char homeBuf[16];
  snprintf(homeBuf, sizeof(homeBuf), "%.1f kW", homeLoadW / 1000.0);
  drawFlowNode(homeX, homeY, 76, 42, "HOME LOAD", homeBuf, COLOR_HOME);

  // Grid Node
  char gridBuf[16];
  snprintf(gridBuf, sizeof(gridBuf), "%s%.1f kW", (gridPowerW >= 0 ? "+" : "-"), abs(gridPowerW) / 1000.0);
  drawFlowNode(gridX, gridY, 76, 42, "GRID NET", gridBuf, COLOR_GRID);
}

void drawHistoryChart() {
  tft.fillRect(0, 25, 320, 180, COLOR_BG);

  tft.setTextColor(COLOR_TEXT, COLOR_BG);
  tft.drawString("24-HOUR ENERGY BALANCE (KW)", 14, 32, 2);

  int gx = 20;
  int gy = 55;
  int gw = 280;
  int gh = 115;

  tft.drawFastHLine(gx, gy + gh, gw, COLOR_CARD_BRD);

  int groupW = gw / 12;
  int barW = 8;

  for (int i = 0; i < 12; i++) {
    int bx = gx + 6 + i * groupW;

    int sH = map((int)(solarHist[i] * 10), 0, 50, 0, gh - 15);
    tft.fillRect(bx, gy + gh - sH, barW, sH, COLOR_SOLAR);

    int hH = map((int)(homeHist[i] * 10), 0, 50, 0, gh - 15);
    tft.fillRect(bx + barW + 2, gy + gh - hH, barW, hH, COLOR_HOME);

    if (i % 2 == 0) {
      char timeBuf[8];
      snprintf(timeBuf, sizeof(timeBuf), "%02d", i * 2);
      tft.setTextColor(COLOR_MUTED, COLOR_BG);
      tft.drawCentreString(timeBuf, bx + barW, gy + gh + 4, 1);
    }
  }

  // Legend
  tft.fillRect(gx + 120, gy + 4, 8, 8, COLOR_SOLAR);
  tft.setTextColor(COLOR_MUTED, COLOR_BG);
  tft.drawString("Solar PV", gx + 132, gy + 4, 1);

  tft.fillRect(gx + 195, gy + 4, 8, 8, COLOR_HOME);
  tft.drawString("Home Load", gx + 207, gy + 4, 1);
}

void renderCurrentView() {
  if (currentView == 0) {
    drawFlowDiagram();
  } else {
    drawHistoryChart();
  }
  drawBottomBar();
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

  drawHeader();
  renderCurrentView();
  updateSelfSufficiencyLED();

  playBeep(2400, 60);
  Serial.println(F("[SYSTEM] Solar & Battery Flow Dashboard Ready."));
}

void loop() {
  // Animation frame for flowing particles (every 60ms)
  if (currentView == 0 && millis() - lastAnimTick > 60) {
    lastAnimTick = millis();
    animStep = (animStep + 1) % 20;
    drawFlowDiagram();
  }

  // Real-time simulated physics update (every 4 seconds)
  if (millis() - lastSimTick > 4000) {
    lastSimTick = millis();
    solarPowerW = constrain(solarPowerW + random(-150, 160), 2800, 4800);
    homeLoadW = constrain(homeLoadW + random(-80, 90), 900, 2400);

    int netPower = solarPowerW - homeLoadW;
    if (netPower > 0) {
      batteryPowerW = -min(netPower, 1800);
      gridPowerW = netPower + batteryPowerW;
    } else {
      batteryPowerW = min(abs(netPower), 2000);
      gridPowerW = netPower + batteryPowerW;
    }

    updateSelfSufficiencyLED();
    if (currentView == 1) {
      drawBottomBar();
    }
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

        // Toggle button: screenY >= 206 && screenX <= 115
        if (screenY >= 206 && screenX <= 115) {
          currentView = (currentView == 0) ? 1 : 0;
          playBeep(2000, 40);
          renderCurrentView();
        }
      }
    }
  }

  delay(15);
}

