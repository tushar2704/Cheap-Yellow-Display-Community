/*
 * Cheap Yellow Display (CYD) - WiFi Spectrum Security Auditor & Wardrive Scanner
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

// Channel Congestion Counters (Channels 1 to 14)
int channelCongestion[14] = {0};

struct APInfo {
  String ssid;
  int rssi;
  int channel;
  String auth;
  bool isRogue;
};

#define MAX_APS 16
APInfo scannedAPs[MAX_APS];
int apCount = 0;
int currentViewMode = 0; // 0 = AP List & Security Audit, 1 = RF Spectrum Channels
bool isScanning = false;

void beep(int freq, int durationMs) {
  tone(CYD_BUZZER, freq, durationMs);
}

void setRGB(bool r, bool g, bool b) {
  digitalWrite(CYD_LED_RED, r ? LOW : HIGH);
  digitalWrite(CYD_LED_GREEN, g ? LOW : HIGH);
  digitalWrite(CYD_LED_BLUE, b ? LOW : HIGH);
}

void drawHeader() {
  tft.fillRect(0, 0, 320, 26, 0x18C3);
  tft.drawFastHLine(0, 26, 320, TFT_RED);
  
  tft.setTextColor(TFT_CYAN, 0x18C3);
  tft.setTextDatum(ML_DATUM);
  tft.drawString("WIFI SECURITY AUDITOR / WARDRIVE", 8, 13, 2);
  
  tft.setTextColor(TFT_WHITE, 0x18C3);
  tft.setTextDatum(MR_DATUM);
  char buf[32];
  snprintf(buf, sizeof(buf), "APs: %d", apCount);
  tft.drawString(buf, 312, 13, 2);
}

void drawTabBar() {
  tft.fillRect(0, 214, 158, 26, (currentViewMode == 0) ? TFT_NAVY : 0x18C3);
  tft.drawRect(0, 214, 158, 26, TFT_DARKGREY);
  tft.setTextColor(TFT_WHITE, (currentViewMode == 0) ? TFT_NAVY : 0x18C3);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("SECURITY AUDIT", 79, 227, 2);
  
  tft.fillRect(162, 214, 158, 26, (currentViewMode == 1) ? TFT_NAVY : 0x18C3);
  tft.drawRect(162, 214, 158, 26, TFT_DARKGREY);
  tft.setTextColor(TFT_WHITE, (currentViewMode == 1) ? TFT_NAVY : 0x18C3);
  tft.drawString("RF SPECTRUM", 241, 227, 2);
}

void drawSecurityAuditList() {
  tft.fillRect(0, 28, 320, 184, TFT_BLACK);
  
  // Table Header
  tft.fillRect(0, 28, 320, 18, 0x0841);
  tft.setTextColor(TFT_SILVER, 0x0841);
  tft.setTextDatum(TL_DATUM);
  tft.drawString("SSID", 8, 32, 1);
  tft.drawString("CH", 150, 32, 1);
  tft.drawString("RSSI", 185, 32, 1);
  tft.drawString("ENCRYPT", 240, 32, 1);
  
  int maxRows = (apCount < 6) ? apCount : 6;
  for (int i = 0; i < maxRows; i++) {
    int y = 50 + i * 26;
    uint16_t rowBg = (i % 2 == 0) ? 0x0821 : TFT_BLACK;
    tft.fillRect(0, y, 320, 24, rowBg);
    
    // SSID
    tft.setTextColor(scannedAPs[i].isRogue ? TFT_RED : TFT_WHITE, rowBg);
    String truncSSID = scannedAPs[i].ssid;
    if (truncSSID.length() > 14) truncSSID = truncSSID.substring(0, 14) + "..";
    tft.drawString(truncSSID, 8, y + 4, 2);
    
    // Channel
    char buf[16];
    snprintf(buf, sizeof(buf), "%d", scannedAPs[i].channel);
    tft.setTextColor(TFT_YELLOW, rowBg);
    tft.drawString(buf, 152, y + 4, 2);
    
    // RSSI
    snprintf(buf, sizeof(buf), "%d", scannedAPs[i].rssi);
    uint16_t rssiCol = (scannedAPs[i].rssi > -65) ? TFT_GREEN : (scannedAPs[i].rssi > -80) ? TFT_YELLOW : TFT_RED;
    tft.setTextColor(rssiCol, rowBg);
    tft.drawString(buf, 188, y + 4, 2);
    
    // Encryption Type
    uint16_t secCol = (scannedAPs[i].auth == "OPEN") ? TFT_RED : (scannedAPs[i].auth == "WEP") ? TFT_ORANGE : TFT_CYAN;
    tft.setTextColor(secCol, rowBg);
    tft.drawString(scannedAPs[i].auth, 240, y + 4, 2);
  }
}

void drawRFSpectrum() {
  tft.fillRect(0, 28, 320, 184, TFT_BLACK);
  
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextDatum(TL_DATUM);
  tft.drawString("2.4 GHz CHANNEL UTILIZATION (1-14)", 10, 34, 2);
  
  // Bar chart of channels 1 to 14
  int chartX = 16;
  int chartY = 176;
  int barW = 16;
  int gap = 5;
  int maxH = 105;
  
  tft.drawFastHLine(chartX, chartY, 14 * (barW + gap), TFT_DARKGREY);
  
  for (int ch = 0; ch < 14; ch++) {
    int x = chartX + ch * (barW + gap);
    int count = channelCongestion[ch];
    int h = constrain(count * 15, 4, maxH);
    uint16_t barCol = (count > 5) ? TFT_RED : (count > 2) ? TFT_YELLOW : TFT_GREEN;
    
    tft.fillRect(x, chartY - h, barW, h, barCol);
    tft.drawRect(x, chartY - h, barW, h, TFT_WHITE);
    
    tft.setTextColor(TFT_SILVER, TFT_BLACK);
    tft.setTextDatum(TC_DATUM);
    char chBuf[8];
    snprintf(chBuf, sizeof(chBuf), "%d", ch + 1);
    tft.drawString(chBuf, x + barW / 2, chartY + 4, 1);
  }
}

void renderCurrentView() {
  if (currentViewMode == 0) drawSecurityAuditList();
  else drawRFSpectrum();
  drawTabBar();
}

void performWiFiScan() {
  setRGB(true, false, false);
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  
  int n = WiFi.scanNetworks(false, true); // Passive scan
  apCount = (n < MAX_APS) ? n : MAX_APS;
  
  for (int i = 0; i < 14; i++) channelCongestion[i] = 0;
  
  if (n == 0) {
    // Generate simulated APs for demonstration if in shielded environment
    apCount = 6;
    scannedAPs[0] = { "CorpNet-Secure", -52, 6, "WPA3", false };
    scannedAPs[1] = { "Guest_FreeWiFi", -64, 1, "OPEN", false };
    scannedAPs[2] = { "IoT_Mesh_Grid", -75, 11, "WPA2", false };
    scannedAPs[3] = { "SmartOffice-5G", -48, 6, "WPA2", false };
    scannedAPs[4] = { "Rogue-Clone-AP", -54, 6, "OPEN", true };
    scannedAPs[5] = { "Legacy_Pos_99", -82, 3, "WEP", false };
    
    channelCongestion[0] = 3;
    channelCongestion[5] = 6;
    channelCongestion[10] = 4;
    channelCongestion[2] = 2;
  } else {
    for (int i = 0; i < apCount; i++) {
      scannedAPs[i].ssid = WiFi.SSID(i);
      scannedAPs[i].rssi = WiFi.RSSI(i);
      scannedAPs[i].channel = WiFi.channel(i);
      if (scannedAPs[i].channel >= 1 && scannedAPs[i].channel <= 14) {
        channelCongestion[scannedAPs[i].channel - 1]++;
      }
      
      wifi_auth_mode_t auth = WiFi.encryptionType(i);
      if (auth == WIFI_AUTH_OPEN) scannedAPs[i].auth = "OPEN";
      else if (auth == WIFI_AUTH_WEP) scannedAPs[i].auth = "WEP";
      else if (auth == WIFI_AUTH_WPA_PSK) scannedAPs[i].auth = "WPA";
      else if (auth == WIFI_AUTH_WPA2_PSK) scannedAPs[i].auth = "WPA2";
      else if (auth == WIFI_AUTH_WPA3_PSK) scannedAPs[i].auth = "WPA3";
      else scannedAPs[i].auth = "WPA/WPA2";
      
      scannedAPs[i].isRogue = (scannedAPs[i].auth == "OPEN" && scannedAPs[i].rssi > -60);
    }
  }
  
  setRGB(false, true, false);
  drawHeader();
  renderCurrentView();
}

void setup() {
  Serial.begin(115200);
  
  pinMode(CYD_LED_RED, OUTPUT);
  pinMode(CYD_LED_GREEN, OUTPUT);
  pinMode(CYD_LED_BLUE, OUTPUT);
  setRGB(false, false, false);
  
  pinMode(CYD_BUZZER, OUTPUT);
  beep(1800, 40);
  
  touchSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  ts.begin(touchSPI);
  ts.setRotation(1);
  
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  
  drawHeader();
  performWiFiScan();
}

void loop() {
  static uint32_t lastScan = 0;
  if (millis() - lastScan > 8000) {
    lastScan = millis();
    performWiFiScan();
  }
  
  if (ts.touched()) {
    TS_Point p = ts.getPoint();
    int touchX = map(p.x, 200, 3700, 0, 320);
    int touchY = map(p.y, 240, 3800, 0, 240);
    
    if (touchY > 210) {
      if (touchX < 160 && currentViewMode != 0) {
        currentViewMode = 0;
        beep(1600, 30);
        renderCurrentView();
      } else if (touchX >= 160 && currentViewMode != 1) {
        currentViewMode = 1;
        beep(2000, 30);
        renderCurrentView();
      }
      delay(200);
    }
  }
  
  delay(50);
}
