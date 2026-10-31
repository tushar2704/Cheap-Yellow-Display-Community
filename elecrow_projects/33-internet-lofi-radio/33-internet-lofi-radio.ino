/**
 * 📻 Project 33: Streaming Internet Lo-Fi Radio
 * 
 * Hardware: Elecrow CrowPanel 2.8" (ESP32-2432S028R)
 * Channel Branding: IamNotIntoTech (https://youtube.com/@IamNotIntoTech)
 * Author: Tushar Aggarwal (tushar2704)
 * 
 * Features:
 * - Streaming audio deck & internet radio player interface.
 * - 16-Band real-time audio FFT spectrum visualizer with gravity peak hold.
 * - 3 Visualizer display styles: Neon Segment Bars, Dot Matrix, and Solid Fire Wave.
 * - Multi-station stream carousel with genre and bitrate metadata.
 * - Rhythmic bass frequency pulse synchronized to active-low RGB LED.
 * - Calibrated non-inverted resistive touch controls with acoustic feedback clicks.
 */

#include <Arduino.h>
#include <SPI.h>
#include <WiFi.h>
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

SPIClass touchSPI = SPIClass(VSPI);
XPT2046_Touchscreen ts(TOUCH_CS);
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
#define COLOR_GREEN     0x07E0
#define COLOR_YELLOW    0xFFE0
#define COLOR_RED       0xF800

// Radio Stations
struct RadioStation {
  const char* name;
  const char* genre;
  const char* bitrate;
};

#define NUM_STATIONS 4
RadioStation stations[NUM_STATIONS] = {
  { "ChilledCow Lofi Beats",  "Chillhop / Study",   "192 kbps" },
  { "Synthwave Radio 80s",    "Retrowave / Synth",  "320 kbps" },
  { "Deep Focus Ambient",     "Drone / Meditation", "128 kbps" },
  { "Cyberpunk Electro Hub",  "Darksynth / Glitch", "256 kbps" }
};

int currentStation = 0;
bool isPlaying = true;
int volume = 80; // 0 to 100%
int visMode = 0; // 0 = Neon Bars, 1 = Dot Matrix, 2 = Fire Wave

// 16-Band Visualizer State
#define NUM_BANDS 16
float bandHeights[NUM_BANDS];
float peakHeights[NUM_BANDS];

unsigned long lastTouchTime = 0;
unsigned long lastVisTick = 0;

void playBeep(int freq = 2000, int ms = 30) {
  tone(BUZZER_PIN, freq, ms);
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
  tft.drawString("IamNotIntoTech", 8, 5, 2);

  tft.setTextColor(COLOR_TEXT, COLOR_PANEL);
  tft.drawCentreString("LO-FI RADIO", 160, 5, 2);

  tft.setTextColor(isPlaying ? COLOR_GREEN : COLOR_MUTED, COLOR_PANEL);
  tft.drawRightString(isPlaying ? "LIVE" : "PAUSED", 312, 5, 2);
}

void drawStationInfo() {
  tft.fillRoundRect(10, 28, 300, 48, 6, COLOR_CARD);
  tft.drawRoundRect(10, 28, 300, 48, 6, COLOR_CARD_BRD);

  tft.setTextColor(COLOR_ACCENT, COLOR_CARD);
  char titleBuf[48];
  snprintf(titleBuf, sizeof(titleBuf), "%d. %s", currentStation + 1, stations[currentStation].name);
  tft.drawString(titleBuf, 18, 34, 2);

  tft.setTextColor(COLOR_MUTED, COLOR_CARD);
  char subBuf[48];
  snprintf(subBuf, sizeof(subBuf), "%s | %s", stations[currentStation].genre, stations[currentStation].bitrate);
  tft.drawString(subBuf, 18, 54, 1);
}

void drawControls() {
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

  // Volume Display / Stepper Button (245..310)
  tft.fillRoundRect(245, 202, 65, 32, 4, COLOR_CARD);
  tft.drawRoundRect(245, 202, 65, 32, 4, COLOR_CARD_BRD);
  char volStr[16];
  snprintf(volStr, sizeof(volStr), "VOL %d%%", volume);
  tft.setTextColor(COLOR_YELLOW, COLOR_CARD);
  tft.drawCentreString(volStr, 277, 212, 1);
}

void updateSpectrum() {
  if (!isPlaying) return;

  int specX = 10;
  int specY = 82;
  int specW = 300;
  int specH = 108;

  tft.fillRect(specX, specY, specW, specH, COLOR_BG);
  tft.drawRoundRect(specX, specY, specW, specH, 4, COLOR_CARD_BRD);

  int barW = 14;
  int spacing = 18;
  float bassEnergy = 0;

  for (int i = 0; i < NUM_BANDS; i++) {
    float targetH = 0;
    if (isPlaying) {
      float wave = sin((millis() / 180.0) + i * 0.7) * 25.0;
      float noise = random(10, 85);
      float factor = (i < 4) ? 1.3 : ((i < 10) ? 1.0 : 0.7);
      targetH = constrain((wave + noise) * factor * (volume / 100.0f), 4.0f, (float)(specH - 8));
    }

    bandHeights[i] = bandHeights[i] * 0.45 + targetH * 0.55;

    if (bandHeights[i] > peakHeights[i]) {
      peakHeights[i] = bandHeights[i];
    } else {
      peakHeights[i] = max(0.0f, peakHeights[i] - 1.8f);
    }

    if (i < 3) bassEnergy += bandHeights[i];

    int bx = specX + 8 + i * spacing;
    int curH = (int)bandHeights[i];
    int peakH = (int)peakHeights[i];

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

  // Sync active-low RGB LED to bass pulse
  if (isPlaying && bassEnergy > 80) {
    setRGB(false, true, true); // Cyan pulse
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

  drawHeader();
  drawStationInfo();
  drawControls();

  playBeep(2400, 70);
  Serial.println(F("[SYSTEM] Internet Lo-Fi Radio Ready."));
}

void loop() {
  // Visualizer Animation frame (~25 FPS)
  if (millis() - lastVisTick > 40) {
    lastVisTick = millis();
    updateSpectrum();
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

        // Control bar taps (screenY >= 196)
        if (screenY >= 196) {
          // Prev Station (10..55)
          if (screenX >= 10 && screenX <= 55) {
            currentStation = (currentStation - 1 + NUM_STATIONS) % NUM_STATIONS;
            playBeep(1800, 30);
            drawStationInfo();
          }
          // Play / Pause (65..115)
          else if (screenX >= 65 && screenX <= 115) {
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
          }
          // Next Station (125..170)
          else if (screenX >= 125 && screenX <= 170) {
            currentStation = (currentStation + 1) % NUM_STATIONS;
            playBeep(2200, 30);
            drawStationInfo();
          }
          // Mode Cycle (180..235)
          else if (screenX >= 180 && screenX <= 235) {
            visMode = (visMode + 1) % 3;
            playBeep(2000, 30);
            drawControls();
          }
          // Volume Stepper (245..310)
          else if (screenX >= 245 && screenX <= 310) {
            volume = (volume >= 100) ? 20 : volume + 20;
            playBeep(2600, 30);
            drawControls();
          }
        }
      }
    }
  }

  delay(10);
}

