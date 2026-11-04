/**
 * @file main.cpp
 * @brief CYD Stealth WiFi Radar Hunter (Hidden Network & RF Privacy Scanner)
 * @author Tushar Aggarwal (@tushar2704)
 * @repository https://github.com/tushar2704/Cheap-Yellow-Display-Community
 * 
 * Hardware:
 * - Cheap Yellow Display (ESP32-2432S028R) 2.8" Resistive Touch
 * - Display: ILI9341 320x240 Landscape
 * - Touch: XPT2046 SPI (CLK=25, MISO=39, MOSI=32, CS=33, IRQ=36)
 * - RF: 2.4GHz WiFi Radio (Active & Stealth Beacon Analysis)
 * - Audio: GPIO 26 Speaker / Piezo buzzer (Sonar ping, Geiger proximity click)
 * - RGB LED: GPIO 4 (Red alert), GPIO 16 (Green safe), GPIO 17 (Blue scan) [Active LOW]
 */

#include <Arduino.h>
#include <SPI.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>

// ---------------- Hardware Pinout ----------------
#define XPT2046_MOSI  32
#define XPT2046_MISO  39
#define XPT2046_CLK   25
#define XPT2046_CS    33
#define XPT2046_IRQ   36

#define LED_RED       4
#define LED_GREEN     16
#define LED_BLUE      17
#define SPEAKER_PIN   26

// ---------------- Display & Touch ----------------
TFT_eSPI tft = TFT_eSPI();
SPIClass touchSPI(VSPI);
XPT2046_Touchscreen ts(XPT2046_CS, XPT2046_IRQ);

// ---------------- Color Palette ----------------
#define COL_BG         0x0000  // Pure Black CRT
#define COL_PANEL      0x0821  // Dark Tactical Slate
#define COL_BORDER     0x18E3  // Border Gray
#define COL_RADAR_GRID 0x02E0  // Dim Phosphor Green
#define COL_RADAR_LINE 0x07E0  // Bright Phosphor Sweep
#define COL_CYAN       0x07FF  // Public AP
#define COL_STEALTH    0xF800  // Hidden AP Red
#define COL_AMBER      0xFDA0  // Warning Amber
#define COL_WHITE      0xFFFF  // Text White
#define COL_GRAY       0x7BEF  // Dim Text

// ---------------- Network Storage ----------------
#define MAX_NETWORKS 30

struct ScannedNetwork {
  char ssid[33];
  char bssidStr[18];
  int32_t rssi;
  uint8_t channel;
  uint8_t encryptionType;
  bool isHidden;
  float angleRad;   // Computed radar angle
  int16_t radarX;   // Screen position on scope
  int16_t radarY;
  float distanceM;  // Estimated distance in meters
};

ScannedNetwork netList[MAX_NETWORKS];
int totalNetworks = 0;
int hiddenCount = 0;
int strongestHiddenIdx = -1;

// ---------------- UI & Radar State ----------------
enum ViewMode {
  VIEW_RADAR,
  VIEW_LOG,
  VIEW_SPECTRUM
};

ViewMode currentView = VIEW_RADAR;
bool soundEnabled = true;
bool geigerMode = false;
bool isScanning = false;
unsigned long lastScanStart = 0;
unsigned long lastTouchTime = 0;
unsigned long lastGeigerClick = 0;

// Radar Animation Geometry
const int radarCenterX = 114;
const int radarCenterY = 130;
const int radarRadius  = 88;
float sweepAngle = 0.0;
float prevSweepAngle = 0.0;

// ---------------- Audio & LED Helpers ----------------
void setRgbLed(bool r, bool g, bool b) {
  digitalWrite(LED_RED, r ? LOW : HIGH);
  digitalWrite(LED_GREEN, g ? LOW : HIGH);
  digitalWrite(LED_BLUE, b ? LOW : HIGH);
}

void playSonarPing() {
  if (!soundEnabled) return;
  tone(SPEAKER_PIN, 2400, 25);
  delay(30);
  tone(SPEAKER_PIN, 1800, 35);
}

void playGeigerClick() {
  if (!soundEnabled) return;
  tone(SPEAKER_PIN, 3200, 6);
}

void playTouchClick() {
  if (!soundEnabled) return;
  tone(SPEAKER_PIN, 1500, 12);
}

// Distance Estimation via Log-Distance Path Loss
float estimateDistance(int rssi) {
  // Typical indoor reference: -40dBm at 1 meter, path loss exp n=2.5
  if (rssi >= -40) return 0.8f;
  float ratio = (-40.0f - (float)rssi) / (10.0f * 2.5f);
  return pow(10.0f, ratio);
}

// ---------------- WiFi Scanning ----------------
void startWiFiScan() {
  isScanning = true;
  lastScanStart = millis();
  setRgbLed(false, false, true); // Blue during scan
  WiFi.scanNetworks(true, true); // Asynchronous, show_hidden = true
}

void processScanResults(int n) {
  totalNetworks = 0;
  hiddenCount = 0;
  strongestHiddenIdx = -1;
  int bestHiddenRssi = -120;

  int count = min(n, MAX_NETWORKS);
  for (int i = 0; i < count; i++) {
    String sName = WiFi.SSID(i);
    bool hidden = false;

    // Check if network is hidden (empty or null characters)
    if (sName.length() == 0 || sName.charAt(0) == '\0' || sName.equals(" ")) {
      hidden = true;
      sName = "[STEALTH AP]";
    }

    strncpy(netList[totalNetworks].ssid, sName.c_str(), 32);
    netList[totalNetworks].ssid[32] = '\0';

    uint8_t* b = WiFi.BSSID(i);
    sprintf(netList[totalNetworks].bssidStr, "%02X:%02X:%02X:%02X:%02X:%02X",
            b[0], b[1], b[2], b[3], b[4], b[5]);

    netList[totalNetworks].rssi = WiFi.RSSI(i);
    netList[totalNetworks].channel = WiFi.channel(i);
    netList[totalNetworks].encryptionType = WiFi.encryptionType(i);
    netList[totalNetworks].isHidden = hidden;
    netList[totalNetworks].distanceM = estimateDistance(netList[totalNetworks].rssi);

    // Compute radar angle based on BSSID hash + channel (deterministic, spread over 360 deg)
    uint32_t bHash = (b[3] << 16) | (b[4] << 8) | b[5];
    float angleDeg = (float)(bHash % 360);
    netList[totalNetworks].angleRad = radians(angleDeg);

    // Compute radar radius proportional to signal strength (-95dBm = radarRadius, -35dBm = 16px)
    int rDist = map(constrain((int)netList[totalNetworks].rssi, -95, -35), -95, -35, radarRadius - 6, 18);
    netList[totalNetworks].radarX = radarCenterX + (int)(rDist * cos(netList[totalNetworks].angleRad));
    netList[totalNetworks].radarY = radarCenterY + (int)(rDist * sin(netList[totalNetworks].angleRad));

    if (hidden) {
      hiddenCount++;
      if (netList[totalNetworks].rssi > bestHiddenRssi) {
        bestHiddenRssi = netList[totalNetworks].rssi;
        strongestHiddenIdx = totalNetworks;
      }
    }

    totalNetworks++;
  }

  isScanning = false;

  // LED indication: Red if stealth APs detected, Green if clean
  if (hiddenCount > 0) {
    setRgbLed(true, false, false);
    playSonarPing();
  } else {
    setRgbLed(false, true, false);
  }
}

// ---------------- GUI Drawing ----------------
void drawHeader() {
  tft.fillRect(0, 0, 320, 24, COL_PANEL);
  tft.drawFastHLine(0, 24, 320, (hiddenCount > 0) ? COL_STEALTH : COL_RADAR_GRID);

  // Tabs: RADAR | LOG | SPECTRUM
  tft.fillRoundRect(3, 3, 56, 18, 3, (currentView == VIEW_RADAR) ? COL_RADAR_LINE : 0x0000);
  tft.setTextColor((currentView == VIEW_RADAR) ? TFT_BLACK : COL_WHITE, (currentView == VIEW_RADAR) ? COL_RADAR_LINE : 0x0000);
  tft.setTextSize(1);
  tft.setCursor(10, 8);
  tft.print("RADAR");

  tft.fillRoundRect(62, 3, 44, 18, 3, (currentView == VIEW_LOG) ? COL_RADAR_LINE : 0x0000);
  tft.setTextColor((currentView == VIEW_LOG) ? TFT_BLACK : COL_WHITE, (currentView == VIEW_LOG) ? COL_RADAR_LINE : 0x0000);
  tft.setCursor(72, 8);
  tft.print("LOG");

  tft.fillRoundRect(109, 3, 58, 18, 3, (currentView == VIEW_SPECTRUM) ? COL_RADAR_LINE : 0x0000);
  tft.setTextColor((currentView == VIEW_SPECTRUM) ? TFT_BLACK : COL_WHITE, (currentView == VIEW_SPECTRUM) ? COL_RADAR_LINE : 0x0000);
  tft.setCursor(114, 8);
  tft.print("SPECTRUM");

  // Audio Toggle Button
  tft.fillRoundRect(172, 3, 46, 18, 3, soundEnabled ? 0x03E0 : 0x4208);
  tft.setTextColor(COL_WHITE, soundEnabled ? 0x03E0 : 0x4208);
  tft.setCursor(178, 8);
  tft.print(soundEnabled ? "BEEP" : "MUTE");

  // Status Summary
  tft.setTextColor(COL_WHITE, COL_PANEL);
  tft.setCursor(224, 8);
  if (hiddenCount > 0) {
    tft.setTextColor(COL_STEALTH, COL_PANEL);
    tft.printf("! %d STEALTH", hiddenCount);
  } else {
    tft.setTextColor(COL_RADAR_LINE, COL_PANEL);
    tft.printf("CLEAN (%d AP)", totalNetworks);
  }
}

void drawRadarStaticScope() {
  tft.fillRect(0, 25, 228, 215, COL_BG);

  // Concentric Rings
  tft.drawCircle(radarCenterX, radarCenterY, radarRadius, COL_RADAR_GRID);
  tft.drawCircle(radarCenterX, radarCenterY, radarRadius * 2 / 3, COL_RADAR_GRID);
  tft.drawCircle(radarCenterX, radarCenterY, radarRadius / 3, COL_RADAR_GRID);
  tft.drawCircle(radarCenterX, radarCenterY, 3, COL_RADAR_LINE);

  // Crosshairs
  tft.drawFastHLine(radarCenterX - radarRadius, radarCenterY, radarRadius * 2, COL_RADAR_GRID);
  tft.drawFastVLine(radarCenterX, radarCenterY - radarRadius, radarRadius * 2, COL_RADAR_GRID);

  // Ring Labels
  tft.setTextColor(COL_RADAR_GRID, COL_BG);
  tft.setTextSize(1);
  tft.setCursor(radarCenterX + 4, radarCenterY - radarRadius + 2);
  tft.print("-90");
  tft.setCursor(radarCenterX + 4, radarCenterY - (radarRadius * 2 / 3) + 2);
  tft.print("-70");
  tft.setCursor(radarCenterX + 4, radarCenterY - (radarRadius / 3) + 2);
  tft.print("-50");
}

void drawSidePanel() {
  tft.fillRect(228, 25, 92, 215, COL_PANEL);
  tft.drawFastVLine(228, 25, 215, COL_BORDER);

  tft.setTextColor(COL_WHITE, COL_PANEL);
  tft.setTextSize(1);
  tft.setCursor(234, 30);
  tft.print("SCAN INTEL");

  tft.setTextColor(COL_GRAY, COL_PANEL);
  tft.setCursor(234, 44);
  tft.printf("TOTAL: %d", totalNetworks);

  tft.setCursor(234, 56);
  tft.setTextColor((hiddenCount > 0) ? COL_STEALTH : COL_RADAR_LINE, COL_PANEL);
  tft.printf("HIDDEN: %d", hiddenCount);

  // Threat Meter
  tft.drawRect(234, 70, 80, 12, COL_BORDER);
  int threatFill = map(constrain(hiddenCount, 0, 5), 0, 5, 0, 78);
  tft.fillRect(235, 71, threatFill, 10, (hiddenCount > 0) ? COL_STEALTH : COL_RADAR_LINE);

  // Nearest Stealth AP Details
  tft.setTextColor(COL_WHITE, COL_PANEL);
  tft.setCursor(234, 88);
  tft.print("NEAREST STEALTH");

  if (strongestHiddenIdx >= 0 && strongestHiddenIdx < totalNetworks) {
    ScannedNetwork& sn = netList[strongestHiddenIdx];
    tft.setTextColor(COL_STEALTH, COL_PANEL);
    tft.setCursor(234, 100);
    tft.print("BSSID:");
    tft.setCursor(234, 110);
    tft.print(sn.bssidStr + 9); // Last 3 octets

    tft.setTextColor(COL_WHITE, COL_PANEL);
    tft.setCursor(234, 124);
    tft.printf("RSSI: %d dBm", sn.rssi);

    tft.setCursor(234, 136);
    tft.printf("CH: %d", sn.channel);

    tft.setTextColor(COL_AMBER, COL_PANEL);
    tft.setCursor(234, 148);
    tft.printf("DIST: ~%.1fm", sn.distanceM);
  } else {
    tft.setTextColor(COL_GRAY, COL_PANEL);
    tft.setCursor(234, 104);
    tft.print("No Hidden");
    tft.setCursor(234, 116);
    tft.print("Signal Near");
  }

  // Quick Action Buttons
  // Scan Button
  tft.fillRoundRect(232, 166, 84, 28, 3, isScanning ? 0x02E0 : 0x05DF);
  tft.setTextColor(TFT_BLACK, isScanning ? 0x02E0 : 0x05DF);
  tft.setCursor(244, 175);
  tft.print(isScanning ? "SCANNING" : "RE-SCAN");

  // Geiger Proximity Button
  tft.fillRoundRect(232, 200, 84, 28, 3, geigerMode ? 0x94A0 : 0x3186);
  tft.setTextColor(COL_WHITE, geigerMode ? 0x94A0 : 0x3186);
  tft.setCursor(238, 209);
  tft.print(geigerMode ? "GEIGER: ON" : "GEIGER: OFF");
}

void renderRadarSweep() {
  // Erase previous sweep line by redrawing static rings & radials in that slice
  int prevEndX = radarCenterX + (int)(radarRadius * cos(prevSweepAngle));
  int prevEndY = radarCenterY + (int)(radarRadius * sin(prevSweepAngle));
  tft.drawLine(radarCenterX, radarCenterY, prevEndX, prevEndY, COL_BG);

  // Redraw crossing grid markers
  tft.drawCircle(radarCenterX, radarCenterY, radarRadius, COL_RADAR_GRID);
  tft.drawCircle(radarCenterX, radarCenterY, radarRadius * 2 / 3, COL_RADAR_GRID);
  tft.drawCircle(radarCenterX, radarCenterY, radarRadius / 3, COL_RADAR_GRID);
  tft.drawFastHLine(radarCenterX - radarRadius, radarCenterY, radarRadius * 2, COL_RADAR_GRID);
  tft.drawFastVLine(radarCenterX, radarCenterY - radarRadius, radarRadius * 2, COL_RADAR_GRID);

  // Draw current sweep line
  int endX = radarCenterX + (int)(radarRadius * cos(sweepAngle));
  int endY = radarCenterY + (int)(radarRadius * sin(sweepAngle));
  tft.drawLine(radarCenterX, radarCenterY, endX, endY, COL_RADAR_LINE);

  prevSweepAngle = sweepAngle;
  sweepAngle += 0.055f; // ~3 degrees per frame
  if (sweepAngle >= TWO_PI) sweepAngle -= TWO_PI;

  // Draw Network Blips
  for (int i = 0; i < totalNetworks; i++) {
    ScannedNetwork& n = netList[i];
    if (n.isHidden) {
      // Hidden AP: Bright Red Diamond with pulsing outer ring
      tft.fillCircle(n.radarX, n.radarY, 3, COL_STEALTH);
      tft.drawCircle(n.radarX, n.radarY, 5, COL_AMBER);
    } else {
      // Regular AP: Cyan / Green Dot
      tft.fillCircle(n.radarX, n.radarY, 2, COL_CYAN);
    }

    // Check if sweep line currently intersects this blip
    float diff = fabs(sweepAngle - n.angleRad);
    if (diff < 0.06f || fabs(diff - TWO_PI) < 0.06f) {
      // Sonar sweep highlight
      tft.drawCircle(n.radarX, n.radarY, 8, COL_WHITE);
      if (n.isHidden) {
        playSonarPing();
      }
    }
  }
}

void drawLogView() {
  tft.fillRect(0, 25, 320, 215, COL_BG);

  tft.setTextColor(COL_RADAR_LINE, COL_BG);
  tft.setTextSize(1);
  tft.setCursor(6, 28);
  tft.print("TYPE    SSID / BSSID               CH   RSSI   DIST");
  tft.drawFastHLine(0, 38, 320, COL_BORDER);

  int y = 44;
  int shown = 0;
  for (int i = 0; i < totalNetworks && shown < 8; i++) {
    ScannedNetwork& n = netList[i];

    if (n.isHidden) {
      tft.setTextColor(COL_STEALTH, COL_BG);
      tft.setCursor(6, y);
      tft.print("[HIDE]  ");
      tft.print(n.bssidStr);
    } else {
      tft.setTextColor(COL_CYAN, COL_BG);
      tft.setCursor(6, y);
      tft.print("[OPEN]  ");
      char cutSsid[18];
      strncpy(cutSsid, n.ssid, 17);
      cutSsid[17] = '\0';
      tft.printf("%-17s", cutSsid);
    }

    tft.setTextColor(COL_WHITE, COL_BG);
    tft.setCursor(206, y);
    tft.printf("%2d", n.channel);

    tft.setCursor(236, y);
    tft.printf("%4ddB", (int)n.rssi);

    tft.setCursor(282, y);
    tft.printf("%4.1fm", n.distanceM);

    y += 18;
    shown++;
  }

  // Footer bar inside Log
  tft.fillRect(0, 216, 320, 24, COL_PANEL);
  tft.drawFastHLine(0, 215, 320, COL_BORDER);
  tft.setTextColor(COL_WHITE, COL_PANEL);
  tft.setCursor(6, 222);
  tft.printf("Showing %d of %d detected transmitters (Hidden: %d)", shown, totalNetworks, hiddenCount);
}

void drawSpectrumView() {
  tft.fillRect(0, 25, 320, 215, COL_BG);

  tft.setTextColor(COL_RADAR_LINE, COL_BG);
  tft.setTextSize(1);
  tft.setCursor(6, 28);
  tft.print("2.4GHz CHANNEL SPECTRUM CONGESTION (CH 1 - 13)");
  tft.drawFastHLine(0, 38, 320, COL_BORDER);

  // Count channels
  int chCount[14] = {0};
  int chHidden[14] = {0};
  for (int i = 0; i < totalNetworks; i++) {
    uint8_t c = netList[i].channel;
    if (c >= 1 && c <= 13) {
      chCount[c]++;
      if (netList[i].isHidden) chHidden[c]++;
    }
  }

  // Draw 13 channel bars
  for (int ch = 1; ch <= 13; ch++) {
    int x = 12 + (ch - 1) * 23;
    int maxBarH = 120;
    int totalH = constrain(chCount[ch] * 18, 0, maxBarH);
    int hiddenH = constrain(chHidden[ch] * 18, 0, totalH);

    // Baseline axis
    tft.drawFastHLine(8, 185, 304, COL_BORDER);

    // Regular channel bar
    if (totalH > 0) {
      tft.fillRect(x, 185 - totalH, 18, totalH, COL_CYAN);
    }
    // Hidden fraction stacked in Red
    if (hiddenH > 0) {
      tft.fillRect(x, 185 - totalH, 18, hiddenH, COL_STEALTH);
    }

    // Channel number label
    tft.setTextColor(COL_WHITE, COL_BG);
    tft.setCursor(x + 2, 192);
    tft.printf("%2d", ch);

    // Count label on top
    if (chCount[ch] > 0) {
      tft.setTextColor((chHidden[ch] > 0) ? COL_STEALTH : COL_CYAN, COL_BG);
      tft.setCursor(x + 2, 172 - totalH);
      tft.printf("%d", chCount[ch]);
    }
  }

  // Legend
  tft.fillRect(0, 216, 320, 24, COL_PANEL);
  tft.drawFastHLine(0, 215, 320, COL_BORDER);

  tft.fillRect(8, 222, 10, 10, COL_CYAN);
  tft.setTextColor(COL_WHITE, COL_PANEL);
  tft.setCursor(22, 224);
  tft.print("Public AP");

  tft.fillRect(100, 222, 10, 10, COL_STEALTH);
  tft.setTextColor(COL_STEALTH, COL_PANEL);
  tft.setCursor(114, 224);
  tft.print("Stealth/Hidden AP");
}

// ---------------- Setup ----------------
void setup() {
  Serial.begin(115200);
  delay(100);

  Serial.println("\n=======================================================");
  Serial.println("  CYD Stealth WiFi Radar Hunter - Project 39");
  Serial.println("  Hidden Network & RF Privacy Scanner");
  Serial.println("  Author: Tushar Aggarwal (@tushar2704)");
  Serial.println("  Channels: @IamNotIntoTech | @Makerrdesigns");
  Serial.println("=======================================================\n");

  // GPIO setup
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  setRgbLed(false, false, false);

  pinMode(SPEAKER_PIN, OUTPUT);

  // Display Init
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(COL_BG);

  // Touch Init (VSPI dedicated pins)
  touchSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  ts.begin(touchSPI);
  ts.setRotation(1);

  // WiFi Init in Station Mode (no connection, scan only)
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  // Initial UI Render
  drawHeader();
  if (currentView == VIEW_RADAR) {
    drawRadarStaticScope();
    drawSidePanel();
  } else if (currentView == VIEW_LOG) {
    drawLogView();
  } else {
    drawSpectrumView();
  }

  // Welcome Sonar Ping
  playSonarPing();

  // Kick off initial background scan
  startWiFiScan();
}

// ---------------- Main Loop ----------------
void loop() {
  unsigned long now = millis();

  // 1. Asynchronous WiFi Scan Status Check
  if (isScanning) {
    int scanStatus = WiFi.scanComplete();
    if (scanStatus >= 0) {
      processScanResults(scanStatus);
      WiFi.scanDelete();

      // Refresh current view with fresh scan data
      drawHeader();
      if (currentView == VIEW_RADAR) {
        drawRadarStaticScope();
        drawSidePanel();
      } else if (currentView == VIEW_LOG) {
        drawLogView();
      } else {
        drawSpectrumView();
      }
    }
  } else {
    // Auto re-scan every 7 seconds
    if (now - lastScanStart > 7000) {
      startWiFiScan();
    }
  }

  // 2. Geiger Counter Proximity Audio Simulation
  if (geigerMode && soundEnabled && hiddenCount > 0 && strongestHiddenIdx >= 0) {
    int rssi = netList[strongestHiddenIdx].rssi;
    // Map RSSI (-95 to -40 dBm) to click delay (800ms down to 60ms)
    unsigned long clickDelay = map(constrain(rssi, -95, -40), -95, -40, 800, 60);
    if (now - lastGeigerClick > clickDelay) {
      lastGeigerClick = now;
      playGeigerClick();
    }
  }

  // 3. Touch Processing
  if (ts.touched()) {
    TS_Point p = ts.getPoint();
    if (p.z > 50 || ts.touched()) {
      if (now - lastTouchTime > 220) {
        lastTouchTime = now;

        int screenX = map(p.x, 240, 3800, 0, 320);
        int screenY = map(p.y, 240, 3800, 0, 240);
        screenX = constrain(screenX, 0, 319);
        screenY = constrain(screenY, 0, 239);

        // Header Tab Taps (Y <= 24)
        if (screenY <= 24) {
          playTouchClick();
          if (screenX >= 3 && screenX <= 59) {
            currentView = VIEW_RADAR;
            drawHeader();
            drawRadarStaticScope();
            drawSidePanel();
          } else if (screenX >= 62 && screenX <= 106) {
            currentView = VIEW_LOG;
            drawHeader();
            drawLogView();
          } else if (screenX >= 109 && screenX <= 167) {
            currentView = VIEW_SPECTRUM;
            drawHeader();
            drawSpectrumView();
          } else if (screenX >= 172 && screenX <= 218) {
            soundEnabled = !soundEnabled;
            drawHeader();
          }
        }
        // Radar Side Panel Buttons (when in RADAR view)
        else if (currentView == VIEW_RADAR && screenX >= 230) {
          // Re-Scan Button (Y: 166 to 194)
          if (screenY >= 166 && screenY <= 194) {
            playTouchClick();
            startWiFiScan();
            drawSidePanel();
          }
          // Geiger Button (Y: 200 to 228)
          else if (screenY >= 200 && screenY <= 228) {
            playTouchClick();
            geigerMode = !geigerMode;
            drawSidePanel();
          }
        }
      }
    }
  }

  // 4. Render Active View Animation
  if (currentView == VIEW_RADAR) {
    renderRadarSweep();
  }

  delay(20);
}

