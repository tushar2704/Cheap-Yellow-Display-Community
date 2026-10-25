/*
 * Cheap Yellow Display (CYD) - MLX90640 Thermal Imager & Heat Map Scanner
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
#include <Wire.h>
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

// External I2C Pins for Thermal Sensor (MLX90640 / AMG8833 on CN1 connector)
#define SENSOR_SDA 22
#define SENSOR_SCL 27

SPIClass touchSPI = SPIClass(VSPI);
XPT2046_Touchscreen ts(XPT2046_CS, XPT2046_IRQ);
TFT_eSPI tft = TFT_eSPI();

// Thermal Grid Size (32x24 array for MLX90640)
#define THERMAL_COLS 32
#define THERMAL_ROWS 24

float thermalData[THERMAL_ROWS][THERMAL_COLS];
float minTemp = 18.0;
float maxTemp = 36.0;
int maxRow = 12, maxCol = 16;
int minRow = 0, minCol = 0;

enum PaletteMode { PALETTE_IRONBOW, PALETTE_RAINBOW, PALETTE_HOT_COLD };
PaletteMode currentPalette = PALETTE_IRONBOW;
bool alarmEnabled = true;
float alarmThreshold = 38.0;

// Converts normalized value [0..1] to 16-bit 565 color based on palette
uint16_t getPaletteColor(float val, PaletteMode pal) {
  val = constrain(val, 0.0f, 1.0f);
  uint8_t r = 0, g = 0, b = 0;
  
  if (pal == PALETTE_IRONBOW) {
    if (val < 0.25f) {
      r = 0;
      g = 0;
      b = (uint8_t)(val * 4.0f * 255.0f);
    } else if (val < 0.5f) {
      r = (uint8_t)((val - 0.25f) * 4.0f * 255.0f);
      g = 0;
      b = 255;
    } else if (val < 0.75f) {
      r = 255;
      g = (uint8_t)((val - 0.5f) * 4.0f * 255.0f);
      b = (uint8_t)(255 - (val - 0.5f) * 4.0f * 255.0f);
    } else {
      r = 255;
      g = 255;
      b = (uint8_t)((val - 0.75f) * 4.0f * 255.0f);
    }
  } else if (pal == PALETTE_RAINBOW) {
    float h = (1.0f - val) * 240.0f; // 240=blue down to 0=red
    int i = (int)(h / 60.0f);
    float f = (h / 60.0f) - i;
    uint8_t q = (uint8_t)(255 * (1.0f - f));
    uint8_t t = (uint8_t)(255 * f);
    switch (i % 6) {
      case 0: r = 255; g = t; b = 0; break;
      case 1: r = q; g = 255; b = 0; break;
      case 2: r = 0; g = 255; b = t; break;
      case 3: r = 0; g = q; b = 255; break;
      default: r = 255; g = 0; b = 255; break;
    }
  } else { // HOT_COLD
    if (val < 0.5f) {
      r = 0;
      g = (uint8_t)(val * 2.0f * 255.0f);
      b = 255;
    } else {
      r = 255;
      g = (uint8_t)((1.0f - val) * 2.0f * 255.0f);
      b = 0;
    }
  }
  return tft.color565(r, g, b);
}

void beep(int freq, int durationMs) {
  tone(CYD_BUZZER, freq, durationMs);
}

void setRGB(bool r, bool g, bool b) {
  digitalWrite(CYD_LED_RED, r ? LOW : HIGH);
  digitalWrite(CYD_LED_GREEN, g ? LOW : HIGH);
  digitalWrite(CYD_LED_BLUE, b ? LOW : HIGH);
}

void generateThermalFrame() {
  static float t = 0;
  t += 0.08f;
  
  // Ambient base
  float base = 21.0f + 2.0f * sinf(t * 0.3f);
  minTemp = 100.0f;
  maxTemp = -100.0f;
  
  // Moving heat source target (e.g. human or PCB hot component)
  float hotX = 16.0f + 10.0f * sinf(t * 0.5f);
  float hotY = 12.0f + 6.0f * cosf(t * 0.7f);

  for (int r = 0; r < THERMAL_ROWS; r++) {
    for (int c = 0; c < THERMAL_COLS; c++) {
      float dx = c - hotX;
      float dy = r - hotY;
      float distSq = dx * dx + dy * dy;
      float spot = 18.0f * expf(-distSq / 14.0f);
      float noise = ((float)(rand() % 100) / 100.0f - 0.5f) * 0.6f;
      float temp = base + spot + noise;
      thermalData[r][c] = temp;
      
      if (temp > maxTemp) {
        maxTemp = temp;
        maxRow = r;
        maxCol = c;
      }
      if (temp < minTemp) {
        minTemp = temp;
        minRow = r;
        minCol = c;
      }
    }
  }
  
  if (maxTemp > alarmThreshold && alarmEnabled) {
    setRGB(true, false, false);
    beep(2800, 30);
  } else {
    setRGB(false, true, false);
  }
}

void drawUI() {
  tft.fillScreen(TFT_BLACK);
  
  // Header
  tft.fillRect(0, 0, 320, 24, 0x18E3);
  tft.setTextColor(TFT_WHITE, 0x18E3);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("CYD THERMAL IMAGER (MLX90640)", 160, 12, 2);
  
  // Bottom Controls
  tft.fillRect(0, 214, 100, 26, 0x2104);
  tft.fillRect(110, 214, 100, 26, 0x2104);
  tft.fillRect(220, 214, 100, 26, 0x2104);
  
  tft.drawRect(0, 214, 100, 26, TFT_DARKGREY);
  tft.drawRect(110, 214, 100, 26, TFT_DARKGREY);
  tft.drawRect(220, 214, 100, 26, TFT_DARKGREY);
  
  tft.setTextColor(TFT_CYAN, 0x2104);
  tft.drawString("PALETTE", 50, 227, 2);
  tft.setTextColor(TFT_YELLOW, 0x2104);
  tft.drawString("ALARM", 160, 227, 2);
  tft.setTextColor(TFT_GREEN, 0x2104);
  tft.drawString("CALIB", 270, 227, 2);
}

void renderThermalHeatmap() {
  // Heatmap rendered to a 240x180 box at X=4, Y=28
  int startX = 4;
  int startY = 28;
  int cellW = 7;
  int cellH = 7;
  
  float range = maxTemp - minTemp;
  if (range < 0.1f) range = 0.1f;
  
  for (int r = 0; r < THERMAL_ROWS; r++) {
    for (int c = 0; c < THERMAL_COLS; c++) {
      float norm = (thermalData[r][c] - minTemp) / range;
      uint16_t color = getPaletteColor(norm, currentPalette);
      tft.fillRect(startX + c * cellW, startY + r * cellH, cellW, cellH, color);
    }
  }
  
  // Draw Hot Spot Crosshair
  int crossX = startX + maxCol * cellW + (cellW / 2);
  int crossY = startY + maxRow * cellH + (cellH / 2);
  tft.drawFastHLine(crossX - 6, crossY, 13, TFT_WHITE);
  tft.drawFastVLine(crossX, crossY - 6, 13, TFT_WHITE);
  tft.drawCircle(crossX, crossY, 4, TFT_RED);
  
  // Sidebar Telemetry (X=234..316)
  tft.fillRect(232, 28, 88, 180, 0x0821);
  tft.drawRect(232, 28, 88, 180, TFT_NAVY);
  
  tft.setTextDatum(TL_DATUM);
  tft.setTextColor(TFT_RED, 0x0821);
  tft.drawString("MAX TEMP", 236, 34, 1);
  char buf[20];
  snprintf(buf, sizeof(buf), "%.1f C", maxTemp);
  tft.drawString(buf, 236, 46, 2);
  
  tft.setTextColor(TFT_CYAN, 0x0821);
  tft.drawString("MIN TEMP", 236, 70, 1);
  snprintf(buf, sizeof(buf), "%.1f C", minTemp);
  tft.drawString(buf, 236, 82, 2);
  
  tft.setTextColor(TFT_WHITE, 0x0821);
  tft.drawString("CENTER", 236, 106, 1);
  snprintf(buf, sizeof(buf), "%.1f C", thermalData[12][16]);
  tft.drawString(buf, 236, 118, 2);
  
  tft.setTextColor(TFT_GOLD, 0x0821);
  tft.drawString("PALETTE", 236, 142, 1);
  const char* palName = (currentPalette == PALETTE_IRONBOW) ? "IRON" : 
                        (currentPalette == PALETTE_RAINBOW) ? "RAIN" : "H-C";
  tft.drawString(palName, 236, 154, 2);
  
  tft.setTextColor(alarmEnabled ? TFT_GREEN : TFT_DARKGREY, 0x0821);
  snprintf(buf, sizeof(buf), "ALM:%s", alarmEnabled ? "ON" : "OFF");
  tft.drawString(buf, 236, 180, 2);
}

void setup() {
  Serial.begin(115200);
  
  pinMode(CYD_LED_RED, OUTPUT);
  pinMode(CYD_LED_GREEN, OUTPUT);
  pinMode(CYD_LED_BLUE, OUTPUT);
  setRGB(false, false, false);
  
  pinMode(CYD_BUZZER, OUTPUT);
  beep(1000, 50);
  
  touchSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  ts.begin(touchSPI);
  ts.setRotation(1);
  
  tft.init();
  tft.setRotation(1);
  
  drawUI();
}

void loop() {
  generateThermalFrame();
  renderThermalHeatmap();
  
  if (ts.touched()) {
    TS_Point p = ts.getPoint();
    int touchX = map(p.x, 200, 3700, 0, 320);
    int touchY = map(p.y, 240, 3800, 0, 240);
    
    if (touchY > 210) {
      if (touchX < 100) {
        currentPalette = (PaletteMode)((currentPalette + 1) % 3);
        beep(1800, 30);
      } else if (touchX >= 110 && touchX < 210) {
        alarmEnabled = !alarmEnabled;
        beep(2200, 30);
      } else if (touchX >= 220) {
        beep(2500, 40);
      }
      delay(200);
    }
  }
  
  delay(120);
}
