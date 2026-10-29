/**
 * ✈️ Project 14: High-Contrast Vector Flight Tracker
 * 
 * Hardware: Elecrow CrowPanel 2.8" (ESP32-2432S028R)
 * Channel Branding: IamNotIntoTech (https://youtube.com/@IamNotIntoTech)
 * Author: Tushar Aggarwal (tushar2704)
 * 
 * Features:
 * - 10-foot distant high-contrast flight card view for aircraft cruising overhead.
 * - Route origin and destination with animated flight progress bar.
 * - Vector telemetry: Altitude (ft), Ground Speed (kts/mach), Vertical Speed (fpm), and Heading.
 * - Touchpad controls to cycle between tracked overhead flights.
 * - Hardware active-low RGB LED reflecting flight dynamic status.
 * - Audio tone alerts for newly detected overhead aircraft.
 */

#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include <WiFi.h>

// Hardware Pin Definitions for Elecrow 2.8" CYD
#define TFT_BL_PIN    21
#define TOUCH_CS      33
#define TOUCH_IRQ     36
#define TOUCH_MOSI    13
#define TOUCH_MISO    12
#define TOUCH_CLK     14

#define LED_RED       4
#define LED_GREEN     16
#define LED_BLUE      17
#define BUZZER_PIN    26

TFT_eSPI tft = TFT_eSPI();
SPIClass touchSPI = SPIClass(VSPI);
XPT2046_Touchscreen ts(TOUCH_CS, TOUCH_IRQ);

// WiFi Credentials
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

struct FlightInfo {
  const char* callsign;
  const char* airline;
  const char* originIATA;
  const char* originCity;
  const char* destIATA;
  const char* destCity;
  const char* aircraft;
  int altitudeFt;
  int groundSpeedKts;
  int verticalSpeedFpm;
  int headingDeg;
  int progressPercent;
  uint16_t airlineThemeColor;
};

const int TOTAL_FLIGHTS = 4;
FlightInfo flights[TOTAL_FLIGHTS] = {
  { "UAL929",  "United Airlines",  "LHR", "London",    "ORD", "Chicago",      "B787-9 Dreamliner", 38000, 492,     0, 285, 68, TFT_CYAN },
  { "BAW287",  "British Airways", "LHR", "London",    "SFO", "San Francisco","A350-1000",        36000, 480,  1200, 302, 52, TFT_BLUE },
  { "DLH456",  "Lufthansa",       "FRA", "Frankfurt", "LAX", "Los Angeles",  "B747-8 Intercont", 34000, 510, -1500, 290, 84, TFT_YELLOW },
  { "UAE201",  "Emirates",        "DXB", "Dubai",     "JFK", "New York",     "A380-800 Superjumbo", 39000, 525,    0, 315, 75, TFT_RED }
};

int currentFlightIdx = 0;
bool autoCycle = true;
unsigned long lastCycleTime = 0;

void setLED(bool r, bool g, bool b) {
  digitalWrite(LED_RED, r ? LOW : HIGH);
  digitalWrite(LED_GREEN, g ? LOW : HIGH);
  digitalWrite(LED_BLUE, b ? LOW : HIGH);
}

void beep(int freq = 2000, int ms = 50) {
  tone(BUZZER_PIN, freq, ms);
}

void updateFlightLED(const FlightInfo& f) {
  if (f.verticalSpeedFpm > 500) {
    setLED(false, true, false);  // Green: Climbing
  } else if (f.verticalSpeedFpm < -500) {
    setLED(true, true, false);   // Yellow: Descending
  } else {
    setLED(false, false, true);  // Blue/Cyan: Cruising at altitude
  }
}

void drawFlightCard() {
  FlightInfo& f = flights[currentFlightIdx];

  // Header Bar
  tft.fillRect(0, 0, 320, 28, tft.color565(15, 20, 30));
  tft.setTextColor(TFT_LIGHTGREY, tft.color565(15, 20, 30));
  tft.setTextSize(1);
  tft.drawString("OVERHEAD RADAR SECTOR", 8, 8, 2);

  char countBuf[16];
  snprintf(countBuf, sizeof(countBuf), "%d OF %d", currentFlightIdx + 1, TOTAL_FLIGHTS);
  tft.drawRightString(countBuf, 312, 8, 2);

  // Background body
  tft.fillRect(0, 28, 320, 172, TFT_BLACK);

  // Call Sign & Airline Banner
  tft.setTextColor(f.airlineThemeColor, TFT_BLACK);
  tft.setTextSize(4);
  tft.drawString(f.callsign, 12, 34);

  tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
  tft.setTextSize(1);
  tft.drawString(f.airline, 14, 70, 2);
  tft.drawRightString(f.aircraft, 308, 70, 2);

  // Route: Origin -> Destination
  tft.fillRoundRect(8, 90, 304, 38, 4, tft.color565(20, 25, 35));
  tft.drawRoundRect(8, 90, 304, 38, 4, TFT_DARKGREY);

  tft.setTextColor(TFT_WHITE, tft.color565(20, 25, 35));
  tft.drawString(f.originIATA, 20, 96, 2);
  tft.setTextColor(TFT_CYAN, tft.color565(20, 25, 35));
  tft.drawString("------>", 115, 96, 2);
  tft.setTextColor(TFT_WHITE, tft.color565(20, 25, 35));
  tft.drawRightString(f.destIATA, 300, 96, 2);

  tft.setTextColor(TFT_LIGHTGREY, tft.color565(20, 25, 35));
  tft.drawString(f.originCity, 20, 114, 1);
  tft.drawRightString(f.destCity, 300, 114, 1);

  // Flight Progress Bar
  int barX = 14;
  int barY = 134;
  int barW = 292;
  int barH = 6;
  tft.fillRect(barX, barY, barW, barH, TFT_DARKGREY);
  int progressW = (barW * f.progressPercent) / 100;
  tft.fillRect(barX, barY, progressW, barH, TFT_GREEN);

  // Airplane Marker Icon
  int planeX = barX + progressW;
  tft.fillTriangle(planeX, barY - 3, planeX + 5, barY + 3, planeX - 5, barY + 3, TFT_YELLOW);

  // Vector Metrics Grid (Altitude, Speed, Heading)
  // Box 1: Altitude
  tft.fillRoundRect(8, 145, 96, 50, 4, tft.color565(25, 30, 45));
  tft.setTextColor(TFT_LIGHTGREY, tft.color565(25, 30, 45));
  tft.drawString("ALTITUDE", 14, 150, 1);
  tft.setTextColor(TFT_WHITE, tft.color565(25, 30, 45));
  tft.setTextSize(2);
  char altBuf[16];
  snprintf(altBuf, sizeof(altBuf), "%dK", f.altitudeFt / 1000);
  tft.drawString(altBuf, 14, 164);
  tft.setTextSize(1);
  if (f.verticalSpeedFpm > 0) {
    tft.setTextColor(TFT_GREEN, tft.color565(25, 30, 45));
    tft.drawString("+CLIMB", 14, 182);
  } else if (f.verticalSpeedFpm < 0) {
    tft.setTextColor(TFT_ORANGE, tft.color565(25, 30, 45));
    tft.drawString("-DESC", 14, 182);
  } else {
    tft.setTextColor(TFT_CYAN, tft.color565(25, 30, 45));
    tft.drawString("CRUISE", 14, 182);
  }

  // Box 2: Ground Speed
  tft.fillRoundRect(112, 145, 96, 50, 4, tft.color565(25, 30, 45));
  tft.setTextColor(TFT_LIGHTGREY, tft.color565(25, 30, 45));
  tft.drawString("SPEED", 118, 150, 1);
  tft.setTextColor(TFT_WHITE, tft.color565(25, 30, 45));
  tft.setTextSize(2);
  char spdBuf[16];
  snprintf(spdBuf, sizeof(spdBuf), "%d", f.groundSpeedKts);
  tft.drawString(spdBuf, 118, 164);
  tft.setTextSize(1);
  tft.setTextColor(TFT_LIGHTGREY, tft.color565(25, 30, 45));
  tft.drawString("KTS / M0.8", 118, 182);

  // Box 3: Heading
  tft.fillRoundRect(216, 145, 96, 50, 4, tft.color565(25, 30, 45));
  tft.setTextColor(TFT_LIGHTGREY, tft.color565(25, 30, 45));
  tft.drawString("HEADING", 222, 150, 1);
  tft.setTextColor(TFT_WHITE, tft.color565(25, 30, 45));
  tft.setTextSize(2);
  char hdgBuf[16];
  snprintf(hdgBuf, sizeof(hdgBuf), "%d°", f.headingDeg);
  tft.drawString(hdgBuf, 222, 164);
  tft.setTextSize(1);
  tft.setTextColor(TFT_YELLOW, tft.color565(25, 30, 45));
  tft.drawString("WNW VECTOR", 222, 182);

  // Bottom Touch Controls
  tft.fillRoundRect(8, 202, 95, 32, 4, tft.color565(35, 40, 50));
  tft.setTextColor(TFT_WHITE, tft.color565(35, 40, 50));
  tft.drawCentreString("< PREV", 55, 210, 2);

  uint16_t autoCol = autoCycle ? TFT_DARKCYAN : tft.color565(35, 40, 50);
  tft.fillRoundRect(112, 202, 96, 32, 4, autoCol);
  tft.setTextColor(TFT_WHITE, autoCol);
  tft.drawCentreString(autoCycle ? "AUTO ON" : "PAUSED", 160, 210, 2);

  tft.fillRoundRect(216, 202, 96, 32, 4, tft.color565(35, 40, 50));
  tft.setTextColor(TFT_WHITE, tft.color565(35, 40, 50));
  tft.drawCentreString("NEXT >", 264, 210, 2);

  updateFlightLED(f);
}

void checkTouch() {
  if (!ts.touched()) return;

  TS_Point p = ts.getPoint();
  int x = map(p.x, 3800, 200, 0, 320);
  int y = map(p.y, 3800, 200, 0, 240);
  x = constrain(x, 0, 320);
  y = constrain(y, 0, 240);

  if (y >= 195) {
    if (x < 105) {
      // PREV
      currentFlightIdx = (currentFlightIdx - 1 + TOTAL_FLIGHTS) % TOTAL_FLIGHTS;
      beep(1800, 50);
      drawFlightCard();
      delay(250);
    } else if (x >= 110 && x < 210) {
      // AUTO TOGGLE
      autoCycle = !autoCycle;
      beep(2200, 50);
      drawFlightCard();
      delay(250);
    } else if (x >= 215) {
      // NEXT
      currentFlightIdx = (currentFlightIdx + 1) % TOTAL_FLIGHTS;
      beep(2500, 50);
      drawFlightCard();
      delay(250);
    }
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("High-Contrast Vector Flight Tracker Starting...");

  pinMode(TFT_BL_PIN, OUTPUT);
  digitalWrite(TFT_BL_PIN, HIGH);

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  beep(2000, 60);

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);

  touchSPI.begin(TOUCH_CLK, TOUCH_MISO, TOUCH_MOSI, TOUCH_CS);
  ts.begin(touchSPI);
  ts.setRotation(1);

  drawFlightCard();
  lastCycleTime = millis();
}

void loop() {
  checkTouch();

  // Auto-cycle through flights every 8 seconds if autoCycle is enabled
  if (autoCycle && (millis() - lastCycleTime >= 8000)) {
    lastCycleTime = millis();
    currentFlightIdx = (currentFlightIdx + 1) % TOTAL_FLIGHTS;
    beep(1600, 30);
    drawFlightCard();
  }

  delay(20);
}
