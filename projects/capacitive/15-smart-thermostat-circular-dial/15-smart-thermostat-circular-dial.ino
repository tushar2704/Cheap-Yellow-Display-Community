/*
 * CYD 2.8" Capacitive (ESP32-2432S028C) - 15: Smart Thermostat Dial
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

float targetTemp = 21.5;
float currentTemp = 20.8;
int mode = 0; // 0=HEAT, 1=COOL, 2=ECO

void drawThermostat() {
  tft.fillRect(0, 0, 320, 240, 0x0821);
  
  // Radial Dial Ring
  uint16_t dialCol = (mode == 0) ? TFT_ORANGE : ((mode == 1) ? TFT_CYAN : TFT_GREEN);
  tft.drawCircle(160, 105, 75, 0x2126);
  tft.drawCircle(160, 105, 76, dialCol);
  tft.drawCircle(160, 105, 77, dialCol);

  // Center Temperatures
  tft.setTextColor(TFT_WHITE, 0x0821);
  tft.setTextSize(4);
  char buf[8];
  snprintf(buf, sizeof(buf), "%.1f", targetTemp);
  tft.setCursor(120, 85);
  tft.print(buf);

  tft.setTextSize(1);
  tft.setTextColor(0x9492, 0x0821);
  tft.setCursor(125, 125);
  tft.printf("ROOM: %.1f C", currentTemp);

  // Mode Buttons
  const char* modes[3] = {"HEAT", "COOL", "ECO"};
  for (int m = 0; m < 3; m++) {
    int bx = 30 + m * 95;
    uint16_t bg = (mode == m) ? dialCol : 0x18C3;
    tft.fillRoundRect(bx, 195, 80, 34, 6, bg);
    tft.setTextColor((mode == m) ? 0x0000 : TFT_WHITE, bg);
    tft.setTextSize(1);
    tft.setCursor(bx + 24, 206);
    tft.print(modes[m]);
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_AUDIO, OUTPUT);
  tft.init();
  tft.setRotation(1);
  touch.begin();
  drawThermostat();
}

void loop() {
  if (touch.available()) {
    int sx = touch.data.y;
    int sy = 240 - touch.data.x;
    uint8_t g = touch.data.gestureID;

    // Up/Down Gestures to adjust target temp
    if (g == 0x01) { // Up
      targetTemp = min(30.0f, targetTemp + 0.5f);
      tone(PIN_AUDIO, 2000, 15);
      drawThermostat();
    } else if (g == 0x02) { // Down
      targetTemp = max(16.0f, targetTemp - 0.5f);
      tone(PIN_AUDIO, 1600, 15);
      drawThermostat();
    } else if (sy >= 195) { // Mode Select
      int clickedMode = (sx - 30) / 95;
      if (clickedMode >= 0 && clickedMode < 3) {
        mode = clickedMode;
        tone(PIN_AUDIO, 2200, 20);
        drawThermostat();
      }
    }
  }
}
