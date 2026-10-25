/*
 * CYD 2.8" Capacitive (ESP32-2432S028C) - 11: Spotify Gesture Companion
 * Author: tushar2704 <tushar.inseec@gmail.com> | Dual Licensed (AGPLv3 / CC BY-NC-ND 4.0)
 */
#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <TFT_eSPI.h>
#include <CST816S.h>

#define I2C_SDA_PIN   33
#define I2C_SCL_PIN   32
#define TOUCH_RST_PIN 25
#define TOUCH_INT_PIN 21
#define PIN_AUDIO 26

TFT_eSPI tft = TFT_eSPI();
CST816S touch(I2C_SDA_PIN, I2C_SCL_PIN, TOUCH_RST_PIN, TOUCH_INT_PIN);

bool isPlaying = true;
bool isLiked = false;
int trackVolume = 75;
int progressSec = 142; // 2:22
int totalSec = 224;    // 3:44
int songIdx = 0;

const char* songs[3] = {"Starboy", "Get Lucky", "Midnight City"};
const char* artists[3] = {"The Weeknd ft. Daft Punk", "Daft Punk ft. Pharrell", "M83"};

void drawPlayer() {
  tft.fillRect(0, 0, 320, 240, 0x0821);
  
  // Header
  tft.fillRect(0, 0, 320, 24, 0x10A2);
  tft.setTextColor(TFT_GREEN, 0x10A2);
  tft.setTextSize(1);
  tft.setCursor(10, 8);
  tft.print("SPOTIFY COMPANION");
  tft.setTextColor(TFT_WHITE, 0x10A2);
  tft.setCursor(220, 8);
  tft.printf("VOL: %d%%", trackVolume);

  // Album Artwork Box (Left)
  tft.fillRoundRect(20, 40, 95, 95, 6, 0x18C3);
  tft.drawRoundRect(20, 40, 95, 95, 6, TFT_GREEN);
  tft.drawCircle(67, 87, 24, TFT_GREEN);
  tft.fillCircle(67, 87, 6, TFT_WHITE);

  // Track Info (Right)
  tft.setTextColor(TFT_WHITE, 0x0821);
  tft.setTextSize(2);
  tft.setCursor(125, 48);
  tft.print(songs[songIdx]);

  tft.setTextSize(1);
  tft.setTextColor(0x9492, 0x0821);
  tft.setCursor(125, 75);
  tft.print(artists[songIdx]);

  tft.setTextColor(isLiked ? TFT_RED : 0x9492, 0x0821);
  tft.setCursor(125, 98);
  tft.print(isLiked ? "<3 LIKED SONG" : "+ TAP TO LIKE");

  // Progress Bar
  tft.fillRoundRect(20, 150, 280, 8, 4, 0x2126);
  int fillW = map(progressSec, 0, totalSec, 0, 280);
  tft.fillRoundRect(20, 150, fillW, 8, 4, TFT_GREEN);

  // Time Stamps
  tft.setTextColor(0x9492, 0x0821);
  tft.setTextSize(1);
  tft.setCursor(20, 164);
  tft.printf("%d:%02d", progressSec / 60, progressSec % 60);
  tft.setCursor(275, 164);
  tft.printf("%d:%02d", totalSec / 60, totalSec % 60);

  // Control Buttons (Prev, Play/Pause, Next)
  tft.fillRoundRect(50, 184, 50, 40, 6, 0x18C3);
  tft.setTextColor(TFT_WHITE, 0x18C3);
  tft.setCursor(65, 198);
  tft.print("<<");

  tft.fillRoundRect(135, 184, 50, 40, 6, TFT_GREEN);
  tft.setTextColor(0x0000, TFT_GREEN);
  tft.setCursor(150, 198);
  tft.print(isPlaying ? "||" : ">");

  tft.fillRoundRect(220, 184, 50, 40, 6, 0x18C3);
  tft.setTextColor(TFT_WHITE, 0x18C3);
  tft.setCursor(235, 198);
  tft.print(">>");
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_AUDIO, OUTPUT);
  tft.init();
  tft.setRotation(1);
  touch.begin();
  drawPlayer();
}

void loop() {
  if (touch.available()) {
    int sx = touch.data.y;
    int sy = 240 - touch.data.x;
    uint8_t g = touch.data.gestureID;

    // Gestures: Swipe Left=Next, Swipe Right=Prev, Swipe Up=Vol+, Swipe Down=Vol-
    if (g == 0x03) { // Next
      songIdx = (songIdx + 1) % 3;
      progressSec = 0;
      tone(PIN_AUDIO, 2200, 20);
      drawPlayer();
      delay(100);
    } else if (g == 0x04) { // Prev
      songIdx = (songIdx - 1 + 3) % 3;
      progressSec = 0;
      tone(PIN_AUDIO, 1800, 20);
      drawPlayer();
      delay(100);
    } else if (g == 0x01) { // Vol Up
      trackVolume = min(100, trackVolume + 5);
      tone(PIN_AUDIO, 2000, 15);
      drawPlayer();
    } else if (g == 0x02) { // Vol Down
      trackVolume = max(0, trackVolume - 5);
      tone(PIN_AUDIO, 1400, 15);
      drawPlayer();
    } else if (g == 0x05) { // Tap
      // Play/Pause button
      if (sx >= 135 && sx <= 185 && sy >= 184 && sy <= 224) {
        isPlaying = !isPlaying;
        tone(PIN_AUDIO, 1600, 25);
        drawPlayer();
      } else if (sx >= 125 && sx <= 220 && sy >= 95 && sy <= 115) {
        isLiked = !isLiked;
        tone(PIN_AUDIO, 2400, 30);
        drawPlayer();
      }
    }
  }
}
