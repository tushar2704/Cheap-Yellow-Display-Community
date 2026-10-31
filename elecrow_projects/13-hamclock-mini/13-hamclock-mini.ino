/**
 * @file 13-hamclock-mini.ino
 * @brief HamClock Mini Dashboard - Solar Conditions & POTA/SOTA Tracker
 * @author Tushar Aggarwal (@tushar2704)
 * @channel IamNotIntoTech (https://youtube.com/@IamNotIntoTech)
 * 
 * Hardware:
 * - Elecrow CrowPanel 2.8" / ESP32-2432S028R
 * - Display: ILI9341 320x240
 * - Touch: XPT2046 Resistive Touch (SPI: CLK=25, MISO=39, MOSI=32, CS=33)
 * - Network: ESP32 Wi-Fi
 * - Audio: GPIO 26 Tone feedback
 * - RGB LED: GPIO 4, 16, 17 [Active LOW]
 */

#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include <WiFi.h>

// Accurate CYD 2.8" Resistive Touch SPI Pinout
#define TOUCH_MOSI    32
#define TOUCH_MISO    39
#define TOUCH_CLK     25
#define TOUCH_CS      33
#define TOUCH_IRQ     36

#define TFT_BL_PIN    21
#define LED_RED       4
#define LED_GREEN     16
#define LED_BLUE      17
#define BUZZER_PIN    26

TFT_eSPI tft = TFT_eSPI();
SPIClass touchSPI = SPIClass(VSPI);
XPT2046_Touchscreen ts(TOUCH_CS); // Polling mode avoids GPIO 36 interrupt lockup

// WiFi Credentials
const char* ssid = "HOMELAB_WIFI";
const char* password = "homelab_secret";

// Ham Radio Data Structure
struct SolarData {
  int sfi;
  int ssn;
  int aIndex;
  int kIndex;
  char aurora[16];
  char solarWind[16];
};

struct BandCondition {
  const char* band;
  const char* day;
  const char* night;
  uint16_t dayColor;
  uint16_t nightColor;
};

struct PotaSpot {
  const char* activator;
  const char* freq;
  const char* mode;
  const char* ref;
  const char* grid;
  int distanceKm;
  int bearingDeg;
};

SolarData currentSolar = { 158, 112, 8, 2, "Normal", "415 km/s" };

BandCondition bands[5] = {
  { "80m", "Poor", "Good", TFT_DARKGREY, TFT_GREEN },
  { "40m", "Fair", "Good", TFT_YELLOW,   TFT_GREEN },
  { "20m", "Good", "Fair", TFT_GREEN,    TFT_YELLOW },
  { "15m", "Good", "Poor", TFT_GREEN,    TFT_DARKGREY },
  { "10m", "Fair", "Poor", TFT_YELLOW,   TFT_DARKGREY }
};

PotaSpot spots[4] = {
  { "W3AAX", "14.285", "SSB", "K-1248", "FM19", 420, 245 },
  { "K4NYM", "7.032",  "CW",  "K-0034", "EM85", 890, 210 },
  { "VA3PJS", "21.074", "FT8", "VE-019", "FN03", 650, 315 },
  { "DL6MHW", "14.040", "CW",  "DL-005", "JO42", 6400, 48 }
};

int currentTab = 0; // 0 = Solar & Bands, 1 = POTA Spots
int selectedSpot = 0;
unsigned long lastUpdate = 0;
unsigned long lastClockTick = 0;
unsigned long lastTouchTime = 0;
int utcHour = 18;
int utcMin = 42;
int utcSec = 15;

void setLED(bool r, bool g, bool b) {
  digitalWrite(LED_RED, r ? LOW : HIGH);
  digitalWrite(LED_GREEN, g ? LOW : HIGH);
  digitalWrite(LED_BLUE, b ? LOW : HIGH);
}

void beep(int freq = 2000, int ms = 50) {
  tone(BUZZER_PIN, freq, ms);
}

void updateLEDStatus() {
  if (currentSolar.kIndex <= 2) {
    setLED(false, true, false); // Green: Quiet / Normal
  } else if (currentSolar.kIndex <= 4) {
    setLED(true, true, false);  // Yellow: Unsettled
  } else {
    setLED(true, false, false); // Red: Geomagnetic Storm Warning!
  }
}

void drawHeader() {
  tft.fillRect(0, 0, 320, 30, TFT_NAVY);
  tft.setTextColor(TFT_CYAN, TFT_NAVY);
  tft.setTextSize(2);
  tft.drawString("HAMCLOCK MINI", 8, 6);

  // UTC Time display
  char timeBuf[16];
  snprintf(timeBuf, sizeof(timeBuf), "%02d:%02d:%02d Z", utcHour, utcMin, utcSec);
  tft.setTextColor(TFT_WHITE, TFT_NAVY);
  tft.drawRightString(timeBuf, 312, 6, 2);
}

void drawTabs() {
  // Tab 0: Solar & HF Bands
  uint16_t c0 = (currentTab == 0) ? TFT_DARKCYAN : TFT_DARKGREY;
  tft.fillRoundRect(10, 208, 145, 28, 4, c0);
  tft.drawRoundRect(10, 208, 145, 28, 4, (currentTab == 0) ? TFT_WHITE : TFT_BLACK);
  tft.setTextColor(TFT_WHITE, c0);
  tft.setTextSize(1);
  tft.drawCentreString("SOLAR & HF BANDS", 82, 216, 2);

  // Tab 1: POTA/SOTA Spots
  uint16_t c1 = (currentTab == 1) ? TFT_DARKCYAN : TFT_DARKGREY;
  tft.fillRoundRect(165, 208, 145, 28, 4, c1);
  tft.drawRoundRect(165, 208, 145, 28, 4, (currentTab == 1) ? TFT_WHITE : TFT_BLACK);
  tft.setTextColor(TFT_WHITE, c1);
  tft.drawCentreString("POTA / SOTA SPOTS", 237, 216, 2);
}

void drawSolarTab() {
  tft.fillRect(0, 31, 320, 175, TFT_BLACK);

  // SFI Card
  tft.fillRoundRect(8, 36, 70, 52, 4, tft.color565(20, 35, 55));
  tft.drawRoundRect(8, 36, 70, 52, 4, TFT_CYAN);
  tft.setTextColor(TFT_LIGHTGREY, tft.color565(20, 35, 55));
  tft.drawString("SFI", 16, 42, 1);
  tft.setTextColor(TFT_YELLOW, tft.color565(20, 35, 55));
  tft.setTextSize(2);
  tft.drawNumber(currentSolar.sfi, 16, 56);

  // SSN Card
  tft.fillRoundRect(84, 36, 70, 52, 4, tft.color565(20, 35, 55));
  tft.drawRoundRect(84, 36, 70, 52, 4, TFT_CYAN);
  tft.setTextColor(TFT_LIGHTGREY, tft.color565(20, 35, 55));
  tft.drawString("SUNSPOTS", 90, 42, 1);
  tft.setTextColor(TFT_ORANGE, tft.color565(20, 35, 55));
  tft.setTextSize(2);
  tft.drawNumber(currentSolar.ssn, 92, 56);

  // A-Index Card
  tft.fillRoundRect(160, 36, 70, 52, 4, tft.color565(20, 35, 55));
  tft.drawRoundRect(160, 36, 70, 52, 4, TFT_CYAN);
  tft.setTextColor(TFT_LIGHTGREY, tft.color565(20, 35, 55));
  tft.drawString("A-INDEX", 168, 42, 1);
  tft.setTextColor(TFT_GREEN, tft.color565(20, 35, 55));
  tft.setTextSize(2);
  tft.drawNumber(currentSolar.aIndex, 170, 56);

  // K-Index Card with Color Gauge
  tft.fillRoundRect(236, 36, 76, 52, 4, tft.color565(20, 35, 55));
  uint16_t kCol = (currentSolar.kIndex <= 2) ? TFT_GREEN : ((currentSolar.kIndex <= 4) ? TFT_YELLOW : TFT_RED);
  tft.drawRoundRect(236, 36, 76, 52, 4, kCol);
  tft.setTextColor(TFT_LIGHTGREY, tft.color565(20, 35, 55));
  tft.drawString("K-INDEX", 244, 42, 1);
  tft.setTextColor(kCol, tft.color565(20, 35, 55));
  tft.setTextSize(2);
  tft.drawNumber(currentSolar.kIndex, 246, 56);

  // HF Propagation Matrix Header
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.setTextSize(1);
  tft.drawString("BAND PROPAGATION FORECAST", 12, 96, 2);
  tft.drawString("DAY", 175, 96, 2);
  tft.drawString("NIGHT", 245, 96, 2);

  tft.drawFastHLine(10, 116, 300, TFT_DARKGREY);

  for (int i = 0; i < 5; i++) {
    int y = 120 + i * 16;
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString(bands[i].band, 20, y, 1);

    tft.setTextColor(bands[i].dayColor, TFT_BLACK);
    tft.drawString(bands[i].day, 175, y, 1);

    tft.setTextColor(bands[i].nightColor, TFT_BLACK);
    tft.drawString(bands[i].night, 245, y, 1);
  }
}

void drawPotaTab() {
  tft.fillRect(0, 31, 320, 175, TFT_BLACK);

  // Top subheader
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.drawString("LIVE PARKS & SUMMITS (POTA/SOTA)", 10, 36, 2);

  for (int i = 0; i < 4; i++) {
    int y = 58 + i * 35;
    uint16_t bg = (i == selectedSpot) ? tft.color565(25, 55, 75) : tft.color565(15, 20, 30);
    uint16_t border = (i == selectedSpot) ? TFT_CYAN : TFT_DARKGREY;

    tft.fillRoundRect(8, y, 304, 32, 4, bg);
    tft.drawRoundRect(8, y, 304, 32, 4, border);

    // Activator & Frequency
    tft.setTextColor(TFT_YELLOW, bg);
    tft.setTextSize(2);
    tft.drawString(spots[i].activator, 16, y + 4);

    tft.setTextColor(TFT_WHITE, bg);
    tft.setTextSize(2);
    tft.drawString(spots[i].freq, 110, y + 4);

    tft.setTextColor(TFT_GREEN, bg);
    tft.setTextSize(1);
    tft.drawString(spots[i].mode, 185, y + 6, 2);

    // Park Ref & Distance
    tft.setTextColor(TFT_MAGENTA, bg);
    tft.drawString(spots[i].ref, 230, y + 6, 2);

    char distBuf[24];
    snprintf(distBuf, sizeof(distBuf), "%d km @ %d deg", spots[i].distanceKm, spots[i].bearingDeg);
    tft.setTextColor(TFT_LIGHTGREY, bg);
    tft.drawString(distBuf, 110, y + 20, 1);
  }
}

void drawUI() {
  drawHeader();
  if (currentTab == 0) {
    drawSolarTab();
  } else {
    drawPotaTab();
  }
  drawTabs();
}

void checkTouch() {
  if (!ts.touched()) return;

  TS_Point p = ts.getPoint();
  if (p.z > 50 || ts.touched()) {
    if (millis() - lastTouchTime > 180) {
      lastTouchTime = millis();

      // Landscape calibration: non-inverted Y
      int x = map(p.x, 240, 3800, 0, 320);
      int y = map(p.y, 240, 3800, 0, 240);
      x = constrain(x, 0, 319);
      y = constrain(y, 0, 239);

      Serial.printf("[TOUCH] Raw X=%d, Y=%d, Z=%d -> Screen (%d, %d)\n", p.x, p.y, p.z, x, y);

      // Bottom Tabs switching (Y >= 200)
      if (y >= 200) {
        if (x < 160 && currentTab != 0) {
          currentTab = 0;
          beep(1800, 40);
          drawUI();
        } else if (x >= 160 && currentTab != 1) {
          currentTab = 1;
          beep(2200, 40);
          drawUI();
        }
      } else if (currentTab == 1 && y >= 58 && y < 200) {
        // Spot selection in POTA tab
        int spotIdx = (y - 58) / 35;
        if (spotIdx >= 0 && spotIdx < 4) {
          selectedSpot = spotIdx;
          beep(2500, 60);
          drawPotaTab();
        }
      }
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(150);

  Serial.println("\n--- [Elecrow 2.8 CYD] 13 HamClock Mini Dashboard ---");
  Serial.println("Channel: @IamNotIntoTech");
  Serial.println("Author: Tushar Aggarwal (@tushar2704)");

  pinMode(TFT_BL_PIN, OUTPUT);
  digitalWrite(TFT_BL_PIN, HIGH);

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  setLED(false, false, false);
  beep(1500, 80);

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);

  // Initialize touch on dedicated CYD SPI pins
  touchSPI.begin(TOUCH_CLK, TOUCH_MISO, TOUCH_MOSI, TOUCH_CS);
  ts.begin(touchSPI);
  ts.setRotation(1);

  updateLEDStatus();
  drawUI();
}

void loop() {
  checkTouch();

  // 1-second clock tick
  if (millis() - lastClockTick >= 1000) {
    lastClockTick = millis();
    utcSec++;
    if (utcSec >= 60) {
      utcSec = 0;
      utcMin++;
      if (utcMin >= 60) {
        utcMin = 0;
        utcHour = (utcHour + 1) % 24;
      }
    }
    // Update only the clock in the top bar to avoid flicker
    char timeBuf[16];
    snprintf(timeBuf, sizeof(timeBuf), "%02d:%02d:%02d Z", utcHour, utcMin, utcSec);
    tft.setTextColor(TFT_WHITE, TFT_NAVY);
    tft.drawRightString(timeBuf, 312, 6, 2);
  }

  // Periodic telemetry refresh simulation (every 30 seconds)
  if (millis() - lastUpdate >= 30000) {
    lastUpdate = millis();
    currentSolar.sfi = 155 + (millis() / 1000) % 8;
    currentSolar.kIndex = 2 + ((millis() / 30000) % 3);
    updateLEDStatus();
    if (currentTab == 0) {
      drawSolarTab();
    }
  }

  delay(15);
}

