/*
 * ═══════════════════════════════════════════════════════════════════════════
 *  CYD 2.8" Capacitive (ESP32-2432S028C) - Project 01: CardFlow Gesture Launcher
 * ═══════════════════════════════════════════════════════════════════════════
 *  Author:       tushar2704 <tushar.inseec@gmail.com>
 *  Repository:   https://github.com/tushar2704/Cheap-Yellow-Display-Community
 *  License:      Dual Licensed:
 *                - GNU AGPLv3 for Source Code
 *                - CC BY-NC-ND 4.0 for Hardware Designs & Media Assets
 *  Credits:      Inspired by community work at @IamNotIntoTech & @Makerrdesigns
 *  Year:         2026
 * ═══════════════════════════════════════════════════════════════════════════
 *  Hardware Specifications:
 *    - Board: ESP32-2432S028C (2.8" Capacitive Touch CYD)
 *    - Screen: ILI9341 320x240 LCD (SPI)
 *    - Touch: CST816S / CST820 Capacitive Glass Touch (I2C: SDA=33, SCL=32, RST=25, INT=21)
 *    - Audio: GPIO 26 (Onboard Transistor Audio Amp)
 *    - RGB LED: GPIO 4 (Red), GPIO 16 (Green), GPIO 17 (Blue) - Active LOW
 *    - LDR Sensor: GPIO 34 (ADC1_CH6)
 * ═══════════════════════════════════════════════════════════════════════════
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
#define PIN_LDR   34

TFT_eSPI tft = TFT_eSPI();
CST816S touch(I2C_SDA_PIN, I2C_SCL_PIN, TOUCH_RST_PIN, TOUCH_INT_PIN);

// ── Color Palette ──
#define COLOR_BG          0x0821  // Deep obsidian navy
#define COLOR_SURFACE     0x18C3  // Card base
#define COLOR_SURFACE_HI  0x2126  // Highlighted card base
#define COLOR_TEXT_WHITE  0xFFFF
#define COLOR_TEXT_MUTED  0x9492
#define COLOR_CYAN        0x07FF
#define COLOR_PURPLE      0x915F
#define COLOR_AMBER       0xFDA0
#define COLOR_EMERALD     0x07E0
#define COLOR_ROSE        0xF814

// ── Application Card Definitions ──
struct AppCard {
  const char* title;
  const char* subtitle;
  const char* metricLabel;
  uint16_t themeColor;
  uint8_t ledR, ledG, ledB;
};

const int NUM_CARDS = 5;
AppCard cards[NUM_CARDS] = {
  { "System Telemetry", "Dual-Core ESP32 @ 240MHz", "CPU Load / RAM Free", COLOR_CYAN, 0, 255, 255 },
  { "Microclimate Hub", "Optical LDR & Lux Monitor", "Ambient Photons", COLOR_AMBER, 255, 180, 0 },
  { "Acoustic Synth", "Piezo Audio Wave Generator", "8-Bit Freq Pitch", COLOR_PURPLE, 200, 0, 255 },
  { "Atmosphere RGB", "Multi-Zone Chroma Sync", "Active Hex Spectrum", COLOR_ROSE, 255, 0, 100 },
  { "RF Spectrum Mesh", "2.4GHz Airwave Inspector", "Live Channel RSSI", COLOR_EMERALD, 0, 255, 50 }
};

int activeCard = 0;
bool cardInteracted = false;
unsigned long lastHeartbeat = 0;
unsigned long interactionExpiry = 0;
int dragStartX = -1;
int lastTouchX = -1;
bool isDragging = false;

// ── Audio Feedback ──
void playToneBeep(int freq, int durationMs) {
  tone(PIN_AUDIO, freq, durationMs);
}

void playSlideClick() {
  playToneBeep(1800, 15);
  delay(18);
  playToneBeep(2400, 18);
}

void playActionChime() {
  playToneBeep(1200, 30);
  delay(35);
  playToneBeep(1800, 40);
  delay(45);
  playToneBeep(2400, 60);
}

// ── RGB LED Control (Active LOW) ──
void setRgbLed(uint8_t r, uint8_t g, uint8_t b) {
  analogWrite(PIN_LED_R, 255 - r);
  analogWrite(PIN_LED_G, 255 - g);
  analogWrite(PIN_LED_B, 255 - b);
}

// ── UI Rendering: Top Status Bar ──
void renderStatusBar() {
  tft.fillRect(0, 0, 320, 26, COLOR_BG);
  tft.setTextSize(1);
  tft.setTextColor(COLOR_TEXT_WHITE, COLOR_BG);
  
  // Clock time (simulated running RTC)
  unsigned long secs = millis() / 1000;
  int hh = (12 + (secs / 3600)) % 24;
  int mm = (secs / 60) % 60;
  int ss = secs % 60;
  char timeStr[16];
  snprintf(timeStr, sizeof(timeStr), "%02d:%02d:%02d", hh, mm, ss);
  tft.setCursor(10, 8);
  tft.print(timeStr);

  // Badge: 2.8" Capacitive
  tft.fillRoundRect(95, 5, 130, 16, 4, 0x28B3);
  tft.setTextColor(0xDF1F, 0x28B3);
  tft.setCursor(103, 9);
  tft.print("CAPACITIVE CST816");

  // Battery / Power Icon
  tft.drawRoundRect(275, 7, 24, 12, 2, COLOR_TEXT_MUTED);
  tft.fillRect(299, 10, 2, 6, COLOR_TEXT_MUTED);
  tft.fillRect(277, 9, 16, 8, COLOR_EMERALD);
  tft.drawFastHLine(0, 25, 320, 0x18C3);
}

// ── UI Rendering: Main Active Card ──
void renderCard(int index, bool animate) {
  AppCard &card = cards[index];
  setRgbLed(card.ledR, card.ledG, card.ledB);

  // Clear card area
  tft.fillRect(0, 26, 320, 184, COLOR_BG);

  int cardX = 20;
  int cardY = 36;
  int cardW = 280;
  int cardH = 164;

  // Outer glow border
  tft.drawRoundRect(cardX - 1, cardY - 1, cardW + 2, cardH + 2, 12, card.themeColor);
  tft.fillRoundRect(cardX, cardY, cardW, cardH, 10, COLOR_SURFACE);

  // Card Header Accent Pill
  tft.fillRoundRect(cardX + 16, cardY + 14, 8, 24, 3, card.themeColor);
  tft.setTextColor(COLOR_TEXT_WHITE, COLOR_SURFACE);
  tft.setTextSize(2);
  tft.setCursor(cardX + 32, cardY + 18);
  tft.print(card.title);

  tft.setTextSize(1);
  tft.setTextColor(COLOR_TEXT_MUTED, COLOR_SURFACE);
  tft.setCursor(cardX + 32, cardY + 44);
  tft.print(card.subtitle);

  // Card Body Metric Area
  tft.fillRoundRect(cardX + 16, cardY + 62, cardW - 32, 54, 6, 0x0841);
  tft.drawRoundRect(cardX + 16, cardY + 62, cardW - 32, 54, 6, 0x2126);

  tft.setCursor(cardX + 26, cardY + 70);
  tft.setTextColor(card.themeColor, 0x0841);
  tft.print(card.metricLabel);

  char valBuf[32];
  if (index == 0) {
    uint32_t freeHeap = ESP.getFreeHeap() / 1024;
    snprintf(valBuf, sizeof(valBuf), "Free: %u KB | 240MHz", freeHeap);
  } else if (index == 1) {
    int ldr = analogRead(PIN_LDR);
    int luxEst = map(ldr, 0, 4095, 10, 1200);
    snprintf(valBuf, sizeof(valBuf), "%d Lux (LDR: %d)", luxEst, ldr);
  } else if (index == 2) {
    snprintf(valBuf, sizeof(valBuf), "Synth: 440Hz Concert A");
  } else if (index == 3) {
    snprintf(valBuf, sizeof(valBuf), "RGB: #%02X%02X%02X Active", card.ledR, card.ledG, card.ledB);
  } else {
    snprintf(valBuf, sizeof(valBuf), "WiFi: 14 Channels Ready");
  }

  tft.setTextSize(1);
  tft.setTextColor(COLOR_TEXT_WHITE, 0x0841);
  tft.setCursor(cardX + 26, cardY + 92);
  tft.print(valBuf);

  // Action Button at Bottom of Card
  tft.fillRoundRect(cardX + 16, cardY + 124, cardW - 32, 28, 6, card.themeColor);
  tft.setTextColor(0x0000, card.themeColor);
  tft.setTextSize(1);
  const char* btnLabel = (cardInteracted && interactionExpiry > millis()) ? "[ ACTIVE TRIGGERED ]" : "TAP TO INTERACT";
  int labelW = strlen(btnLabel) * 6;
  tft.setCursor(cardX + 16 + (cardW - 32 - labelW) / 2, cardY + 134);
  tft.print(btnLabel);
}

// ── UI Rendering: Bottom Gesture Indicator Dots ──
void renderCardDots() {
  tft.fillRect(0, 210, 320, 30, COLOR_BG);
  int startX = 160 - (NUM_CARDS * 16) / 2;
  for (int i = 0; i < NUM_CARDS; i++) {
    int cx = startX + i * 16;
    int cy = 222;
    if (i == activeCard) {
      tft.fillRoundRect(cx - 6, cy - 3, 16, 6, 3, cards[activeCard].themeColor);
    } else {
      tft.fillCircle(cx, cy, 3, 0x4208);
    }
  }

  // Visual Gesture Guide Hint
  tft.setTextColor(COLOR_TEXT_MUTED, COLOR_BG);
  tft.setTextSize(1);
  tft.setCursor(20, 224);
  tft.print("< SWIPE >");

  tft.setCursor(240, 224);
  tft.print("FLICK / TAP");
}

void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println(">>> 01-cardflow-gesture-launcher Starting on ESP32-2432S028C <<<");

  pinMode(PIN_LED_R, OUTPUT);
  pinMode(PIN_LED_G, OUTPUT);
  pinMode(PIN_LED_B, OUTPUT);
  pinMode(PIN_AUDIO, OUTPUT);
  pinMode(PIN_LDR, INPUT);

  // Initial LED off
  setRgbLed(0, 0, 0);

  // Display Init
  tft.init();
  tft.setRotation(1); // Landscape 320x240
  tft.fillScreen(COLOR_BG);

  // Capacitive Touch Init
  touch.begin();
  Serial.println("CST816S Capacitive Touch Initialized on I2C SDA:33 SCL:32");

  renderStatusBar();
  renderCard(activeCard, false);
  renderCardDots();

  playSlideClick();
}

void loop() {
  // Read Capacitive Touch & Gestures
  if (touch.available()) {
    int tx = touch.data.x;
    int ty = touch.data.y;
    uint8_t gesture = touch.data.gestureID;

    // Map portrait coordinates from CST816S to landscape if needed
    // In landscape rotation 1 (320x240):
    // CST816 raw is typically 240x320: x in [0..239], y in [0..319]
    int screenX = ty;
    int screenY = 240 - tx;

    // Hardware gesture checking:
    // 0x01 = Swipe Up, 0x02 = Swipe Down, 0x03 = Swipe Left, 0x04 = Swipe Right, 0x05 = Click, 0x0C = Long Press
    if (gesture == 0x03 || gesture == 0x02) {
      // Swipe Left -> Next Card
      activeCard = (activeCard + 1) % NUM_CARDS;
      playSlideClick();
      renderCard(activeCard, true);
      renderCardDots();
      delay(150);
    } else if (gesture == 0x04 || gesture == 0x01) {
      // Swipe Right -> Prev Card
      activeCard = (activeCard - 1 + NUM_CARDS) % NUM_CARDS;
      playSlideClick();
      renderCard(activeCard, true);
      renderCardDots();
      delay(150);
    } else if (gesture == 0x05 || gesture == 0x00) {
      // Tap or touch release inside button area
      if (screenY >= 140 && screenY <= 200) {
        cardInteracted = true;
        interactionExpiry = millis() + 1500;
        playActionChime();
        renderCard(activeCard, false);
      }
    }
  }

  // Periodic Telemetry Refresh (every 1 second)
  if (millis() - lastHeartbeat >= 1000) {
    lastHeartbeat = millis();
    renderStatusBar();
    if (cardInteracted && millis() > interactionExpiry) {
      cardInteracted = false;
      renderCard(activeCard, false);
    }
  }
}
