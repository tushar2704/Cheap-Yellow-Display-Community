/*
 * Cheap Yellow Display (CYD) - GPS Outdoor Navigator, Compass & Speedometer
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

// GPS NMEA Serial Pins (CN1 connector)
#define GPS_RX 27
#define GPS_TX 22

SPIClass touchSPI = SPIClass(VSPI);
XPT2046_Touchscreen ts(XPT2046_CS, XPT2046_IRQ);
TFT_eSPI tft = TFT_eSPI();

// Navigation Telemetry
float currentSpeedKmh = 48.5;
float headingDegrees = 135.0; // Heading (0..360)
float latitude = 37.7749;
float longitude = -122.4194;
float altitudeMeters = 142.0;
int satellitesTracked = 9;
float hdop = 1.1;

bool useMph = false;

void beep(int freq, int durationMs) {
  tone(CYD_BUZZER, freq, durationMs);
}

void setRGB(bool r, bool g, bool b) {
  digitalWrite(CYD_LED_RED, r ? LOW : HIGH);
  digitalWrite(CYD_LED_GREEN, g ? LOW : HIGH);
  digitalWrite(CYD_LED_BLUE, b ? LOW : HIGH);
}

void drawHeader() {
  tft.fillRect(0, 0, 320, 26, 0x01E8);
  tft.drawFastHLine(0, 26, 320, TFT_CYAN);
  
  tft.setTextColor(TFT_WHITE, 0x01E8);
  tft.setTextDatum(ML_DATUM);
  tft.drawString("CYD GPS OUTDOOR NAVIGATOR", 8, 13, 2);
  
  tft.setTextDatum(MR_DATUM);
  tft.setTextColor(TFT_GREEN, 0x01E8);
  char buf[32];
  snprintf(buf, sizeof(buf), "3D FIX (%d SAT)", satellitesTracked);
  tft.drawString(buf, 312, 13, 2);
}

void drawSpeedometerCard() {
  // Speed Hero Card (Left)
  tft.fillRect(6, 32, 148, 108, 0x10A2);
  tft.drawRect(6, 32, 148, 108, 0x2965);
  
  tft.setTextColor(TFT_SILVER, 0x10A2);
  tft.setTextDatum(TL_DATUM);
  tft.drawString(useMph ? "SPEED (MPH)" : "SPEED (KM/H)", 14, 38, 1);
  
  float dispSpeed = useMph ? (currentSpeedKmh * 0.621371f) : currentSpeedKmh;
  tft.setTextColor(TFT_GREEN, 0x10A2);
  tft.setTextDatum(MC_DATUM);
  char buf[32];
  snprintf(buf, sizeof(buf), "%.1f", dispSpeed);
  tft.drawString(buf, 80, 78, 7);
  
  tft.setTextColor(TFT_YELLOW, 0x10A2);
  snprintf(buf, sizeof(buf), "ALT: %.0f m", altitudeMeters);
  tft.drawString(buf, 80, 124, 2);
}

void drawCompassRose(int cx, int cy, int radius, float heading) {
  tft.fillRect(cx - radius - 8, cy - radius - 8, (radius + 8) * 2, (radius + 8) * 2, 0x0821);
  tft.drawCircle(cx, cy, radius, TFT_SILVER);
  tft.drawCircle(cx, cy, radius - 6, 0x2965);
  
  // Cardinal Marks (N, S, E, W)
  const char* cardinals[] = { "N", "E", "S", "W" };
  for (int i = 0; i < 4; i++) {
    float angle = (i * 90.0f - 90.0f) * DEG_TO_RAD;
    int tx = cx + (int)(cosf(angle) * (radius - 14));
    int ty = cy + (int)(sinf(angle) * (radius - 14));
    tft.setTextColor((i == 0) ? TFT_RED : TFT_WHITE, 0x0821);
    tft.setTextDatum(MC_DATUM);
    tft.drawString(cardinals[i], tx, ty, 2);
  }
  
  // Rotating Heading Needle
  float rad = (heading - 90.0f) * DEG_TO_RAD;
  int tipX = cx + (int)(cosf(rad) * (radius - 4));
  int tipY = cy + (int)(sinf(rad) * (radius - 4));
  tft.drawLine(cx, cy, tipX, tipY, TFT_RED);
  tft.drawCircle(cx, cy, 3, TFT_WHITE);
  
  // Heading Readout
  char hbuf[16];
  snprintf(hbuf, sizeof(hbuf), "%.0f deg", heading);
  tft.setTextColor(TFT_CYAN, 0x0821);
  tft.setTextDatum(BC_DATUM);
  tft.drawString(hbuf, cx, cy + radius + 10, 2);
}

void drawCoordinatesCard() {
  tft.fillRect(6, 146, 308, 60, 0x10A2);
  tft.drawRect(6, 146, 308, 60, 0x2965);
  
  tft.setTextColor(TFT_SILVER, 0x10A2);
  tft.setTextDatum(TL_DATUM);
  tft.drawString("CURRENT WGS84 COORDINATES", 14, 150, 1);
  
  tft.setTextColor(TFT_WHITE, 0x10A2);
  char buf[64];
  snprintf(buf, sizeof(buf), "LAT: %+.5f   LON: %+.5f", latitude, longitude);
  tft.drawString(buf, 14, 166, 2);
  
  snprintf(buf, sizeof(buf), "HDOP: %.1f | Precision: <2.5m CEP", hdop);
  tft.setTextColor(TFT_DARKGREY, 0x10A2);
  tft.drawString(buf, 14, 186, 2);
}

void drawBottomBar() {
  tft.fillRect(6, 212, 148, 26, 0x18C3);
  tft.drawRect(6, 212, 148, 26, TFT_DARKGREY);
  tft.setTextColor(TFT_WHITE, 0x18C3);
  tft.setTextDatum(MC_DATUM);
  tft.drawString(useMph ? "UNIT: MPH" : "UNIT: KM/H", 80, 225, 2);
  
  tft.fillRect(166, 212, 148, 26, 0x18C3);
  tft.drawRect(166, 212, 148, 26, TFT_DARKGREY);
  tft.setTextColor(TFT_GREEN, 0x18C3);
  tft.drawString("MARK WAYPOINT", 240, 225, 2);
}

void renderAll() {
  drawHeader();
  drawSpeedometerCard();
  drawCompassRose(236, 86, 44, headingDegrees);
  drawCoordinatesCard();
  drawBottomBar();
}

void simulateGPSTelemetry() {
  static uint32_t lastTick = 0;
  if (millis() - lastTick > 1000) {
    lastTick = millis();
    
    // Smooth driving trajectory
    currentSpeedKmh = constrain(currentSpeedKmh + ((rand() % 9) - 4) * 0.8f, 0.0f, 130.0f);
    headingDegrees = fmodf(headingDegrees + ((rand() % 7) - 3) * 1.5f + 360.0f, 360.0f);
    latitude += 0.00008f;
    longitude += 0.00005f;
    
    setRGB(false, true, false); // Green GPS fix
    
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
  beep(1600, 40);
  
  touchSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  ts.begin(touchSPI);
  ts.setRotation(1);
  
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  
  renderAll();
}

void loop() {
  simulateGPSTelemetry();
  
  if (ts.touched()) {
    TS_Point p = ts.getPoint();
    int touchX = map(p.x, 200, 3700, 0, 320);
    int touchY = map(p.y, 240, 3800, 0, 240);
    
    if (touchY > 210) {
      if (touchX < 160) {
        useMph = !useMph;
        beep(1800, 25);
      } else {
        beep(2400, 50);
      }
      renderAll();
      delay(250);
    }
  }
  
  delay(50);
}
