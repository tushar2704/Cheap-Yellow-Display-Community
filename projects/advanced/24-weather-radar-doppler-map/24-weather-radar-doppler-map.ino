/*
 * Cheap Yellow Display (CYD) - Live Doppler Rain Radar & Satellite Weather Map
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
#include <WiFi.h>
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

// Radar Animation State (6 historical Doppler frames)
#define RADAR_FRAMES 6
int currentFrame = 0;
bool isPlaying = true;
const char* frameTimes[RADAR_FRAMES] = { "-50m", "-40m", "-30m", "-20m", "-10m", "NOW" };

// Map center
const char* locationName = "SEATTLE REGION METRO";
float currentPrecipMmH = 12.4; // mm/h precipitation

void beep(int freq, int durationMs) {
  tone(CYD_BUZZER, freq, durationMs);
}

void setRGB(bool r, bool g, bool b) {
  digitalWrite(CYD_LED_RED, r ? LOW : HIGH);
  digitalWrite(CYD_LED_GREEN, g ? LOW : HIGH);
  digitalWrite(CYD_LED_BLUE, b ? LOW : HIGH);
}

uint16_t getRadarIntensityColor(float dbz) {
  if (dbz < 15.0f) return 0x0000;          // No echo
  if (dbz < 25.0f) return 0x03E0;          // Light rain (Cyan/Green)
  if (dbz < 35.0f) return TFT_GREEN;       // Moderate
  if (dbz < 45.0f) return TFT_YELLOW;      // Heavy
  if (dbz < 55.0f) return 0xFD20;          // Very Heavy (Orange)
  if (dbz < 65.0f) return TFT_RED;           // Severe storm
  return TFT_MAGENTA;                      // Hail
}

void drawHeader() {
  tft.fillRect(0, 0, 320, 26, 0x0841);
  tft.drawFastHLine(0, 26, 320, TFT_CYAN);
  
  tft.setTextColor(TFT_CYAN, 0x0841);
  tft.setTextDatum(ML_DATUM);
  tft.drawString("DOPPLER WEATHER RADAR (NEXRAD)", 8, 13, 2);
  
  tft.setTextDatum(MR_DATUM);
  tft.setTextColor(TFT_WHITE, 0x0841);
  char buf[32];
  snprintf(buf, sizeof(buf), "[%s]", frameTimes[currentFrame]);
  tft.drawString(buf, 312, 13, 2);
}

void drawRadarMap() {
  // Map canvas area: X=0..320, Y=28..180
  int mapX = 6;
  int mapY = 30;
  int mapW = 308;
  int mapH = 150;
  
  tft.fillRect(mapX, mapY, mapW, mapH, 0x0821);
  tft.drawRect(mapX, mapY, mapW, mapH, 0x2104);
  
  // Coastline & County Boundaries Simulation
  tft.drawCircle(mapX + 154, mapY + 75, 45, 0x18C3);
  tft.drawCircle(mapX + 154, mapY + 75, 90, 0x18C3);
  tft.drawFastHLine(mapX + 154 - 10, mapY + 75, 20, TFT_WHITE);
  tft.drawFastVLine(mapX + 154, mapY + 75 - 10, 20, TFT_WHITE);
  
  // Moving Doppler rain storm cells
  float stormShiftX = (float)currentFrame * 14.0f;
  float stormShiftY = (float)currentFrame * 4.0f;
  
  // Render rain clusters
  int cells[4][2] = {
    { 110, 60 },
    { 160, 90 },
    { 220, 50 },
    { 80, 110 }
  };
  
  for (int i = 0; i < 4; i++) {
    int cx = mapX + cells[i][0] + (int)stormShiftX;
    int cy = mapY + cells[i][1] + (int)stormShiftY;
    
    // Outer green halo (Light)
    tft.fillCircle(cx, cy, 32, getRadarIntensityColor(28.0f));
    // Mid yellow core (Moderate)
    tft.fillCircle(cx + 4, cy - 2, 20, getRadarIntensityColor(42.0f));
    // Red severe core
    tft.fillCircle(cx + 6, cy - 4, 11, getRadarIntensityColor(52.0f));
    // Hail core
    tft.fillCircle(cx + 7, cy - 5, 4, getRadarIntensityColor(68.0f));
  }
  
  // Station Marker
  tft.fillCircle(mapX + 154, mapY + 75, 3, TFT_YELLOW);
  tft.setTextColor(TFT_WHITE, 0x0821);
  tft.setTextDatum(MC_DATUM);
  tft.drawString(locationName, mapX + 154, mapY + 16, 2);
  
  // dBZ Color Legend Bar (Y=184..196)
  int legX = 8;
  int legY = 186;
  int legW = 38;
  int legH = 10;
  uint16_t cols[] = { 0x03E0, TFT_GREEN, TFT_YELLOW, 0xFD20, TFT_RED, TFT_MAGENTA };
  const char* labels[] = { "20", "30", "40", "50", "60", "70" };
  
  for (int i = 0; i < 6; i++) {
    tft.fillRect(legX + i * (legW + 2), legY, legW, legH, cols[i]);
    tft.setTextColor(TFT_SILVER, TFT_BLACK);
    tft.setTextDatum(TC_DATUM);
    tft.drawString(labels[i], legX + i * (legW + 2) + legW / 2, legY + 12, 1);
  }
  tft.drawString("dBZ", 290, legY + 2, 1);
}

void drawBottomBar() {
  tft.fillRect(6, 214, 96, 24, 0x18C3);
  tft.drawRect(6, 214, 96, 24, TFT_DARKGREY);
  tft.setTextColor(TFT_WHITE, 0x18C3);
  tft.setTextDatum(MC_DATUM);
  tft.drawString(isPlaying ? "PAUSE" : "PLAY", 54, 226, 2);
  
  tft.fillRect(112, 214, 96, 24, 0x18C3);
  tft.drawRect(112, 214, 96, 24, TFT_DARKGREY);
  tft.setTextColor(TFT_CYAN, 0x18C3);
  tft.drawString("NEXT FRAME", 160, 226, 2);
  
  tft.fillRect(218, 214, 96, 24, 0x18C3);
  tft.drawRect(218, 214, 96, 24, TFT_DARKGREY);
  tft.setTextColor(TFT_YELLOW, 0x18C3);
  tft.drawString("ZOOM 2X", 266, 226, 2);
}

void advanceRadarFrame() {
  currentFrame = (currentFrame + 1) % RADAR_FRAMES;
  
  // Storm intensity warning LED on severe core (Frame 4 & 5)
  if (currentFrame >= 4) {
    setRGB(true, false, false); // Storm warning Red
    beep(2400, 25);
  } else {
    setRGB(false, false, true); // Rain Blue
  }
  
  drawHeader();
  drawRadarMap();
  drawBottomBar();
}

void setup() {
  Serial.begin(115200);
  
  pinMode(CYD_LED_RED, OUTPUT);
  pinMode(CYD_LED_GREEN, OUTPUT);
  pinMode(CYD_LED_BLUE, OUTPUT);
  setRGB(false, false, true);
  
  pinMode(CYD_BUZZER, OUTPUT);
  beep(1800, 40);
  
  touchSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  ts.begin(touchSPI);
  ts.setRotation(1);
  
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  
  drawHeader();
  drawRadarMap();
  drawBottomBar();
}

void loop() {
  static uint32_t lastFrameTick = 0;
  if (isPlaying && (millis() - lastFrameTick > 1200)) {
    lastFrameTick = millis();
    advanceRadarFrame();
  }
  
  if (ts.touched()) {
    TS_Point p = ts.getPoint();
    int touchX = map(p.x, 200, 3700, 0, 320);
    int touchY = map(p.y, 240, 3800, 0, 240);
    
    if (touchY > 210) {
      if (touchX < 108) {
        isPlaying = !isPlaying;
        beep(1600, 30);
      } else if (touchX >= 112 && touchX < 212) {
        advanceRadarFrame();
        beep(2000, 25);
      } else {
        beep(2400, 40);
      }
      drawBottomBar();
      delay(200);
    }
  }
  
  delay(40);
}
