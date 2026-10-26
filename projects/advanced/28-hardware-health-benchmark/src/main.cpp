/**
 * @file main.cpp
 * @brief 28 - Hardware Health & Benchmark Suite for Cheap Yellow Display (CYD 2.8")
 * @author Tushar Aggarwal (tushar2704)
 * @repository https://github.com/tushar2704/Cheap-Yellow-Display-Community
 * 
 * Community Channels:
 * - Tech & Coding: https://youtube.com/@IamNotIntoTech
 * - 3D Printing & Enclosure: https://youtube.com/@Makerrdesigns
 * 
 * Hardware:
 * - ESP32-2432S028R (2.8" Resistive Touch CYD)
 * - ILI9341 SPI Display (320x240)
 * - XPT2046 Resistive Touch Controller
 * - FM8002A Onboard Speaker Amplifier (GPIO 26)
 * - RGB LED (GPIO 4, 16, 17 - Active LOW)
 * - Light Dependent Resistor LDR (GPIO 34)
 */

#include <Arduino.h>
#include <SPI.h>
#include <WiFi.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include "esp_system.h"

// Hardware Pin Definitions
#define XPT2046_IRQ   36
#define XPT2046_MOSI  13
#define XPT2046_MISO  12
#define XPT2046_CLK   14
#define XPT2046_CS    33

#define LED_RED       4
#define LED_GREEN     16
#define LED_BLUE      17
#define SPEAKER_PIN   26
#define LDR_PIN       34

// Touch Screen Calibration Constants
#define TS_MINX 300
#define TS_MAXX 3800
#define TS_MINY 200
#define TS_MAXY 3700

// Initialize Display & Touch
TFT_eSPI tft = TFT_eSPI();
SPIClass touchSPI(VSPI);
XPT2046_Touchscreen ts(XPT2046_CS, XPT2046_IRQ);

// Tab Navigation
enum AppTab {
  TAB_BENCH = 0,
  TAB_DISPLAY,
  TAB_TOUCH,
  TAB_AUDIO,
  TAB_SENSORS,
  TAB_SYSTEM
};

AppTab currentTab = TAB_BENCH;
AppTab previousTab = TAB_SYSTEM; // Force initial draw

// Benchmark Results
struct BenchStats {
  uint32_t fillTimeMs;
  uint32_t linesTimeMs;
  uint32_t circlesTimeMs;
  uint32_t textTimeMs;
  uint32_t totalScore;
  float fps;
  float mpixelsPerSec;
  bool isCompleted;
} bench = {0, 0, 0, 0, 0, 0.0f, 0.0f, false};

// Display Test State
int fullScreenColorIdx = -1; // -1 = normal tab, >=0 = full screen color test
const uint16_t testColors[] = {
  TFT_RED, TFT_GREEN, TFT_BLUE, TFT_WHITE,
  TFT_BLACK, TFT_YELLOW, TFT_CYAN, TFT_MAGENTA
};
const char* testColorNames[] = {
  "Pure Red", "Pure Green", "Pure Blue", "Pure White",
  "Pure Black", "Pure Yellow", "Pure Cyan", "Pure Magenta"
};

// Touch Test State
int lastTouchX = -1;
int lastTouchY = -1;
int lastRawX = 0, lastRawY = 0, lastRawZ = 0;
bool targetHits[9] = {false};

// Sensor Graph Buffer
#define GRAPH_WIDTH 200
#define GRAPH_HEIGHT 60
int ldrHistory[GRAPH_WIDTH];
int ldrHistoryIdx = 0;
unsigned long lastSensorUpdate = 0;

// WiFi Scan Cache
struct WifiNetwork {
  String ssid;
  int32_t rssi;
  wifi_auth_mode_t authmode;
};
#define MAX_SCAN_RESULTS 6
WifiNetwork wifiNetworks[MAX_SCAN_RESULTS];
int wifiNetworkCount = 0;
bool wifiScanDone = false;

// Audio Sweep State
bool isAudioPlaying = false;
unsigned long audioSweepStart = 0;

// Helper: RGB LED Control (Active LOW)
void setRgbLed(bool r, bool g, bool b) {
  digitalWrite(LED_RED, r ? LOW : HIGH);
  digitalWrite(LED_GREEN, g ? LOW : HIGH);
  digitalWrite(LED_BLUE, b ? LOW : HIGH);
}

// Helper: Audio Beep
void playBeep(int freq, int durationMs) {
  tone(SPEAKER_PIN, freq, durationMs);
}

// Forward Declarations
void drawTabBar();
void renderCurrentTab();
void runBenchmark();
void renderBenchTab();
void renderDisplayTab();
void renderTouchTab();
void renderAudioTab();
void renderSensorsTab();
void renderSystemTab();
void scanNearbyWifi();

void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println("\n[CYD-28] Hardware Health & Benchmark Suite Initializing...");

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  pinMode(SPEAKER_PIN, OUTPUT);
  pinMode(LDR_PIN, INPUT);

  setRgbLed(false, false, false);

  // Initialize Display
  tft.init();
  tft.setRotation(1); // Landscape 320x240
  tft.fillScreen(TFT_BLACK);

  // Initialize Touchscreen
  touchSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  ts.begin(touchSPI);
  ts.setRotation(1);

  // Initialize LDR History
  for (int i = 0; i < GRAPH_WIDTH; i++) {
    ldrHistory[i] = 2048;
  }

  // Chime on startup
  setRgbLed(false, true, false);
  playBeep(1200, 70);
  delay(90);
  playBeep(1800, 90);
  delay(100);
  setRgbLed(false, false, false);

  drawTabBar();
  renderCurrentTab();
}

void drawTabBar() {
  tft.fillRect(0, 0, 320, 26, 0x18C3); // Header dark navy

  const char* tabNames[] = {"BENCH", "DISP", "TOUCH", "AUDIO", "SENS", "SYS"};
  int tabW = 320 / 6;

  for (int i = 0; i < 6; i++) {
    int x = i * tabW;
    bool isActive = (currentTab == (AppTab)i);
    uint16_t bg = isActive ? 0xFDC0 : 0x2104; // Yellow active, dark inactive
    uint16_t fg = isActive ? TFT_BLACK : TFT_LIGHTGREY;

    tft.fillRect(x + 1, 2, tabW - 2, 22, bg);
    tft.setTextColor(fg, bg);
    tft.setTextSize(1);
    tft.setTextDatum(MC_DATUM);
    tft.drawString(tabNames[i], x + (tabW / 2), 13);
  }
}

// -------------------------------------------------------------
// TAB 0: BENCHMARK
// -------------------------------------------------------------
void runBenchmark() {
  tft.fillRect(0, 26, 320, 214, TFT_BLACK);
  tft.setTextColor(0xFDC0, TFT_BLACK);
  tft.setTextSize(2);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("RUNNING SPI BENCHMARK...", 160, 110);
  tft.setTextSize(1);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString("Testing render throughput & fillrate", 160, 135);
  delay(400);

  // Test 1: Fullscreen Fills (5 frames)
  uint32_t t0 = millis();
  for (int i = 0; i < 5; i++) {
    tft.fillScreen(TFT_RED);
    tft.fillScreen(TFT_GREEN);
    tft.fillScreen(TFT_BLUE);
    tft.fillScreen(TFT_BLACK);
  }
  bench.fillTimeMs = millis() - t0;

  // Test 2: Lines (500 random lines)
  t0 = millis();
  for (int i = 0; i < 500; i++) {
    tft.drawLine(random(320), random(26, 240), random(320), random(26, 240), random(0xFFFF));
  }
  bench.linesTimeMs = millis() - t0;

  // Test 3: Circles (250 circles)
  t0 = millis();
  for (int i = 0; i < 250; i++) {
    tft.drawCircle(random(20, 300), random(40, 220), random(5, 30), random(0xFFFF));
  }
  bench.circlesTimeMs = millis() - t0;

  // Test 4: Text Blitting (100 string draws)
  t0 = millis();
  tft.setTextSize(1);
  for (int i = 0; i < 100; i++) {
    tft.setTextColor(random(0xFFFF), TFT_BLACK);
    tft.drawString("CYD 2.8 ESP32 TEST", random(20, 200), random(30, 220));
  }
  bench.textTimeMs = millis() - t0;

  // Compute Metrics
  uint32_t totalTimeMs = bench.fillTimeMs + bench.linesTimeMs + bench.circlesTimeMs + bench.textTimeMs;
  bench.fps = (20.0f * 1000.0f) / (float)bench.fillTimeMs;
  bench.mpixelsPerSec = (20.0f * 320.0f * 240.0f) / ((float)bench.fillTimeMs * 1000.0f);
  bench.totalScore = (uint32_t)((1000000.0f / (float)totalTimeMs) * 10.0f);
  bench.isCompleted = true;

  playBeep(2400, 100);
  drawTabBar();
  renderBenchTab();
}

void renderBenchTab() {
  tft.fillRect(0, 26, 320, 214, 0x0821);

  // Score Badge
  tft.fillRoundRect(12, 34, 140, 80, 6, 0x18C3);
  tft.drawRoundRect(12, 34, 140, 80, 6, 0xFDC0);
  tft.setTextColor(TFT_LIGHTGREY, 0x18C3);
  tft.setTextDatum(TL_DATUM);
  tft.setTextSize(1);
  tft.drawString("BENCHMARK SCORE", 20, 42);

  tft.setTextColor(0xFDC0, 0x18C3);
  tft.setTextSize(3);
  tft.drawString(bench.isCompleted ? String(bench.totalScore) : "----", 24, 58);

  tft.setTextSize(1);
  tft.setTextColor(0x07E0, 0x18C3);
  if (bench.isCompleted) {
    tft.drawString(String(bench.fps, 1) + " FPS | " + String(bench.mpixelsPerSec, 2) + " MP/s", 20, 94);
  } else {
    tft.drawString("Not benchmarked yet", 20, 94);
  }

  // Detailed Metrics
  tft.fillRoundRect(162, 34, 146, 80, 6, 0x18C3);
  tft.drawRoundRect(162, 34, 146, 80, 6, 0x39C7);
  tft.setTextColor(TFT_CYAN, 0x18C3);
  tft.setTextSize(1);
  tft.drawString("EXECUTION TIMES", 170, 42);

  tft.setTextColor(TFT_WHITE, 0x18C3);
  tft.drawString("Fill 20x:   " + String(bench.fillTimeMs) + " ms", 170, 56);
  tft.drawString("500 Lines:  " + String(bench.linesTimeMs) + " ms", 170, 70);
  tft.drawString("250 Circles:" + String(bench.circlesTimeMs) + " ms", 170, 84);
  tft.drawString("100 Strings:" + String(bench.textTimeMs) + " ms", 170, 98);

  // Hardware Ratings Table
  tft.fillRoundRect(12, 122, 296, 52, 6, 0x1082);
  tft.drawRoundRect(12, 122, 296, 52, 6, 0x2945);
  tft.setTextColor(0x07FF, 0x1082);
  tft.drawString("HARDWARE SPECS & BUS RATING", 20, 128);
  tft.setTextColor(TFT_WHITE, 0x1082);
  tft.drawString("Display: ILI9341 240x320 | SPI 40MHz VSPI", 20, 142);
  tft.drawString("Color Depth: 16-bit RGB565 (65k Colors)", 20, 156);

  // Run Benchmark Button
  tft.fillRoundRect(12, 182, 296, 36, 8, 0xFDC0);
  tft.setTextColor(TFT_BLACK, 0xFDC0);
  tft.setTextSize(2);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("TAP TO RUN BENCHMARK", 160, 200);
}

// -------------------------------------------------------------
// TAB 1: DISPLAY PURITY & GRADIENT
// -------------------------------------------------------------
void renderDisplayTab() {
  if (fullScreenColorIdx >= 0) {
    // Render full screen solid color test
    tft.fillScreen(testColors[fullScreenColorIdx]);
    tft.setTextColor(fullScreenColorIdx == 3 || fullScreenColorIdx == 5 ? TFT_BLACK : TFT_WHITE, testColors[fullScreenColorIdx]);
    tft.setTextSize(2);
    tft.setTextDatum(MC_DATUM);
    tft.drawString(testColorNames[fullScreenColorIdx], 160, 110);
    tft.setTextSize(1);
    tft.drawString("Tap anywhere to cycle color / exit", 160, 135);
    return;
  }

  tft.fillRect(0, 26, 320, 214, 0x0821);
  tft.setTextColor(TFT_WHITE, 0x0821);
  tft.setTextSize(1);
  tft.setTextDatum(TL_DATUM);
  tft.drawString("COLOR PURITY & PIXEL HEALTH (Tap box to test full-screen)", 12, 34);

  // 8 Color Swatches
  int boxW = 34;
  int boxH = 34;
  for (int i = 0; i < 8; i++) {
    int x = 12 + i * 38;
    int y = 50;
    tft.fillRect(x, y, boxW, boxH, testColors[i]);
    tft.drawRect(x, y, boxW, boxH, TFT_WHITE);
  }

  // 16-level Grayscale Ramp
  tft.drawString("16-STEP GAMMA & CONTRAST LINEARITY RAMP", 12, 94);
  int rampW = 18;
  for (int i = 0; i < 16; i++) {
    uint8_t c = i * 17; // 0 to 255
    uint16_t color = tft.color565(c, c, c);
    int x = 12 + i * 18;
    tft.fillRect(x, 110, rampW, 24, color);
    tft.drawRect(x, 110, rampW, 24, 0x39C7);
  }

  // Backlight & Contrast Test Area
  tft.fillRoundRect(12, 144, 296, 68, 6, 0x18C3);
  tft.drawRoundRect(12, 144, 296, 68, 6, 0x39C7);
  tft.setTextColor(0xFDC0, 0x18C3);
  tft.drawString("DIAGNOSTIC GUIDELINES", 20, 150);
  tft.setTextColor(TFT_LIGHTGREY, 0x18C3);
  tft.drawString("- Check for stuck (sub)pixels or dead pixels on black/white", 20, 166);
  tft.drawString("- Inspect edge backlight bleed on pure black test", 20, 180);
  tft.drawString("- Ensure all 16 gradient bars are clearly discernible", 20, 194);
}

// -------------------------------------------------------------
// TAB 2: TOUCH CALIBRATION & ACCURACY
// -------------------------------------------------------------
void renderTouchTab() {
  tft.fillRect(0, 26, 320, 214, 0x0821);

  // Clear / Reset Button
  tft.fillRoundRect(240, 32, 70, 24, 4, 0xF800);
  tft.setTextColor(TFT_WHITE, 0xF800);
  tft.setTextSize(1);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("CLEAR", 275, 44);

  // Live Touch Metrics Panel
  tft.setTextColor(TFT_CYAN, 0x0821);
  tft.setTextDatum(TL_DATUM);
  tft.drawString("TOUCHSCREEN LINEARITY & ACCURACY TEST", 12, 32);

  tft.setTextColor(TFT_WHITE, 0x0821);
  char buf[64];
  snprintf(buf, sizeof(buf), "Raw ADC: X=%04d Y=%04d Z=%03d | Screen: X=%03d Y=%03d",
           lastRawX, lastRawY, lastRawZ, lastTouchX >= 0 ? lastTouchX : 0, lastTouchY >= 0 ? lastTouchY : 0);
  tft.drawString(buf, 12, 46);

  // Draw 9 Calibration Alignment Targets
  const int targetPoints[9][2] = {
    {30, 80},  {160, 80},  {290, 80},
    {30, 140}, {160, 140}, {290, 140},
    {30, 200}, {160, 200}, {290, 200}
  };

  for (int i = 0; i < 9; i++) {
    int tx = targetPoints[i][0];
    int ty = targetPoints[i][1];
    uint16_t c = targetHits[i] ? 0x07E0 : 0xFDC0; // Green if hit, yellow if pending

    tft.drawCircle(tx, ty, 8, c);
    tft.drawCircle(tx, ty, 2, c);
    tft.drawLine(tx - 12, ty, tx + 12, ty, c);
    tft.drawLine(tx, ty - 12, tx, ty + 12, c);
  }

  tft.setTextColor(0x5AEB, 0x0821);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("Tap the 9 targets above to verify accuracy or draw freely", 160, 226);
}

// -------------------------------------------------------------
// TAB 3: AUDIO & SPEAKER DIAGNOSTICS
// -------------------------------------------------------------
void renderAudioTab() {
  tft.fillRect(0, 26, 320, 214, 0x0821);

  tft.setTextColor(0xFDC0, 0x0821);
  tft.setTextSize(1);
  tft.setTextDatum(TL_DATUM);
  tft.drawString("ONBOARD SPEAKER & FM8002A AMPLIFIER TEST (PIN 26)", 12, 34);

  // Audio Buttons
  struct AudioTestBtn {
    int x, y, w, h;
    const char* label;
    int freq;
    uint16_t color;
  };
  AudioTestBtn btns[] = {
    {12, 54, 94, 44, "440 Hz (A4)", 440, 0x18C3},
    {113, 54, 94, 44, "1000 Hz", 1000, 0x18C3},
    {214, 54, 94, 44, "2000 Hz", 2000, 0x18C3},
    {12, 106, 145, 44, "SWEEP 100-3000Hz", -1, 0x0410},
    {163, 106, 145, 44, "STOP AUDIO", 0, 0xF800}
  };

  for (int i = 0; i < 5; i++) {
    tft.fillRoundRect(btns[i].x, btns[i].y, btns[i].w, btns[i].h, 6, btns[i].color);
    tft.drawRoundRect(btns[i].x, btns[i].y, btns[i].w, btns[i].h, 6, 0x39C7);
    tft.setTextColor(TFT_WHITE, btns[i].color);
    tft.setTextDatum(MC_DATUM);
    tft.drawString(btns[i].label, btns[i].x + (btns[i].w / 2), btns[i].y + (btns[i].h / 2));
  }

  // Audio Amplifier Specs Card
  tft.fillRoundRect(12, 158, 296, 68, 6, 0x18C3);
  tft.drawRoundRect(12, 158, 296, 68, 6, 0x2945);
  tft.setTextColor(0x07FF, 0x18C3);
  tft.setTextDatum(TL_DATUM);
  tft.drawString("AMPLIFIER SPECIFICATIONS & WARNING", 20, 166);
  tft.setTextColor(TFT_LIGHTGREY, 0x18C3);
  tft.drawString("- Circuit: FM8002A single-channel 2.0W class-AB power amp", 20, 180);
  tft.drawString("- Audio driven via ESP32 DAC/LEDC tone on GPIO 26", 20, 194);
  tft.drawString("- If distorted, check speaker connector or fit an 8-ohm 1W cone", 20, 208);
}

// -------------------------------------------------------------
// TAB 4: SENSORS (LDR, HALL, TEMP, RGB)
// -------------------------------------------------------------
void renderSensorsTab() {
  tft.fillRect(0, 26, 320, 214, 0x0821);

  // LDR Light Sensor Title & Readout
  int rawLdr = analogRead(LDR_PIN);
  float luxEst = (4095.0f - (float)rawLdr) * (1000.0f / 4095.0f); // Inverted LDR divider

  tft.setTextColor(0xFDC0, 0x0821);
  tft.setTextSize(1);
  tft.setTextDatum(TL_DATUM);
  tft.drawString("LIGHT DEPENDENT RESISTOR (LDR / GPIO 34)", 12, 34);

  tft.setTextColor(TFT_WHITE, 0x0821);
  char ldrBuf[64];
  snprintf(ldrBuf, sizeof(ldrBuf), "Raw ADC: %04d | Approx Lux: %03.0f lx", rawLdr, luxEst);
  tft.drawString(ldrBuf, 12, 48);

  // LDR History Graph Frame
  tft.drawRect(12, 62, GRAPH_WIDTH, GRAPH_HEIGHT, 0x39C7);
  tft.fillRect(13, 63, GRAPH_WIDTH - 2, GRAPH_HEIGHT - 2, TFT_BLACK);

  // Plot history
  for (int i = 0; i < GRAPH_WIDTH - 1; i++) {
    int idx1 = (ldrHistoryIdx + i) % GRAPH_WIDTH;
    int idx2 = (ldrHistoryIdx + i + 1) % GRAPH_WIDTH;
    int y1 = 63 + (GRAPH_HEIGHT - 4) - map(ldrHistory[idx1], 0, 4095, 0, GRAPH_HEIGHT - 4);
    int y2 = 63 + (GRAPH_HEIGHT - 4) - map(ldrHistory[idx2], 0, 4095, 0, GRAPH_HEIGHT - 4);
    tft.drawLine(13 + i, y1, 13 + i + 1, y2, 0xFDC0);
  }

  // Internal Sensors Panel
  tft.fillRoundRect(220, 62, 88, GRAPH_HEIGHT, 4, 0x18C3);
  tft.drawRoundRect(220, 62, 88, GRAPH_HEIGHT, 4, 0x39C7);
  tft.setTextColor(TFT_CYAN, 0x18C3);
  tft.drawString("INTERNAL", 226, 68);
  tft.setTextColor(TFT_WHITE, 0x18C3);
  tft.drawString("Temp: " + String(temperatureRead(), 1) + "C", 226, 84);
  tft.drawString("Hall: " + String(hallRead()), 226, 100);

  // RGB LED Test Buttons
  tft.setTextColor(0x07FF, 0x0821);
  tft.drawString("ONBOARD RGB LED EXERCISER (GPIO 4, 16, 17)", 12, 132);

  struct LedBtn {
    int x, y, w, h;
    const char* label;
    uint16_t color;
    bool r, g, b;
  };
  LedBtn ledBtns[] = {
    {12, 148, 68, 32, "RED", TFT_RED, true, false, false},
    {88, 148, 68, 32, "GREEN", 0x07E0, false, true, false},
    {164, 148, 68, 32, "BLUE", 0x001F, false, false, true},
    {240, 148, 68, 32, "WHITE", TFT_WHITE, true, true, true},
    {12, 186, 296, 32, "TURN OFF RGB LED", 0x2104, false, false, false}
  };

  for (int i = 0; i < 5; i++) {
    tft.fillRoundRect(ledBtns[i].x, ledBtns[i].y, ledBtns[i].w, ledBtns[i].h, 4, ledBtns[i].color);
    uint16_t textColor = (i == 1 || i == 3) ? TFT_BLACK : TFT_WHITE;
    tft.setTextColor(textColor, ledBtns[i].color);
    tft.setTextDatum(MC_DATUM);
    tft.drawString(ledBtns[i].label, ledBtns[i].x + (ledBtns[i].w / 2), ledBtns[i].y + (ledBtns[i].h / 2));
  }
}

// -------------------------------------------------------------
// TAB 5: SYSTEM & WIFI SCANNER
// -------------------------------------------------------------
void scanNearbyWifi() {
  tft.fillRect(12, 130, 296, 100, 0x0821);
  tft.setTextColor(0xFDC0, 0x0821);
  tft.setTextSize(1);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("Scanning 2.4GHz WiFi channels...", 160, 160);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  int n = WiFi.scanNetworks(false, false, false, 200);
  wifiNetworkCount = min(n, MAX_SCAN_RESULTS);
  for (int i = 0; i < wifiNetworkCount; i++) {
    wifiNetworks[i].ssid = WiFi.SSID(i);
    wifiNetworks[i].rssi = WiFi.RSSI(i);
    wifiNetworks[i].authmode = WiFi.encryptionType(i);
  }
  wifiScanDone = true;
  WiFi.scanDelete();

  renderSystemTab();
}

void renderSystemTab() {
  tft.fillRect(0, 26, 320, 214, 0x0821);

  // ESP32 Chip Specs Card
  tft.fillRoundRect(12, 32, 296, 68, 6, 0x18C3);
  tft.drawRoundRect(12, 32, 296, 68, 6, 0x39C7);

  tft.setTextColor(0xFDC0, 0x18C3);
  tft.setTextSize(1);
  tft.setTextDatum(TL_DATUM);
  tft.drawString("ESP32 HARDWARE PROFILE", 20, 38);

  tft.setTextColor(TFT_WHITE, 0x18C3);
  esp_chip_info_t chip_info;
  esp_chip_info(&chip_info);

  tft.drawString("Model: ESP32-D0WDQ6 (Rev " + String(chip_info.revision) + ") Dual-Core 240MHz", 20, 52);
  tft.drawString("Flash: " + String(ESP.getFlashChipSize() / (1024 * 1024)) + "MB SPI @ " + String(ESP.getFlashChipSpeed() / 1000000) + "MHz", 20, 66);
  tft.drawString("Free Heap: " + String(ESP.getFreeHeap() / 1024) + " KB | Min Free: " + String(ESP.getMinFreeHeap() / 1024) + " KB", 20, 80);

  // WiFi Networks Section
  tft.setTextColor(0x07FF, 0x0821);
  tft.drawString("WIFI RF SCANNER", 12, 106);

  // Rescan Button
  tft.fillRoundRect(220, 102, 88, 20, 4, 0xFDC0);
  tft.setTextColor(TFT_BLACK, 0xFDC0);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("SCAN WIFI", 264, 112);

  if (!wifiScanDone) {
    tft.setTextColor(TFT_LIGHTGREY, 0x0821);
    tft.setTextDatum(TL_DATUM);
    tft.drawString("Tap 'SCAN WIFI' above to probe local 2.4GHz RF spectrum.", 12, 126);
  } else {
    tft.setTextDatum(TL_DATUM);
    for (int i = 0; i < wifiNetworkCount; i++) {
      int y = 126 + (i * 17);
      tft.setTextColor(TFT_WHITE, 0x0821);
      String truncatedSsid = wifiNetworks[i].ssid.substring(0, 16);
      tft.drawString(truncatedSsid, 16, y);

      // RSSI Bar
      int rssiW = map(constrain(wifiNetworks[i].rssi, -100, -30), -100, -30, 5, 60);
      uint16_t rssiCol = (wifiNetworks[i].rssi > -65) ? 0x07E0 : (wifiNetworks[i].rssi > -80 ? 0xFDC0 : 0xF800);
      tft.fillRect(160, y + 2, rssiW, 8, rssiCol);
      tft.drawRect(160, y + 2, 60, 8, 0x39C7);

      tft.setTextColor(rssiCol, 0x0821);
      tft.drawString(String(wifiNetworks[i].rssi) + " dBm", 230, y);
    }
  }

  // Footer Uptime
  tft.setTextColor(0x5AEB, 0x0821);
  tft.setTextDatum(TL_DATUM);
  tft.drawString("Uptime: " + String(millis() / 1000) + "s | MAC: " + WiFi.macAddress(), 12, 226);
}

void renderCurrentTab() {
  switch (currentTab) {
    case TAB_BENCH:   renderBenchTab(); break;
    case TAB_DISPLAY: renderDisplayTab(); break;
    case TAB_TOUCH:   renderTouchTab(); break;
    case TAB_AUDIO:   renderAudioTab(); break;
    case TAB_SENSORS: renderSensorsTab(); break;
    case TAB_SYSTEM:  renderSystemTab(); break;
  }
}

void loop() {
  // Handle Audio Sweep if Active
  if (isAudioPlaying) {
    unsigned long elapsed = millis() - audioSweepStart;
    if (elapsed < 3000) {
      int currentFreq = 100 + (elapsed * 2900 / 3000);
      tone(SPEAKER_PIN, currentFreq);
    } else {
      noTone(SPEAKER_PIN);
      isAudioPlaying = false;
    }
  }

  // Periodic Sensor Sampling
  if (millis() - lastSensorUpdate > 100) {
    lastSensorUpdate = millis();
    int rawLdr = analogRead(LDR_PIN);
    ldrHistory[ldrHistoryIdx] = rawLdr;
    ldrHistoryIdx = (ldrHistoryIdx + 1) % GRAPH_WIDTH;

    // If on sensors tab, update LDR line graph live
    if (currentTab == TAB_SENSORS) {
      // Fast line refresh without full redraw
      int prevIdx = (ldrHistoryIdx - 2 + GRAPH_WIDTH) % GRAPH_WIDTH;
      int currIdx = (ldrHistoryIdx - 1 + GRAPH_WIDTH) % GRAPH_WIDTH;
      int xPrev = 13 + ((ldrHistoryIdx - 2 + GRAPH_WIDTH) % GRAPH_WIDTH);
      int xCurr = 13 + ((ldrHistoryIdx - 1 + GRAPH_WIDTH) % GRAPH_WIDTH);
      
      if (xCurr < 13 + GRAPH_WIDTH - 1) {
        int y1 = 63 + (GRAPH_HEIGHT - 4) - map(ldrHistory[prevIdx], 0, 4095, 0, GRAPH_HEIGHT - 4);
        int y2 = 63 + (GRAPH_HEIGHT - 4) - map(ldrHistory[currIdx], 0, 4095, 0, GRAPH_HEIGHT - 4);
        tft.drawLine(xCurr, 63, xCurr, 63 + GRAPH_HEIGHT - 2, TFT_BLACK); // clear scan column
        tft.drawLine(xPrev, y1, xCurr, y2, 0xFDC0);
      }
    }
  }

  // Handle Touch Screen Input
  if (ts.touched()) {
    TS_Point p = ts.getPoint();
    lastRawX = p.x;
    lastRawY = p.y;
    lastRawZ = p.z;

    int x = map(p.x, TS_MINX, TS_MAXX, 0, 320);
    int y = map(p.y, TS_MINY, TS_MAXY, 0, 240);
    x = constrain(x, 0, 319);
    y = constrain(y, 0, 239);

    lastTouchX = x;
    lastTouchY = y;

    // Full screen color test dismiss
    if (fullScreenColorIdx >= 0) {
      fullScreenColorIdx++;
      if (fullScreenColorIdx >= 8) {
        fullScreenColorIdx = -1; // Exit test
      }
      playBeep(1800, 30);
      drawTabBar();
      renderDisplayTab();
      delay(250);
      return;
    }

    // Tab Bar Touches (y: 0 to 26)
    if (y <= 26) {
      int tabW = 320 / 6;
      AppTab selectedTab = (AppTab)(x / tabW);
      if (selectedTab != currentTab) {
        currentTab = selectedTab;
        playBeep(1400, 25);
        drawTabBar();
        renderCurrentTab();
        delay(150);
      }
      return;
    }

    // Tab-Specific Actions
    switch (currentTab) {
      case TAB_BENCH: {
        // Run Benchmark button: y >= 182 && y <= 218
        if (y >= 182 && y <= 218) {
          playBeep(2000, 40);
          runBenchmark();
          delay(200);
        }
        break;
      }

      case TAB_DISPLAY: {
        // Swatches (y: 50 to 84)
        if (y >= 50 && y <= 84) {
          for (int i = 0; i < 8; i++) {
            int bx = 12 + i * 38;
            if (x >= bx && x <= bx + 34) {
              fullScreenColorIdx = i;
              playBeep(1800, 30);
              renderDisplayTab();
              delay(200);
              break;
            }
          }
        }
        break;
      }

      case TAB_TOUCH: {
        // Clear Button (x: 240 to 310, y: 32 to 56)
        if (x >= 240 && x <= 310 && y >= 32 && y <= 56) {
          for (int i = 0; i < 9; i++) targetHits[i] = false;
          playBeep(800, 50);
          renderTouchTab();
          delay(150);
          return;
        }

        // Draw crosshair trail on canvas
        tft.fillCircle(x, y, 2, TFT_WHITE);

        // Check 9 Target Hits
        const int targetPoints[9][2] = {
          {30, 80},  {160, 80},  {290, 80},
          {30, 140}, {160, 140}, {290, 140},
          {30, 200}, {160, 200}, {290, 200}
        };
        for (int i = 0; i < 9; i++) {
          int dx = abs(x - targetPoints[i][0]);
          int dy = abs(y - targetPoints[i][1]);
          if (dx < 15 && dy < 15 && !targetHits[i]) {
            targetHits[i] = true;
            playBeep(2200, 30);
            tft.fillCircle(targetPoints[i][0], targetPoints[i][1], 8, 0x07E0);
          }
        }
        break;
      }

      case TAB_AUDIO: {
        // 440 Hz
        if (x >= 12 && x <= 106 && y >= 54 && y <= 98) {
          playBeep(440, 500);
        }
        // 1000 Hz
        else if (x >= 113 && x <= 207 && y >= 54 && y <= 98) {
          playBeep(1000, 500);
        }
        // 2000 Hz
        else if (x >= 214 && x <= 308 && y >= 54 && y <= 98) {
          playBeep(2000, 500);
        }
        // Sweep
        else if (x >= 12 && x <= 157 && y >= 106 && y <= 150) {
          isAudioPlaying = true;
          audioSweepStart = millis();
        }
        // Stop
        else if (x >= 163 && x <= 308 && y >= 106 && y <= 150) {
          noTone(SPEAKER_PIN);
          isAudioPlaying = false;
        }
        delay(150);
        break;
      }

      case TAB_SENSORS: {
        // RGB Buttons (y: 148 to 180)
        if (y >= 148 && y <= 180) {
          if (x >= 12 && x <= 80)        setRgbLed(true, false, false);
          else if (x >= 88 && x <= 156)   setRgbLed(false, true, false);
          else if (x >= 164 && x <= 232)  setRgbLed(false, false, true);
          else if (x >= 240 && x <= 308)  setRgbLed(true, true, true);
          playBeep(1600, 20);
          delay(120);
        }
        // Turn Off RGB (y: 186 to 218)
        else if (y >= 186 && y <= 218) {
          setRgbLed(false, false, false);
          playBeep(1000, 20);
          delay(120);
        }
        break;
      }

      case TAB_SYSTEM: {
        // Scan WiFi Button (x: 220 to 308, y: 102 to 122)
        if (x >= 220 && x <= 308 && y >= 102 && y <= 122) {
          playBeep(2000, 40);
          scanNearbyWifi();
          delay(200);
        }
        break;
      }
    }
  }

  delay(20);
}
