/*
 * Cheap Yellow Display (CYD) - Smart Plant & Greenhouse Microclimate Station
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

// 4 Plant Zones Moisture (% 0..100)
int zoneMoisture[4] = { 68, 42, 85, 24 }; // Zone 4 dry!
const char* zoneNames[4] = { "Monstera", "Basil", "Fern", "Bonsai" };

// Microclimate
float temperatureC = 24.8;
float humidityPct = 62.0;
float vpdKpa = 1.15; // Vapor Pressure Deficit (0.8 - 1.2 optimal)
int lightLux = 840;

bool pumpActive = false;
uint32_t pumpStartTime = 0;

void beep(int freq, int durationMs) {
  tone(CYD_BUZZER, freq, durationMs);
}

void setRGB(bool r, bool g, bool b) {
  digitalWrite(CYD_LED_RED, r ? LOW : HIGH);
  digitalWrite(CYD_LED_GREEN, g ? LOW : HIGH);
  digitalWrite(CYD_LED_BLUE, b ? LOW : HIGH);
}

float calculateVPD(float temp, float rh) {
  // Saturation vapor pressure SVP (kPa)
  float svp = 0.61078f * expf((17.27f * temp) / (temp + 237.3f));
  // Actual vapor pressure AVP
  float avp = svp * (rh / 100.0f);
  return svp - avp;
}

void drawHeader() {
  tft.fillRect(0, 0, 320, 26, 0x0320); // Forest green
  tft.drawFastHLine(0, 26, 320, TFT_GREEN);
  
  tft.setTextColor(TFT_WHITE, 0x0320);
  tft.setTextDatum(ML_DATUM);
  tft.drawString("SMART GREENHOUSE & SOIL MONITOR", 8, 13, 2);
  
  tft.setTextDatum(MR_DATUM);
  tft.setTextColor(TFT_YELLOW, 0x0320);
  char buf[32];
  snprintf(buf, sizeof(buf), "VPD: %.2f kPa", vpdKpa);
  tft.drawString(buf, 312, 13, 2);
}

void drawMicroclimateCard() {
  // Top Environment Bar
  tft.fillRect(6, 32, 308, 38, 0x10A2);
  tft.drawRect(6, 32, 308, 38, 0x2965);
  
  tft.setTextColor(TFT_WHITE, 0x10A2);
  tft.setTextDatum(ML_DATUM);
  char buf[64];
  snprintf(buf, sizeof(buf), "TEMP: %.1f C   RH: %.0f%%   LIGHT: %d lx", temperatureC, humidityPct, lightLux);
  tft.drawString(buf, 12, 51, 2);
}

void drawZonesMoisture() {
  // 4 Zone Soil Bars
  int barStartX = 88;
  int barW = 160;
  int barH = 18;
  
  for (int i = 0; i < 4; i++) {
    int y = 78 + i * 32;
    
    // Zone Name
    tft.fillRect(6, y - 2, 76, 22, 0x0821);
    tft.drawRect(6, y - 2, 76, 22, TFT_DARKGREY);
    tft.setTextColor(TFT_SILVER, 0x0821);
    tft.setTextDatum(MC_DATUM);
    tft.drawString(zoneNames[i], 44, y + 9, 2);
    
    // Bar frame
    tft.drawRect(barStartX, y, barW, barH, TFT_WHITE);
    int m = zoneMoisture[i];
    int fillW = (m * (barW - 4)) / 100;
    
    uint16_t mCol = (m < 30) ? TFT_RED : (m < 50) ? TFT_YELLOW : TFT_GREEN;
    tft.fillRect(barStartX + 2, y + 2, fillW, barH - 4, mCol);
    tft.fillRect(barStartX + 2 + fillW, y + 2, (barW - 4) - fillW, barH - 4, 0x10A2);
    
    // Percentage text
    char pbuf[16];
    snprintf(pbuf, sizeof(pbuf), "%d%%", m);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextDatum(ML_DATUM);
    tft.drawString(pbuf, barStartX + barW + 10, y + 9, 2);
  }
}

void drawBottomBar() {
  tft.fillRect(6, 212, 148, 26, pumpActive ? TFT_CYAN : 0x18C3);
  tft.drawRect(6, 212, 148, 26, TFT_DARKGREY);
  tft.setTextColor(pumpActive ? TFT_BLACK : TFT_WHITE, pumpActive ? TFT_CYAN : 0x18C3);
  tft.setTextDatum(MC_DATUM);
  tft.drawString(pumpActive ? "IRRIGATION: ON" : "WATER ZONE 4", 80, 225, 2);
  
  tft.fillRect(166, 212, 148, 26, 0x18C3);
  tft.drawRect(166, 212, 148, 26, TFT_DARKGREY);
  tft.setTextColor(TFT_GREEN, 0x18C3);
  tft.drawString("CALIB DRY/WET", 240, 225, 2);
}

void renderAll() {
  drawHeader();
  drawMicroclimateCard();
  drawZonesMoisture();
  drawBottomBar();
}

void simulatePlantEcosystem() {
  static uint32_t lastTick = 0;
  if (millis() - lastTick > 3000) {
    lastTick = millis();
    
    // Ambient jitter
    temperatureC += ((rand() % 5) - 2) * 0.1f;
    humidityPct = constrain(humidityPct + ((rand() % 5) - 2) * 0.2f, 30.0f, 95.0f);
    lightLux = constrain(lightLux + (rand() % 41 - 20), 100, 2500);
    vpdKpa = calculateVPD(temperatureC, humidityPct);
    
    // Natural soil drying
    for (int i = 0; i < 4; i++) {
      if (rand() % 3 == 0) {
        zoneMoisture[i] = max(10, zoneMoisture[i] - 1);
      }
    }
    
    // Pump action
    if (pumpActive) {
      zoneMoisture[3] = min(90, zoneMoisture[3] + 12);
      if (millis() - pumpStartTime > 6000) {
        pumpActive = false;
        beep(1400, 30);
      }
    }
    
    // Alert LED if any zone is critically dry (<25%)
    if (zoneMoisture[3] < 25) {
      setRGB(true, false, false); // Red dry alert
      beep(2600, 30);
    } else {
      setRGB(false, true, false); // Green healthy
    }
    
    renderAll();
  }
}

void setup() {
  Serial.begin(115200);
  
  pinMode(CYD_LED_RED, OUTPUT);
  pinMode(CYD_LED_GREEN, OUTPUT);
  pinMode(CYD_LED_BLUE, OUTPUT);
  setRGB(false, true, false);
  
  pinMode(CYD_BUZZER, OUTPUT);
  beep(1400, 40);
  
  touchSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  ts.begin(touchSPI);
  ts.setRotation(1);
  
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  
  renderAll();
}

void loop() {
  simulatePlantEcosystem();
  
  if (ts.touched()) {
    TS_Point p = ts.getPoint();
    int touchX = map(p.x, 200, 3700, 0, 320);
    int touchY = map(p.y, 240, 3800, 0, 240);
    
    if (touchY > 210) {
      if (touchX < 160) {
        pumpActive = true;
        pumpStartTime = millis();
        beep(2200, 40);
      } else {
        zoneMoisture[0] = 75;
        zoneMoisture[1] = 60;
        zoneMoisture[2] = 80;
        zoneMoisture[3] = 70;
        beep(1800, 30);
      }
      renderAll();
      delay(250);
    }
  }
  
  delay(50);
}
