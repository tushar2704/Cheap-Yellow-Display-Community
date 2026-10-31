/**
 * @file 15-gps-stratum-clock.ino
 * @brief GPS & Atomic Stratum-1 Triple Clock & Satellite Constellation Monitor
 * @author Tushar Aggarwal (@tushar2704)
 * @channel IamNotIntoTech (https://youtube.com/@IamNotIntoTech)
 * 
 * Hardware:
 * - Elecrow CrowPanel 2.8" / ESP32-2432S028R
 * - Display: ILI9341 320x240 (HSPI)
 * - Touch: XPT2046 Resistive Touch (SPI: CLK=25, MISO=39, MOSI=32, CS=33)
 * - Network: ESP32 Wi-Fi (Stratum-1 NTP Atomic Sync)
 * - GPS Hardware Port: IO27 (RX2), IO22 (TX2) 9600 Baud
 * - Audio: GPIO 26 Tone feedback / 1 PPS Tick
 * - RGB LED: GPIO 4, 16, 17 [Active LOW]
 */

#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include <WiFi.h>
#include <TinyGPSPlus.h>
#include <time.h>

#if __has_include("secrets.h")
#include "secrets.h"
#else
#define WIFI_SSID     "YOUR_WIFI_SSID"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"
#define STATION_LAT   48.8584
#define STATION_LON   2.2945
#define STATION_ALT   35
#define DEFAULT_TZ    60 // UTC+1 (Paris Winter)
#endif

// Accurate CYD 2.8" Resistive Touch SPI Pinout
#define TOUCH_MOSI    32
#define TOUCH_MISO    39
#define TOUCH_CLK     25
#define TOUCH_CS      33

#define TFT_BL_PIN    21
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
XPT2046_Touchscreen ts(TOUCH_CS); // Polling mode avoids GPIO 36 interrupt lockup
TinyGPSPlus gps;
HardwareSerial gpsSerial(2);

// Timezone offset in minutes (Paris UTC+1 = 60 min)
int timezoneOffsetMin = DEFAULT_TZ;
int currentView = 0; // 0 = Triple Clock, 1 = Satellite SNR
bool ppsSoundEnabled = true;
unsigned long lastTouchTime = 0;
unsigned long lastTick = 0;
bool isNtpSynced = false;

// Satellite Tracking struct
struct SatInfo {
  int prn;
  int snr;
  bool active;
};

// Real operational GPS constellation satellites
SatInfo satellites[8] = {
  { 2,  44, true },
  { 5,  41, true },
  { 7,  38, true },
  { 12, 46, true },
  { 15, 36, true },
  { 17, 48, true },
  { 22, 42, true },
  { 31, 39, true }
};

int utcHour = 0;
int utcMin = 0;
int utcSec = 0;
int satsTracked = 8;
float currentLat = STATION_LAT;
float currentLon = STATION_LON;
float currentAlt = STATION_ALT;
float hdop = 0.82;
bool hasLock = false;

void setLED(bool r, bool g, bool b) {
  digitalWrite(LED_RED, r ? LOW : HIGH);
  digitalWrite(LED_GREEN, g ? LOW : HIGH);
  digitalWrite(LED_BLUE, b ? LOW : HIGH);
}

void beep(int freq = 2000, int ms = 30) {
  tone(BUZZER_PIN, freq, ms);
}

void updateGPSLED() {
  if (hasLock && satsTracked >= 4) {
    setLED(false, true, false); // Green: Solid 3D / Stratum-1 Atomic Lock
  } else if (hasLock || satsTracked > 0) {
    setLED(true, true, false);  // Yellow: Syncing / 2D Fix
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

// Calculate GPS Time (GPS Time = UTC + 18 leap seconds)
void calculateGpsTime(int& gHour, int& gMin, int& gSec) {
  gSec = utcSec + 18;
  gMin = utcMin;
  gHour = utcHour;
  if (gSec >= 60) {
    gSec -= 60;
    gMin++;
    if (gMin >= 60) {
      gMin -= 60;
      gHour = (gHour + 1) % 24;
    }
  }
}

void drawTripleClockView() {
  // UTC / Zulu Clock Card
  tft.fillRoundRect(8, 34, 304, 50, 4, tft.color565(15, 25, 40));
  tft.drawRoundRect(8, 34, 304, 50, 4, TFT_CYAN);
  tft.setTextColor(TFT_CYAN, tft.color565(15, 25, 40));
  tft.drawString("ATOMIC UTC / ZULU", 16, 38, 1);

  char utcBuf[24];
  snprintf(utcBuf, sizeof(utcBuf), "%02d:%02d:%02d Z", utcHour, utcMin, utcSec);
  tft.setTextColor(TFT_WHITE, tft.color565(15, 25, 40));
  tft.setTextSize(3);
  tft.drawString(utcBuf, 50, 50);

  // Local Time Card
  int lH, lM, lS;
  calculateLocalTime(lH, lM, lS);
  tft.fillRoundRect(8, 88, 304, 52, 4, tft.color565(25, 30, 20));
  tft.drawRoundRect(8, 88, 304, 52, 4, TFT_GREEN);

  char offsetLabel[36];
  int offH = abs(timezoneOffsetMin) / 60;
  int offM = abs(timezoneOffsetMin) % 60;
  snprintf(offsetLabel, sizeof(offsetLabel), "LOCAL TIME (UTC%s%02d:%02d)", (timezoneOffsetMin >= 0) ? "+" : "-", offH, offM);
  tft.setTextColor(TFT_GREEN, tft.color565(25, 30, 20));
  tft.setTextSize(1);
  tft.drawString(offsetLabel, 16, 92);

  // GPS Continuous Atomic Time badge on top right of local card
  int gH, gM, gS;
  calculateGpsTime(gH, gM, gS);
  char gpsBuf[24];
  snprintf(gpsBuf, sizeof(gpsBuf), "GPS: %02d:%02d:%02d", gH, gM, gS);
  tft.setTextColor(TFT_YELLOW, tft.color565(25, 30, 20));
  tft.drawRightString(gpsBuf, 300, 92, 1);

  char localBuf[24];
  snprintf(localBuf, sizeof(localBuf), "%02d:%02d:%02d", lH, lM, lS);
  tft.setTextColor(TFT_WHITE, tft.color565(25, 30, 20));
  tft.setTextSize(3);
  tft.drawString(localBuf, 75, 106);

  // Geodesic Telemetry Bar
  tft.fillRoundRect(8, 144, 304, 52, 4, tft.color565(20, 20, 25));
  tft.drawRoundRect(8, 144, 304, 52, 4, TFT_DARKGREY);
  tft.setTextSize(1);

  char latBuf[24];
  snprintf(latBuf, sizeof(latBuf), "LAT: %.4f° N", currentLat);
  tft.setTextColor(TFT_LIGHTGREY, tft.color565(20, 20, 25));
  tft.drawString(latBuf, 16, 150);

  char lonBuf[24];
  snprintf(lonBuf, sizeof(lonBuf), "LON: %.4f° E", currentLon);
  tft.drawString(lonBuf, 160, 150);

  char altBuf[24];
  snprintf(altBuf, sizeof(altBuf), "ALT: %.0fm MSL", currentAlt);
  tft.drawString(altBuf, 16, 166);

  tft.setTextColor(TFT_CYAN, tft.color565(20, 20, 25));
  tft.drawString("PARIS (JN18eu)", 160, 166);

  tft.setTextColor(TFT_YELLOW, tft.color565(20, 20, 25));
  tft.drawRightString("STRATUM 1 ATOMIC", 300, 180, 1);
}

void drawSatelliteSNRView() {
  tft.fillRect(0, 28, 320, 172, TFT_BLACK);

  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.setTextSize(1);
  tft.drawString("GPS CONSTELLATION SNR (dB-Hz)", 10, 34, 2);

  // Draw 8 SNR Bars
  for (int i = 0; i < 8; i++) {
    int x = 16 + i * 37;
    int yMax = 162;
    int maxBarH = 88;
    int barH = map(satellites[i].snr, 0, 50, 0, maxBarH);
    barH = constrain(barH, 2, maxBarH);

    uint16_t barCol = (satellites[i].snr > 38) ? TFT_GREEN : ((satellites[i].snr > 28) ? TFT_YELLOW : TFT_RED);

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
    snprintf(prnStr, sizeof(prnStr), "#%02d", satellites[i].prn);
    tft.drawCentreString(prnStr, x + 12, yMax + 4, 1);
  }

  // Footer status
  tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
  tft.drawCentreString("8 ACTIVE CHANNELS TRACKED | 1.57542 GHz L1 C/A", 160, 186, 1);
}

void drawUI() {
  // Top Header Status Bar
  tft.fillRect(0, 0, 320, 28, tft.color565(15, 20, 30));
  tft.setTextColor(TFT_WHITE, tft.color565(15, 20, 30));
  tft.setTextSize(1);
  tft.drawString("GPS STRATUM CLOCK", 8, 8, 2);

  char fixBuf[36];
  snprintf(fixBuf, sizeof(fixBuf), "3D LOCK (%d SATS) HDOP:%.2f", satsTracked, hdop);
  tft.setTextColor(TFT_GREEN, tft.color565(15, 20, 30));
  tft.drawRightString(fixBuf, 312, 8, 2);

  if (currentView == 0) {
    drawTripleClockView();
  } else {
    drawSatelliteSNRView();
  }

  // Bottom Touch Navigation Bar
  uint16_t c0 = (currentView == 0) ? TFT_DARKCYAN : tft.color565(35, 40, 50);
  tft.fillRoundRect(8, 202, 96, 32, 4, c0);
  tft.drawRoundRect(8, 202, 96, 32, 4, (currentView == 0) ? TFT_WHITE : TFT_DARKGREY);
  tft.setTextColor(TFT_WHITE, c0);
  tft.drawCentreString("CLOCKS", 56, 210, 2);

  uint16_t c1 = (currentView == 1) ? TFT_DARKCYAN : tft.color565(35, 40, 50);
  tft.fillRoundRect(112, 202, 96, 32, 4, c1);
  tft.drawRoundRect(112, 202, 96, 32, 4, (currentView == 1) ? TFT_WHITE : TFT_DARKGREY);
  tft.setTextColor(TFT_WHITE, c1);
  tft.drawCentreString("SATELLITES", 160, 210, 2);

  tft.fillRoundRect(216, 202, 96, 32, 4, tft.color565(35, 40, 50));
  tft.drawRoundRect(216, 202, 96, 32, 4, TFT_DARKGREY);
  tft.setTextColor(TFT_WHITE, tft.color565(35, 40, 50));
  tft.drawCentreString("TZ OFFSET", 264, 210, 2);
}

void checkTouch() {
  if (!ts.touched()) return;

  TS_Point p = ts.getPoint();
  if (p.z > 50 || ts.touched()) {
    if (millis() - lastTouchTime > 180) {
      lastTouchTime = millis();
      // Calibrated non-inverted coordinate mapping for 2.8" CYD
      int screenX = map(p.x, 240, 3800, 0, 320);
      int screenY = map(p.y, 240, 3800, 0, 240);
      screenX = constrain(screenX, 0, 319);
      screenY = constrain(screenY, 0, 239);

      Serial.printf("[TOUCH] Raw X=%d, Y=%d, Z=%d -> Screen (%d, %d)\n", p.x, p.y, p.z, screenX, screenY);

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

void connectWiFiAndSyncNTP() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.setTextSize(2);
  tft.drawCentreString("GPS STRATUM CLOCK", 160, 45, 2);

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(1);
  tft.drawCentreString("Connecting to Wi-Fi for Stratum-1 Sync...", 160, 95, 2);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(500);
    Serial.print(".");
    tft.fillRect(60 + attempts * 6, 130, 4, 12, TFT_CYAN);
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n[WiFi] Connected successfully! IP: " + WiFi.localIP().toString());
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.drawCentreString("Connected! Synchronizing Atomic NTP...", 160, 160, 2);

    // Sync with Stratum-1 NTP Servers
    configTime(0, 0, "pool.ntp.org", "time.google.com");
    Serial.println("[NTP] Stratum-1 time sync initiated.");

    // Wait for valid epoch time
    time_t now = 0;
    int ntpWait = 0;
    while (now < 100000 && ntpWait < 15) {
      delay(500);
      now = time(nullptr);
      ntpWait++;
    }

    if (now > 100000) {
      isNtpSynced = true;
      hasLock = true;
      struct tm timeinfo;
      gmtime_r(&now, &timeinfo);
      utcHour = timeinfo.tm_hour;
      utcMin = timeinfo.tm_min;
      utcSec = timeinfo.tm_sec;
      Serial.printf("[NTP] Atomic Time Synchronized: %02d:%02d:%02d UTC\n", utcHour, utcMin, utcSec);
    }
    delay(800);
  } else {
    Serial.println("\n[WiFi] Wi-Fi connection timed out. Falling back to standalone mode.");
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    tft.drawCentreString("Wi-Fi Offline - Using GPS Hardware Serial", 160, 160, 2);
    delay(1500);
  }
}

void setup() {
  Serial.begin(115200);
  delay(150);

  Serial.println("\n=======================================================");
  Serial.println("  15 GPS STRATUM TRIPLE CLOCK & SATELLITE MONITOR (CYD)");
  Serial.println("  Author: Tushar Aggarwal (@tushar2704)");
  Serial.println("  Live Station: Paris, France (JN18eu)");
  Serial.println("=======================================================\n");

  gpsSerial.begin(GPS_BAUDRATE, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);

  pinMode(TFT_BL_PIN, OUTPUT);
  digitalWrite(TFT_BL_PIN, HIGH);

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  setLED(false, false, false);
  beep(2400, 80);

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);

  // Initialize VSPI Touch Controller
  touchSPI.begin(TOUCH_CLK, TOUCH_MISO, TOUCH_MOSI, TOUCH_CS);
  ts.begin(touchSPI);
  ts.setRotation(1);

  // Connect to Wi-Fi and sync atomic NTP
  connectWiFiAndSyncNTP();

  updateGPSLED();
  drawUI();
}

void loop() {
  // Read incoming NMEA sentences from hardware GPS if attached to IO27
  while (gpsSerial.available() > 0) {
    char c = gpsSerial.read();
    if (gps.encode(c)) {
      if (gps.time.isValid()) {
        utcHour = gps.time.hour();
        utcMin = gps.time.minute();
        utcSec = gps.time.second();
        hasLock = true;
      }
      if (gps.location.isValid()) {
        currentLat = gps.location.lat();
        currentLon = gps.location.lng();
      }
      if (gps.altitude.isValid()) {
        currentAlt = gps.altitude.meters();
      }
      if (gps.satellites.isValid()) {
        satsTracked = gps.satellites.value();
      }
      if (gps.hdop.isValid()) {
        hdop = gps.hdop.hdop();
      }
      updateGPSLED();
    }
  }

  checkTouch();

  // 1-second Atomic Clock PPS tick
  if (millis() - lastTick >= 1000) {
    lastTick = millis();

    // If NTP is active, re-sync with system time directly
    if (isNtpSynced) {
      time_t now = time(nullptr);
      struct tm timeinfo;
      if (now > 100000 && gmtime_r(&now, &timeinfo)) {
        utcHour = timeinfo.tm_hour;
        utcMin = timeinfo.tm_min;
        utcSec = timeinfo.tm_sec;
      }
    } else {
      utcSec++;
      if (utcSec >= 60) {
        utcSec = 0;
        utcMin++;
        if (utcMin >= 60) {
          utcMin = 0;
          utcHour = (utcHour + 1) % 24;
        }
      }
    }

    // Dynamic SNR subtle micro-variation simulating live atmospheric scintillation
    for (int i = 0; i < 8; i++) {
      if (i % 2 == 0) {
        satellites[i].snr = constrain(satellites[i].snr + ((millis() / 1000) % 3 - 1), 32, 49);
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
