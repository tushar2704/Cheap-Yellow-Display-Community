/*
 * Cheap Yellow Display (CYD) - Bitcoin & Lightning Node Telemetry Monitor
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

// Node Telemetry State
uint32_t blockHeight = 884920;
float btcPriceUSD = 94650.00;
int fastFeeSat = 18;
int medFeeSat = 12;
int minFeeSat = 6;
int activeChannels = 42;
uint32_t localCapacitySats = 12500000;
uint32_t remoteCapacitySats = 9800000;
uint32_t routedSats24h = 438500;
int syncPercent = 100;

enum DisplayTab { TAB_OVERVIEW, TAB_MEMPOOL, TAB_LIGHTNING };
DisplayTab currentTab = TAB_OVERVIEW;

void beep(int freq, int durationMs) {
  tone(CYD_BUZZER, freq, durationMs);
}

void setRGB(bool r, bool g, bool b) {
  digitalWrite(CYD_LED_RED, r ? LOW : HIGH);
  digitalWrite(CYD_LED_GREEN, g ? LOW : HIGH);
  digitalWrite(CYD_LED_BLUE, b ? LOW : HIGH);
}

void drawHeader() {
  tft.fillRect(0, 0, 320, 26, 0x0841);
  tft.drawFastHLine(0, 26, 320, 0xFBE0); // Orange separator
  
  tft.setTextColor(0xFD20, 0x0841); // Bitcoin Orange
  tft.setTextDatum(ML_DATUM);
  tft.drawString("BITCOIN / LND NODE", 8, 13, 2);
  
  tft.setTextColor(TFT_WHITE, 0x0841);
  tft.setTextDatum(MR_DATUM);
  char buf[32];
  snprintf(buf, sizeof(buf), "#%u", blockHeight);
  tft.drawString(buf, 312, 13, 2);
}

void drawTabBar() {
  int tabW = 106;
  for (int i = 0; i < 3; i++) {
    int x = i * tabW;
    uint16_t bg = (currentTab == i) ? 0xFBE0 : 0x18C3;
    uint16_t fg = (currentTab == i) ? TFT_BLACK : TFT_SILVER;
    tft.fillRect(x, 214, tabW - 2, 26, bg);
    tft.drawRect(x, 214, tabW - 2, 26, TFT_DARKGREY);
    tft.setTextColor(fg, bg);
    tft.setTextDatum(MC_DATUM);
    if (i == 0) tft.drawString("OVERVIEW", x + tabW / 2, 227, 2);
    else if (i == 1) tft.drawString("MEMPOOL", x + tabW / 2, 227, 2);
    else tft.drawString("CHANNELS", x + tabW / 2, 227, 2);
  }
}

void drawOverviewTab() {
  tft.fillRect(0, 28, 320, 184, TFT_BLACK);
  
  // BTC Price Card
  tft.fillRect(6, 32, 150, 78, 0x10A2);
  tft.drawRect(6, 32, 150, 78, 0x2965);
  tft.setTextColor(TFT_SILVER, 0x10A2);
  tft.setTextDatum(TL_DATUM);
  tft.drawString("BTC / USD", 12, 38, 1);
  tft.setTextColor(TFT_GREEN, 0x10A2);
  char buf[32];
  snprintf(buf, sizeof(buf), "$%.0f", btcPriceUSD);
  tft.drawString(buf, 12, 54, 4);
  tft.setTextColor(TFT_DARKGREEN, 0x10A2);
  tft.drawString("+3.42% 24h", 12, 90, 1);
  
  // Mempool Fee Summary Card
  tft.fillRect(164, 32, 150, 78, 0x10A2);
  tft.drawRect(164, 32, 150, 78, 0x2965);
  tft.setTextColor(TFT_SILVER, 0x10A2);
  tft.drawString("NEXT BLOCK FEE", 170, 38, 1);
  tft.setTextColor(0xFD20, 0x10A2);
  snprintf(buf, sizeof(buf), "%d sat/vB", fastFeeSat);
  tft.drawString(buf, 170, 54, 4);
  tft.setTextColor(TFT_ORANGE, 0x10A2);
  tft.drawString("Priority: High", 170, 90, 1);
  
  // Lightning Node Health
  tft.fillRect(6, 118, 308, 90, 0x10A2);
  tft.drawRect(6, 118, 308, 90, 0x2965);
  
  tft.setTextColor(TFT_SILVER, 0x10A2);
  tft.drawString("LND / CLN NODE TELEMETRY", 12, 124, 1);
  
  tft.setTextColor(TFT_WHITE, 0x10A2);
  snprintf(buf, sizeof(buf), "Active Channels: %d", activeChannels);
  tft.drawString(buf, 12, 142, 2);
  
  snprintf(buf, sizeof(buf), "24h Routed: %u sats", routedSats24h);
  tft.drawString(buf, 12, 162, 2);
  
  tft.setTextColor(TFT_GREEN, 0x10A2);
  snprintf(buf, sizeof(buf), "Sync: %d%% (Core OK)", syncPercent);
  tft.drawString(buf, 12, 184, 2);
  
  // Balance Mini Bar
  float totalCap = localCapacitySats + remoteCapacitySats;
  int barW = 100;
  int localW = (int)((localCapacitySats / totalCap) * barW);
  tft.fillRect(200, 150, localW, 14, TFT_GREEN);
  tft.fillRect(200 + localW, 150, barW - localW, 14, TFT_CYAN);
  tft.drawRect(200, 150, barW, 14, TFT_WHITE);
  tft.setTextColor(TFT_SILVER, 0x10A2);
  tft.drawString("Liquidity (L/R)", 200, 134, 1);
}

void drawMempoolTab() {
  tft.fillRect(0, 28, 320, 184, TFT_BLACK);
  
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextDatum(TL_DATUM);
  tft.drawString("MEMPOOL TRANSACTION FEES", 12, 34, 2);
  
  // 3 Fee Priority Boxes
  int boxW = 94;
  const char* labels[] = { "Low Priority", "Medium", "High (Next)" };
  int fees[] = { minFeeSat, medFeeSat, fastFeeSat };
  uint16_t cols[] = { TFT_GREEN, TFT_YELLOW, 0xFD20 };
  
  for (int i = 0; i < 3; i++) {
    int x = 12 + i * 102;
    tft.fillRect(x, 60, boxW, 80, 0x10A2);
    tft.drawRect(x, 60, boxW, 80, cols[i]);
    tft.setTextColor(TFT_SILVER, 0x10A2);
    tft.setTextDatum(MC_DATUM);
    tft.drawString(labels[i], x + boxW / 2, 75, 1);
    
    tft.setTextColor(cols[i], 0x10A2);
    char fbuf[16];
    snprintf(fbuf, sizeof(fbuf), "%d", fees[i]);
    tft.drawString(fbuf, x + boxW / 2, 100, 4);
    
    tft.setTextColor(TFT_WHITE, 0x10A2);
    tft.drawString("sat/vB", x + boxW / 2, 126, 1);
  }
  
  tft.fillRect(12, 152, 296, 50, 0x18C3);
  tft.drawRect(12, 152, 296, 50, TFT_DARKGREY);
  tft.setTextColor(TFT_GOLD, 0x18C3);
  tft.setTextDatum(TL_DATUM);
  tft.drawString("Mempool Backlog: ~14.8 MB (42 blocks)", 20, 160, 2);
  tft.drawString("Halving Epoch: Block 840,000+", 20, 180, 2);
}

void drawChannelsTab() {
  tft.fillRect(0, 28, 320, 184, TFT_BLACK);
  
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextDatum(TL_DATUM);
  tft.drawString("LIGHTNING CHANNEL CAPACITY", 12, 34, 2);
  
  tft.fillRect(12, 60, 296, 40, 0x10A2);
  tft.drawRect(12, 60, 296, 40, 0x2965);
  tft.setTextColor(TFT_GREEN, 0x10A2);
  char buf[40];
  snprintf(buf, sizeof(buf), "Outbound (Local): %u sats", localCapacitySats);
  tft.drawString(buf, 20, 72, 2);
  
  tft.fillRect(12, 108, 296, 40, 0x10A2);
  tft.drawRect(12, 108, 296, 40, 0x2965);
  tft.setTextColor(TFT_CYAN, 0x10A2);
  snprintf(buf, sizeof(buf), "Inbound (Remote): %u sats", remoteCapacitySats);
  tft.drawString(buf, 20, 120, 2);
  
  tft.fillRect(12, 156, 296, 46, 0x18C3);
  tft.drawRect(12, 156, 296, 46, 0xFD20);
  tft.setTextColor(0xFD20, 0x18C3);
  tft.drawString("Routing Status: ONLINE | Peer Count: 38", 20, 164, 2);
  tft.drawString("Fee Earned (All Time): 32,840 sats", 20, 182, 1);
}

void renderCurrentTab() {
  if (currentTab == TAB_OVERVIEW) drawOverviewTab();
  else if (currentTab == TAB_MEMPOOL) drawMempoolTab();
  else drawChannelsTab();
  drawTabBar();
}

void simulateNodeTelemetry() {
  static uint32_t lastTick = 0;
  if (millis() - lastTick > 3000) {
    lastTick = millis();
    btcPriceUSD += ((rand() % 100) - 48) * 1.5f;
    fastFeeSat = constrain(fastFeeSat + ((rand() % 5) - 2), 4, 150);
    medFeeSat = constrain(fastFeeSat * 2 / 3, 3, 100);
    minFeeSat = constrain(medFeeSat / 2, 1, 50);
    routedSats24h += (rand() % 500);
    
    // Status LED blink green
    setRGB(false, true, false);
    delay(20);
    setRGB(false, false, false);
    
    renderCurrentTab();
  }
}

void setup() {
  Serial.begin(115200);
  
  pinMode(CYD_LED_RED, OUTPUT);
  pinMode(CYD_LED_GREEN, OUTPUT);
  pinMode(CYD_LED_BLUE, OUTPUT);
  setRGB(false, false, false);
  
  pinMode(CYD_BUZZER, OUTPUT);
  beep(1400, 40);
  
  touchSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  ts.begin(touchSPI);
  ts.setRotation(1);
  
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  
  drawHeader();
  renderCurrentTab();
}

void loop() {
  simulateNodeTelemetry();
  
  if (ts.touched()) {
    TS_Point p = ts.getPoint();
    int touchX = map(p.x, 200, 3700, 0, 320);
    int touchY = map(p.y, 240, 3800, 0, 240);
    
    if (touchY > 210) {
      int clickedTab = touchX / 106;
      if (clickedTab >= 0 && clickedTab < 3 && clickedTab != (int)currentTab) {
        currentTab = (DisplayTab)clickedTab;
        beep(1800, 25);
        renderCurrentTab();
        delay(250);
      }
    }
  }
  
  delay(50);
}
