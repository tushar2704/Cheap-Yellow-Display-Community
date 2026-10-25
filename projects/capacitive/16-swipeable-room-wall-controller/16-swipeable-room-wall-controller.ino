/*
 * CYD 2.8" Capacitive (ESP32-2432S028C) - 16: Swipeable Room Controller
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

struct Room { const char* name; bool lightsOn; int brightness; int temp; };
Room rooms[4] = {
  {"Living Room", true, 80, 21},
  {"Kitchen", false, 0, 20},
  {"Master Bed", true, 40, 22},
  {"Home Office", true, 100, 22}
};
int curRoom = 0;

void drawRoom() {
  tft.fillRect(0, 0, 320, 240, 0x0821);
  Room &r = rooms[curRoom];

  // Room Header
  tft.fillRect(0, 0, 320, 30, 0x10A2);
  tft.setTextColor(TFT_WHITE, 0x10A2);
  tft.setTextSize(1);
  tft.setCursor(15, 10);
  tft.print(r.name);
  tft.setTextColor(TFT_CYAN, 0x10A2);
  tft.setCursor(200, 10);
  tft.printf("ROOM %d/4", curRoom + 1);

  // Light Card
  tft.fillRoundRect(20, 42, 280, 68, 6, 0x18C3);
  tft.setTextColor(TFT_WHITE, 0x18C3);
  tft.setCursor(35, 54);
  tft.print("Main Lights");

  tft.fillRoundRect(220, 50, 65, 26, 4, r.lightsOn ? TFT_YELLOW : 0x2126);
  tft.setTextColor(r.lightsOn ? 0x0000 : TFT_WHITE, r.lightsOn ? TFT_YELLOW : 0x2126);
  tft.setCursor(235, 58);
  tft.print(r.lightsOn ? "ON" : "OFF");

  // Brightness Bar
  tft.fillRoundRect(35, 84, 240, 10, 4, 0x2126);
  tft.fillRoundRect(35, 84, (r.brightness * 240) / 100, 10, 4, TFT_YELLOW);

  // Climate Card
  tft.fillRoundRect(20, 120, 280, 68, 6, 0x18C3);
  tft.setTextColor(TFT_WHITE, 0x18C3);
  tft.setCursor(35, 132);
  tft.printf("Thermostat: %d C", r.temp);

  tft.fillRoundRect(190, 134, 40, 34, 4, 0x2126);
  tft.setTextColor(TFT_WHITE, 0x2126);
  tft.setCursor(205, 144);
  tft.print("-");

  tft.fillRoundRect(245, 134, 40, 34, 4, 0x2126);
  tft.setCursor(260, 144);
  tft.print("+");

  // Page Indicator Dots
  for (int i = 0; i < 4; i++) {
    tft.fillCircle(130 + i * 20, 215, (i == curRoom) ? 5 : 3, (i == curRoom) ? TFT_CYAN : 0x3186);
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_AUDIO, OUTPUT);
  tft.init();
  tft.setRotation(1);
  touch.begin();
  drawRoom();
}

void loop() {
  if (touch.available()) {
    int sx = touch.data.y;
    int sy = 240 - touch.data.x;
    uint8_t g = touch.data.gestureID;

    if (g == 0x03) { // Swipe Left
      curRoom = (curRoom + 1) % 4;
      tone(PIN_AUDIO, 2000, 15);
      drawRoom();
      delay(120);
    } else if (g == 0x04) { // Swipe Right
      curRoom = (curRoom - 1 + 4) % 4;
      tone(PIN_AUDIO, 1800, 15);
      drawRoom();
      delay(120);
    } else if (sy >= 50 && sy <= 76 && sx >= 220 && sx <= 285) {
      rooms[curRoom].lightsOn = !rooms[curRoom].lightsOn;
      rooms[curRoom].brightness = rooms[curRoom].lightsOn ? 80 : 0;
      tone(PIN_AUDIO, 2200, 20);
      drawRoom();
    } else if (sy >= 134 && sy <= 168) {
      if (sx >= 190 && sx <= 230) { rooms[curRoom].temp--; tone(PIN_AUDIO, 1600, 15); drawRoom(); }
      else if (sx >= 245 && sx <= 285) { rooms[curRoom].temp++; tone(PIN_AUDIO, 2000, 15); drawRoom(); }
    }
  }
}
