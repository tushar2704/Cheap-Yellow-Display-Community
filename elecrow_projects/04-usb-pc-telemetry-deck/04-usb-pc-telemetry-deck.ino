/**
 * @file 04-usb-pc-telemetry-deck.ino
 * @brief USB Serial PC Telemetry & Macro Deck
 * @author Tushar Aggarwal (@tushar2704)
 * @channel IamNotIntoTech (https://youtube.com/@IamNotIntoTech)
 * 
 * Hardware:
 * - Elecrow CrowPanel 2.8" / ESP32-2432S028R
 * - Display: ILI9341 320x240
 * - Touch: XPT2046 Resistive Touch
 * - Communication: CH340 USB Serial @ 115200 baud (JSON protocol)
 * - Audio: GPIO 26 Tone feedback
 * - RGB LED: GPIO 4, 16, 17 [Active LOW]
 */

#include <ArduinoJson.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>

#define XPT2046_IRQ   36
#define XPT2046_MOSI  13
#define XPT2046_MISO  12
#define XPT2046_CLK   14
#define XPT2046_CS    33

#define LED_RED       4
#define LED_GREEN     16
#define LED_BLUE      17
#define LDR_PIN       34
#define SPEAKER_PIN   26

#define TS_MINX 300
#define TS_MAXX 3800
#define TS_MINY 200
#define TS_MAXY 3700

SPIClass touchSPI(VSPI);
XPT2046_Touchscreen ts(XPT2046_CS, XPT2046_IRQ);
TFT_eSPI tft = TFT_eSPI();

// Telemetry State
int cpuUsage = 38;
int gpuUsage = 52;
int ramUsage = 47;
int vramUsage = 64;
int cpuTemp = 48;
int gpuTemp = 58;
bool isHostConnected = false;
unsigned long lastHostPing = 0;
unsigned long lastSimUpdate = 0;
unsigned long lastTouchTime = 0;

struct MacroTile {
  const char* label;
  const char* cmdId;
  uint16_t color;
};

#define MACRO_COUNT 6
MacroTile macros[MACRO_COUNT] = {
  {"MUTE MIC",   "MUTE_MIC",   0xF800}, // Red
  {"TERMINAL",   "OPEN_TERM",  0x07E0}, // Green
  {"GIT PULL",   "GIT_SYNC",   0x041F}, // Blue
  {"RUN BUILD",  "RUN_BUILD",  0xFD20}, // Orange
  {"AI PROMPT",  "AI_ASSIST",  0x780F}, // Purple
  {"LOCK PC",    "LOCK_HOST",  0x8410}  // Dark Gray
};

void setRgbLed(bool r, bool g, bool b) {
  digitalWrite(LED_RED, r ? LOW : HIGH);
  digitalWrite(LED_GREEN, g ? LOW : HIGH);
  digitalWrite(LED_BLUE, b ? LOW : HIGH);
}

void playBeep(int freq, int durationMs) {
  tone(SPEAKER_PIN, freq, durationMs);
}

void drawStaticLayout() {
  tft.fillScreen(0x0821);

  // Header
  tft.fillRect(0, 0, 320, 24, 0x0010);
  tft.setTextColor(0xFFE0, 0x0010);
  tft.setTextSize(1);
  tft.setCursor(6, 4);
  tft.print("PC TELEMETRY & MACRO DECK  ");
  tft.setTextColor(0x07E0, 0x0010);
  tft.print("IamNotIntoTech");

  // Left Section Divider (X: 148)
  tft.drawFastVLine(148, 24, 200, 0x2965);

  // Left Section Header
  tft.setTextColor(0x07FF, 0x0821);
  tft.setCursor(8, 28);
  tft.print("HARDWARE SENSORS");

  // Right Section Header
  tft.setTextColor(0xFDC0, 0x0821);
  tft.setCursor(156, 28);
  tft.print("TOUCH MACRO TILES");

  // Footer
  tft.fillRect(0, 224, 320, 16, 0x18C3);
  tft.setTextColor(TFT_WHITE, 0x18C3);
  tft.setCursor(8, 228);
  tft.print("CH340 Serial 115200 Baud | @IamNotIntoTech");
}

void drawBarGauge(int x, int y, int w, int h, int percent, uint16_t color) {
  tft.drawRect(x, y, w, h, TFT_WHITE);
  tft.fillRect(x + 1, y + 1, w - 2, h - 2, 0x0000);
  int fillW = map(percent, 0, 100, 0, w - 2);
  fillW = constrain(fillW, 0, w - 2);
  tft.fillRect(x + 1, y + 1, fillW, h - 2, color);
}

void drawTelemetry() {
  int startY = 44;
  int barW = 132;
  int barH = 10;

  // CPU Gauge
  tft.setTextColor(TFT_WHITE, 0x0821);
  tft.setTextSize(1);
  tft.setCursor(8, startY);
  tft.printf("CPU: %2d%%  (%d C)", cpuUsage, cpuTemp);
  uint16_t cpuCol = (cpuUsage > 85) ? 0xF800 : (cpuUsage > 60 ? 0xFD20 : 0x07E0);
  drawBarGauge(8, startY + 12, barW, barH, cpuUsage, cpuCol);

  // GPU Gauge
  tft.setTextColor(TFT_WHITE, 0x0821);
  tft.setCursor(8, startY + 30);
  tft.printf("GPU: %2d%%  (%d C)", gpuUsage, gpuTemp);
  uint16_t gpuCol = (gpuUsage > 85) ? 0xF800 : (gpuUsage > 60 ? 0xFD20 : 0x041F);
  drawBarGauge(8, startY + 42, barW, barH, gpuUsage, gpuCol);

  // RAM Gauge
  tft.setTextColor(TFT_WHITE, 0x0821);
  tft.setCursor(8, startY + 60);
  tft.printf("RAM: %2d%%  USED", ramUsage);
  drawBarGauge(8, startY + 72, barW, barH, ramUsage, 0x780F);

  // VRAM Gauge
  tft.setTextColor(TFT_WHITE, 0x0821);
  tft.setCursor(8, startY + 90);
  tft.printf("VRAM: %2d%% USED", vramUsage);
  drawBarGauge(8, startY + 102, barW, barH, vramUsage, 0x07FF);

  // Host Status Pill
  int statY = startY + 124;
  tft.fillRoundRect(8, statY, 132, 22, 3, isHostConnected ? 0x03E0 : 0x4208);
  tft.drawRoundRect(8, statY, 132, 22, 3, TFT_WHITE);
  tft.setTextColor(TFT_WHITE, isHostConnected ? 0x03E0 : 0x4208);
  tft.setCursor(14, statY + 6);
  tft.print(isHostConnected ? "USB HOST: ACTIVE" : "USB HOST: SIMULATION");
}

void drawMacroTiles() {
  int startX = 154;
  int startY = 44;
  int tileW = 76;
  int tileH = 52;
  int gap = 6;

  for (int i = 0; i < MACRO_COUNT; i++) {
    int col = i % 2;
    int row = i / 2;
    int tx = startX + col * (tileW + gap);
    int ty = startY + row * (tileH + gap);

    tft.fillRoundRect(tx, ty, tileW, tileH, 4, 0x0000);
    tft.drawRoundRect(tx, ty, tileW, tileH, 4, macros[i].color);

    // Accent top bar
    tft.fillRoundRect(tx + 2, ty + 2, tileW - 4, 6, 2, macros[i].color);

    tft.setTextColor(TFT_WHITE, 0x0000);
    tft.setTextSize(1);
    tft.setCursor(tx + 8, ty + 22);
    tft.print(macros[i].label);
  }
}

void triggerMacro(int macroIdx) {
  playBeep(2400, 60);
  setRgbLed(false, false, true); // Blue flash

  // Dispatch JSON Action Packet to Host PC over USB Serial
  StaticJsonDocument<128> doc;
  doc["type"] = "macro_trigger";
  doc["id"] = macros[macroIdx].cmdId;
  doc["label"] = macros[macroIdx].label;
  doc["timestamp"] = millis();

  serializeJson(doc, Serial);
  Serial.println();

  delay(60);
  setRgbLed(false, true, false);
}

void parseSerialTelemetry() {
  while (Serial.available()) {
    String line = Serial.readStringUntil('\n');
    line.trim();
    if (line.length() > 0 && line.startsWith("{")) {
      StaticJsonDocument<256> doc;
      DeserializationError err = deserializeJson(doc, line);
      if (!err) {
        if (doc.containsKey("cpu")) cpuUsage = doc["cpu"];
        if (doc.containsKey("gpu")) gpuUsage = doc["gpu"];
        if (doc.containsKey("ram")) ramUsage = doc["ram"];
        if (doc.containsKey("vram")) vramUsage = doc["vram"];
        if (doc.containsKey("cpuTemp")) cpuTemp = doc["cpuTemp"];
        if (doc.containsKey("gpuTemp")) gpuTemp = doc["gpuTemp"];

        isHostConnected = true;
        lastHostPing = millis();
        drawTelemetry();
      }
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(150);

  Serial.println("\n--- [Elecrow 2.8 CYD] 04 USB PC Telemetry & Macro Deck ---");
  Serial.println("Channel: @IamNotIntoTech");
  Serial.println("Author: Tushar Aggarwal (@tushar2704)");

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  setRgbLed(true, false, false);

  pinMode(SPEAKER_PIN, OUTPUT);
  pinMode(LDR_PIN, INPUT);

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);

  touchSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  ts.begin(touchSPI);
  ts.setRotation(1);

  // Splash
  tft.fillRect(0, 0, 320, 240, 0x0821);
  tft.fillRoundRect(20, 30, 280, 180, 8, 0x0000);
  tft.drawRoundRect(20, 30, 280, 180, 8, 0x041F);

  tft.setTextColor(0xFDC0, 0x0000);
  tft.setTextSize(2);
  tft.setCursor(44, 55);
  tft.print("IamNotIntoTech");

  tft.setTextColor(TFT_WHITE, 0x0000);
  tft.setTextSize(1);
  tft.setCursor(44, 90);
  tft.print("ELECROW 2.8\" CYD PROJECT SUITE");

  tft.setTextColor(0x07FF, 0x0000);
  tft.setCursor(44, 115);
  tft.print("Project 04: USB Serial Telemetry & Macro");

  tft.setTextColor(TFT_DARKGREY, 0x0000);
  tft.setCursor(44, 140);
  tft.print("CPU/GPU Gauges | 6-Tile Macro Controller");

  playBeep(1500, 80);
  delay(100);
  playBeep(2000, 100);
  delay(500);

  drawStaticLayout();
  drawTelemetry();
  drawMacroTiles();
  setRgbLed(false, true, false);
}

void loop() {
  parseSerialTelemetry();

  // If no host ping for 5s, revert to simulation mode
  if (isHostConnected && (millis() - lastHostPing > 5000)) {
    isHostConnected = false;
  }

  // Fluctuate telemetry gently in demo mode
  if (!isHostConnected && (millis() - lastSimUpdate > 1500)) {
    lastSimUpdate = millis();
    cpuUsage = constrain(cpuUsage + random(-4, 5), 15, 95);
    gpuUsage = constrain(gpuUsage + random(-6, 7), 20, 98);
    ramUsage = constrain(ramUsage + random(-2, 3), 30, 85);
    vramUsage = constrain(vramUsage + random(-3, 4), 40, 90);
    cpuTemp = map(cpuUsage, 15, 95, 42, 78);
    gpuTemp = map(gpuUsage, 20, 98, 48, 84);
    drawTelemetry();
  }

  // Touch handling for 6 Macro Tiles
  if (ts.touched()) {
    TS_Point p = ts.getPoint();
    if (millis() - lastTouchTime > 220) {
      lastTouchTime = millis();

      int screenX = map(p.x, TS_MINX, TS_MAXX, 0, 320);
      int screenY = map(p.y, TS_MINY, TS_MAXY, 0, 240);
      screenX = constrain(screenX, 0, 319);
      screenY = constrain(screenY, 0, 239);

      int startX = 154;
      int startY = 44;
      int tileW = 76;
      int tileH = 52;
      int gap = 6;

      for (int i = 0; i < MACRO_COUNT; i++) {
        int col = i % 2;
        int row = i / 2;
        int tx = startX + col * (tileW + gap);
        int ty = startY + row * (tileH + gap);

        if (screenX >= tx && screenX <= tx + tileW && screenY >= ty && screenY <= ty + tileH) {
          triggerMacro(i);
          break;
        }
      }
    }
  }

  delay(20);
}
