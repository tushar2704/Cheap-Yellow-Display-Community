/*
 * ═══════════════════════════════════════════════════════════════════════════
 *  CYD 2.8" Capacitive (ESP32-2432S028C) - 02: Pull-Down Quick Settings Shade
 * ═══════════════════════════════════════════════════════════════════════════
 *  Author:       tushar2704 <tushar.inseec@gmail.com>
 *  Repository:   https://github.com/tushar2704/Cheap-Yellow-Display-Community
 *  License:      Dual Licensed (AGPLv3 / CC BY-NC-ND 4.0)
 *  Credits:      Inspired by @IamNotIntoTech & @Makerrdesigns
 *  Year:         2026
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

// State Variables
bool shadeOpen = false;
int shadeY = 0; // 0 = closed, 240 = fully open
int brightness = 80;
int volume = 60;
bool wifiOn = true;
bool btOn = true;
bool flashlightOn = false;
bool dndOn = false;
unsigned long lastTick = 0;

void setRgb(uint8_t r, uint8_t g, uint8_t b) {
  analogWrite(PIN_LED_R, 255 - r);
  analogWrite(PIN_LED_G, 255 - g);
  analogWrite(PIN_LED_B, 255 - b);
}

void playChime(int f, int ms) {
  tone(PIN_AUDIO, f, ms);
}

void drawDesktop() {
  tft.fillRect(0, 0, 320, 240, 0x0821);
  
  // Status Bar
  tft.fillRect(0, 0, 320, 24, 0x10A2);
  tft.setTextColor(TFT_WHITE, 0x10A2);
  tft.setTextSize(1);
  tft.setCursor(10, 8);
  tft.print("10:42 AM");
  
  tft.setCursor(110, 8);
  tft.setTextColor(TFT_CYAN, 0x10A2);
  tft.print("SWIPE DOWN FOR SHADE");
  
  tft.setCursor(275, 8);
  tft.setTextColor(TFT_GREEN, 0x10A2);
  tft.print("98%");
  tft.drawFastHLine(0, 24, 320, 0x2126);

  // App Grid on Desktop
  const char* apps[4] = {"Terminal", "Files", "Weather", "Sensors"};
  uint16_t appColors[4] = {TFT_CYAN, TFT_YELLOW, TFT_MAGENTA, TFT_GREEN};
  for (int i = 0; i < 4; i++) {
    int ax = 35 + i * 70;
    int ay = 70;
    tft.fillRoundRect(ax, ay, 48, 48, 10, appColors[i]);
    tft.drawRoundRect(ax, ay, 48, 48, 10, TFT_WHITE);
    tft.setTextColor(TFT_WHITE, 0x0821);
    tft.setTextSize(1);
    tft.setCursor(ax - 2, ay + 56);
    tft.print(apps[i]);
  }

  // Desktop Widget
  tft.fillRoundRect(25, 150, 270, 70, 8, 0x18C3);
  tft.drawRoundRect(25, 150, 270, 70, 8, 0x2945);
  tft.setTextColor(TFT_WHITE, 0x18C3);
  tft.setCursor(40, 165);
  tft.setTextSize(2);
  tft.print("CYD Glass OS");
  tft.setTextSize(1);
  tft.setTextColor(0x9492, 0x18C3);
  tft.setCursor(40, 192);
  int ldr = analogRead(PIN_LDR);
  tft.printf("Ambient Lux: %d | Touch CST816 Ready", ldr);
}

void drawShade() {
  tft.fillRect(0, 0, 320, 240, 0x0842);

  // Shade Header
  tft.fillRoundRect(10, 8, 300, 32, 6, 0x18C3);
  tft.setTextColor(TFT_WHITE, 0x18C3);
  tft.setTextSize(1);
  tft.setCursor(20, 18);
  tft.print("QUICK SETTINGS");
  tft.setTextColor(TFT_CYAN, 0x18C3);
  tft.setCursor(200, 18);
  tft.print("[ SWIPE UP TO CLOSE ]");

  // Quick Toggle Tiles (2x2)
  struct ToggleBtn { int x, y; const char* name; bool state; uint16_t col; };
  ToggleBtn btns[4] = {
    {20, 48, "Wi-Fi", wifiOn, TFT_CYAN},
    {165, 48, "Bluetooth", btOn, TFT_BLUE},
    {20, 96, "Flashlight", flashlightOn, TFT_YELLOW},
    {165, 96, "Do Not Disturb", dndOn, TFT_PURPLE}
  };

  for (int i = 0; i < 4; i++) {
    uint16_t bg = btns[i].state ? btns[i].col : 0x2126;
    tft.fillRoundRect(btns[i].x, btns[i].y, 135, 40, 6, bg);
    tft.setTextColor(btns[i].state ? 0x0000 : TFT_WHITE, bg);
    tft.setTextSize(1);
    tft.setCursor(btns[i].x + 12, btns[i].y + 14);
    tft.printf("%s: %s", btns[i].name, btns[i].state ? "ON" : "OFF");
  }

  // Brightness Slider
  tft.setTextColor(TFT_WHITE, 0x0842);
  tft.setTextSize(1);
  tft.setCursor(20, 146);
  tft.printf("Brightness: %d%%", brightness);
  tft.fillRoundRect(20, 160, 280, 16, 4, 0x18C3);
  tft.fillRoundRect(20, 160, (brightness * 280) / 100, 16, 4, TFT_YELLOW);

  // Volume Slider
  tft.setCursor(20, 186);
  tft.printf("Volume: %d%%", volume);
  tft.fillRoundRect(20, 200, 280, 16, 4, 0x18C3);
  tft.fillRoundRect(20, 200, (volume * 280) / 100, 16, 4, TFT_CYAN);

  // Bottom drag handle pill
  tft.fillRoundRect(130, 228, 60, 5, 2, 0x632C);
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_LED_R, OUTPUT);
  pinMode(PIN_LED_G, OUTPUT);
  pinMode(PIN_LED_B, OUTPUT);
  pinMode(PIN_AUDIO, OUTPUT);
  pinMode(PIN_LDR, INPUT);
  setRgb(0, 0, 0);

  tft.init();
  tft.setRotation(1);
  touch.begin();

  drawDesktop();
  playChime(1800, 20);
}

void loop() {
  if (touch.available()) {
    int tx = touch.data.x;
    int ty = touch.data.y;
    uint8_t gesture = touch.data.gestureID;
    int sx = ty;
    int sy = 240 - tx;

    // Gesture: Swipe Down opens shade
    if (!shadeOpen && (gesture == 0x02 || (sy > 30 && gesture == 0x00))) {
      shadeOpen = true;
      playChime(2200, 25);
      drawShade();
      delay(100);
    }
    // Gesture: Swipe Up closes shade
    else if (shadeOpen && (gesture == 0x01 || sy > 220)) {
      shadeOpen = false;
      playChime(1600, 25);
      drawDesktop();
      delay(100);
    }
    // Touch interactions inside shade
    else if (shadeOpen) {
      // Toggle 1: WiFi
      if (sx >= 20 && sx <= 155 && sy >= 48 && sy <= 88) {
        wifiOn = !wifiOn;
        playChime(2000, 20);
        drawShade();
      }
      // Toggle 2: Bluetooth
      else if (sx >= 165 && sx <= 300 && sy >= 48 && sy <= 88) {
        btOn = !btOn;
        playChime(2000, 20);
        drawShade();
      }
      // Toggle 3: Flashlight
      else if (sx >= 20 && sx <= 155 && sy >= 96 && sy <= 136) {
        flashlightOn = !flashlightOn;
        setRgb(flashlightOn ? 255 : 0, flashlightOn ? 255 : 0, flashlightOn ? 255 : 0);
        playChime(2400, 20);
        drawShade();
      }
      // Toggle 4: DND
      else if (sx >= 165 && sx <= 300 && sy >= 96 && sy <= 136) {
        dndOn = !dndOn;
        playChime(1800, 20);
        drawShade();
      }
      // Brightness slider
      else if (sx >= 20 && sx <= 300 && sy >= 155 && sy <= 180) {
        brightness = constrain(map(sx, 20, 300, 0, 100), 0, 100);
        drawShade();
      }
      // Volume slider
      else if (sx >= 20 && sx <= 300 && sy >= 195 && sy <= 220) {
        volume = constrain(map(sx, 20, 300, 0, 100), 0, 100);
        playChime(1000 + volume * 15, 20);
        drawShade();
      }
    }
  }

  if (!shadeOpen && millis() - lastTick > 1000) {
    lastTick = millis();
    int ldr = analogRead(PIN_LDR);
    tft.fillRect(40, 190, 240, 14, 0x18C3);
    tft.setTextColor(0x9492, 0x18C3);
    tft.setTextSize(1);
    tft.setCursor(40, 192);
    tft.printf("Ambient Lux: %d | Touch CST816 Ready", ldr);
  }
}
