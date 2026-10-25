/**
 * @file 10-spotify-car-thing-remote.ino
 * @brief Spotify Car Thing Touch Remote & Now Playing Canvas for Cheap Yellow Display 2.8" (ESP32-2432S028R)
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
 * - Onboard LDR Sensor (GPIO 34)
 */

#include <WiFi.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>

// Enter your WiFi Credentials
const char* WIFI_SSID     = "YOUR_WIFI_SSID";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

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
#define LDR_PIN 34
#define TFT_BL 21

#define TS_MINX 300
#define TS_MAXX 3800
#define TS_MINY 200
#define TS_MAXY 3700

SPIClass touchSPI(VSPI);
XPT2046_Touchscreen ts(XPT2046_CS, XPT2046_IRQ);
TFT_eSPI tft = TFT_eSPI();

// Track Playback State
struct TrackInfo {
  String title;
  String artist;
  String album;
  int durationSec;
  int progressSec;
  bool isPlaying;
  int volumePct;
};

TrackInfo currentTrack = {
  "Starboy",
  "The Weeknd, Daft Punk",
  "Starboy (Deluxe)",
  230,
  95,
  true,
  75
};

void setRgb(bool r, bool g, bool b) {
  digitalWrite(LED_RED, r ? LOW : HIGH);
  digitalWrite(LED_GREEN, g ? LOW : HIGH);
  digitalWrite(LED_BLUE, b ? LOW : HIGH);
}

void beep(int freq, int ms) {
  tone(SPEAKER_PIN, freq, ms);
}

void drawHeader() {
  tft.fillRect(0, 0, 320, 30, 0x1082);
  tft.setTextColor(0x1DB9, 0x1082); // Spotify Green
  tft.setTextSize(1);
  tft.setCursor(10, 10);
  tft.print("🟢 Spotify Car Thing Remote");

  // Device Connected Badge
  tft.setTextColor(TFT_WHITE, 0x1082);
  tft.setCursor(205, 10);
  tft.printf("VOL: %d%%", currentTrack.volumePct);
}

void drawAlbumArt(int x, int y, int size) {
  // Stylized Vinyl / Album Cover Art Frame
  tft.fillRoundRect(x, y, size, size, 8, 0x18C3);
  tft.drawRoundRect(x, y, size, size, 8, 0x1DB9);

  // Decorative vinyl groove circles
  tft.drawCircle(x + size / 2, y + size / 2, size / 3, 0x3186);
  tft.drawCircle(x + size / 2, y + size / 2, size / 4, 0x2124);
  tft.fillCircle(x + size / 2, y + size / 2, 10, 0x1DB9);
  tft.fillCircle(x + size / 2, y + size / 2, 4, TFT_BLACK);
}

void drawTrackDetails() {
  int x = 118;
  int y = 42;

  tft.fillRect(x, y, 195, 75, 0x0000);

  // Title
  tft.setTextColor(TFT_WHITE, 0x0000);
  tft.setTextSize(2);
  tft.setCursor(x, y);
  String t = currentTrack.title;
  if (t.length() > 14) t = t.substring(0, 14);
  tft.print(t);

  // Artist
  tft.setTextColor(0x1DB9, 0x0000);
  tft.setTextSize(1);
  tft.setCursor(x, y + 24);
  String a = currentTrack.artist;
  if (a.length() > 22) a = a.substring(0, 22);
  tft.print(a);

  // Album
  tft.setTextColor(TFT_LIGHTGREY, 0x0000);
  tft.setCursor(x, y + 40);
  String alb = currentTrack.album;
  if (alb.length() > 22) alb = alb.substring(0, 22);
  tft.print(alb);

  // Time metrics
  tft.setTextColor(TFT_DARKGREY, 0x0000);
  tft.setCursor(x, y + 58);
  tft.printf("%d:%02d / %d:%02d",
             currentTrack.progressSec / 60, currentTrack.progressSec % 60,
             currentTrack.durationSec / 60, currentTrack.durationSec % 60);
}

void drawScrubber() {
  // Scrubber bar (12, 128, 296, 8)
  tft.fillRoundRect(12, 128, 296, 8, 4, 0x2104);
  int filledW = map(currentTrack.progressSec, 0, currentTrack.durationSec, 0, 296);
  tft.fillRoundRect(12, 128, filledW, 8, 4, 0x1DB9);
}

void drawTransportButtons() {
  // PREV BUTTON (20, 148, 60, 48)
  tft.fillRoundRect(20, 148, 60, 48, 8, 0x18C3);
  tft.drawRoundRect(20, 148, 60, 48, 8, 0x3186);
  tft.setTextColor(TFT_WHITE, 0x18C3);
  tft.setTextSize(2);
  tft.setCursor(38, 162);
  tft.print("|<<");

  // PLAY / PAUSE BUTTON (95, 144, 130, 56)
  uint16_t playBg = currentTrack.isPlaying ? 0x1DB9 : 0xFDC0;
  tft.fillRoundRect(95, 144, 130, 56, 12, playBg);
  tft.drawRoundRect(95, 144, 130, 56, 12, TFT_WHITE);
  tft.setTextColor(TFT_BLACK, playBg);
  tft.setTextSize(2);
  tft.setCursor(currentTrack.isPlaying ? 130 : 138, 162);
  tft.print(currentTrack.isPlaying ? "PAUSE" : "PLAY");

  // NEXT BUTTON (240, 148, 60, 48)
  tft.fillRoundRect(240, 148, 60, 48, 8, 0x18C3);
  tft.drawRoundRect(240, 148, 60, 48, 8, 0x3186);
  tft.setTextColor(TFT_WHITE, 0x18C3);
  tft.setCursor(256, 162);
  tft.print(">>|");

  // Volume Bar under transport (20, 208, 280, 8)
  tft.fillRoundRect(20, 208, 280, 6, 3, 0x2104);
  int volW = map(currentTrack.volumePct, 0, 100, 0, 280);
  tft.fillRoundRect(20, 208, volW, 6, 3, 0x07FF);
}

void drawFullUI() {
  tft.fillScreen(0x0000);
  drawHeader();
  drawAlbumArt(12, 38, 92);
  drawTrackDetails();
  drawScrubber();
  drawTransportButtons();

  // Footer Channel Bar
  tft.fillRect(0, 222, 320, 18, 0x0000);
  tft.setTextColor(0x07E0, 0x0000);
  tft.setTextSize(1);
  tft.setCursor(6, 226);
  tft.print("CYD Community | @IamNotIntoTech | @Makerrdesigns");

  setRgb(false, currentTrack.isPlaying, !currentTrack.isPlaying);
}

void setup() {
  Serial.begin(115200);

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  pinMode(SPEAKER_PIN, OUTPUT);
  setRgb(false, false, false);

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(0x0000);

  touchSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  ts.begin(touchSPI);
  ts.setRotation(1);

  drawFullUI();
  beep(1800, 60);
}

void loop() {
  // Check Touch Input
  if (ts.touched()) {
    TS_Point p = ts.getPoint();
    int x = map(p.x, TS_MINX, TS_MAXX, 0, 320);
    int y = map(p.y, TS_MINY, TS_MAXY, 0, 240);

    // PREV BUTTON (20, 148, 60, 48)
    if (x >= 20 && x <= 80 && y >= 148 && y <= 196) {
      beep(1600, 30);
      currentTrack.progressSec = 0;
      drawScrubber();
      drawTrackDetails();
      delay(200);
    }
    // PLAY / PAUSE BUTTON (95, 144, 130, 56)
    else if (x >= 95 && x <= 225 && y >= 144 && y <= 200) {
      beep(2000, 40);
      currentTrack.isPlaying = !currentTrack.isPlaying;
      drawTransportButtons();
      setRgb(false, currentTrack.isPlaying, !currentTrack.isPlaying);
      delay(200);
    }
    // NEXT BUTTON (240, 148, 60, 48)
    else if (x >= 240 && x <= 300 && y >= 148 && y <= 196) {
      beep(2200, 30);
      currentTrack.progressSec = 0;
      if (currentTrack.title == "Starboy") {
        currentTrack.title = "Blinding Lights";
        currentTrack.durationSec = 200;
      } else {
        currentTrack.title = "Starboy";
        currentTrack.durationSec = 230;
      }
      drawTrackDetails();
      drawScrubber();
      delay(200);
    }
    // Volume Scrubber Tap (20, 204, 280, 14)
    else if (x >= 20 && x <= 300 && y >= 202 && y <= 220) {
      currentTrack.volumePct = map(x, 20, 300, 0, 100);
      beep(2400, 20);
      drawHeader();
      drawTransportButtons();
      delay(150);
    }
  }

  // Playback timer progression
  static unsigned long lastSec = 0;
  if (millis() - lastSec > 1000) {
    lastSec = millis();
    if (currentTrack.isPlaying) {
      currentTrack.progressSec++;
      if (currentTrack.progressSec >= currentTrack.durationSec) {
        currentTrack.progressSec = 0;
      }
      drawScrubber();
      drawTrackDetails();
    }
  }

  delay(25);
}
