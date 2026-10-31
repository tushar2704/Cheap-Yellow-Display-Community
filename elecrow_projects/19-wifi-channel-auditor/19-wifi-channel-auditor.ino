/**
 * 📡 Project 19: Wi-Fi Channel & Spectrum Auditor
 * 
 * Hardware: Elecrow CrowPanel 2.8" (ESP32-2432S028R)
 * Channel Branding: IamNotIntoTech (https://youtube.com/@IamNotIntoTech)
 * Author: Tushar Aggarwal (tushar2704)
 * 
 * Features:
 * - 2.4GHz full-band spectrum analyzer across 802.11 channels 1 to 14.
 * - Channel overlapping parabola curves and RSSI signal waterfall.
 * - Automated Best Channel recommendation algorithm (evaluates co-channel & adjacent interference).
 * - Detailed Access Point (AP) list table with SSID, Channel, dBm, and Encryption type.
 * - Hardware active-low RGB LED indicating overall spectrum congestion.
 * - Acoustic scan completion chime.
 * - Calibrated non-inverted resistive touch controls.
 */

#include <Arduino.h>
#include <SPI.h>
#include <WiFi.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include <math.h>

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

TFT_eSPI tft = TFT_eSPI();
SPIClass touchSPI = SPIClass(VSPI);
XPT2046_Touchscreen ts(TOUCH_CS);

int currentView = 0; // 0 = Spectrum Curves, 1 = AP List Table
int totalAPs = 0;
int channelCount[15] = {0};
int channelMaxRssi[15] = {0};
int bestChannel = 1;
bool isScanning = false;
unsigned long lastTouchTime = 0;

struct APInfo {
  char ssid[32];
  int rssi;
  int channel;
  const char* encType;
};

const int MAX_APS = 16;
APInfo apList[MAX_APS];
int apCount = 0;

void setLED(bool r, bool g, bool b) {
  digitalWrite(LED_RED, r ? LOW : HIGH);
  digitalWrite(LED_GREEN, g ? LOW : HIGH);
  digitalWrite(LED_BLUE, b ? LOW : HIGH);
}

void beep(int freq = 2000, int ms = 40) {
  tone(BUZZER_PIN, freq, ms);
}

const char* getEncTypeStr(wifi_auth_mode_t authmode) {
  switch (authmode) {
    case WIFI_AUTH_OPEN: return "OPEN";
    case WIFI_AUTH_WEP: return "WEP";
    case WIFI_AUTH_WPA_PSK: return "WPA";
    case WIFI_AUTH_WPA2_PSK: return "WPA2";
    case WIFI_AUTH_WPA_WPA2_PSK: return "WPA/2";
    case WIFI_AUTH_WPA3_PSK: return "WPA3";
    default: return "SECURE";
  }
}

void performScan() {
  isScanning = true;
  setLED(true, true, false); // Yellow while scanning
  tft.fillRect(0, 0, 320, 26, tft.color565(30, 30, 40));
  tft.setTextColor(TFT_YELLOW, tft.color565(30, 30, 40));
  tft.drawCentreString("SCANNING 2.4GHz CHANNELS...", 160, 6, 2);

  memset(channelCount, 0, sizeof(channelCount));
  for (int i = 0; i < 15; i++) channelMaxRssi[i] = -100;

  int n = WiFi.scanNetworks(false, true); // Include hidden networks
  totalAPs = n;
  apCount = (n > MAX_APS) ? MAX_APS : n;

  for (int i = 0; i < n; i++) {
    int ch = WiFi.channel(i);
    int rssi = WiFi.RSSI(i);
    if (ch >= 1 && ch <= 14) {
      channelCount[ch]++;
      if (rssi > channelMaxRssi[ch]) {
        channelMaxRssi[ch] = rssi;
      }
    }

    if (i < MAX_APS) {
      strncpy(apList[i].ssid, WiFi.SSID(i).c_str(), sizeof(apList[i].ssid) - 1);
      if (strlen(apList[i].ssid) == 0) strncpy(apList[i].ssid, "[HIDDEN]", sizeof(apList[i].ssid) - 1);
      apList[i].rssi = rssi;
      apList[i].channel = ch;
      apList[i].encType = getEncTypeStr(WiFi.encryptionType(i));
    }
  }

  // Calculate best channel among standard non-overlapping 1, 6, 11
  int minScore = 999;
  int primaryChannels[3] = { 1, 6, 11 };
  bestChannel = 1;

  for (int idx = 0; idx < 3; idx++) {
    int c = primaryChannels[idx];
    // Score based on AP count and max RSSI
    int score = channelCount[c] * 10;
    if (c > 1) score += channelCount[c - 1] * 3;
    if (c < 13) score += channelCount[c + 1] * 3;
    if (channelMaxRssi[c] > -60) score += 15;

    if (score < minScore) {
      minScore = score;
      bestChannel = c;
    }
  }

  WiFi.scanDelete();
  isScanning = false;

  // Sound scan complete sweep
  beep(1800, 40);
  delay(50);
  beep(2400, 60);

  // Status LED based on congestion
  if (totalAPs < 8) {
    setLED(false, true, false); // Green: Low congestion
  } else if (totalAPs < 18) {
    setLED(true, true, false);  // Yellow: Moderate congestion
  } else {
    setLED(true, false, false); // Red: High saturation
  }
}

void drawHeader() {
  tft.fillRect(0, 0, 320, 26, tft.color565(15, 25, 40));
  tft.setTextColor(TFT_WHITE, tft.color565(15, 25, 40));
  tft.drawString("WIFI SPECTRUM AUDITOR", 8, 6, 2);

  char statBuf[32];
  snprintf(statBuf, sizeof(statBuf), "APS:%d  BEST:CH %d", totalAPs, bestChannel);
  tft.setTextColor(TFT_GREEN, tft.color565(15, 25, 40));
  tft.drawRightString(statBuf, 312, 6, 2);
}

void drawSpectrumView() {
  tft.fillRect(0, 26, 320, 178, TFT_BLACK);

  // Draw grid & dBm axis
  for (int db = -90; db <= -40; db += 15) {
    int gy = map(db, -100, -30, 170, 40);
    tft.drawFastHLine(24, gy, 288, tft.color565(30, 35, 45));
    tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
    char dbBuf[8];
    snprintf(dbBuf, sizeof(dbBuf), "%d", db);
    tft.drawString(dbBuf, 2, gy - 4, 1);
  }

  // Draw Channel baseline labels
  for (int ch = 1; ch <= 13; ch++) {
    int cx = 32 + (ch - 1) * 22;
    tft.setTextColor((ch == 1 || ch == 6 || ch == 11) ? TFT_CYAN : TFT_LIGHTGREY, TFT_BLACK);
    char chBuf[4];
    snprintf(chBuf, sizeof(chBuf), "%d", ch);
    tft.drawCentreString(chBuf, cx, 176, 1);

    if (channelCount[ch] > 0) {
      int peakRssi = constrain(channelMaxRssi[ch], -95, -35);
      int peakY = map(peakRssi, -100, -30, 170, 40);
      int lobeW = 22; // Overlap across adjacent channels

      uint16_t lobeColor = (ch == 1 || ch == 6 || ch == 11) ? TFT_CYAN : TFT_YELLOW;
      if (channelCount[ch] >= 4) lobeColor = TFT_RED;

      // Draw Parabolic curve
      for (int dx = -lobeW; dx <= lobeW; dx++) {
        int px = cx + dx;
        if (px >= 24 && px <= 310) {
          float normX = (float)dx / lobeW;
          float py = 172 - (172 - peakY) * (1.0 - normX * normX);
          tft.drawPixel(px, (int)py, lobeColor);
        }
      }

      // Draw peak AP count pill
      tft.setTextColor(TFT_WHITE, lobeColor);
      char countStr[4];
      snprintf(countStr, sizeof(countStr), "%d", channelCount[ch]);
      tft.drawCentreString(countStr, cx, peakY - 10, 1);
    }
  }

  // Draw Recommended Channel Banner
  tft.fillRoundRect(24, 188, 272, 14, 2, tft.color565(20, 45, 25));
  tft.setTextColor(TFT_GREEN, tft.color565(20, 45, 25));
  char recBuf[48];
  snprintf(recBuf, sizeof(recBuf), "RECOMMENDED: CHANNEL %d (LEAST INTERFERENCE)", bestChannel);
  tft.drawCentreString(recBuf, 160, 190, 1);
}

void drawAPListView() {
  tft.fillRect(0, 26, 320, 178, TFT_BLACK);

  // Table Header
  tft.fillRoundRect(4, 30, 312, 18, 2, tft.color565(25, 30, 40));
  tft.setTextColor(TFT_LIGHTGREY, tft.color565(25, 30, 40));
  tft.drawString("SSID", 8, 34, 1);
  tft.drawString("CH", 185, 34, 1);
  tft.drawString("RSSI", 220, 34, 1);
  tft.drawString("SEC", 270, 34, 1);

  // Rows (up to 7 visible)
  int visibleRows = (apCount > 7) ? 7 : apCount;
  for (int i = 0; i < visibleRows; i++) {
    int y = 52 + i * 20;
    uint16_t rowBg = (i % 2 == 0) ? tft.color565(15, 18, 24) : TFT_BLACK;

    tft.fillRect(4, y, 312, 18, rowBg);

    // SSID (truncated to fit)
    char truncSSID[18];
    strncpy(truncSSID, apList[i].ssid, 17);
    truncSSID[17] = '\0';
    tft.setTextColor(TFT_WHITE, rowBg);
    tft.drawString(truncSSID, 8, y + 4, 1);

    // Channel
    tft.setTextColor(TFT_CYAN, rowBg);
    tft.drawNumber(apList[i].channel, 188, y + 4, 1);

    // RSSI with quality color
    int rssi = apList[i].rssi;
    uint16_t rssiColor = (rssi > -65) ? TFT_GREEN : ((rssi > -80) ? TFT_YELLOW : TFT_RED);
    tft.setTextColor(rssiColor, rowBg);
    char rssiStr[8];
    snprintf(rssiStr, sizeof(rssiStr), "%ddBm", rssi);
    tft.drawString(rssiStr, 220, y + 4, 1);

    // Encryption
    tft.setTextColor(TFT_LIGHTGREY, rowBg);
    tft.drawString(apList[i].encType, 270, y + 4, 1);
  }
}

void drawUI() {
  drawHeader();
  if (currentView == 0) {
    drawSpectrumView();
  } else {
    drawAPListView();
  }

  // Bottom Navigation Buttons (Y: 206 to 240)
  tft.fillRoundRect(8, 206, 96, 30, 4, tft.color565(35, 40, 50));
  tft.setTextColor(TFT_WHITE, tft.color565(35, 40, 50));
  tft.drawCentreString("RESCAN", 56, 214, 2);

  uint16_t viewBtnBg = (currentView == 0) ? TFT_DARKCYAN : tft.color565(35, 40, 50);
  tft.fillRoundRect(112, 206, 96, 30, 4, viewBtnBg);
  tft.setTextColor(TFT_WHITE, viewBtnBg);
  tft.drawCentreString(currentView == 0 ? "SPECTRUM" : "AP TABLE", 160, 214, 2);

  tft.fillRoundRect(216, 206, 96, 30, 4, tft.color565(20, 50, 30));
  tft.setTextColor(TFT_GREEN, tft.color565(20, 50, 30));
  char bChBuf[16];
  snprintf(bChBuf, sizeof(bChBuf), "USE CH %d", bestChannel);
  tft.drawCentreString(bChBuf, 264, 214, 2);
}

void checkTouch() {
  if (isScanning) return;

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
          if (screenX < 105) {
            // RESCAN
            beep(2000, 40);
            performScan();
            drawUI();
          } else if (screenX >= 110 && screenX < 210) {
            // TOGGLE VIEW
            currentView = (currentView == 0) ? 1 : 0;
            beep(2200, 40);
            drawUI();
          } else if (screenX >= 215) {
            // BEST CHANNEL CONFIRMATION
            beep(2600, 70);
          }
        }
      }
    }
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("Wi-Fi Channel & Spectrum Auditor Starting for @IamNotIntoTech...");

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

  // Initialize VSPI Touch Controller
  touchSPI.begin(TOUCH_CLK, TOUCH_MISO, TOUCH_MOSI, TOUCH_CS);
  ts.begin(touchSPI);
  ts.setRotation(1);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  // Initial Scan
  performScan();
  drawUI();
}

void loop() {
  checkTouch();
  delay(15);
}

