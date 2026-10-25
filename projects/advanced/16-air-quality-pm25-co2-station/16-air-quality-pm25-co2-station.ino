/*
 * Cheap Yellow Display (CYD) - Environmental AQI, PM2.5, CO2 & TVOC Station
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
#include <Wire.h>
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

// Sensor I2C Pins (CN1 connector)
#define SENSOR_SDA 22
#define SENSOR_SCL 27

SPIClass touchSPI = SPIClass(VSPI);
XPT2046_Touchscreen ts(XPT2046_CS, XPT2046_IRQ);
TFT_eSPI tft = TFT_eSPI();

// Environmental Metrics
int aqiIndex = 42;           // EPA AQI (0-500)
float pm25 = 10.4;          // ug/m3
float pm10 = 18.2;          // ug/m3
int co2Ppm = 680;           // CO2 ppm
int tvocPpb = 85;           // TVOC ppb
float ambientTemp = 23.4;   // Celsius
float ambientHum = 48.0;    // % RH

// Sparkline history buffer (30 points)
#define HIST_POINTS 30
int aqiHistory[HIST_POINTS];

bool alertMuted = false;

void beep(int freq, int durationMs) {
  if (!alertMuted) {
    tone(CYD_BUZZER, freq, durationMs);
  }
}

void setRGB(bool r, bool g, bool b) {
  digitalWrite(CYD_LED_RED, r ? LOW : HIGH);
  digitalWrite(CYD_LED_GREEN, g ? LOW : HIGH);
  digitalWrite(CYD_LED_BLUE, b ? LOW : HIGH);
}

// Map AQI to EPA color
uint16_t getAqiColor(int aqi) {
  if (aqi <= 50) return TFT_GREEN;          // Good (0-50)
  if (aqi <= 100) return TFT_YELLOW;        // Moderate (51-100)
  if (aqi <= 150) return 0xFD20;            // Unhealthy for Sensitive (101-150)
  if (aqi <= 200) return TFT_RED;           // Unhealthy (151-200)
  if (aqi <= 300) return TFT_MAGENTA;       // Very Unhealthy (201-300)
  return 0x8000;                            // Hazardous (301-500)
}

const char* getAqiStatus(int aqi) {
  if (aqi <= 50) return "GOOD";
  if (aqi <= 100) return "MODERATE";
  if (aqi <= 150) return "SENSITIVE";
  if (aqi <= 200) return "UNHEALTHY";
  if (aqi <= 300) return "VERY UNHEALTHY";
  return "HAZARDOUS";
}

void drawHeader() {
  tft.fillRect(0, 0, 320, 26, 0x01E8);
  tft.drawFastHLine(0, 26, 320, 0x03EF);
  
  tft.setTextColor(TFT_WHITE, 0x01E8);
  tft.setTextDatum(ML_DATUM);
  tft.drawString("CYD AIR QUALITY & CO2 STATION", 8, 13, 2);
  
  tft.setTextDatum(MR_DATUM);
  tft.setTextColor(alertMuted ? TFT_DARKGREY : TFT_GREEN, 0x01E8);
  tft.drawString(alertMuted ? "ALM:MUTED" : "ALM:ACTIVE", 312, 13, 2);
}

void drawSparkline(int x, int y, int w, int h) {
  tft.fillRect(x, y, w, h, 0x10A2);
  tft.drawRect(x, y, w, h, 0x2965);
  
  int maxAqi = 150;
  for (int i = 0; i < HIST_POINTS - 1; i++) {
    int x1 = x + 2 + (i * (w - 4)) / (HIST_POINTS - 1);
    int y1 = y + h - 2 - (aqiHistory[i] * (h - 4)) / maxAqi;
    int x2 = x + 2 + ((i + 1) * (w - 4)) / (HIST_POINTS - 1);
    int y2 = y + h - 2 - (aqiHistory[i + 1] * (h - 4)) / maxAqi;
    y1 = constrain(y1, y + 2, y + h - 2);
    y2 = constrain(y2, y + 2, y + h - 2);
    tft.drawLine(x1, y1, x2, y2, TFT_CYAN);
  }
}

void renderDashboard() {
  // AQI Hero Card (Left)
  uint16_t aqiCol = getAqiColor(aqiIndex);
  tft.fillRect(6, 32, 130, 110, 0x0841);
  tft.drawRect(6, 32, 130, 110, aqiCol);
  
  tft.setTextColor(TFT_SILVER, 0x0841);
  tft.setTextDatum(TL_DATUM);
  tft.drawString("EPA AQI INDEX", 12, 38, 1);
  
  tft.setTextColor(aqiCol, 0x0841);
  tft.setTextDatum(MC_DATUM);
  char buf[32];
  snprintf(buf, sizeof(buf), "%d", aqiIndex);
  tft.drawString(buf, 71, 75, 7);
  
  tft.setTextColor(aqiCol, 0x0841);
  tft.drawString(getAqiStatus(aqiIndex), 71, 126, 2);
  
  // Right Grid Metrics (CO2, PM2.5, TVOC, Temp/Hum)
  int cardW = 86;
  int cardH = 52;
  
  // CO2 Card
  tft.fillRect(142, 32, cardW, cardH, 0x10A2);
  tft.drawRect(142, 32, cardW, cardH, (co2Ppm > 1000) ? TFT_RED : TFT_GREEN);
  tft.setTextColor(TFT_SILVER, 0x10A2);
  tft.setTextDatum(TL_DATUM);
  tft.drawString("CO2 (PPM)", 146, 36, 1);
  tft.setTextColor((co2Ppm > 1000) ? TFT_RED : TFT_GREEN, 0x10A2);
  snprintf(buf, sizeof(buf), "%d", co2Ppm);
  tft.drawString(buf, 146, 50, 4);
  
  // PM2.5 Card
  tft.fillRect(232, 32, cardW, cardH, 0x10A2);
  tft.drawRect(232, 32, cardW, cardH, 0x2965);
  tft.setTextColor(TFT_SILVER, 0x10A2);
  tft.drawString("PM2.5 (ug)", 236, 36, 1);
  tft.setTextColor(TFT_WHITE, 0x10A2);
  snprintf(buf, sizeof(buf), "%.1f", pm25);
  tft.drawString(buf, 236, 50, 4);
  
  // TVOC Card
  tft.fillRect(142, 90, cardW, cardH, 0x10A2);
  tft.drawRect(142, 90, cardW, cardH, 0x2965);
  tft.setTextColor(TFT_SILVER, 0x10A2);
  tft.drawString("TVOC (PPB)", 146, 94, 1);
  tft.setTextColor(TFT_YELLOW, 0x10A2);
  snprintf(buf, sizeof(buf), "%d", tvocPpb);
  tft.drawString(buf, 146, 108, 4);
  
  // Temp & Humidity Card
  tft.fillRect(232, 90, cardW, cardH, 0x10A2);
  tft.drawRect(232, 90, cardW, cardH, 0x2965);
  tft.setTextColor(TFT_SILVER, 0x10A2);
  tft.drawString("TEMP / RH", 236, 94, 1);
  tft.setTextColor(TFT_CYAN, 0x10A2);
  snprintf(buf, sizeof(buf), "%.1fC", ambientTemp);
  tft.drawString(buf, 236, 108, 2);
  snprintf(buf, sizeof(buf), "%.0f%% RH", ambientHum);
  tft.drawString(buf, 236, 126, 2);
  
  // Sparkline History Bar (Bottom)
  tft.setTextColor(TFT_SILVER, TFT_BLACK);
  tft.setTextDatum(TL_DATUM);
  tft.drawString("24H AQI ROLLING TREND", 10, 148, 1);
  drawSparkline(6, 162, 308, 46);
  
  // Bottom Action Buttons
  tft.fillRect(6, 214, 150, 24, 0x18C3);
  tft.drawRect(6, 214, 150, 24, TFT_DARKGREY);
  tft.setTextColor(TFT_WHITE, 0x18C3);
  tft.setTextDatum(MC_DATUM);
  tft.drawString(alertMuted ? "UNMUTE ALARM" : "MUTE ALARM", 81, 226, 2);
  
  tft.fillRect(164, 214, 150, 24, 0x18C3);
  tft.drawRect(164, 214, 150, 24, TFT_DARKGREY);
  tft.setTextColor(TFT_GREEN, 0x18C3);
  tft.drawString("ZERO CALIB", 239, 226, 2);
}

void simulateSensorReadings() {
  static uint32_t lastTick = 0;
  if (millis() - lastTick > 2500) {
    lastTick = millis();
    
    // Slight jitter in environmental readings
    co2Ppm = constrain(co2Ppm + ((rand() % 15) - 7), 400, 2200);
    pm25 = constrain(pm25 + (((rand() % 20) - 10) * 0.1f), 2.0f, 150.0f);
    pm10 = pm25 * 1.6f + ((rand() % 10) * 0.2f);
    tvocPpb = constrain(tvocPpb + ((rand() % 9) - 4), 20, 950);
    ambientTemp += (((rand() % 7) - 3) * 0.05f);
    ambientHum += (((rand() % 5) - 2) * 0.1f);
    
    // Recompute AQI roughly from PM2.5
    aqiIndex = (int)(pm25 * 3.8f);
    if (aqiIndex < 15) aqiIndex = 15;
    
    // Shift sparkline history
    for (int i = 0; i < HIST_POINTS - 1; i++) {
      aqiHistory[i] = aqiHistory[i + 1];
    }
    aqiHistory[HIST_POINTS - 1] = aqiIndex;
    
    // Ventilation alarm trigger if CO2 > 1200 or AQI > 100
    if (co2Ppm > 1200 || aqiIndex > 100) {
      setRGB(true, false, false);
      beep(2400, 60);
    } else if (aqiIndex > 50) {
      setRGB(true, true, false); // Yellow
    } else {
      setRGB(false, true, false); // Green
    }
    
    renderDashboard();
  }
}

void setup() {
  Serial.begin(115200);
  
  pinMode(CYD_LED_RED, OUTPUT);
  pinMode(CYD_LED_GREEN, OUTPUT);
  pinMode(CYD_LED_BLUE, OUTPUT);
  setRGB(false, true, false);
  
  pinMode(CYD_BUZZER, OUTPUT);
  beep(1200, 40);
  
  touchSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  ts.begin(touchSPI);
  ts.setRotation(1);
  
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  
  // Seed history
  for (int i = 0; i < HIST_POINTS; i++) {
    aqiHistory[i] = 35 + (rand() % 20);
  }
  
  drawHeader();
  renderDashboard();
}

void loop() {
  simulateSensorReadings();
  
  if (ts.touched()) {
    TS_Point p = ts.getPoint();
    int touchX = map(p.x, 200, 3700, 0, 320);
    int touchY = map(p.y, 240, 3800, 0, 240);
    
    if (touchY > 210) {
      if (touchX < 160) {
        alertMuted = !alertMuted;
        beep(1600, 30);
      } else {
        co2Ppm = 450;
        pm25 = 6.2;
        aqiIndex = 25;
        beep(2200, 50);
      }
      drawHeader();
      renderDashboard();
      delay(250);
    }
  }
  
  delay(50);
}
