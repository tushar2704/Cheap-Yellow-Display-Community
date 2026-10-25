/*
 * CYD 2.8" Capacitive (ESP32-2432S028C) - 12: Candlestick Finance Navigator
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

struct Candle { float open, high, low, close; };
const int NUM_CANDLES = 16;
Candle candles[NUM_CANDLES] = {
  {64200, 64800, 63900, 64600},
  {64600, 65200, 64400, 65100},
  {65100, 65400, 64800, 64900},
  {64900, 65100, 64200, 64300},
  {64300, 64700, 64100, 64650},
  {64650, 65800, 64500, 65600},
  {65600, 66200, 65400, 66100},
  {66100, 66500, 65800, 65900},
  {65900, 66200, 65300, 65500},
  {65500, 65900, 65200, 65800},
  {65800, 66700, 65600, 66500},
  {66500, 67100, 66300, 66900},
  {66900, 67400, 66700, 67200},
  {67200, 67300, 66400, 66600},
  {66600, 67200, 66500, 67100},
  {67100, 67800, 67000, 67650}
};

int inspectIdx = 15;

void drawChart() {
  tft.fillRect(0, 0, 320, 240, 0x0821);

  // Header & Active Inspection
  Candle &c = candles[inspectIdx];
  tft.fillRect(0, 0, 320, 34, 0x10A2);
  tft.setTextColor(TFT_WHITE, 0x10A2);
  tft.setTextSize(1);
  tft.setCursor(10, 6);
  tft.print("BTC/USDT 1D CANDLESTICK");

  tft.setCursor(10, 20);
  tft.setTextColor(c.close >= c.open ? TFT_GREEN : TFT_RED, 0x10A2);
  tft.printf("O:%.0f H:%.0f L:%.0f C:%.0f", c.open, c.high, c.low, c.close);

  // Chart Grid
  float minP = 63500;
  float maxP = 68000;
  int startX = 20;
  int cW = 14;
  int gap = 4;

  for (int i = 0; i < NUM_CANDLES; i++) {
    int cx = startX + i * (cW + gap);
    int yHigh = map(candles[i].high, minP, maxP, 195, 45);
    int yLow = map(candles[i].low, minP, maxP, 195, 45);
    int yOpen = map(candles[i].open, minP, maxP, 195, 45);
    int yClose = map(candles[i].close, minP, maxP, 195, 45);

    uint16_t col = (candles[i].close >= candles[i].open) ? TFT_GREEN : TFT_RED;

    // Wick
    tft.drawFastVLine(cx + cW / 2, yHigh, yLow - yHigh, col);

    // Body
    int bodyTop = min(yOpen, yClose);
    int bodyH = max(2, abs(yClose - yOpen));
    tft.fillRect(cx, bodyTop, cW, bodyH, col);

    // Active inspection cursor
    if (i == inspectIdx) {
      tft.drawRect(cx - 2, 42, cW + 4, 156, TFT_CYAN);
    }
  }

  // Bottom Volume Pane
  tft.fillRect(0, 205, 320, 35, 0x0000);
  tft.drawFastHLine(0, 204, 320, 0x2945);
  tft.setTextColor(0x9492, 0x0000);
  tft.setCursor(15, 218);
  tft.print("Drag across candles to inspect OHLC metrics");
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_AUDIO, OUTPUT);
  tft.init();
  tft.setRotation(1);
  touch.begin();
  drawChart();
}

void loop() {
  if (touch.available()) {
    int sx = touch.data.y;
    int idx = (sx - 20) / 18;
    if (idx >= 0 && idx < NUM_CANDLES && idx != inspectIdx) {
      inspectIdx = idx;
      tone(PIN_AUDIO, 2000, 10);
      drawChart();
    }
  }
}
