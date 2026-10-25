/*
 * CYD 2.8" Capacitive (ESP32-2432S028C) - 20: Capacitive Touch Diagnostics
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

int touchCount = 0;
int lastX = -1, lastY = -1;
uint8_t lastG = 0;
float velocity = 0;
unsigned long lastTouchTime = 0;

void drawDiagnosticsUI() {
  tft.fillRect(0, 0, 320, 240, 0x0821);

  // Header
  tft.fillRect(0, 0, 320, 26, 0x10A2);
  tft.setTextColor(TFT_WHITE, 0x10A2);
  tft.setTextSize(1);
  tft.setCursor(10, 8);
  tft.print("CST816S CAPACITIVE DIAGNOSTICS");

  // Telemetry Grid
  tft.fillRoundRect(10, 32, 300, 75, 6, 0x18C3);
  tft.drawRoundRect(10, 32, 300, 75, 6, TFT_CYAN);

  tft.setTextColor(TFT_WHITE, 0x18C3);
  tft.setCursor(20, 42);
  tft.printf("Touch Coordinate: X:%03d  Y:%03d", lastX, lastY);
  tft.setCursor(20, 58);
  tft.printf("Velocity: %.1f px/sec | Total Hits: %d", velocity, touchCount);
  tft.setCursor(20, 74);
  tft.printf("I2C Address: 0x15 | Driver: CST816S Fast 400kHz");
  tft.setCursor(20, 90);
  tft.printf("Gesture Register: 0x%02X (%s)", lastG, 
    lastG == 0x01 ? "UP" : (lastG == 0x02 ? "DOWN" : (lastG == 0x03 ? "LEFT" : (lastG == 0x04 ? "RIGHT" : (lastG == 0x05 ? "CLICK" : "IDLE")))));

  // Interactive Touch Canvas Box
  tft.drawRect(10, 115, 300, 115, 0x2945);
  tft.setTextColor(0x9492, 0x0821);
  tft.setCursor(20, 125);
  tft.print("Interactive Touch Pad: Trace finger to measure vector speed");
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_AUDIO, OUTPUT);
  tft.init();
  tft.setRotation(1);
  touch.begin();
  drawDiagnosticsUI();
}

void loop() {
  if (touch.available()) {
    int sx = touch.data.y;
    int sy = 240 - touch.data.x;
    lastG = touch.data.gestureID;
    touchCount++;

    unsigned long now = millis();
    if (lastX > 0 && lastY > 0 && now > lastTouchTime) {
      float dist = sqrt(sq(sx - lastX) + sq(sy - lastY));
      velocity = (dist / (now - lastTouchTime)) * 1000.0f;
    }
    lastTouchTime = now;
    lastX = sx;
    lastY = sy;

    tone(PIN_AUDIO, 2000, 8);
    drawDiagnosticsUI();

    // Crosshairs on touch point
    if (sy >= 115 && sy <= 230 && sx >= 10 && sx <= 310) {
      tft.fillCircle(sx, sy, 8, TFT_MAGENTA);
      tft.drawCircle(sx, sy, 14, TFT_WHITE);
    }
  }
}
