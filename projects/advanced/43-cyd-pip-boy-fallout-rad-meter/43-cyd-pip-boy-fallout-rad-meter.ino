/**
 * @file main.cpp
 * @brief CYD Pip-Boy 3000 - Wasteland Geiger Deck & 2.4GHz RF Rads Meter
 * @author Tushar Aggarwal (@tushar2704)
 * @repository https://github.com/tushar2704/Cheap-Yellow-Display-Community
 * 
 * Hardware:
 * - Cheap Yellow Display (ESP32-2432S028R) 2.8" Resistive Touch
 * - Display: ILI9341 320x240 Landscape
 * - Touch: XPT2046 SPI (CLK=25, MISO=39, MOSI=32, CS=33, IRQ=36)
 * - Audio: GPIO 26 Speaker / Piezo buzzer (Geiger clicks & chiptune radio)
 * - RGB LED: GPIO 4 (Red), GPIO 16 (Green), GPIO 17 (Blue) [Active LOW]
 * - Radio: ESP32 Promiscuous 2.4GHz Sniffer mapped to Simulated Rads/hr
 */

#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include <WiFi.h>
#include "esp_wifi.h"

// ---------------- Pinout ----------------
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

// ---------------- Vault-Tec Phosphor Palette ----------------
#define COL_BG          0x0041  // Deep CRT Phosphor Black
#define COL_GRID        0x01C2  // Scanline dark green
#define COL_GREEN_DIM   0x03E0  // Dim Phosphor Green
#define COL_GREEN_MID   0x05E0  // Mid Phosphor Green
#define COL_GREEN_BRT   0x07E0  // High Phosphor Green (0x07E0)
#define COL_AMBER       0xFDA0  // Rad Warning
#define COL_ALERT_RED   0xF800  // Critical Rads
#define COL_WHITE       0xFFFF  // Clean White

// ---------------- Application Tabs ----------------
enum PipTab {
  TAB_STAT = 0,
  TAB_RADAR = 1,
  TAB_RADIO = 2
};

PipTab currentTab = TAB_STAT;

// ---------------- Sniffer & Rad Telemetry ----------------
volatile uint32_t packetCount = 0;
volatile uint32_t channelPackets[14] = {0};
uint8_t currentChannel = 1;
unsigned long lastChannelHop = 0;
unsigned long lastMetricsCalc = 0;

float currentRads = 0.12f;      // uSv/h
float peakRads = 0.12f;
uint32_t packetsPerSec = 0;
int geigerClickDelay = 1000;
unsigned long lastGeigerClick = 0;

// Radio tune state
bool radioPlaying = false;
int radioNoteIndex = 0;
unsigned long lastRadioNote = 0;

// Simple chiptune theme notes (Fallout melody snippet)
const int TUNE_NOTES[] = { 392, 440, 494, 587, 523, 440, 392, 330, 294, 392, 440, 523 };
const int TUNE_DURS[]  = { 200, 200, 300, 400, 200, 200, 400, 300, 300, 300, 300, 500 };
#define TOTAL_TUNE_NOTES 12

// ---------------- Promiscuous Sniffer Callback ----------------
void IRAM_ATTR wifiPromiscuousCallback(void* buf, wifi_promiscuous_pkt_type_t type) {
  packetCount++;
  if (currentChannel >= 1 && currentChannel <= 13) {
    channelPackets[currentChannel]++;
  }
}

// ---------------- RGB Control ----------------
void setRGB(bool r, bool g, bool b) {
  digitalWrite(LED_RED, r ? LOW : HIGH);
  digitalWrite(LED_GREEN, g ? LOW : HIGH);
  digitalWrite(LED_BLUE, b ? LOW : HIGH);
}

// ---------------- Geiger Click Generator ----------------
void triggerGeigerClick() {
  int freq = random(2600, 3400);
  tone(SPEAKER_PIN, freq, 2);
}

// ---------------- Drawing Helpers ----------------
void drawPipBoyFrame() {
  tft.fillScreen(COL_BG);
  
  // Outer CRT Bevel
  tft.drawRoundRect(2, 2, 316, 236, 6, COL_GREEN_DIM);
  tft.drawRoundRect(4, 4, 312, 232, 5, COL_GREEN_MID);

  // Top Vault-Tec Header
  tft.fillRect(6, 6, 308, 22, 0x00A1);
  tft.drawFastHLine(6, 28, 308, COL_GREEN_BRT);
  
  // Navigation Tabs
  const char* tabs[] = { "1. STAT", "2. RADAR", "3. RADIO" };
  for (int i = 0; i < 3; i++) {
    int tabX = 14 + (i * 96);
    if (i == (int)currentTab) {
      tft.fillRoundRect(tabX - 4, 8, 86, 18, 3, COL_GREEN_BRT);
      tft.setTextFont(2);
      tft.setTextColor(COL_BG, COL_GREEN_BRT);
      tft.drawCentreString(tabs[i], tabX + 39, 10, 2);
    } else {
      tft.setTextFont(2);
      tft.setTextColor(COL_GREEN_DIM, 0x00A1);
      tft.drawCentreString(tabs[i], tabX + 39, 10, 2);
    }
  }

  // Bottom Status Bar
  tft.drawFastHLine(6, 212, 308, COL_GREEN_MID);
  tft.fillRect(6, 214, 308, 20, 0x00A1);
  
  tft.setTextFont(1);
  tft.setTextColor(COL_GREEN_BRT, 0x00A1);
  tft.setCursor(12, 218);
  tft.print("HP 185/210");
  
  tft.setCursor(120, 218);
  tft.print("LVL 42 WANDERER");
  
  tft.setCursor(240, 218);
  tft.print("AP 95/95");
}

// Draw Vault Boy Line Art Mascot
void drawVaultBoy(int cx, int cy, float rads) {
  // Clear Mascot Area
  tft.fillRect(cx - 40, cy - 40, 80, 85, COL_BG);

  uint16_t col = (rads > 100.0f) ? COL_ALERT_RED : ((rads > 35.0f) ? COL_AMBER : COL_GREEN_BRT);

  if (rads > 120.0f) {
    // Critical Radiation: Skull Graphic
    tft.drawCircle(cx, cy - 8, 22, col);
    tft.fillCircle(cx - 8, cy - 10, 5, COL_BG);
    tft.fillCircle(cx + 8, cy - 10, 5, COL_BG);
    tft.drawCircle(cx - 8, cy - 10, 5, col);
    tft.drawCircle(cx + 8, cy - 10, 5, col);
    // Nose triangle
    tft.drawTriangle(cx, cy - 4, cx - 3, cy, cx + 3, cy, col);
    // Teeth
    tft.drawRect(cx - 10, cy + 6, 20, 10, col);
    for (int x = cx - 6; x <= cx + 6; x += 4) {
      tft.drawFastVLine(x, cy + 6, 10, col);
    }
    tft.setTextFont(2);
    tft.setTextColor(COL_ALERT_RED, COL_BG);
    tft.drawCentreString("CRITICAL RADS!", cx, cy + 28, 2);
  } else {
    // Healthy / Slightly Dizzy Vault Boy Face
    tft.drawCircle(cx, cy - 10, 24, col); // Head
    // Hair swoops
    tft.drawArc(cx - 10, cy - 26, 12, 10, 45, 180, col, true);
    tft.drawArc(cx + 6, cy - 28, 14, 12, 180, 315, col, true);
    
    // Eyes
    if (rads > 40.0f) {
      // Dizzy X eyes
      tft.drawLine(cx - 12, cy - 14, cx - 6, cy - 8, col);
      tft.drawLine(cx - 6, cy - 14, cx - 12, cy - 8, col);
      tft.drawLine(cx + 6, cy - 14, cx + 12, cy - 8, col);
      tft.drawLine(cx + 12, cy - 14, cx + 6, cy - 8, col);
      // Wobbly mouth
      tft.drawLine(cx - 10, cy + 2, cx - 4, cy - 1, col);
      tft.drawLine(cx - 4, cy - 1, cx + 4, cy + 3, col);
      tft.drawLine(cx + 4, cy + 3, cx + 10, cy, col);
    } else {
      // Happy Winking / Smiling
      tft.fillCircle(cx - 8, cy - 11, 3, col);
      tft.drawLine(cx + 6, cy - 11, cx + 12, cy - 11, col); // Wink eye
      // Smile
      tft.drawArc(cx, cy - 6, 14, 12, 110, 250, col, true);
    }

    // Torso & Vault 101 Suit
    tft.drawRect(cx - 14, cy + 14, 28, 22, col);
    tft.drawLine(cx - 14, cy + 20, cx - 26, cy + 32, col); // Left Arm
    
    // Thumbs-up Right Arm
    tft.drawLine(cx + 14, cy + 20, cx + 24, cy + 14, col);
    tft.drawLine(cx + 24, cy + 14, cx + 24, cy + 6, col);  // Thumb pointing up!
    tft.drawCircle(cx + 24, cy + 5, 2, col);
  }
}

// ---------------- Draw Tab 1: STAT ----------------
bool statBoxInitialized = false;
int lastVaultBoyMood = -1;

void renderTabStat(bool fullRedraw = false) {
  int currentMood = (currentRads > 120.0f) ? 2 : ((currentRads > 40.0f) ? 1 : 0);
  if (fullRedraw || currentMood != lastVaultBoyMood) {
    lastVaultBoyMood = currentMood;
    drawVaultBoy(85, 115, currentRads);
  }

  if (fullRedraw || !statBoxInitialized) {
    statBoxInitialized = true;
    tft.drawRoundRect(165, 38, 145, 168, 6, COL_GREEN_DIM);
    tft.fillRect(167, 40, 141, 164, 0x0061);
    
    tft.setTextFont(2);
    tft.setTextColor(COL_GREEN_BRT, 0x0061);
    tft.drawCentreString("GEIGER DOSIMETER", 237, 46, 2);
    
    tft.setTextFont(2);
    tft.setTextColor(COL_GREEN_MID, 0x0061);
    tft.drawCentreString("uSv/hr (RADS)", 237, 118, 2);
    
    tft.drawRect(175, 140, 125, 14, COL_GREEN_DIM);
  }

  // Clear only dynamic rads text area
  tft.fillRect(170, 70, 136, 44, 0x0061);
  tft.setTextFont(6);
  uint16_t radCol = (currentRads > 100.0f) ? COL_ALERT_RED : ((currentRads > 35.0f) ? COL_AMBER : COL_GREEN_BRT);
  tft.setTextColor(radCol, 0x0061);
  
  char radBuf[16];
  snprintf(radBuf, sizeof(radBuf), "%4.1f", currentRads);
  tft.drawCentreString(radBuf, 237, 72, 6);

  // Analog Gauge Arc Bar
  int filledW = map((int)constrain(currentRads, 0, 150), 0, 150, 0, 121);
  tft.fillRect(177, 142, filledW, 10, radCol);
  tft.fillRect(177 + filledW, 142, 121 - filledW, 10, 0x0020);

  // Clear & update 3 lines of telemetry
  tft.fillRect(172, 160, 134, 42, 0x0061);
  tft.setTextFont(1);
  tft.setTextColor(COL_WHITE, 0x0061);
  tft.setCursor(175, 162);
  tft.printf("PEAK: %5.1f uSv/h", peakRads);
  tft.setCursor(175, 176);
  tft.printf("RF PKTS: %4d /sec", packetsPerSec);
  tft.setCursor(175, 190);
  tft.printf("CH: %02d | RSSI BURST", currentChannel);
}

// ---------------- Draw Tab 2: RADAR / SPECTRUM ----------------
void renderTabRadar() {
  tft.fillRect(8, 34, 304, 172, COL_BG);

  tft.setTextFont(2);
  tft.setTextColor(COL_GREEN_BRT, COL_BG);
  tft.setCursor(16, 40);
  tft.print("2.4GHz RF SPECTRUM // 13 CHANNELS");

  // Draw 13 Histogram bars
  int startX = 20;
  int barW = 18;
  int maxH = 100;
  int baseLineY = 175;

  tft.drawFastHLine(startX, baseLineY, 280, COL_GREEN_MID);

  for (int ch = 1; ch <= 13; ch++) {
    int bx = startX + (ch - 1) * 21;
    uint32_t count = channelPackets[ch];
    int bh = map(constrain((int)count, 0, 80), 0, 80, 0, maxH);
    
    uint16_t bCol = (ch == currentChannel) ? COL_WHITE : COL_GREEN_BRT;
    if (count > 50) bCol = COL_ALERT_RED;
    else if (count > 25) bCol = COL_AMBER;

    // Clear bar area
    tft.fillRect(bx, baseLineY - maxH, barW, maxH, COL_BG);
    // Draw bar
    if (bh > 0) {
      tft.fillRect(bx, baseLineY - bh, barW, bh, bCol);
    }
    
    // Channel number label
    tft.setTextFont(1);
    tft.setTextColor(COL_GREEN_MID, COL_BG);
    tft.setCursor(bx + 4, baseLineY + 6);
    tft.printf("%d", ch);
  }

  tft.setTextFont(1);
  tft.setTextColor(COL_WHITE, COL_BG);
  tft.setCursor(16, 195);
  tft.printf("PROMISCUOUS RX: %u TOTAL PACKETS DETECTED", packetCount);
}

// ---------------- Draw Tab 3: RADIO ----------------
void renderTabRadio() {
  tft.fillRect(8, 34, 304, 172, COL_BG);

  tft.drawRoundRect(14, 40, 292, 160, 8, COL_GREEN_MID);
  tft.fillRect(16, 42, 288, 156, 0x0061);

  tft.setTextFont(4);
  tft.setTextColor(COL_GREEN_BRT, 0x0061);
  tft.drawCentreString("DIAMOND CITY RADIO", 160, 52, 4);

  tft.setTextFont(2);
  tft.setTextColor(COL_GREEN_DIM, 0x0061);
  tft.drawCentreString("WASTELAND 8-BIT BROADCAST", 160, 85, 2);

  // Play / Pause Button
  uint16_t btnCol = radioPlaying ? COL_ALERT_RED : COL_GREEN_BRT;
  tft.fillRoundRect(80, 115, 160, 42, 6, btnCol);
  tft.setTextFont(4);
  tft.setTextColor(COL_BG, btnCol);
  tft.drawCentreString(radioPlaying ? "STOP RADIO" : "TUNE IN", 160, 122, 4);

  tft.setTextFont(1);
  tft.setTextColor(COL_WHITE, 0x0061);
  tft.drawCentreString("TAP BUTTON TO TOGGLE PIEZO CHIPTUNE TUNES", 160, 170, 1);
}

// ---------------- Redraw Current Tab Content ----------------
void refreshTabContent(bool fullRedraw = false) {
  if (currentTab == TAB_STAT) {
    renderTabStat(fullRedraw);
  } else if (currentTab == TAB_RADAR) {
    renderTabRadar();
  } else if (currentTab == TAB_RADIO) {
    if (fullRedraw) renderTabRadio();
  }
}

// ---------------- Setup ----------------
void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println(F("[PIP-BOY 3000] Initializing Wasteland Geiger Deck..."));

  // Pins
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  pinMode(SPEAKER_PIN, OUTPUT);
  setRGB(false, false, false);
  noTone(SPEAKER_PIN);

  // Initialize Display
  tft.init();
  tft.setRotation(1);

  // Initialize Touch
  touchSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  ts.begin(touchSPI);
  ts.setRotation(1);

  // Initialize WiFi in Promiscuous Sniffer Mode
  WiFi.mode(WIFI_MODE_STA);
  WiFi.disconnect();
  delay(100);

  esp_wifi_set_promiscuous(true);
  esp_wifi_set_promiscuous_rx_cb(&wifiPromiscuousCallback);
  esp_wifi_set_channel(currentChannel, WIFI_SECOND_CHAN_NONE);

  // Draw Initial Screen
  drawPipBoyFrame();
  refreshTabContent(true);

  // Startup Geiger Burst
  for (int i = 0; i < 5; i++) {
    triggerGeigerClick();
    delay(40);
  }
}

// ---------------- Main Loop ----------------
void loop() {
  unsigned long now = millis();

  // 1. Channel Hopping across 1..13
  if (now - lastChannelHop >= 150) {
    lastChannelHop = now;
    currentChannel++;
    if (currentChannel > 13) currentChannel = 1;
    esp_wifi_set_channel(currentChannel, WIFI_SECOND_CHAN_NONE);
  }

  // 2. Metrics Calculation (Every 1000ms)
  if (now - lastMetricsCalc >= 1000) {
    lastMetricsCalc = now;
    packetsPerSec = packetCount;
    packetCount = 0; // Reset for next window

    // Calculate simulated radiation from real RF traffic
    // Base cosmic background: 0.10 uSv/h
    // Every 5 pkts/sec adds ~0.8 uSv/h up to 180 uSv/h
    float rawRads = 0.10f + (packetsPerSec * 0.45f);
    currentRads = (currentRads * 0.4f) + (rawRads * 0.6f); // Smoothed filter
    if (currentRads > peakRads) peakRads = currentRads;

    // Adjust geiger click interval (exponential ramp)
    if (packetsPerSec > 100) geigerClickDelay = 15;
    else if (packetsPerSec > 40) geigerClickDelay = 45;
    else if (packetsPerSec > 15) geigerClickDelay = 120;
    else if (packetsPerSec > 3) geigerClickDelay = 350;
    else geigerClickDelay = 900;

    // RGB Alert status
    if (currentRads > 100.0f) {
      setRGB(true, false, false); // Critical Red
    } else if (currentRads > 35.0f) {
      setRGB(true, true, false);  // Warning Amber
    } else {
      setRGB(false, true, false); // Safe Vault Green
    }

    // Refresh UI content
    refreshTabContent();
  }

  // 3. Audio: Geiger Clicks (when radio is OFF)
  if (!radioPlaying) {
    if (now - lastGeigerClick >= (unsigned long)geigerClickDelay) {
      lastGeigerClick = now + random(-10, 15);
      triggerGeigerClick();
    }
  } else {
    // 4. Audio: Radio Chiptune Player
    if (now - lastRadioNote >= (unsigned long)TUNE_DURS[radioNoteIndex]) {
      lastRadioNote = now;
      tone(SPEAKER_PIN, TUNE_NOTES[radioNoteIndex], TUNE_DURS[radioNoteIndex] - 30);
      radioNoteIndex = (radioNoteIndex + 1) % TOTAL_TUNE_NOTES;
    }
  }

  // 5. Touch Input Handling
  if (ts.touched()) {
    TS_Point p = ts.getPoint();
    int tx = map(p.x, 240, 3800, 0, 320);
    int ty = map(p.y, 240, 3800, 0, 240);
    tx = constrain(tx, 0, 319);
    ty = constrain(ty, 0, 239);

    // Tab Navigation Tap (Top Bar: Y < 36)
    if (ty < 36) {
      PipTab newTab = currentTab;
      if (tx < 105) newTab = TAB_STAT;
      else if (tx < 210) newTab = TAB_RADAR;
      else newTab = TAB_RADIO;

      if (newTab != currentTab) {
        currentTab = newTab;
        statBoxInitialized = false;
        lastVaultBoyMood = -1;
        tone(SPEAKER_PIN, 1800, 30);
        drawPipBoyFrame();
        refreshTabContent(true);
        delay(150);
      }
    }
    // Radio Play/Pause Tap (When on TAB_RADIO and tapping center button)
    else if (currentTab == TAB_RADIO && tx >= 80 && tx <= 240 && ty >= 110 && ty <= 165) {
      radioPlaying = !radioPlaying;
      if (!radioPlaying) noTone(SPEAKER_PIN);
      renderTabRadio();
      tone(SPEAKER_PIN, radioPlaying ? 1200 : 600, 60);
      delay(250);
    }
  }

  delay(10);
}

