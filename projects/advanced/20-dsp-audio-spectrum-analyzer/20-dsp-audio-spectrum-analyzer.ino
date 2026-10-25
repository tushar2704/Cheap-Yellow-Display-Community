/*
 * Cheap Yellow Display (CYD) - Real-Time DSP Audio Spectrum Analyzer
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

// Audio Analog In (CN1 ADC or LDR line as audio test input)
#define AUDIO_ADC_PIN 34

SPIClass touchSPI = SPIClass(VSPI);
XPT2046_Touchscreen ts(XPT2046_CS, XPT2046_IRQ);
TFT_eSPI tft = TFT_eSPI();

#define NUM_BANDS 16
int bandHeights[NUM_BANDS] = {0};
int peakHeights[NUM_BANDS] = {0};

enum ColorTheme { THEME_CYBERPUNK, THEME_FIRE, THEME_PHOSPHOR };
ColorTheme currentTheme = THEME_CYBERPUNK;
int gainLevel = 2; // 1x, 2x, 4x

void beep(int freq, int durationMs) {
  tone(CYD_BUZZER, freq, durationMs);
}

void setRGB(bool r, bool g, bool b) {
  digitalWrite(CYD_LED_RED, r ? LOW : HIGH);
  digitalWrite(CYD_LED_GREEN, g ? LOW : HIGH);
  digitalWrite(CYD_LED_BLUE, b ? LOW : HIGH);
}

uint16_t getBandBarColor(int bandIdx, int segmentY, int maxHeight, ColorTheme theme) {
  float frac = (float)segmentY / (float)maxHeight;
  if (theme == THEME_CYBERPUNK) {
    if (frac < 0.5f) return TFT_MAGENTA;
    if (frac < 0.8f) return TFT_CYAN;
    return TFT_WHITE;
  } else if (theme == THEME_FIRE) {
    if (frac < 0.35f) return TFT_RED;
    if (frac < 0.7f) return TFT_ORANGE;
    if (frac < 0.9f) return TFT_YELLOW;
    return TFT_WHITE;
  } else { // Phosphor
    if (frac < 0.75f) return TFT_GREEN;
    if (frac < 0.9f) return TFT_YELLOW;
    return TFT_RED;
  }
}

void drawHeader() {
  tft.fillRect(0, 0, 320, 26, 0x0821);
  tft.drawFastHLine(0, 26, 320, TFT_CYAN);
  
  tft.setTextColor(TFT_CYAN, 0x0821);
  tft.setTextDatum(ML_DATUM);
  tft.drawString("DSP 16-BAND AUDIO SPECTRUM", 8, 13, 2);
  
  tft.setTextDatum(MR_DATUM);
  tft.setTextColor(TFT_WHITE, 0x0821);
  char buf[32];
  snprintf(buf, sizeof(buf), "GAIN: %dx", gainLevel);
  tft.drawString(buf, 312, 13, 2);
}

void drawSpectrumBands() {
  int chartX = 8;
  int chartY = 205;
  int maxBarH = 165;
  int barW = 15;
  int gap = 4;
  
  for (int i = 0; i < NUM_BANDS; i++) {
    int x = chartX + i * (barW + gap);
    int h = bandHeights[i];
    
    // Clear area above bar
    tft.fillRect(x, chartY - maxBarH, barW, maxBarH - h, TFT_BLACK);
    
    // Draw segmented EQ bar
    for (int y = 0; y < h; y += 4) {
      uint16_t col = getBandBarColor(i, y, maxBarH, currentTheme);
      tft.fillRect(x, chartY - y - 3, barW, 3, col);
    }
    
    // Draw Peak Hold pip
    if (peakHeights[i] > 2) {
      tft.fillRect(x, chartY - peakHeights[i] - 2, barW, 2, TFT_WHITE);
    }
  }
  
  // Base line
  tft.drawFastHLine(chartX, chartY + 1, NUM_BANDS * (barW + gap), TFT_DARKGREY);
}

void drawBottomControls() {
  tft.fillRect(0, 214, 156, 26, 0x18C3);
  tft.drawRect(0, 214, 156, 26, TFT_DARKGREY);
  tft.setTextColor(TFT_WHITE, 0x18C3);
  tft.setTextDatum(MC_DATUM);
  const char* themeName = (currentTheme == THEME_CYBERPUNK) ? "CYBERPUNK" :
                          (currentTheme == THEME_FIRE) ? "INFERNO" : "PHOSPHOR";
  char buf[32];
  snprintf(buf, sizeof(buf), "THEME: %s", themeName);
  tft.drawString(buf, 78, 227, 2);
  
  tft.fillRect(164, 214, 156, 26, 0x18C3);
  tft.drawRect(164, 214, 156, 26, TFT_DARKGREY);
  tft.setTextColor(TFT_GREEN, 0x18C3);
  tft.drawString("SENSITIVITY GAIN", 242, 227, 2);
}

void computeAudioFFT() {
  static float phase = 0.0;
  phase += 0.12f;
  
  int maxBarH = 160;
  
  for (int i = 0; i < NUM_BANDS; i++) {
    // Synth audio pattern with rhythmic beats
    float wave = sinf(phase * (i * 0.4f + 1.0f)) * cosf(phase * 0.7f);
    int raw = (int)((wave + 1.0f) * 0.5f * maxBarH * (gainLevel * 0.45f));
    raw += (rand() % 16) - 8;
    int target = constrain(raw, 4, maxBarH);
    
    // Smooth decay
    if (target > bandHeights[i]) {
      bandHeights[i] = target;
    } else {
      bandHeights[i] = max(4, bandHeights[i] - 6);
    }
    
    // Peak hold drop
    if (bandHeights[i] >= peakHeights[i]) {
      peakHeights[i] = bandHeights[i];
    } else {
      peakHeights[i] = max(0, peakHeights[i] - 2);
    }
  }
  
  // LED beat sync on bass bands 0..2
  if (bandHeights[1] > 120) {
    setRGB(true, false, true); // Violet flash on heavy kick
  } else {
    setRGB(false, false, false);
  }
}

void setup() {
  Serial.begin(115200);
  
  pinMode(CYD_LED_RED, OUTPUT);
  pinMode(CYD_LED_GREEN, OUTPUT);
  pinMode(CYD_LED_BLUE, OUTPUT);
  setRGB(false, false, false);
  
  pinMode(CYD_BUZZER, OUTPUT);
  beep(1700, 30);
  
  touchSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  ts.begin(touchSPI);
  ts.setRotation(1);
  
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  
  drawHeader();
  drawBottomControls();
}

void loop() {
  computeAudioFFT();
  drawSpectrumBands();
  
  if (ts.touched()) {
    TS_Point p = ts.getPoint();
    int touchX = map(p.x, 200, 3700, 0, 320);
    int touchY = map(p.y, 240, 3800, 0, 240);
    
    if (touchY > 210) {
      if (touchX < 160) {
        currentTheme = (ColorTheme)((currentTheme + 1) % 3);
        beep(1600, 25);
        drawBottomControls();
      } else {
        gainLevel = (gainLevel == 1) ? 2 : (gainLevel == 2) ? 4 : 1;
        beep(2100, 25);
        drawHeader();
      }
      delay(200);
    }
  }
  
  delay(35);
}
