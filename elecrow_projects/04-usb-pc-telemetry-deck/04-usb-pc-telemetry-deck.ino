/**
 * @file 04-usb-pc-telemetry-deck.ino
 * @brief USB Serial PC Telemetry & Macro Deck (Live Hardware Calibrated)
 * @author Tushar Aggarwal (@tushar2704)
 * @channel IamNotIntoTech (https://youtube.com/@IamNotIntoTech)
 * 
 * Hardware:
 * - Elecrow CrowPanel 2.8" / ESP32-2432S028R
 * - Display: ILI9341 320x240
 * - Touch: XPT2046 Resistive Touch (SPI: CLK=25, MISO=39, MOSI=32, CS=33)
 * - Communication: CH340 USB Serial @ 115200 baud (JSON protocol)
 * - Audio: GPIO 26 Tone feedback
 * - RGB LED: GPIO 4, 16, 17 [Active LOW]
 */

#include <ArduinoJson.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>

// Accurate CYD 2.8" Resistive Touch SPI Pinout
#define XPT2046_MOSI  32
#define XPT2046_MISO  39
#define XPT2046_CLK   25
#define XPT2046_CS    33
#define XPT2046_IRQ   36

#define LED_RED       4
#define LED_GREEN     16
#define LED_BLUE      17
#define LDR_PIN       34
#define SPEAKER_PIN   26

SPIClass touchSPI(VSPI);
XPT2046_Touchscreen ts(XPT2046_CS);
TFT_eSPI tft = TFT_eSPI();

// Live Telemetry State
int cpuUsage = 0;
int gpuUsage = 0;
int ramUsage = 0;
int vramUsage = 0;
int cpuTemp = 0;
int gpuTemp = 0;
bool isHostConnected = false;
unsigned long lastHostPing = 0;
unsigned long lastTouchTime = 0;
unsigned long footerClearTime = 0;
String serialBuffer = "";

struct MacroTile {
  const char* label;
  const char* cmdId;
  uint16_t color;
};

#define MACRO_COUNT 6
MacroTile macros[MACRO_COUNT] = {
  {"MUTE AUDIO", "MUTE_VOL",      0xF800}, // Red
  {"TERMINAL",   "OPEN_TERM",     0x07E0}, // Green
  {"DESKTOP",    "SHOW_DESKTOP",  0x041F}, // Blue
  {"TASK MGR",   "OPEN_TASKMGR",  0xFD20}, // Orange
  {"YOUTUBE",    "OPEN_YOUTUBE",  0xF800}, // YouTube Red
  {"LOCK PC",    "LOCK_HOST",     0x8410}  // Dark Gray
};

void setRgbLed(bool r, bool g, bool b) {
  digitalWrite(LED_RED, r ? LOW : HIGH);
  digitalWrite(LED_GREEN, g ? LOW : HIGH);
  digitalWrite(LED_BLUE, b ? LOW : HIGH);
}

void playBeep(int freq, int durationMs) {
  tone(SPEAKER_PIN, freq, durationMs);
}

void drawBarGauge(int x, int y, int w, int h, int percent, uint16_t color) {
  tft.drawRect(x, y, w, h, TFT_WHITE);
  tft.fillRect(x + 1, y + 1, w - 2, h - 2, 0x0000);
  int fillW = map(percent, 0, 100, 0, w - 2);
  fillW = constrain(fillW, 0, w - 2);
  if (fillW > 0) {
    tft.fillRect(x + 1, y + 1, fillW, h - 2, color);
  }
}

void updateFooter(const char* message, uint16_t fg = TFT_WHITE, uint16_t bg = 0x18C3) {
  tft.fillRect(0, 224, 320, 16, bg);
  tft.setTextColor(fg, bg);
  tft.setTextSize(1);
  tft.setCursor(8, 228);
  tft.print(message);
}

void drawTelemetry() {
  int startY = 44;
  int barW = 132;
  int barH = 10;

  // CPU Gauge
  tft.fillRect(8, startY, 134, 10, 0x0821);
  tft.setTextColor(TFT_WHITE, 0x0821);
  tft.setTextSize(1);
  tft.setCursor(8, startY);
  if (isHostConnected) {
    tft.printf("CPU: %2d%%  (%d C)", cpuUsage, cpuTemp);
  } else {
    tft.print("CPU: --%  (-- C)");
  }
  uint16_t cpuCol = (cpuUsage > 85) ? 0xF800 : (cpuUsage > 60 ? 0xFD20 : 0x07E0);
  drawBarGauge(8, startY + 12, barW, barH, isHostConnected ? cpuUsage : 0, cpuCol);

  // GPU Gauge
  tft.fillRect(8, startY + 30, 134, 10, 0x0821);
  tft.setTextColor(TFT_WHITE, 0x0821);
  tft.setCursor(8, startY + 30);
  if (isHostConnected) {
    tft.printf("GPU: %2d%%  (%d C)", gpuUsage, gpuTemp);
  } else {
    tft.print("GPU: --%  (-- C)");
  }
  uint16_t gpuCol = (gpuUsage > 85) ? 0xF800 : (gpuUsage > 60 ? 0xFD20 : 0x041F);
  drawBarGauge(8, startY + 42, barW, barH, isHostConnected ? gpuUsage : 0, gpuCol);

  // RAM Gauge
  tft.fillRect(8, startY + 60, 134, 10, 0x0821);
  tft.setTextColor(TFT_WHITE, 0x0821);
  tft.setCursor(8, startY + 60);
  if (isHostConnected) {
    tft.printf("RAM: %2d%%  USED", ramUsage);
  } else {
    tft.print("RAM: --%  USED");
  }
  drawBarGauge(8, startY + 72, barW, barH, isHostConnected ? ramUsage : 0, 0x780F);

  // VRAM Gauge
  tft.fillRect(8, startY + 90, 134, 10, 0x0821);
  tft.setTextColor(TFT_WHITE, 0x0821);
  tft.setCursor(8, startY + 90);
  if (isHostConnected) {
    tft.printf("VRAM: %2d%% USED", vramUsage);
  } else {
    tft.print("VRAM: --% USED");
  }
  drawBarGauge(8, startY + 102, barW, barH, isHostConnected ? vramUsage : 0, 0x07FF);

  // Host Status Pill
  int statY = startY + 124;
  uint16_t pillBg = isHostConnected ? 0x03E0 : 0x8000;
  tft.fillRoundRect(8, statY, 132, 22, 4, pillBg);
  tft.drawRoundRect(8, statY, 132, 22, 4, TFT_WHITE);
  tft.setTextColor(TFT_WHITE, pillBg);
  tft.setCursor(14, statY + 7);
  tft.print(isHostConnected ? "USB: LIVE PC DATA" : "USB: RUN BRIDGE.PY");

  // Helper text below pill
  tft.fillRect(8, statY + 28, 134, 12, 0x0821);
  tft.setTextColor(isHostConnected ? 0x07E0 : 0xFD20, 0x0821);
  tft.setCursor(8, statY + 28);
  tft.print(isHostConnected ? "[Link 115200 Active]" : "[Waiting on COM5...]");
}

void drawMacroTileState(int i, bool pressed) {
  int startX = 154;
  int startY = 38;
  int tileW = 76;
  int tileH = 54;
  int gap = 6;

  int col = i % 2;
  int row = i / 2;
  int tx = startX + col * (tileW + gap);
  int ty = startY + row * (tileH + gap);

  uint16_t bg = pressed ? macros[i].color : 0x0000;
  uint16_t fg = pressed ? TFT_BLACK : TFT_WHITE;
  uint16_t border = pressed ? TFT_WHITE : macros[i].color;

  tft.fillRoundRect(tx, ty, tileW, tileH, 5, bg);
  tft.drawRoundRect(tx, ty, tileW, tileH, 5, border);

  if (!pressed) {
    tft.fillRoundRect(tx + 2, ty + 2, tileW - 4, 6, 2, macros[i].color);
  }

  tft.setTextColor(fg, bg);
  tft.setTextSize(1);

  if (i == 4) { // Special branding for YouTube Channel
    tft.setCursor(tx + 16, ty + 16);
    tft.print("YOUTUBE");
    tft.setTextColor(pressed ? TFT_BLACK : 0xFDC0, bg);
    tft.setCursor(tx + 4, ty + 32);
    tft.print("@IamNotInto");
  } else {
    tft.setCursor(tx + 10, ty + 24);
    tft.print(macros[i].label);
  }
}

void drawMacroTiles() {
  for (int i = 0; i < MACRO_COUNT; i++) {
    drawMacroTileState(i, false);
  }
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
  tft.print("PC HARDWARE SENSORS");

  // Right Section Header
  tft.setTextColor(0xFDC0, 0x0821);
  tft.setCursor(156, 28);
  tft.print("TOUCH MACRO TILES");

  updateFooter("CH340 Serial 115200 Baud | @IamNotIntoTech");
}

void triggerMacro(int macroIdx) {
  // 1. Visual press animation on the tile
  drawMacroTileState(macroIdx, true);

  // 2. Audible & RGB confirmation
  playBeep(2400, 60);
  setRgbLed(false, false, true); // Blue flash

  // 3. Update footer with loud confirmation
  char msg[64];
  snprintf(msg, sizeof(msg), "⚡ [ACTION] -> %s", macros[macroIdx].label);
  updateFooter(msg, TFT_BLACK, 0xFDC0);
  footerClearTime = millis() + 2500;

  // 4. Dispatch JSON Action Packet to Host PC over USB Serial
  StaticJsonDocument<128> doc;
  doc["type"] = "macro_trigger";
  doc["id"] = macros[macroIdx].cmdId;
  doc["label"] = macros[macroIdx].label;
  doc["timestamp"] = millis();

  serializeJson(doc, Serial);
  Serial.println();

  delay(120);

  // 5. Restore tile state
  drawMacroTileState(macroIdx, false);
  setRgbLed(isHostConnected ? false : true, isHostConnected ? true : false, false);
}

void parseSerialTelemetry() {
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n') {
      serialBuffer.trim();
      if (serialBuffer.startsWith("{")) {
        StaticJsonDocument<256> doc;
        DeserializationError err = deserializeJson(doc, serialBuffer);
        if (!err) {
          if (doc.containsKey("cpu")) cpuUsage = doc["cpu"];
          if (doc.containsKey("gpu")) gpuUsage = doc["gpu"];
          if (doc.containsKey("ram")) ramUsage = doc["ram"];
          if (doc.containsKey("vram")) vramUsage = doc["vram"];
          if (doc.containsKey("cpuTemp")) cpuTemp = doc["cpuTemp"];
          if (doc.containsKey("gpuTemp")) gpuTemp = doc["gpuTemp"];

          if (!isHostConnected) {
            isHostConnected = true;
            playBeep(2000, 50);
            setRgbLed(false, true, false);
          }
          lastHostPing = millis();
          drawTelemetry();
        }
      }
      serialBuffer = "";
    } else if (c != '\r') {
      if (serialBuffer.length() < 256) {
        serialBuffer += c;
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

  // Initialize resistive touch on dedicated CYD pins
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
  tft.print("Live PC Telemetry | 6 Macro Action Tiles");

  playBeep(1500, 80);
  delay(100);
  playBeep(2000, 100);
  delay(500);

  drawStaticLayout();
  drawTelemetry();
  drawMacroTiles();
  setRgbLed(true, true, false); // Amber = waiting for PC bridge
}

void loop() {
  parseSerialTelemetry();

  // Host timeout: 4s of no PC data reverts to standby
  if (isHostConnected && (millis() - lastHostPing > 4000)) {
    isHostConnected = false;
    setRgbLed(true, true, false);
    drawTelemetry();
  }

  // Clear macro confirmation in footer
  if (footerClearTime > 0 && millis() > footerClearTime) {
    footerClearTime = 0;
    updateFooter(isHostConnected ? "Live PC Stream Active | @IamNotIntoTech" : "Run bridge.py on PC for Live Telemetry | @IamNotIntoTech");
  }

  // Touch handling
  if (ts.touched()) {
    TS_Point p = ts.getPoint();
    if (p.z > 50 || ts.touched()) {
      if (millis() - lastTouchTime > 180) {
        lastTouchTime = millis();

        // Landscape calibration: non-inverted Y
        int screenX = map(p.x, 240, 3800, 0, 320);
        int screenY = map(p.y, 240, 3800, 0, 240);
        screenX = constrain(screenX, 0, 319);
        screenY = constrain(screenY, 0, 239);

        Serial.printf("[TOUCH] Raw X=%d, Y=%d, Z=%d -> Screen (%d, %d)\n", p.x, p.y, p.z, screenX, screenY);

        // Right side: 6 Macro Tiles (X: 148 to 320, Y: 28 to 220)
        if (screenX >= 148 && screenY >= 28 && screenY <= 220) {
          int col = (screenX >= 235) ? 1 : 0;
          int row = 0;
          if (screenY >= 155) row = 2;
          else if (screenY >= 95) row = 1;
          else row = 0;

          int idx = row * 2 + col;
          if (idx >= 0 && idx < MACRO_COUNT) {
            triggerMacro(idx);
          }
        }
        // Left side: Sensor Gauges tapped
        else if (screenX < 148 && screenY >= 28 && screenY <= 220) {
          playBeep(1800, 40);
          updateFooter("⚡ [TOUCH] HARDWARE SENSORS TAPPED", TFT_YELLOW, 0x0010);
          footerClearTime = millis() + 2000;
        }
      }
    }
  }

  delay(15);
}
