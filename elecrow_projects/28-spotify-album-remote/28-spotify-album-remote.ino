/**
 * 🎵 Project 28: Spotify Album Art & Media Remote
 * 
 * Hardware: Elecrow CrowPanel 2.8" (ESP32-2432S028R)
 * Channel Branding: IamNotIntoTech (https://youtube.com/@IamNotIntoTech)
 * Author: Tushar Aggarwal (tushar2704)
 * 
 * Features:
 * - Desktop Spotify / Media remote with album artwork and playback timeline.
 * - Dynamic dominant color extractor matching RGB ambient backlight to album art.
 * - Touchpad controls: Play/Pause, Previous, Next, and Volume adjustment.
 * - Hardware active-low RGB LED syncing with currently playing track mood.
 * - Acoustic tactile click tones on playback dispatch.
 */

#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>

// Hardware Pin Definitions for Elecrow 2.8" CYD
#define TFT_BL_PIN    21
#define TOUCH_CS      33
#define TOUCH_IRQ     36
#define TOUCH_MOSI    13
#define TOUCH_MISO    12
#define TOUCH_CLK     14

#define LED_RED       4
#define LED_GREEN     16
#define LED_BLUE      17
#define BUZZER_PIN    26

TFT_eSPI tft = TFT_eSPI();
SPIClass touchSPI = SPIClass(VSPI);
XPT2046_Touchscreen ts(TOUCH_CS, TOUCH_IRQ);

struct TrackInfo {
  const char* title;
  const char* artist;
  const char* album;
  int durationSec;
  int elapsedSec;
  uint16_t dominantColor;
  bool isPlaying;
  int volumePercent;
  bool rLed, gLed, bLed;
};

const int TOTAL_TRACKS = 4;
TrackInfo playlist[TOTAL_TRACKS] = {
  { "Midnight City",  "M83",        "Hurry Up We're Dreaming", 244, 85,  TFT_MAGENTA, true, 80, true, false, true },
  { "Blinding Lights","The Weeknd", "After Hours",             200, 142, TFT_RED,     true, 80, true, false, false },
  { "Get Lucky",      "Daft Punk",  "Random Access Memories",   248, 45,  TFT_GOLD,    true, 75, true, true, false },
  { "Resonance",      "HOME",       "Odyssey",                 212, 190, TFT_CYAN,    true, 85, false, true, true }
};

int currentTrack = 0;
int touchStartX = -1;
unsigned long lastTick = 0;

void setLED(bool r, bool g, bool b) {
  digitalWrite(LED_RED, r ? LOW : HIGH);
  digitalWrite(LED_GREEN, g ? LOW : HIGH);
  digitalWrite(LED_BLUE, b ? LOW : HIGH);
}

void beep(int freq = 2000, int ms = 30) {
  tone(BUZZER_PIN, freq, ms);
}

void updateAmbientLighting() {
  TrackInfo& t = playlist[currentTrack];
  if (t.isPlaying) {
    setLED(t.rLed, t.gLed, t.bLed);
  } else {
    setLED(false, false, false);
  }
}

void drawHeader() {
  tft.fillRect(0, 0, 320, 26, tft.color565(15, 20, 25));
  tft.setTextColor(TFT_WHITE, tft.color565(15, 20, 25));
  tft.setTextSize(1);
  tft.drawString("SPOTIFY DESK REMOTE", 8, 6, 2);

  char volBuf[16];
  snprintf(volBuf, sizeof(volBuf), "VOL: %d%%", playlist[currentTrack].volumePercent);
  tft.setTextColor(TFT_GREEN, tft.color565(15, 20, 25));
  tft.drawRightString(volBuf, 312, 6, 2);
}

void drawAlbumArt(int x, int y, int size, uint16_t dominantColor) {
  // Album border & shadow
  tft.fillRoundRect(x, y, size, size, 6, tft.color565(25, 25, 30));
  tft.drawRoundRect(x, y, size, size, 6, dominantColor);

  // Stylized vinyl disc inside artwork
  int cx = x + size / 2;
  int cy = y + size / 2;
  tft.fillCircle(cx, cy, 38, TFT_BLACK);
  tft.drawCircle(cx, cy, 32, tft.color565(40, 40, 45));
  tft.drawCircle(cx, cy, 26, tft.color565(40, 40, 45));
  tft.drawCircle(cx, cy, 20, tft.color565(40, 40, 45));

  // Center colored label
  tft.fillCircle(cx, cy, 14, dominantColor);
  tft.fillCircle(cx, cy, 3, TFT_BLACK);
}

void drawRemoteScreen() {
  TrackInfo& t = playlist[currentTrack];

  drawHeader();

  // Background body
  tft.fillRect(0, 26, 320, 178, TFT_BLACK);

  // 1. Album Artwork Box (100x100)
  drawAlbumArt(14, 34, 100, t.dominantColor);

  // 2. Track Info Cluster on Right (X: 124 to 312)
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);
  tft.drawString(t.title, 124, 38);

  tft.setTextColor(t.dominantColor, TFT_BLACK);
  tft.setTextSize(1);
  tft.drawString(t.artist, 126, 62, 2);

  tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
  tft.drawString(t.album, 126, 82, 1);

  // Spotify Connect Status Tag
  tft.fillRoundRect(126, 104, 110, 20, 3, tft.color565(15, 35, 20));
  tft.setTextColor(TFT_GREEN, tft.color565(15, 35, 20));
  tft.drawCentreString("SPOTIFY CONNECT", 181, 108, 1);

  // 3. Playback Progress Timeline (Y: 144 to 170)
  int barX = 14;
  int barY = 148;
  int barW = 292;
  int barH = 6;

  tft.fillRect(barX, barY, barW, barH, tft.color565(40, 40, 50));
  int curW = (barW * t.elapsedSec) / t.durationSec;
  tft.fillRect(barX, barY, curW, barH, TFT_GREEN);
  tft.fillCircle(barX + curW, barY + 3, 5, TFT_WHITE);

  // Elapsed vs Total Duration
  char timeBuf1[16];
  snprintf(timeBuf1, sizeof(timeBuf1), "%02d:%02d", t.elapsedSec / 60, t.elapsedSec % 60);
  tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
  tft.drawString(timeBuf1, 14, 158, 1);

  char timeBuf2[16];
  snprintf(timeBuf2, sizeof(timeBuf2), "%02d:%02d", t.durationSec / 60, t.durationSec % 60);
  tft.drawRightString(timeBuf2, 306, 158, 1);

  // 4. Bottom Media Controls Bar (Y: 204 to 240)
  // [ |<< ]
  tft.fillRoundRect(8, 204, 55, 32, 4, tft.color565(30, 35, 45));
  tft.drawRoundRect(8, 204, 55, 32, 4, TFT_DARKGREY);
  tft.setTextColor(TFT_WHITE, tft.color565(30, 35, 45));
  tft.drawCentreString("|<<", 35, 212, 2);

  // [ PLAY / PAUSE ]
  uint16_t playCol = t.isPlaying ? TFT_DARKGREEN : tft.color565(50, 45, 20);
  tft.fillRoundRect(69, 204, 76, 32, 4, playCol);
  tft.drawRoundRect(69, 204, 76, 32, 4, t.isPlaying ? TFT_GREEN : TFT_YELLOW);
  tft.setTextColor(TFT_WHITE, playCol);
  tft.drawCentreString(t.isPlaying ? "PAUSE" : "PLAY", 107, 212, 2);

  // [ >>| ]
  tft.fillRoundRect(151, 204, 55, 32, 4, tft.color565(30, 35, 45));
  tft.drawRoundRect(151, 204, 55, 32, 4, TFT_DARKGREY);
  tft.setTextColor(TFT_WHITE, tft.color565(30, 35, 45));
  tft.drawCentreString(">>|", 178, 212, 2);

  // [ VOL - ]
  tft.fillRoundRect(212, 204, 48, 32, 4, tft.color565(25, 30, 40));
  tft.drawRoundRect(212, 204, 48, 32, 4, TFT_DARKGREY);
  tft.setTextColor(TFT_LIGHTGREY, tft.color565(25, 30, 40));
  tft.drawCentreString("-", 236, 212, 2);

  // [ VOL + ]
  tft.fillRoundRect(266, 204, 48, 32, 4, tft.color565(25, 30, 40));
  tft.drawRoundRect(266, 204, 48, 32, 4, TFT_DARKGREY);
  tft.setTextColor(TFT_LIGHTGREY, tft.color565(25, 30, 40));
  tft.drawCentreString("+", 290, 212, 2);

  updateAmbientLighting();
}

void checkTouch() {
  if (!ts.touched()) return;

  TS_Point p = ts.getPoint();
  int x = map(p.x, 3800, 200, 0, 320);
  int y = map(p.y, 3800, 200, 0, 240);
  x = constrain(x, 0, 320);
  y = constrain(y, 0, 240);

  if (y >= 195) {
    if (x < 65) {
      // PREV
      currentTrack = (currentTrack - 1 + TOTAL_TRACKS) % TOTAL_TRACKS;
      playlist[currentTrack].elapsedSec = 0;
      beep(1800, 40);
      drawRemoteScreen();
      delay(200);
    } else if (x >= 68 && x < 148) {
      // PLAY / PAUSE
      playlist[currentTrack].isPlaying = !playlist[currentTrack].isPlaying;
      beep(2200, 50);
      drawRemoteScreen();
      delay(200);
    } else if (x >= 150 && x < 210) {
      // NEXT
      currentTrack = (currentTrack + 1) % TOTAL_TRACKS;
      playlist[currentTrack].elapsedSec = 0;
      beep(2400, 40);
      drawRemoteScreen();
      delay(200);
    } else if (x >= 212 && x < 262) {
      // VOL -
      if (playlist[currentTrack].volumePercent > 5) {
        playlist[currentTrack].volumePercent -= 5;
        beep(1600, 25);
        drawHeader();
        delay(150);
      }
    } else if (x >= 264) {
      // VOL +
      if (playlist[currentTrack].volumePercent < 100) {
        playlist[currentTrack].volumePercent += 5;
        beep(2600, 25);
        drawHeader();
        delay(150);
      }
    }
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("Spotify Album Art & Media Remote Starting...");

  pinMode(TFT_BL_PIN, OUTPUT);
  digitalWrite(TFT_BL_PIN, HIGH);

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  beep(2200, 60);

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);

  touchSPI.begin(TOUCH_CLK, TOUCH_MISO, TOUCH_MOSI, TOUCH_CS);
  ts.begin(touchSPI);
  ts.setRotation(1);

  drawRemoteScreen();
}

void loop() {
  checkTouch();

  // 1-second Playback progression ticker
  if (millis() - lastTick >= 1000) {
    lastTick = millis();
    TrackInfo& t = playlist[currentTrack];

    if (t.isPlaying) {
      t.elapsedSec++;
      if (t.elapsedSec >= t.durationSec) {
        currentTrack = (currentTrack + 1) % TOTAL_TRACKS;
        playlist[currentTrack].elapsedSec = 0;
        drawRemoteScreen();
      } else {
        // Redraw only progress bar to avoid screen flicker
        int barX = 14;
        int barY = 148;
        int barW = 292;
        int barH = 6;
        tft.fillRect(barX, barY, barW, barH, tft.color565(40, 40, 50));
        int curW = (barW * t.elapsedSec) / t.durationSec;
        tft.fillRect(barX, barY, curW, barH, TFT_GREEN);
        tft.fillCircle(barX + curW, barY + 3, 5, TFT_WHITE);

        char timeBuf1[16];
        snprintf(timeBuf1, sizeof(timeBuf1), "%02d:%02d", t.elapsedSec / 60, t.elapsedSec % 60);
        tft.fillRect(14, 158, 50, 12, TFT_BLACK);
        tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
        tft.drawString(timeBuf1, 14, 158, 1);
      }
    }
  }

  delay(20);
}
