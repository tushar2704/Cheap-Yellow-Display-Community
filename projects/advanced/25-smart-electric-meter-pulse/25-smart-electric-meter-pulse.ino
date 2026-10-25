/*
 * Cheap Yellow Display (CYD) - Smart Home Electric Meter Pulse Counter & Cost Estimator
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
#define CYD_LDR 34 // Onboard optical pulse detector

SPIClass touchSPI = SPIClass(VSPI);
XPT2046_Touchscreen ts(XPT2046_CS, XPT2046_IRQ);
TFT_eSPI tft = TFT_eSPI();

// Metering Constants
#define IMPULSES_PER_KWH 1000 // Standard DIN rail / utility meter pulse constant

volatile uint32_t totalPulses = 14280;
float currentKw = 2.45;
float todayKwh = 14.28;
float tariffPerKwh = 0.28; // $ / kWh
float projectedMonthlyCost = 120.50;

float peakKwLimit = 5.0; // Overload threshold
bool isOverload = false;

void beep(int freq, int durationMs) {
  tone(CYD_BUZZER, freq, durationMs);
}

void setRGB(bool r, bool g, bool b) {
  digitalWrite(CYD_LED_RED, r ? LOW : HIGH);
  digitalWrite(CYD_LED_GREEN, g ? LOW : HIGH);
  digitalWrite(CYD_LED_BLUE, b ? LOW : HIGH);
}

void drawHeader() {
  tft.fillRect(0, 0, 320, 26, isOverload ? TFT_RED : 0x0841);
  tft.drawFastHLine(0, 26, 320, TFT_YELLOW);
  
  tft.setTextColor(TFT_WHITE, isOverload ? TFT_RED : 0x0841);
  tft.setTextDatum(ML_DATUM);
  tft.drawString(isOverload ? "PEAK DEMAND OVERLOAD WARNING!" : "ELECTRIC METER PULSE COUNTER", 8, 13, 2);
  
  tft.setTextDatum(MR_DATUM);
  tft.setTextColor(isOverload ? TFT_YELLOW : TFT_GREEN, isOverload ? TFT_RED : 0x0841);
  char buf[32];
  snprintf(buf, sizeof(buf), "$%.2f/kWh", tariffPerKwh);
  tft.drawString(buf, 312, 13, 2);
}

void drawDashboard() {
  // Current Power Draw Hero Card (Left)
  tft.fillRect(6, 32, 150, 110, 0x10A2);
  tft.drawRect(6, 32, 150, 110, isOverload ? TFT_RED : TFT_YELLOW);
  
  tft.setTextColor(TFT_SILVER, 0x10A2);
  tft.setTextDatum(TL_DATUM);
  tft.drawString("ACTIVE POWER DRAW", 14, 38, 1);
  
  tft.setTextColor(isOverload ? TFT_RED : TFT_YELLOW, 0x10A2);
  tft.setTextDatum(MC_DATUM);
  char buf[32];
  snprintf(buf, sizeof(buf), "%.2f", currentKw);
  tft.drawString(buf, 81, 76, 7);
  
  tft.setTextColor(TFT_WHITE, 0x10A2);
  tft.drawString("KILOWATTS (kW)", 81, 126, 2);
  
  // Right Grid Cards (Today's Energy & Projected Cost)
  // Today's Consumption
  tft.fillRect(164, 32, 150, 52, 0x10A2);
  tft.drawRect(164, 32, 150, 52, 0x2965);
  tft.setTextColor(TFT_SILVER, 0x10A2);
  tft.setTextDatum(TL_DATUM);
  tft.drawString("TODAY'S ENERGY", 170, 36, 1);
  tft.setTextColor(TFT_CYAN, 0x10A2);
  snprintf(buf, sizeof(buf), "%.2f kWh", todayKwh);
  tft.drawString(buf, 170, 52, 4);
  
  // Today's Cost & Monthly Forecast
  tft.fillRect(164, 90, 150, 52, 0x10A2);
  tft.drawRect(164, 90, 150, 52, 0x2965);
  tft.setTextColor(TFT_SILVER, 0x10A2);
  tft.drawString("ESTIMATED COST", 170, 94, 1);
  tft.setTextColor(TFT_GREEN, 0x10A2);
  float costToday = todayKwh * tariffPerKwh;
  snprintf(buf, sizeof(buf), "$%.2f today", costToday);
  tft.drawString(buf, 170, 108, 2);
  snprintf(buf, sizeof(buf), "~$%.0f / month", (costToday * 30.0f));
  tft.drawString(buf, 170, 124, 2);
  
  // Optical Impulses Telemetry Bar
  tft.fillRect(6, 148, 308, 56, 0x0821);
  tft.drawRect(6, 148, 308, 56, 0x2965);
  
  tft.setTextColor(TFT_SILVER, 0x0821);
  tft.drawString("PULSE TELEMETRY (1000 imp/kWh)", 14, 154, 1);
  
  tft.setTextColor(TFT_WHITE, 0x0821);
  snprintf(buf, sizeof(buf), "Cumulative Pulses: %u", totalPulses);
  tft.drawString(buf, 14, 172, 2);
  
  snprintf(buf, sizeof(buf), "Peak Threshold: %.1f kW", peakKwLimit);
  tft.setTextColor(isOverload ? TFT_RED : TFT_GOLD, 0x0821);
  tft.drawString(buf, 14, 188, 1);
  
  // Bottom Buttons
  tft.fillRect(6, 212, 148, 26, 0x18C3);
  tft.drawRect(6, 212, 148, 26, TFT_DARKGREY);
  tft.setTextColor(TFT_WHITE, 0x18C3);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("ADJUST TARIFF", 80, 225, 2);
  
  tft.fillRect(166, 212, 148, 26, 0x18C3);
  tft.drawRect(166, 212, 148, 26, TFT_DARKGREY);
  tft.setTextColor(TFT_GREEN, 0x18C3);
  tft.drawString("RESET COUNTER", 240, 225, 2);
}

void simulatePulseCounting() {
  static uint32_t lastTick = 0;
  if (millis() - lastTick > 1500) {
    lastTick = millis();
    
    // Grid power load fluctuation
    currentKw = constrain(currentKw + ((rand() % 9) - 4) * 0.15f, 0.45f, 6.50f);
    isOverload = (currentKw >= peakKwLimit);
    
    // Generate optical LED pulse
    totalPulses += (uint32_t)(currentKw * 4);
    todayKwh = (float)totalPulses / (float)IMPULSES_PER_KWH;
    
    if (isOverload) {
      setRGB(true, false, false); // Red overload
      beep(2800, 40);
    } else {
      setRGB(false, true, false); // Green pulse flash
      delay(15);
      setRGB(false, false, false);
    }
    
    drawHeader();
    drawDashboard();
  }
}

void setup() {
  Serial.begin(115200);
  
  pinMode(CYD_LED_RED, OUTPUT);
  pinMode(CYD_LED_GREEN, OUTPUT);
  pinMode(CYD_LED_BLUE, OUTPUT);
  setRGB(false, false, false);
  
  pinMode(CYD_BUZZER, OUTPUT);
  beep(1400, 30);
  
  touchSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  ts.begin(touchSPI);
  ts.setRotation(1);
  
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  
  drawHeader();
  drawDashboard();
}

void loop() {
  simulatePulseCounting();
  
  if (ts.touched()) {
    TS_Point p = ts.getPoint();
    int touchX = map(p.x, 200, 3700, 0, 320);
    int touchY = map(p.y, 240, 3800, 0, 240);
    
    if (touchY > 210) {
      if (touchX < 160) {
        tariffPerKwh += 0.02f;
        if (tariffPerKwh > 0.50f) tariffPerKwh = 0.18f;
        beep(1800, 25);
      } else {
        totalPulses = 0;
        todayKwh = 0.00;
        beep(2200, 40);
      }
      drawHeader();
      drawDashboard();
      delay(250);
    }
  }
  
  delay(40);
}
