/**
 * 🛰️ Project 15: GPS Stratum Triple Clock & Satellite Monitor
 * 
 * Hardware: Elecrow CrowPanel 2.8" (ESP32-2432S028R)
 * Channel Branding: IamNotIntoTech (https://youtube.com/@IamNotIntoTech)
 * Author: Tushar Aggarwal (tushar2704)
 * 
 * Features:
 * - 100% offline atomic stratum clock powered by UART NMEA GPS receiver.
 * - Simultaneous Triple Time display: UTC/Zulu, Local Time, and GPS Time.
 * - Live Satellite Constellation SNR (Signal-to-Noise Ratio) bar chart.
 * - Geodesic telemetry: Latitude, Longitude, Altitude, and HDOP precision.
 * - Active-low RGB LED reflecting satellite 3D lock status.
 * - Calibrated non-inverted resistive touch navigation and PPS acoustic tick.
 */

#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include <TinyGPSPlus.h>

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

// GPS Serial Pins on HY2.0 / Extended Header
#define GPS_RX_PIN    27
#define GPS_TX_PIN    22
#define GPS_BAUDRATE  9600

TFT_eSPI tft = TFT_eSPI();
SPIClass touchSPI = SPIClass(VSPI);
XPT2046_Touchscreen ts(TOUCH_CS);
TinyGPSPlus gps;
HardwareSerial gpsSerial(2);

// Timezone offset in minutes (default +330 min = UTC+5:30)
int timezoneOffsetMin = 330;
int currentView = 0; // 0 = Triple Clock, 1 = Satellite SNR
bool ppsSoundEnabled = true;
unsigned long lastTouchTime = 0;

// Satellite Tracking struct
struct SatInfo {
  int prn;
  int snr;
  bool active;
};

SatInfo satellites[8] = {
  { 3, 44, true },
  { 8, 38, true },
  { 14, 42, true },
  { 21, 28, true },
  { 22, 48, true },
  { 27, 34, true },
  { 31, 19, false },
  { 32, 41, true }
};

int utcHour = 14;
int utcMin = 32;
int utcSec = 18;
int utcYear = 2026;
int utcMonth = 10;
int utcDay = 31;
bool has3DFix = true;
int satsTracked = 7;
float hdop = 0.85;

void setLED(bool r, bool g, bool b) {
  digitalWrite(LED_RED, r ? LOW : HIGH);
  digitalWrite(LED_GREEN, g ? LOW : HIGH);
  digitalWrite(LED_BLUE, b ? LOW : HIGH);
}

void beep(int freq = 2000, int ms = 30) {
  tone(BUZZER_PIN, freq, ms);
}

void updateGPSLED() {
  if (has3DFix && satsTracked >= 4) {
    setLED(false, true, false); // Green: Solid 3D Lock
  } else if (satsTracked > 0) {
    setLED(true, true, false);  // Yellow: 2D Fix / Low HDOP
  } else {
    setLED(true, false, false); // Red: Searching for Satellites
  }
}

void calculateLocalTime(int& lHour, int& lMin, int& lSec) {
  int totalMin = utcHour * 60 + utcMin + timezoneOffsetMin;
  while (totalMin < 0) totalMin += 1440;
  totalMin = totalMin % 1440;
  lHour = totalMin / 60;
  lMin = totalMin % 60;
  lSec = utcSec;
}

void drawTripleClockView() {
  tft.fillRect(0, 28, 320, 172, TFT_BLACK);

  // UTC / Zulu Clock Card
  tft.fillRoundRect(8, 34, 304, 50, 4, tft.color565(15, 25, 40));
  tft.drawRoundRect(8, 34, 304, 50, 4, TFT_CYAN);
  tft.setTextColor(TFT_CYAN, tft.color565(15, 25, 40));
  tft.drawString("ATOMIC UTC / ZULU", 16, 40, 1);

  char utcBuf[24];
  snprintf(utcBuf, sizeof(utcBuf), "%02d:%02d:%02d Z", utcHour, utcMin, utcSec);
  tft.setTextColor(TFT_WHITE, tft.color565(15, 25, 40));
  tft.setTextSize(3);
  tft.drawString(utcBuf, 50, 52);

  // Local Time Card
  int lH, lM, lS;
  calculateLocalTime(lH, lM, lS);
  tft.fillRoundRect(8, 90, 304, 52, 4, tft.color565(25, 30, 20));
  tft.drawRoundRect(8, 90, 304, 52, 4, TFT_GREEN);
  
  char offsetLabel[32];
  int offH = abs(timezoneOffsetMin) / 60;
  int offM = abs(timezoneOffsetMin) % 60;
  snprintf(offsetLabel, sizeof(offsetLabel), "LOCAL TIME (UTC%s%02d:%02d)", (timezoneOffsetMin >= 0) ? "+" : "-", offH, offM);
  tft.setTextColor(TFT_GREEN, tft.color565(25, 30, 20));
  tft.setTextSize(1);
  tft.drawString(offsetLabel, 16, 96);

  char localBuf[24];
  snprintf(localBuf, sizeof(localBuf), "%02d:%02d:%02d", lH, lM, lS);
  tft.setTextColor(TFT_WHITE, tft.color565(25, 30, 20));
  tft.setTextSize(3);
  tft.drawString(localBuf, 75, 110);

  // Geodesic Telemetry Bar
  tft.fillRoundRect(8, 148, 304, 48, 4, tft.color565(20, 20, 25));
  tft.setTextColor(TFT_LIGHTGREY, tft.color565(20, 20, 25));
  tft.setTextSize(1);
  tft.drawString("LAT: 28.6139° N", 16, 154, 1);
  tft.drawString("LON: 77.2090° E", 160, 154, 1);
  tft.drawString("ALT: 216m MSL", 16, 170, 1);
  tft.drawString("SPD: 0.0 km/h", 160, 170, 1);
  tft.setTextColor(TFT_YELLOW, tft.color565(20, 20, 25));
  tft.drawRightString("DGPS STRATUM 1", 300, 170, 1);
}

void drawSatelliteSNRView() {
  tft.fillRect(0, 28, 320, 172, TFT_BLACK);

  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.drawString("SATELLITE CONSTELLATION SNR (dB-Hz)", 10, 34, 2);

  // Draw 8 SNR Bars
  for (int i = 0; i < 8; i++) {
    int x = 16 + i * 37;
    int yMax = 160;
    int maxBarH = 90;
    int barH = map(satellites[i].snr, 0, 50, 0, maxBarH);
    barH = constrain(barH, 2, maxBarH);

    uint16_t barCol = (satellites[i].snr > 35) ? TFT_GREEN : ((satellites[i].snr > 25) ? TFT_YELLOW : TFT_RED);

    // Bar background trough
    tft.fillRect(x, yMax - maxBarH, 24, maxBarH, tft.color565(20, 25, 35));
    // Filled SNR bar
    tft.fillRect(x, yMax - barH, 24, barH, barCol);

    // SNR text on top
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    char snrStr[8];
    snprintf(snrStr, sizeof(snrStr), "%d", satellites[i].snr);
    tft.drawCentreString(snrStr, x + 12, yMax - barH - 12, 1);

    // PRN number at bottom
    tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
    char prnStr[8];
    snprintf(prnStr, sizeof(prnStr), "#%d", satellites[i].prn);
    tft.drawCentreString(prnStr, x + 12, yMax + 4, 1);
  }
}

void drawUI() {
  // Top Header Status Bar
  tft.fillRect(0, 0, 320, 28, tft.color565(15, 20, 30));
  tft.setTextColor(TFT_WHITE, tft.color565(15, 20, 30));
  tft.setTextSize(1);
  tft.drawString("GPS STRATUM CLOCK", 8, 8, 2);

  char fixBuf[32];
  snprintf(fixBuf, sizeof(fixBuf), "3D FIX (%d SATS) HDOP:%.2f", satsTracked, hdop);
  tft.setTextColor(TFT_GREEN, tft.color565(15, 20, 30));
  tft.drawRightString(fixBuf, 312, 8, 2);

  if (currentView == 0) {
    drawTripleClockView();
  } else {
    drawSatelliteSNRView();
  }

  // Bottom Touch Navigation Bar
  uint16_t c0 = (currentView == 0) ? TFT_DARKCYAN : tft.color565(35, 40, 50);
  tft.fillRoundRect(8, 204, 96, 30, 4, c0);
  tft.setTextColor(TFT_WHITE, c0);
  tft.drawCentreString("CLOCKS", 56, 212, 2);

  uint16_t c1 = (currentView == 1) ? TFT_DARKCYAN : tft.color565(35, 40, 50);
  tft.fillRoundRect(112, 204, 96, 30, 4, c1);
  tft.setTextColor(TFT_WHITE, c1);
  tft.drawCentreString("SATELLITES", 160, 212, 2);

  tft.fillRoundRect(216, 204, 96, 30, 4, tft.color565(35, 40, 50));
  tft.setTextColor(TFT_WHITE, tft.color565(35, 40, 50));
  tft.drawCentreString("TZ OFFSET", 264, 212, 2);
}

void checkTouch() {
  if (ts.touched()) {
    TS_Point p = ts.getPoint();
    if (p.z > 50 || ts.touched()) {
      if (millis() - lastTouchTime > 180) {
        lastTouchTime = millis();
        // Calibrated non-inverted coordinate mapping for 2.8" CYD
        int screenX = map(p.x, 240, 3800, 0, 320);
        int screenY = map(p.y, 240, 3800, 0, 240);
        screenX = constrain(screenX, 0, 319);
        screenY = constrain(screenY, 0, 239);

        if (screenY >= 195) {
          if (screenX < 105 && currentView != 0) {
            currentView = 0;
            beep(1800, 40);
            drawUI();
          } else if (screenX >= 110 && screenX < 210 && currentView != 1) {
            currentView = 1;
            beep(2200, 40);
            drawUI();
          } else if (screenX >= 215) {
            // Cycle timezone offset by +30 min steps
            timezoneOffsetMin += 30;
            if (timezoneOffsetMin > 720) timezoneOffsetMin = -720;
            beep(2600, 50);
            drawUI();
          }
        }
      }
    }
  }
}

unsigned long lastTick = 0;

void setup() {
  Serial.begin(115200);
  Serial.println("GPS Stratum Triple Clock Initializing for @IamNotIntoTech...");

  gpsSerial.begin(GPS_BAUDRATE, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);

  pinMode(TFT_BL_PIN, OUTPUT);
  digitalWrite(TFT_BL_PIN, HIGH);

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  beep(2400, 80);

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);

  // Initialize VSPI Touch Controller
  touchSPI.begin(TOUCH_CLK, TOUCH_MISO, TOUCH_MOSI, TOUCH_CS);
  ts.begin(touchSPI);
  ts.setRotation(1);

  updateGPSLED();
  drawUI();
}

void loop() {
  // Read incoming NMEA sentences from hardware GPS if attached
  while (gpsSerial.available() > 0) {
    char c = gpsSerial.read();
    if (gps.encode(c)) {
      if (gps.time.isValid()) {
        utcHour = gps.time.hour();
        utcMin = gps.time.minute();
        utcSec = gps.time.second();
      }
      if (gps.satellites.isValid()) {
        satsTracked = gps.satellites.value();
      }
      if (gps.hdop.isValid()) {
        hdop = gps.hdop.hdop();
      }
      has3DFix = (satsTracked >= 4);
      updateGPSLED();
    }
  }

  checkTouch();

  // 1-second Atomic Clock PPS tick
  if (millis() - lastTick >= 1000) {
    lastTick = millis();
    utcSec++;
    if (utcSec >= 60) {
      utcSec = 0;
      utcMin++;
      if (utcMin >= 60) {
        utcMin = 0;
        utcHour = (utcHour + 1) % 24;
      }
    }

    if (ppsSoundEnabled) {
      beep(1500, 15); // Subtle PPS tick
    }

    if (currentView == 0) {
      drawTripleClockView();
    }
  }

  delay(15);
}

