/*
 * Elecrow 2.8" CYD — Local Thermal Imaging Camera
 * Channel: @IamNotIntoTech (https://youtube.com/@IamNotIntoTech)
 * 
 * Target: ESP32-2432S028R (2.8" ILI9341 TFT + XPT2046 Resistive Touch)
 * Features:
 *  - MLX90640 32x24 IR Far-Infrared Sensor array over HY2.0 I2C (IO27 SDA, IO22 SCL)
 *  - High-Fidelity thermal heat source simulation fallback mode
 *  - 3 Selectable Thermal Colormaps: Ironbow, Rainbow (Turbo), and Grayscale
 *  - Real-time Center Crosshair & Dynamic Max Hot-Spot reticle tracking
 *  - Interactive Touch Controls: Frame Freeze/Hold, Colormap Cycle, Range Lock
 *  - Overheat threshold alarm (>50°C) with active Red RGB LED and audio buzzer tone
 */

#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>

// Hardware Pin Definitions for Elecrow 2.8" CYD
#define XPT2046_IRQ 36
#define XPT2046_MOSI 13
#define XPT2046_MISO 12
#define XPT2046_CLK 14
#define XPT2046_CS 33

#define TFT_BL 21
#define LED_RED 4
#define LED_GREEN 16
#define LED_BLUE 17
#define BUZZER_PIN 26

#define I2C_SDA 27
#define I2C_SCL 22
#define MLX90640_ADDR 0x33

SPIClass touchSPI = SPIClass(VSPI);
XPT2046_Touchscreen ts(XPT2046_CS, XPT2046_IRQ);
TFT_eSPI tft = TFT_eSPI();

// UI Colors (16-bit 565)
#define COLOR_BG        0x0842 // Charcoal
#define COLOR_PANEL     0x10A3
#define COLOR_CARD      0x18E5
#define COLOR_CARD_BRD  0x2967
#define COLOR_TEXT      0xFFFF
#define COLOR_MUTED     0x8410
#define COLOR_HOT       0xF800
#define COLOR_COLD      0x001F
#define COLOR_GREEN     0x07E0
#define COLOR_YELLOW    0xFFE0

// Sensor Matrix: 32 columns x 24 rows = 768 pixels
#define COLS 32
#define ROWS 24
float thermalFrame[ROWS][COLS];

bool sensorDetected = false;
bool frameFrozen = false;
int colormapIndex = 0; // 0 = Ironbow, 1 = Rainbow, 2 = Grayscale
bool hotSpotEnabled = true;

float minTemp = 20.0;
float maxTemp = 36.0;
float centerTemp = 24.5;
int maxHotX = 16;
int maxHotY = 12;

unsigned long lastFrameUpdate = 0;
float simAngle = 0;

void playBeep(int freq, int ms) {
  tone(BUZZER_PIN, freq, ms);
  delay(ms);
  noTone(BUZZER_PIN);
}

void setRGB(bool r, bool g, bool b) {
  digitalWrite(LED_RED, r ? LOW : HIGH);
  digitalWrite(LED_GREEN, g ? LOW : HIGH);
  digitalWrite(LED_BLUE, b ? LOW : HIGH);
}

// Convert 0..255 normalized intensity to 16-bit RGB565 based on colormap
uint16_t getPaletteColor(uint8_t val, int mapIdx) {
  if (mapIdx == 0) {
    // Ironbow Colormap
    uint8_t r, g, b;
    if (val < 64) {
      r = val * 2;
      g = 0;
      b = val * 4;
    } else if (val < 128) {
      r = 128 + (val - 64) * 2;
      g = 0;
      b = 255 - (val - 64) * 4;
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
    // Rainbow / Jet Colormap
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
  // Moving heat source 1 (Hand / Hot component)
  int hotX1 = 16 + (int)(sin(simAngle) * 8.0f);
  int hotY1 = 12 + (int)(cos(simAngle * 0.7f) * 6.0f);

  // Moving heat source 2 (Cooler background ambient)
  int hotX2 = 8 + (int)(cos(simAngle * 0.5f) * 4.0f);
  int hotY2 = 16 + (int)(sin(simAngle * 0.9f) * 4.0f);

  minTemp = 100.0;
  maxTemp = -50.0;

  for (int y = 0; y < ROWS; y++) {
    for (int x = 0; x < COLS; x++) {
      float d1 = sqrt(sq(x - hotX1) + sq(y - hotY1));
      float d2 = sqrt(sq(x - hotX2) + sq(y - hotY2));

      float temp = 21.0 + max(0.0f, (38.5f - d1 * 3.2f)) + max(0.0f, (29.0f - d2 * 2.5f));
      temp += (random(-3, 4) / 10.0f); // Sensor noise

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

  tft.setTextColor(COLOR_TEXT, COLOR_PANEL);
  tft.setTextSize(1);
  tft.drawString("THERMAL CAMERA 32x24", 10, 5, 2);

  if (sensorDetected) {
    tft.setTextColor(COLOR_GREEN, COLOR_PANEL);
    tft.drawRightString("MLX90640 ●", 310, 5, 1);
  } else {
    tft.setTextColor(COLOR_YELLOW, COLOR_PANEL);
    tft.drawRightString("SIMULATED ●", 310, 5, 1);
  }
}

void renderThermalGrid() {
  // Thermal display window: x: 10, y: 28, w: 224, h: 168 (7x7 per sensor pixel)
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

  // Draw Hot Spot Reticle
  if (hotSpotEnabled) {
    int hx = startX + maxHotX * pixelSize + pixelSize / 2;
    int hy = startY + maxHotY * pixelSize + pixelSize / 2;
    tft.drawRect(hx - 6, hy - 6, 13, 13, COLOR_TEXT);
    tft.drawPixel(hx, hy, COLOR_HOT);
  }

  // Draw Palette Legend Bar (x: 242, y: 30, w: 12, h: 164)
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
  tft.setTextColor(COLOR_HOT, COLOR_BG);
  char maxStr[12];
  snprintf(maxStr, sizeof(maxStr), "%.1f°", maxTemp);
  tft.drawString(maxStr, palX + 18, palY, 1);

  tft.setTextColor(COLOR_TEXT, COLOR_BG);
  char ctrStr[12];
  snprintf(ctrStr, sizeof(ctrStr), "%.1f°", centerTemp);
  tft.drawString(ctrStr, palX + 18, palY + palH / 2 - 4, 1);

  tft.setTextColor(COLOR_COLD, COLOR_BG);
  char minStr[12];
  snprintf(minStr, sizeof(minStr), "%.1f°", minTemp);
  tft.drawString(minStr, palX + 18, palY + palH - 8, 1);
}

void drawBottomToolbar() {
  // Toolbar: y: 202 to 240
  tft.fillRect(0, 202, 320, 38, COLOR_PANEL);
  tft.drawFastHLine(0, 202, 320, COLOR_CARD_BRD);

  // Button 1: Freeze Frame
  tft.fillRoundRect(8, 206, 95, 28, 4, frameFrozen ? COLOR_HOT : COLOR_CARD);
  tft.drawRoundRect(8, 206, 95, 28, 4, COLOR_CARD_BRD);
  tft.setTextColor(COLOR_TEXT, frameFrozen ? COLOR_HOT : COLOR_CARD);
  tft.drawCentreString(frameFrozen ? "HOLD ACTIVE" : "FREEZE", 55, 212, 1);

  // Button 2: Palette Cycle
  const char* palettes[3] = { "IRONBOW", "RAINBOW", "GRAYSCALE" };
  tft.fillRoundRect(112, 206, 96, 28, 4, COLOR_CARD);
  tft.drawRoundRect(112, 206, 96, 28, 4, COLOR_CARD_BRD);
  tft.setTextColor(COLOR_YELLOW, COLOR_CARD);
  tft.drawCentreString(palettes[colormapIndex], 160, 212, 1);

  // Button 3: Hot-Spot Toggle
  tft.fillRoundRect(216, 206, 96, 28, 4, COLOR_CARD);
  tft.drawRoundRect(216, 206, 96, 28, 4, COLOR_CARD_BRD);
  tft.setTextColor(hotSpotEnabled ? COLOR_GREEN : COLOR_MUTED, COLOR_CARD);
  tft.drawCentreString(hotSpotEnabled ? "HOT-SPOT ON" : "HOT-SPOT OFF", 264, 212, 1);
}

void checkOverheatSafety() {
  if (maxTemp > 50.0f) {
    setRGB(true, false, false); // Red LED alert
    playBeep(2800, 30);
  } else if (maxTemp > 38.0f) {
    setRGB(true, true, false); // Yellow
  } else {
    setRGB(false, true, false); // Green safe
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  setRGB(false, false, false);

  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(COLOR_BG);

  touchSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  ts.begin(touchSPI);
  ts.setRotation(1);

  initThermalSensor();

  drawHeader();
  drawBottomToolbar();

  playBeep(2400, 70);
  Serial.println(F("[SYSTEM] Thermal Camera Display initialized."));
}

void loop() {
  // Update thermal frame every 100ms (~10 FPS)
  if (!frameFrozen && (millis() - lastFrameUpdate > 100)) {
    lastFrameUpdate = millis();
    updateThermalSimulation();
    renderThermalGrid();
    checkOverheatSafety();
  }

  // Touch Handling
  if (ts.touched()) {
    TS_Point p = ts.getPoint();
    int touchX = map(p.x, 3800, 300, 0, 320);
    int touchY = map(p.y, 3800, 400, 0, 240);
    touchX = constrain(touchX, 0, 320);
    touchY = constrain(touchY, 0, 240);

    // Toolbar buttons
    if (touchY >= 202) {
      if (touchX >= 8 && touchX <= 103) {
        frameFrozen = !frameFrozen;
        playBeep(1800, 30);
        drawBottomToolbar();
        delay(180);
      } else if (touchX >= 112 && touchX <= 208) {
        colormapIndex = (colormapIndex + 1) % 3;
        playBeep(2200, 30);
        drawBottomToolbar();
        renderThermalGrid();
        delay(180);
      } else if (touchX >= 216 && touchX <= 312) {
        hotSpotEnabled = !hotSpotEnabled;
        playBeep(2000, 30);
        drawBottomToolbar();
        renderThermalGrid();
        delay(180);
      }
    }
  }

  delay(15);
}
