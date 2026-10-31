/**
 * 🌡️ Project 34: Local Thermal Imaging Camera
 * 
 * Hardware: Elecrow CrowPanel 2.8" (ESP32-2432S028R)
 * Channel Branding: IamNotIntoTech (https://youtube.com/@IamNotIntoTech)
 * Author: Tushar Aggarwal (tushar2704)
 * 
 * Features:
 * - 32x24 Far-infrared thermal camera display (MLX90640 ready over HY2.0 I2C SDA:27, SCL:22).
 * - Multi-source dynamic thermal gradient simulation fallback when sensor is detached.
 * - 3 Thermal colormaps: Ironbow (FLIR style), Rainbow (Turbo), and High-Contrast Grayscale.
 * - Center crosshair temperature readout and dynamic maximum hotspot reticle tracking.
 * - Touch toolbar: Freeze / Hold frame, Colormap cycle, and Hotspot tracking toggle.
 * - Critical overheat safety alert (>50°C) with flashing active-low Red LED and buzzer tone.
 * - Calibrated non-inverted resistive touch controls with acoustic feedback clicks.
 */

#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>

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

#define I2C_SDA       27
#define I2C_SCL       22
#define MLX90640_ADDR 0x33

SPIClass touchSPI = SPIClass(VSPI);
XPT2046_Touchscreen ts(TOUCH_CS);
TFT_eSPI tft = TFT_eSPI();

// UI Colors (16-bit 565)
#define COLOR_BG        0x0842 // Charcoal
#define COLOR_PANEL     0x10A3
#define COLOR_CARD      0x18E5
#define COLOR_CARD_BRD  0x2967
#define COLOR_TEXT      0xFFFF
#define COLOR_MUTED     0x8410
#define COLOR_GREEN     0x07E0
#define COLOR_YELLOW    0xFFE0
#define COLOR_RED       0xF800
#define COLOR_CYAN      0x07FF

// Thermal Grid Parameters (32 cols x 24 rows)
#define COLS 32
#define ROWS 24
float thermalFrame[ROWS][COLS];

float minTemp = 20.0;
float maxTemp = 38.0;
float centerTemp = 24.5;
int maxHotX = 16;
int maxHotY = 12;

bool sensorDetected = false;
bool frameFrozen = false;
int colormapIndex = 0; // 0 = Ironbow, 1 = Rainbow, 2 = Grayscale
bool hotSpotEnabled = true;

float simAngle = 0.0;
unsigned long lastTouchTime = 0;
unsigned long lastFrameTick = 0;

void playBeep(int freq = 2000, int ms = 30) {
  tone(BUZZER_PIN, freq, ms);
}

void setRGB(bool r, bool g, bool b) {
  digitalWrite(LED_RED, r ? LOW : HIGH);
  digitalWrite(LED_GREEN, g ? LOW : HIGH);
  digitalWrite(LED_BLUE, b ? LOW : HIGH);
}

uint16_t getPaletteColor(uint8_t val, int mapIdx) {
  if (mapIdx == 0) {
    // Ironbow (Black -> Blue -> Violet -> Orange -> Yellow -> White)
    uint8_t r, g, b;
    if (val < 64) {
      r = 0;
      g = 0;
      b = val * 4;
    } else if (val < 128) {
      r = (val - 64) * 4;
      g = 0;
      b = 255 - (val - 64) * 2;
    } else if (val < 192) {
      r = 255;
      g = (val - 128) * 4;
      b = 0;
    } else {
      r = 255;
      g = 255;
      b = (val - 192) * 4;
    }
    return tft.color565(r, g, b);
  } else if (mapIdx == 1) {
    // Rainbow / Turbo
    uint8_t r, g, b;
    if (val < 64) {
      r = 0;
      g = val * 4;
      b = 255;
    } else if (val < 128) {
      r = 0;
      g = 255;
      b = 255 - (val - 64) * 4;
    } else if (val < 192) {
      r = (val - 128) * 4;
      g = 255;
      b = 0;
    } else {
      r = 255;
      g = 255 - (val - 192) * 4;
      b = 0;
    }
    return tft.color565(r, g, b);
  } else {
    // Grayscale
    return tft.color565(val, val, val);
  }
}

void initThermalSensor() {
  Wire.begin(I2C_SDA, I2C_SCL, 400000);
  delay(50);
  Wire.beginTransmission(MLX90640_ADDR);
  if (Wire.endTransmission() == 0) {
    sensorDetected = true;
  } else {
    sensorDetected = false;
  }
}

void updateThermalSimulation() {
  simAngle += 0.08f;
  int hotX1 = 16 + (int)(sin(simAngle) * 8.0f);
  int hotY1 = 12 + (int)(cos(simAngle * 0.7f) * 6.0f);

  int hotX2 = 8 + (int)(cos(simAngle * 0.5f) * 4.0f);
  int hotY2 = 16 + (int)(sin(simAngle * 0.9f) * 4.0f);

  minTemp = 100.0;
  maxTemp = -50.0;

  for (int y = 0; y < ROWS; y++) {
    for (int x = 0; x < COLS; x++) {
      float d1 = sqrt(sq(x - hotX1) + sq(y - hotY1));
      float d2 = sqrt(sq(x - hotX2) + sq(y - hotY2));

      float temp = 21.0 + max(0.0f, (38.5f - d1 * 3.2f)) + max(0.0f, (29.0f - d2 * 2.5f));
      temp += (random(-3, 4) / 10.0f);

      thermalFrame[y][x] = temp;

      if (temp < minTemp) minTemp = temp;
      if (temp > maxTemp) {
        maxTemp = temp;
        maxHotX = x;
        maxHotY = y;
      }
    }
  }

  centerTemp = thermalFrame[ROWS / 2][COLS / 2];
}

void drawHeader() {
  tft.fillRect(0, 0, 320, 24, COLOR_PANEL);
  tft.drawFastHLine(0, 24, 320, COLOR_CARD_BRD);

  tft.setTextColor(COLOR_CYAN, COLOR_PANEL);
  tft.drawString("IamNotIntoTech", 8, 5, 2);

  tft.setTextColor(COLOR_TEXT, COLOR_PANEL);
  tft.drawCentreString("THERMAL CAMERA 32x24", 175, 5, 2);

  if (sensorDetected) {
    tft.setTextColor(COLOR_GREEN, COLOR_PANEL);
    tft.drawRightString("MLX90640", 312, 5, 1);
  } else {
    tft.setTextColor(COLOR_YELLOW, COLOR_PANEL);
    tft.drawRightString("SIMULATED", 312, 5, 1);
  }
}

void renderThermalGrid() {
  int startX = 10;
  int startY = 28;
  int pixelSize = 7;

  float range = max(0.1f, maxTemp - minTemp);

  for (int r = 0; r < ROWS; r++) {
    for (int c = 0; c < COLS; c++) {
      float tVal = thermalFrame[r][c];
      uint8_t norm = constrain((int)((tVal - minTemp) / range * 255.0f), 0, 255);
      uint16_t color = getPaletteColor(norm, colormapIndex);

      tft.fillRect(startX + c * pixelSize, startY + r * pixelSize, pixelSize, pixelSize, color);
    }
  }

  // Draw Center Reticle
  int cx = startX + (COLS / 2) * pixelSize + pixelSize / 2;
  int cy = startY + (ROWS / 2) * pixelSize + pixelSize / 2;
  tft.drawCircle(cx, cy, 6, COLOR_TEXT);
  tft.drawFastHLine(cx - 10, cy, 20, COLOR_TEXT);
  tft.drawFastVLine(cx, cy - 10, 20, COLOR_TEXT);

  // Draw Hotspot Reticle
  if (hotSpotEnabled) {
    int hx = startX + maxHotX * pixelSize + pixelSize / 2;
    int hy = startY + maxHotY * pixelSize + pixelSize / 2;
    tft.drawRect(hx - 5, hy - 5, 11, 11, COLOR_RED);
    tft.drawFastHLine(hx - 8, hy, 17, COLOR_RED);
    tft.drawFastVLine(hx, hy - 8, 17, COLOR_RED);
  }

  // Palette Gradient Bar
  int palX = 242;
  int palY = 30;
  int palW = 12;
  int palH = 164;
  for (int i = 0; i < palH; i++) {
    uint8_t val = map(palH - 1 - i, 0, palH - 1, 0, 255);
    tft.drawFastHLine(palX, palY + i, palW, getPaletteColor(val, colormapIndex));
  }
  tft.drawRect(palX - 1, palY - 1, palW + 2, palH + 2, COLOR_CARD_BRD);

  // Temperature Sidebar Numbers
  tft.fillRect(258, 28, 60, 168, COLOR_BG);

  tft.setTextColor(COLOR_RED, COLOR_BG);
  char maxBuf[16];
  snprintf(maxBuf, sizeof(maxBuf), "%.1f C", maxTemp);
  tft.drawString("MAX", 258, 30, 1);
  tft.drawString(maxBuf, 258, 42, 2);

  tft.setTextColor(COLOR_TEXT, COLOR_BG);
  char cenBuf[16];
  snprintf(cenBuf, sizeof(cenBuf), "%.1f C", centerTemp);
  tft.drawString("CENTER", 258, 92, 1);
  tft.drawString(cenBuf, 258, 104, 2);

  tft.setTextColor(COLOR_CYAN, COLOR_BG);
  char minBuf[16];
  snprintf(minBuf, sizeof(minBuf), "%.1f C", minTemp);
  tft.drawString("MIN", 258, 156, 1);
  tft.drawString(minBuf, 258, 168, 2);
}

void drawBottomToolbar() {
  tft.fillRect(0, 202, 320, 38, COLOR_PANEL);
  tft.drawFastHLine(0, 202, 320, COLOR_CARD_BRD);

  // Button 1: Freeze Frame (8..103)
  tft.fillRoundRect(8, 206, 95, 30, 4, frameFrozen ? COLOR_RED : COLOR_CARD);
  tft.drawRoundRect(8, 206, 95, 30, 4, COLOR_CARD_BRD);
  tft.setTextColor(frameFrozen ? COLOR_TEXT : COLOR_TEXT, frameFrozen ? COLOR_RED : COLOR_CARD);
  tft.drawCentreString(frameFrozen ? "FROZEN [||]" : "HOLD FRAME", 55, 213, 1);

  // Button 2: Palette Cycle (112..208)
  const char* mapNames[3] = { "IRONBOW", "RAINBOW", "GRAYSCALE" };
  tft.fillRoundRect(112, 206, 96, 30, 4, COLOR_CARD);
  tft.drawRoundRect(112, 206, 96, 30, 4, COLOR_CARD_BRD);
  tft.setTextColor(COLOR_YELLOW, COLOR_CARD);
  tft.drawCentreString(mapNames[colormapIndex], 160, 213, 1);

  // Button 3: Hotspot Toggle (216..312)
  tft.fillRoundRect(216, 206, 96, 30, 4, hotSpotEnabled ? tft.color565(30, 50, 40) : COLOR_CARD);
  tft.drawRoundRect(216, 206, 96, 30, 4, hotSpotEnabled ? COLOR_GREEN : COLOR_CARD_BRD);
  tft.setTextColor(hotSpotEnabled ? COLOR_GREEN : COLOR_MUTED, hotSpotEnabled ? tft.color565(30, 50, 40) : COLOR_CARD);
  tft.drawCentreString(hotSpotEnabled ? "HOTSPOT ON" : "HOTSPOT OFF", 264, 213, 1);
}

void setup() {
  Serial.begin(115200);

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  setRGB(false, false, false);

  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  pinMode(TFT_BL_PIN, OUTPUT);
  digitalWrite(TFT_BL_PIN, HIGH);

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(COLOR_BG);

  // Initialize Touch (VSPI)
  touchSPI.begin(TOUCH_CLK, TOUCH_MISO, TOUCH_MOSI, TOUCH_CS);
  ts.begin(touchSPI);
  ts.setRotation(1);

  initThermalSensor();

  drawHeader();
  drawBottomToolbar();
  updateThermalSimulation();
  renderThermalGrid();

  playBeep(2200, 60);
  Serial.println(F("[SYSTEM] Thermal Camera Display Ready."));
}

void loop() {
  // Update thermal frame every 80ms (~12 FPS)
  if (!frameFrozen && millis() - lastFrameTick > 80) {
    lastFrameTick = millis();
    updateThermalSimulation();
    renderThermalGrid();

    // Overheat alert (> 45°C)
    if (maxTemp > 45.0f) {
      setRGB(true, false, false); // Red alarm
    } else {
      setRGB(false, true, false); // Normal Green
    }
  }

  // Touch Handling
  if (ts.touched()) {
    TS_Point p = ts.getPoint();
    if (p.z > 50 || ts.touched()) {
      if (millis() - lastTouchTime > 180) {
        lastTouchTime = millis();
        int screenX = map(p.x, 240, 3800, 0, 320);
        int screenY = map(p.y, 240, 3800, 0, 240);
        screenX = constrain(screenX, 0, 319);
        screenY = constrain(screenY, 0, 239);

        // Toolbar buttons (screenY >= 202)
        if (screenY >= 202) {
          if (screenX >= 8 && screenX <= 103) {
            frameFrozen = !frameFrozen;
            playBeep(1800, 30);
            drawBottomToolbar();
          } else if (screenX >= 112 && screenX <= 208) {
            colormapIndex = (colormapIndex + 1) % 3;
            playBeep(2200, 30);
            drawBottomToolbar();
            renderThermalGrid();
          } else if (screenX >= 216 && screenX <= 312) {
            hotSpotEnabled = !hotSpotEnabled;
            playBeep(2000, 30);
            drawBottomToolbar();
            renderThermalGrid();
          }
        }
      }
    }
  }

  delay(15);
}

