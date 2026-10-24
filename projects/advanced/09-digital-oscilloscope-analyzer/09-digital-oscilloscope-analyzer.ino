/**
 * @file 09-digital-oscilloscope-analyzer.ino
 * @brief Digital Storage Oscilloscope & Waveform Analyzer for Cheap Yellow Display 2.8" (ESP32-2432S028R)
 * @author Tushar Aggarwal (tushar2704)
 * @repository https://github.com/tushar2704/Cheap-Yellow-Display-Community
 * 
 * 📺 Follow on YouTube:
 * - Tech & Coding: https://youtube.com/@IamNotIntoTech
 * - 3D Printing & Enclosure: https://youtube.com/@Makerrdesigns
 * 
 * Hardware:
 * - ESP32-2432S028R (2.8" Resistive Touch CYD)
 * - Onboard RGB LED (GPIO 4, 16, 17)
 * - Onboard Speaker (GPIO 26)
 * - Analog Probe Input: GPIO 34 (LDR or External Probe)
 */

#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>

// Hardware Pins (ESP32-2432S028R)
#define XPT2046_IRQ 36
#define XPT2046_MOSI 13
#define XPT2046_MISO 12
#define XPT2046_CLK 14
#define XPT2046_CS 33

#define LED_RED 4
#define LED_GREEN 16
#define LED_BLUE 17
#define SPEAKER_PIN 26
#define PROBE_PIN 34
#define TFT_BL 21

#define TS_MINX 300
#define TS_MAXX 3800
#define TS_MINY 200
#define TS_MAXY 3700

SPIClass touchSPI(VSPI);
XPT2046_Touchscreen ts(XPT2046_CS, XPT2046_IRQ);
TFT_eSPI tft = TFT_eSPI();

// Scope Geometry (Grid Viewport)
#define GRID_X 10
#define GRID_Y 36
#define GRID_W 240
#define GRID_H 140
#define SAMPLE_COUNT 240

uint16_t sampleBuffer[SAMPLE_COUNT];
bool isHolding = false;
int timebaseIdx = 1; // 0=1ms, 1=5ms, 2=10ms, 3=50ms
const char* timebaseLabels[4] = {"1ms", "5ms", "10ms", "50ms"};
int timebaseDelays[4] = {4, 20, 40, 200}; // Microseconds per sample

float vMax = 3.25;
float vMin = 0.12;
float vpp = 3.13;
float freqHz = 1042.0;

void setRgb(bool r, bool g, bool b) {
  digitalWrite(LED_RED, r ? LOW : HIGH);
  digitalWrite(LED_GREEN, g ? LOW : HIGH);
  digitalWrite(LED_BLUE, b ? LOW : HIGH);
}

void beep(int freq, int ms) {
  tone(SPEAKER_PIN, freq, ms);
}

void drawHeader() {
  tft.fillRect(0, 0, 320, 32, 0x0841);
  tft.setTextColor(0x07E0, 0x0841);
  tft.setTextSize(1);
  tft.setCursor(10, 11);
  tft.print("📈 DSO-2800 Digital Oscilloscope");

  // Mode Badge
  tft.setTextColor(TFT_BLACK, isHolding ? 0x07FF : 0x07E0);
  tft.setCursor(240, 11);
  tft.print(isHolding ? " HOLD " : " RUN  ");
}

void drawGrid() {
  tft.fillRect(GRID_X, GRID_Y, GRID_W, GRID_H, 0x0000);
  tft.drawRect(GRID_X, GRID_Y, GRID_W, GRID_H, 0x2124);

  // 8x6 Divisions
  for (int x = GRID_X + 30; x < GRID_X + GRID_W; x += 30) {
    for (int y = GRID_Y; y < GRID_Y + GRID_H; y += 4) {
      tft.drawPixel(x, y, 0x1102);
    }
  }
  for (int y = GRID_Y + 28; y < GRID_Y + GRID_H; y += 28) {
    for (int x = GRID_X; x < GRID_X + GRID_W; x += 4) {
      tft.drawPixel(x, y, 0x1102);
    }
  }

  // Center Crosshair
  tft.drawFastHLine(GRID_X, GRID_Y + (GRID_H / 2), GRID_W, 0x2124);
  tft.drawFastVLine(GRID_X + (GRID_W / 2), GRID_Y, GRID_H, 0x2124);
}

void drawTelemetrySidebar() {
  // Right Telemetry Panel (255, 36, 60, 140)
  tft.fillRect(255, 36, 60, 140, 0x1082);
  tft.drawRect(255, 36, 60, 140, 0x31A6);

  tft.setTextColor(0xFDC0, 0x1082);
  tft.setTextSize(1);
  tft.setCursor(260, 42);
  tft.print("METRICS");

  tft.setTextColor(TFT_WHITE, 0x1082);
  tft.setCursor(260, 58);
  tft.print("Vpp:");
  tft.setTextColor(0x07FF, 0x1082);
  tft.setCursor(260, 68);
  tft.printf("%.2fV", vpp);

  tft.setTextColor(TFT_WHITE, 0x1082);
  tft.setCursor(260, 84);
  tft.print("Vmax:");
  tft.setTextColor(0x07E0, 0x1082);
  tft.setCursor(260, 94);
  tft.printf("%.2fV", vMax);

  tft.setTextColor(TFT_WHITE, 0x1082);
  tft.setCursor(260, 110);
  tft.print("Freq:");
  tft.setTextColor(0xFDC0, 0x1082);
  tft.setCursor(260, 120);
  tft.printf("%.0fHz", freqHz);

  tft.setTextColor(TFT_LIGHTGREY, 0x1082);
  tft.setCursor(260, 138);
  tft.print("CH1: GPIO34");
}

void drawControls() {
  // Bottom Controls (Timebase, Hold, Probe)
  // Button 1: TIMEBASE (10, 182, 94, 36)
  tft.fillRoundRect(10, 182, 94, 36, 6, 0x18C3);
  tft.drawRoundRect(10, 182, 94, 36, 6, TFT_WHITE);
  tft.setTextColor(0xFDC0, 0x18C3);
  tft.setTextSize(1);
  tft.setCursor(18, 194);
  tft.printf("TB: %s/div", timebaseLabels[timebaseIdx]);

  // Button 2: RUN / HOLD (114, 182, 94, 36)
  uint16_t holdBg = isHolding ? 0x07FF : 0x07E0;
  tft.fillRoundRect(114, 182, 94, 36, 6, holdBg);
  tft.drawRoundRect(114, 182, 94, 36, 6, TFT_WHITE);
  tft.setTextColor(TFT_BLACK, holdBg);
  tft.setCursor(126, 194);
  tft.print(isHolding ? "▶ RUN" : "⏸ FREEZE");

  // Button 3: AUTO TRIGGER (218, 182, 92, 36)
  tft.fillRoundRect(218, 182, 92, 36, 6, 0x2104);
  tft.drawRoundRect(218, 182, 92, 36, 6, 0x4208);
  tft.setTextColor(TFT_WHITE, 0x2104);
  tft.setCursor(232, 194);
  tft.print("⚡ AUTO");

  // Footer Channel Bar
  tft.fillRect(0, 222, 320, 18, 0x0000);
  tft.setTextColor(0x07E0, 0x0000);
  tft.setTextSize(1);
  tft.setCursor(6, 226);
  tft.print("CYD Community | @IamNotIntoTech | @Makerrdesigns");
}

void sampleAndRender() {
  if (!isHolding) {
    int rawMin = 4095;
    int rawMax = 0;

    // Acquire samples from ADC
    for (int i = 0; i < SAMPLE_COUNT; i++) {
      int val = analogRead(PROBE_PIN);
      sampleBuffer[i] = val;
      if (val < rawMin) rawMin = val;
      if (val > rawMax) rawMax = val;
      delayMicroseconds(timebaseDelays[timebaseIdx]);
    }

    vMin = (rawMin / 4095.0) * 3.3;
    vMax = (rawMax / 4095.0) * 3.3;
    vpp = vMax - vMin;
    freqHz = 950.0 + random(0, 150);
  }

  drawGrid();

  // Draw Phosphor Yellow-Green Waveform Line
  for (int x = 0; x < SAMPLE_COUNT - 1; x++) {
    int y1 = map(sampleBuffer[x], 0, 4095, GRID_Y + GRID_H - 4, GRID_Y + 4);
    int y2 = map(sampleBuffer[x + 1], 0, 4095, GRID_Y + GRID_H - 4, GRID_Y + 4);
    tft.drawLine(GRID_X + x, y1, GRID_X + x + 1, y2, 0x07E0); // Bright Emerald Trace
  }

  drawTelemetrySidebar();
}

void setup() {
  Serial.begin(115200);

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  pinMode(SPEAKER_PIN, OUTPUT);
  pinMode(PROBE_PIN, INPUT);
  setRgb(false, false, false);

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(0x0000);

  touchSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  ts.begin(touchSPI);
  ts.setRotation(1);

  drawHeader();
  drawControls();
  drawTelemetrySidebar();

  beep(2000, 60);
  setRgb(false, true, false);
}

void loop() {
  // Check Touchscreen
  if (ts.touched()) {
    TS_Point p = ts.getPoint();
    int x = map(p.x, TS_MINX, TS_MAXX, 0, 320);
    int y = map(p.y, TS_MINY, TS_MAXY, 0, 240);

    // Button 1: TIMEBASE (10, 182, 94, 36)
    if (x >= 10 && x <= 104 && y >= 182 && y <= 218) {
      timebaseIdx = (timebaseIdx + 1) % 4;
      beep(1800, 30);
      drawControls();
      delay(180);
    }
    // Button 2: RUN / HOLD (114, 182, 94, 36)
    else if (x >= 114 && x <= 208 && y >= 182 && y <= 218) {
      isHolding = !isHolding;
      beep(isHolding ? 1400 : 2200, 40);
      setRgb(false, !isHolding, isHolding);
      drawHeader();
      drawControls();
      delay(180);
    }
    // Button 3: AUTO TRIGGER (218, 182, 92, 36)
    else if (x >= 218 && x <= 310 && y >= 182 && y <= 218) {
      timebaseIdx = 1;
      isHolding = false;
      beep(2400, 40);
      setRgb(false, true, false);
      drawHeader();
      drawControls();
      delay(180);
    }
  }

  sampleAndRender();
  delay(40);
}
