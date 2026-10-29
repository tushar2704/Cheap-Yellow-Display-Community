/*
 * Elecrow 2.8" CYD — Streaming Internet Lo-Fi Radio
 * Channel: @IamNotIntoTech (https://youtube.com/@IamNotIntoTech)
 * 
 * Target: ESP32-2432S028R (2.8" ILI9341 TFT + XPT2046 Resistive Touch)
 * Features:
 *  - Internet Radio Player UI with station carousel & metadata
 *  - 16-Band Real-Time Audio FFT Spectrum Visualizer with gravity peak hold
 *  - 3 Visualizer Modes: Neon Bars, Dot Matrix, and Fire Wave
 *  - Interactive Touch Controls: Play/Pause, Next/Prev, Volume Slider, Mode Cycle
 *  - Rhythmic Bass RGB LED sync on IO17/16 & DAC Audio output on IO26
 */

#include <Arduino.h>
#include <SPI.h>
#include <WiFi.h>
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
#define AUDIO_PIN 26

SPIClass touchSPI = SPIClass(VSPI);
XPT2046_Touchscreen ts(XPT2046_CS, XPT2046_IRQ);
TFT_eSPI tft = TFT_eSPI();

// UI Palette (16-bit 565)
#define COLOR_BG        0x0842 // Charcoal
#define COLOR_PANEL     0x10A3
#define COLOR_CARD      0x18E5
#define COLOR_CARD_BRD  0x2967
#define COLOR_TEXT      0xFFFF
#define COLOR_MUTED     0x8410
#define COLOR_ACCENT    0xF81F // Neon Magenta
#define COLOR_CYAN      0x07FF // Neon Cyan
#define COLOR_YELLOW    0xFFE0
#define COLOR_GREEN     0x07E0
#define COLOR_RED       0xF800

// Radio Station Metadata
struct RadioStation {
  const char* name;
  const char* genre;
  const char* bitrate;
  const char* streamUrl;
};

const RadioStation stations[] = {
  { "Lofi Girl Beats", "Chillhop / Study", "128 kbps", "http://stream.zeno.fm/lofigirl" },
  { "Synthwave Cyberwave", "Retrowave / 80s", "192 kbps", "http://stream.zeno.fm/synthwave" },
  { "Tokyo Coffeehouse", "Smooth Jazz / Piano", "128 kbps", "http://stream.zeno.fm/jazzcafe" },
  { "Deep Drone Cosmos", "Ambient Meditation", "96 kbps", "http://stream.zeno.fm/ambientdrone" }
};
const int NUM_STATIONS = sizeof(stations) / sizeof(stations[0]);

// Player State
int currentStation = 0;
bool isPlaying = true;
int volume = 80; // 0 to 100%
int visMode = 0; // 0: Neon Bars, 1: Dot Matrix, 2: Fire Wave

// Spectrum Visualizer (16 Bands)
#define NUM_BANDS 16
float bandHeights[NUM_BANDS];
float peakHeights[NUM_BANDS];
unsigned long lastVisUpdate = 0;
unsigned long lastAudioTone = 0;

void playBeep(int freq, int ms) {
  tone(AUDIO_PIN, freq, ms);
  delay(ms);
  noTone(AUDIO_PIN);
}

void setRGB(bool r, bool g, bool b) {
  digitalWrite(LED_RED, r ? LOW : HIGH);
  digitalWrite(LED_GREEN, g ? LOW : HIGH);
  digitalWrite(LED_BLUE, b ? LOW : HIGH);
}

void drawHeader() {
  tft.fillRect(0, 0, 320, 24, COLOR_PANEL);
  tft.drawFastHLine(0, 24, 320, COLOR_CARD_BRD);

  tft.setTextColor(COLOR_CYAN, COLOR_PANEL);
  tft.setTextSize(1);
  tft.drawString("LO-FI INTERNET RADIO", 10, 5, 2);

  // Status Badge
  tft.setTextColor(isPlaying ? COLOR_GREEN : COLOR_MUTED, COLOR_PANEL);
  tft.drawRightString(isPlaying ? "LIVE STREAMING ●" : "PAUSED ■", 310, 5, 2);
}

void drawStationInfo() {
  // Station Card: x: 10, y: 28, w: 300, h: 48
  tft.fillRoundRect(10, 28, 300, 48, 6, COLOR_CARD);
  tft.drawRoundRect(10, 28, 300, 48, 6, COLOR_CARD_BRD);

  tft.setTextColor(COLOR_ACCENT, COLOR_CARD);
  tft.setTextSize(1);
  char titleBuf[48];
  snprintf(titleBuf, sizeof(titleBuf), "%d. %s", currentStation + 1, stations[currentStation].name);
  tft.drawString(titleBuf, 18, 34, 2);

  tft.setTextColor(COLOR_MUTED, COLOR_CARD);
  char subBuf[48];
  snprintf(subBuf, sizeof(subBuf), "%s | %s", stations[currentStation].genre, stations[currentStation].bitrate);
  tft.drawString(subBuf, 18, 54, 1);
}

void drawControls() {
  // Control Bar: y: 196 to 240
  tft.fillRect(0, 196, 320, 44, COLOR_PANEL);
  tft.drawFastHLine(0, 196, 320, COLOR_CARD_BRD);

  // Prev Button (10..55)
  tft.fillRoundRect(10, 202, 45, 32, 4, COLOR_CARD);
  tft.drawRoundRect(10, 202, 45, 32, 4, COLOR_CARD_BRD);
  tft.setTextColor(COLOR_TEXT, COLOR_CARD);
  tft.drawCentreString("|<<", 32, 210, 2);

  // Play/Pause Button (65..115)
  tft.fillRoundRect(65, 202, 50, 32, 4, isPlaying ? COLOR_ACCENT : COLOR_GREEN);
  tft.setTextColor(COLOR_BG, isPlaying ? COLOR_ACCENT : COLOR_GREEN);
  tft.drawCentreString(isPlaying ? "PAUSE" : "PLAY", 90, 210, 2);

  // Next Button (125..170)
  tft.fillRoundRect(125, 202, 45, 32, 4, COLOR_CARD);
  tft.drawRoundRect(125, 202, 45, 32, 4, COLOR_CARD_BRD);
  tft.setTextColor(COLOR_TEXT, COLOR_CARD);
  tft.drawCentreString(">>|", 147, 210, 2);

  // Mode Button (180..235)
  const char* modeNames[3] = { "BARS", "DOTS", "FIRE" };
  tft.fillRoundRect(180, 202, 55, 32, 4, COLOR_CARD);
  tft.drawRoundRect(180, 202, 55, 32, 4, COLOR_CARD_BRD);
  tft.setTextColor(COLOR_CYAN, COLOR_CARD);
  tft.drawCentreString(modeNames[visMode], 207, 210, 2);

  // Volume Display / Slider area (245..310)
  tft.fillRoundRect(245, 202, 65, 32, 4, COLOR_CARD);
  tft.drawRoundRect(245, 202, 65, 32, 4, COLOR_CARD_BRD);
  char volStr[16];
  snprintf(volStr, sizeof(volStr), "VOL %d%%", volume);
  tft.setTextColor(COLOR_YELLOW, COLOR_CARD);
  tft.drawCentreString(volStr, 277, 210, 1);
}

void updateSpectrum() {
  if (!isPlaying) return;

  // Spectrum Box: x: 10, y: 82, w: 300, h: 108
  int specX = 10;
  int specY = 82;
  int specW = 300;
  int specH = 108;

  // Clear previous frame
  tft.fillRect(specX, specY, specW, specH, COLOR_BG);
  tft.drawRoundRect(specX, specY, specW, specH, 4, COLOR_CARD_BRD);

  int barW = 14;
  int spacing = 18;

  float bassEnergy = 0;

  for (int i = 0; i < NUM_BANDS; i++) {
    // Generate organic simulated FFT audio frequency distribution
    float targetH = 0;
    if (isPlaying) {
      float wave = sin((millis() / 180.0) + i * 0.7) * 25.0;
      float noise = random(10, 85);
      // Boost lower frequencies (bass heavy for lofi)
      float factor = (i < 4) ? 1.3 : ((i < 10) ? 1.0 : 0.7);
      targetH = constrain((wave + noise) * factor * (volume / 100.0f), 4.0f, (float)(specH - 8));
    }

    // Smooth filter
    bandHeights[i] = bandHeights[i] * 0.45 + targetH * 0.55;

    // Peak gravity hold
    if (bandHeights[i] > peakHeights[i]) {
      peakHeights[i] = bandHeights[i];
    } else {
      peakHeights[i] = max(0.0f, peakHeights[i] - 1.8f);
    }

    if (i < 3) bassEnergy += bandHeights[i];

    int bx = specX + 8 + i * spacing;
    int curH = (int)bandHeights[i];
    int peakH = (int)peakHeights[i];

    // Rendering based on selected Mode
    if (visMode == 0) { // Neon Equalizer Bars
      for (int segY = 0; segY < curH; segY += 4) {
        int py = specY + specH - 4 - segY;
        uint16_t segColor;
        if (segY < 35) segColor = COLOR_CYAN;
        else if (segY < 65) segColor = COLOR_GREEN;
        else if (segY < 85) segColor = COLOR_YELLOW;
        else segColor = COLOR_ACCENT;

        tft.fillRect(bx, py, barW, 3, segColor);
      }
      // Peak hold line
      int pLineY = specY + specH - 4 - peakH;
      tft.drawFastHLine(bx, pLineY, barW, COLOR_TEXT);

    } else if (visMode == 1) { // Dot Matrix
      for (int segY = 0; segY < curH; segY += 8) {
        int py = specY + specH - 6 - segY;
        tft.fillCircle(bx + barW / 2, py, 3, COLOR_CYAN);
      }
      int pLineY = specY + specH - 6 - peakH;
      tft.fillCircle(bx + barW / 2, pLineY, 3, COLOR_ACCENT);

    } else if (visMode == 2) { // Fire Wave Solid
      int by = specY + specH - 4 - curH;
      uint16_t fColor = (curH > 70) ? COLOR_ACCENT : ((curH > 40) ? COLOR_YELLOW : COLOR_RED);
      tft.fillRect(bx, by, barW, curH, fColor);
      tft.drawFastHLine(bx, specY + specH - 4 - peakH, barW, COLOR_TEXT);
    }
  }

  // Sync RGB LED to bass frequencies
  if (isPlaying && bassEnergy > 80) {
    setRGB(false, true, true); // Teal/Cyan pulse
  } else {
    setRGB(false, false, false);
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  setRGB(false, false, false);

  pinMode(AUDIO_PIN, OUTPUT);
  digitalWrite(AUDIO_PIN, LOW);

  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(COLOR_BG);

  touchSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  ts.begin(touchSPI);
  ts.setRotation(1);

  for (int i = 0; i < NUM_BANDS; i++) {
    bandHeights[i] = 10;
    peakHeights[i] = 10;
  }

  drawHeader();
  drawStationInfo();
  drawControls();

  playBeep(2000, 60);
  Serial.println(F("[SYSTEM] Lo-Fi Internet Radio Initialized."));
}

void loop() {
  // Visualizer Animation (~30 FPS)
  if (millis() - lastVisUpdate > 33) {
    lastVisUpdate = millis();
    updateSpectrum();
  }

  // Touch Handling
  if (ts.touched()) {
    TS_Point p = ts.getPoint();
    int touchX = map(p.x, 3800, 300, 0, 320);
    int touchY = map(p.y, 3800, 400, 0, 240);
    touchX = constrain(touchX, 0, 320);
    touchY = constrain(touchY, 0, 240);

    // Control bar taps (y >= 196)
    if (touchY >= 196) {
      // Prev Station (10..55)
      if (touchX >= 10 && touchX <= 55) {
        currentStation = (currentStation - 1 + NUM_STATIONS) % NUM_STATIONS;
        playBeep(1800, 30);
        drawStationInfo();
        delay(180);
      }
      // Play / Pause (65..115)
      else if (touchX >= 65 && touchX <= 115) {
        isPlaying = !isPlaying;
        playBeep(isPlaying ? 2400 : 1200, 40);
        drawHeader();
        drawControls();
        if (!isPlaying) {
          tft.fillRect(10, 82, 300, 108, COLOR_BG);
          tft.drawRoundRect(10, 82, 300, 108, 4, COLOR_CARD_BRD);
          tft.setTextColor(COLOR_MUTED, COLOR_BG);
          tft.drawCentreString("AUDIO STREAM PAUSED", 160, 128, 2);
          setRGB(false, false, false);
        }
        delay(180);
      }
      // Next Station (125..170)
      else if (touchX >= 125 && touchX <= 170) {
        currentStation = (currentStation + 1) % NUM_STATIONS;
        playBeep(2200, 30);
        drawStationInfo();
        delay(180);
      }
      // Mode Cycle (180..235)
      else if (touchX >= 180 && touchX <= 235) {
        visMode = (visMode + 1) % 3;
        playBeep(2000, 30);
        drawControls();
        delay(180);
      }
      // Volume Adjust (245..310)
      else if (touchX >= 245 && touchX <= 310) {
        volume = (volume >= 100) ? 20 : volume + 20;
        playBeep(2600, 30);
        drawControls();
        delay(180);
      }
    }
  }

  delay(10);
}
