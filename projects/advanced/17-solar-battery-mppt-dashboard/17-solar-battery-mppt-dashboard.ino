/*
 * Cheap Yellow Display (CYD) - Solar & Battery Storage MPPT Dashboard
 * Hardware: ESP32-2432S028R (2.8" TFT 320x240 ILI9341, XPT2046 Resistive Touch)
 * 
 * Target: 2.8" CYD ESP32-2432S028R
 * Author: tushar2704 <tushar.inseec@gmail.com>
 * Channels: @IamNotIntoTech | @Makerrdesigns
 * Dual Licensed: AGPL-3.0 for firmware code | CC BY-NC-ND 4.0 for media/assets
 * Year: 2026
 */

#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>

// Touch Pins
#define XPT2046_IRQ 36
#define XPT2046_MOSI 32
#define XPT2046_MISO 39
#define XPT2046_CLK 25
#define XPT2046_CS 33

// Onboard Peripherals
#define CYD_LED_RED 4
#define CYD_LED_GREEN 16
#define CYD_LED_BLUE 17
#define CYD_BUZZER 26
#define CYD_LDR 34

SPIClass touchSPI = SPIClass(VSPI);
XPT2046_Touchscreen ts(XPT2046_CS, XPT2046_IRQ);
TFT_eSPI tft = TFT_eSPI();

// Telemetry State
float solarWatts = 640.0;
float solarVolts = 38.4;
float solarAmps = 16.6;

int batterySoc = 88; // %
float batteryVolts = 26.8; // 24V nominal LiFePO4
float batteryAmps = 12.4; // Charging current (+) or Discharging (-)
float batteryTemp = 24.2;

float loadWatts = 320.0;
float dailyYieldKwh = 3.84;

bool inverterActive = true;

void beep(int freq, int durationMs) {
  tone(CYD_BUZZER, freq, durationMs);
}

void setRGB(bool r, bool g, bool b) {
  digitalWrite(CYD_LED_RED, r ? LOW : HIGH);
  digitalWrite(CYD_LED_GREEN, g ? LOW : HIGH);
  digitalWrite(CYD_LED_BLUE, b ? LOW : HIGH);
}

void drawHeader() {
  tft.fillRect(0, 0, 320, 26, 0x02E5);
  tft.drawFastHLine(0, 26, 320, TFT_YELLOW);
  
  tft.setTextColor(TFT_YELLOW, 0x02E5);
  tft.setTextDatum(ML_DATUM);
  tft.drawString("OFF-GRID MPPT SOLAR MONITOR", 8, 13, 2);
  
  tft.setTextColor(TFT_WHITE, 0x02E5);
  tft.setTextDatum(MR_DATUM);
  char buf[32];
  snprintf(buf, sizeof(buf), "%.2f kWh", dailyYieldKwh);
  tft.drawString(buf, 312, 13, 2);
}

void drawEnergyFlowDiagram() {
  // Solar PV Box (Left)
  tft.fillRect(8, 34, 90, 78, 0x10A2);
  tft.drawRect(8, 34, 90, 78, TFT_YELLOW);
  tft.setTextColor(TFT_YELLOW, 0x10A2);
  tft.setTextDatum(TL_DATUM);
  tft.drawString("SOLAR PV", 14, 38, 1);
  char buf[32];
  snprintf(buf, sizeof(buf), "%.0fW", solarWatts);
  tft.drawString(buf, 14, 52, 4);
  snprintf(buf, sizeof(buf), "%.1fV | %.1fA", solarVolts, solarAmps);
  tft.drawString(buf, 14, 88, 1);
  
  // MPPT Center Box
  tft.fillRect(114, 46, 92, 54, 0x0841);
  tft.drawRect(114, 46, 92, 54, TFT_CYAN);
  tft.setTextColor(TFT_CYAN, 0x0841);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("MPPT CHARGER", 160, 60, 1);
  tft.setTextColor(TFT_WHITE, 0x0841);
  tft.drawString("98.4% EFF", 160, 80, 2);
  
  // AC Inverter / Load Box (Right)
  tft.fillRect(222, 34, 90, 78, 0x10A2);
  tft.drawRect(222, 34, 90, 78, TFT_ORANGE);
  tft.setTextColor(TFT_ORANGE, 0x10A2);
  tft.setTextDatum(TL_DATUM);
  tft.drawString("AC LOAD", 228, 38, 1);
  snprintf(buf, sizeof(buf), "%.0fW", loadWatts);
  tft.drawString(buf, 228, 52, 4);
  snprintf(buf, sizeof(buf), "230V 50Hz", loadWatts);
  tft.drawString(buf, 228, 88, 1);
  
  // Connecting flow arrows
  tft.drawFastHLine(98, 73, 16, TFT_YELLOW);
  tft.drawFastHLine(206, 73, 16, TFT_ORANGE);
  
  // Battery Section (Bottom Hero)
  tft.fillRect(8, 120, 304, 86, 0x0821);
  tft.drawRect(8, 120, 304, 86, (batterySoc > 20) ? TFT_GREEN : TFT_RED);
  
  tft.setTextColor(TFT_SILVER, 0x0821);
  tft.setTextDatum(TL_DATUM);
  tft.drawString("LIFEPO4 STORAGE BATTERY (24V BANK)", 16, 126, 1);
  
  // SOC Big Value
  tft.setTextColor((batterySoc > 20) ? TFT_GREEN : TFT_RED, 0x0821);
  snprintf(buf, sizeof(buf), "%d%%", batterySoc);
  tft.drawString(buf, 16, 142, 4);
  
  // Battery metrics text
  tft.setTextColor(TFT_WHITE, 0x0821);
  snprintf(buf, sizeof(buf), "%.2f V | %+.1f A", batteryVolts, batteryAmps);
  tft.drawString(buf, 16, 178, 2);
  
  // Battery Fill Bar Visual
  int barX = 130;
  int barY = 146;
  int barW = 166;
  int barH = 26;
  tft.drawRect(barX, barY, barW, barH, TFT_WHITE);
  tft.drawRect(barX + barW, barY + 6, 6, barH - 12, TFT_WHITE); // Battery terminal pip
  int fillW = (batterySoc * (barW - 4)) / 100;
  uint16_t socColor = (batterySoc > 50) ? TFT_GREEN : (batterySoc > 20) ? TFT_YELLOW : TFT_RED;
  tft.fillRect(barX + 2, barY + 2, fillW, barH - 4, socColor);
  tft.fillRect(barX + 2 + fillW, barY + 2, (barW - 4) - fillW, barH - 4, 0x10A2);
  
  // Bottom Action Bar
  tft.fillRect(8, 212, 146, 24, 0x18C3);
  tft.drawRect(8, 212, 146, 24, TFT_DARKGREY);
  tft.setTextColor(TFT_WHITE, 0x18C3);
  tft.setTextDatum(MC_DATUM);
  tft.drawString(inverterActive ? "INVERTER: ON" : "INVERTER: OFF", 81, 224, 2);
  
  tft.fillRect(166, 212, 146, 24, 0x18C3);
  tft.drawRect(166, 212, 146, 24, TFT_DARKGREY);
  tft.setTextColor(TFT_GREEN, 0x18C3);
  tft.drawString("RESET YIELD", 239, 224, 2);
}

void simulateSolarCycle() {
  static uint32_t lastTick = 0;
  if (millis() - lastTick > 2500) {
    lastTick = millis();
    
    // Slight solar fluctuations
    solarWatts = constrain(solarWatts + ((rand() % 41) - 20), 120.0f, 950.0f);
    solarAmps = solarWatts / solarVolts;
    
    if (inverterActive) {
      loadWatts = constrain(loadWatts + ((rand() % 31) - 15), 180.0f, 850.0f);
    } else {
      loadWatts = 0;
    }
    
    // Net battery power = solar - load
    float netPower = solarWatts - loadWatts;
    batteryAmps = netPower / batteryVolts;
    
    if (batteryAmps > 0 && batterySoc < 100) {
      batterySoc = constrain(batterySoc + (rand() % 2 == 0 ? 1 : 0), 0, 100);
      setRGB(false, true, false); // Green charging
    } else if (batteryAmps < 0) {
      setRGB(true, true, false); // Yellow discharging
    }
    
    dailyYieldKwh += (solarWatts / 3600.0f / 1000.0f) * 2.5f;
    
    drawEnergyFlowDiagram();
  }
}

void setup() {
  Serial.begin(115200);
  
  pinMode(CYD_LED_RED, OUTPUT);
  pinMode(CYD_LED_GREEN, OUTPUT);
  pinMode(CYD_LED_BLUE, OUTPUT);
  setRGB(false, true, false);
  
  pinMode(CYD_BUZZER, OUTPUT);
  beep(1500, 30);
  
  touchSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  ts.begin(touchSPI);
  ts.setRotation(1);
  
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  
  drawHeader();
  drawEnergyFlowDiagram();
}

void loop() {
  simulateSolarCycle();
  
  if (ts.touched()) {
    TS_Point p = ts.getPoint();
    int touchX = map(p.x, 200, 3700, 0, 320);
    int touchY = map(p.y, 240, 3800, 0, 240);
    
    if (touchY > 210) {
      if (touchX < 160) {
        inverterActive = !inverterActive;
        beep(1800, 30);
      } else {
        dailyYieldKwh = 0.0;
        beep(2200, 40);
      }
      drawHeader();
      drawEnergyFlowDiagram();
      delay(250);
    }
  }
  
  delay(50);
}
