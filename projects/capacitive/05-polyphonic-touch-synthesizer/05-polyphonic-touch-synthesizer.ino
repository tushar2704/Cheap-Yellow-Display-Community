/*
 * CYD 2.8" Capacitive (ESP32-2432S028C) - 05: Touch Synthesizer & Ribbon
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
#define PIN_LED_R 4
#define PIN_LED_G 16
#define PIN_LED_B 17
#define PIN_AUDIO 26

TFT_eSPI tft = TFT_eSPI();
CST816S touch(I2C_SDA_PIN, I2C_SCL_PIN, TOUCH_RST_PIN, TOUCH_INT_PIN);

const int NUM_KEYS = 10;
const int KEY_W = 32;
const int KEY_H = 120;
int noteFreqs[NUM_KEYS] = {261, 293, 329, 349, 392, 440, 493, 523, 587, 659}; // C4 to E5
const char* noteNames[NUM_KEYS] = {"C4", "D4", "E4", "F4", "G4", "A4", "B4", "C5", "D5", "E5"};

int octave = 0; // -1, 0, +1
int activeKey = -1;
int pitchBendFreq = 0;

void drawKeyboard() {
  tft.fillRect(0, 0, 320, 240, 0x0821);
  
  // Header
  tft.fillRect(0, 0, 320, 26, 0x10A2);
  tft.setTextColor(TFT_WHITE, 0x10A2);
  tft.setTextSize(1);
  tft.setCursor(10, 8);
  tft.print("TOUCH SYNTHESIZER");
  tft.setCursor(180, 8);
  tft.setTextColor(TFT_CYAN, 0x10A2);
  tft.printf("OCTAVE: %+d  [ - / + ]", octave);

  // Top Ribbon Pitch Bend Strip
  tft.fillRoundRect(10, 32, 300, 32, 4, 0x18C3);
  tft.drawRoundRect(10, 32, 300, 32, 4, TFT_MAGENTA);
  tft.setTextColor(TFT_MAGENTA, 0x18C3);
  tft.setCursor(85, 42);
  tft.print("<< CONTINUOUS PITCH RIBBON >>");

  // 10 White Keys
  for (int i = 0; i < NUM_KEYS; i++) {
    int kx = i * KEY_W;
    tft.fillRoundRect(kx + 1, 75, KEY_W - 2, KEY_H, 4, TFT_WHITE);
    tft.drawRoundRect(kx + 1, 75, KEY_W - 2, KEY_H, 4, 0x2126);
    tft.setTextColor(0x0000, TFT_WHITE);
    tft.setTextSize(1);
    tft.setCursor(kx + 8, 175);
    tft.print(noteNames[i]);
  }

  // Black Keys (Accidentals)
  int blackKeyIndices[7] = {0, 1, 3, 4, 5, 7, 8};
  for (int b = 0; b < 7; b++) {
    int bx = (blackKeyIndices[b] + 1) * KEY_W - 10;
    tft.fillRoundRect(bx, 75, 20, 70, 3, 0x0000);
  }

  // Bottom Visualizer Bar
  tft.fillRect(0, 205, 320, 35, 0x0000);
  tft.drawFastHLine(0, 205, 320, 0x2126);
  tft.setTextColor(0x9492, 0x0000);
  tft.setCursor(15, 218);
  tft.print("Slide finger on Ribbon for Glissando | Tap keys to play");
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_AUDIO, OUTPUT);
  tft.init();
  tft.setRotation(1);
  touch.begin();
  drawKeyboard();
}

void loop() {
  if (touch.available()) {
    int sx = touch.data.y;
    int sy = 240 - touch.data.x;

    // Ribbon Pitch Bend Strip Hit
    if (sy >= 32 && sy <= 64 && sx >= 10 && sx <= 310) {
      int bendFreq = map(sx, 10, 310, 150, 1200);
      tone(PIN_AUDIO, bendFreq);
      tft.fillRect(15, 218, 290, 12, 0x0000);
      tft.setTextColor(TFT_MAGENTA, 0x0000);
      tft.setCursor(15, 218);
      tft.printf("Ribbon Active: %d Hz", bendFreq);
    }
    // Piano Keys Hit
    else if (sy >= 75 && sy <= 195) {
      int key = constrain(sx / KEY_W, 0, NUM_KEYS - 1);
      int f = noteFreqs[key];
      if (octave == -1) f /= 2;
      else if (octave == 1) f *= 2;
      tone(PIN_AUDIO, f);

      tft.fillRect(15, 218, 290, 12, 0x0000);
      tft.setTextColor(TFT_GREEN, 0x0000);
      tft.setCursor(15, 218);
      tft.printf("Note: %s (%d Hz)", noteNames[key], f);
    }
    // Octave Shift Buttons in Header
    else if (sy <= 26) {
      if (sx >= 230 && sx <= 260) { octave = max(-1, octave - 1); drawKeyboard(); }
      else if (sx >= 270 && sx <= 310) { octave = min(1, octave + 1); drawKeyboard(); }
    }
  } else {
    noTone(PIN_AUDIO);
  }
}
