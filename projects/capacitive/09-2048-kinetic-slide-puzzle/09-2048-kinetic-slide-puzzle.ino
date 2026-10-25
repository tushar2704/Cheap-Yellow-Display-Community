/*
 * CYD 2.8" Capacitive (ESP32-2432S028C) - 09: 2048 Kinetic Slide Puzzle
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

int board[4][4];
int score = 0;

uint16_t getTileColor(int val) {
  switch (val) {
    case 2:    return 0xCE79;
    case 4:    return 0xCE55;
    case 8:    return 0xF4AB;
    case 16:   return 0xF3E6;
    case 32:   return 0xF324;
    case 64:   return 0xF240;
    case 128:  return 0xEE47;
    case 256:  return 0xEE05;
    case 512:  return 0x05FD;
    case 1024: return 0x915F;
    case 2048: return 0x07E0;
    default:   return 0x2126;
  }
}

void spawnRandom() {
  int emptyR[16], emptyC[16], count = 0;
  for (int r = 0; r < 4; r++) {
    for (int c = 0; c < 4; c++) {
      if (board[r][c] == 0) { emptyR[count] = r; emptyC[count] = c; count++; }
    }
  }
  if (count > 0) {
    int idx = random(0, count);
    board[emptyR[idx]][emptyC[idx]] = (random(0, 10) == 0) ? 4 : 2;
  }
}

void drawBoard() {
  tft.fillRect(0, 0, 320, 240, 0x18C3);
  
  // Header
  tft.setTextColor(TFT_WHITE, 0x18C3);
  tft.setTextSize(2);
  tft.setCursor(20, 8);
  tft.print("2048");

  tft.setTextSize(1);
  tft.setCursor(180, 12);
  tft.printf("SCORE: %d", score);

  // 4x4 Grid
  int startX = 55;
  int startY = 32;
  int tileSize = 48;
  int pad = 5;

  for (int r = 0; r < 4; r++) {
    for (int c = 0; c < 4; c++) {
      int tx = startX + c * (tileSize + pad);
      int ty = startY + r * (tileSize + pad);
      int val = board[r][c];
      uint16_t col = getTileColor(val);

      tft.fillRoundRect(tx, ty, tileSize, tileSize, 4, col);
      if (val > 0) {
        tft.setTextColor(val <= 4 ? 0x0000 : TFT_WHITE, col);
        tft.setTextSize(val >= 1000 ? 1 : 2);
        char buf[8];
        snprintf(buf, sizeof(buf), "%d", val);
        int tw = strlen(buf) * (val >= 1000 ? 6 : 12);
        tft.setCursor(tx + (tileSize - tw) / 2, ty + (tileSize - 14) / 2);
        tft.print(buf);
      }
    }
  }
}

bool slideLeft() {
  bool moved = false;
  for (int r = 0; r < 4; r++) {
    int target = 0;
    for (int c = 1; c < 4; c++) {
      if (board[r][c] != 0) {
        int val = board[r][c];
        board[r][c] = 0;
        if (board[r][target] == 0) {
          board[r][target] = val;
          moved = true;
        } else if (board[r][target] == val) {
          board[r][target] *= 2;
          score += board[r][target];
          target++;
          moved = true;
        } else {
          target++;
          board[r][target] = val;
          if (target != c) moved = true;
        }
      }
    }
  }
  return moved;
}

void rotateBoard() {
  int temp[4][4];
  for (int r = 0; r < 4; r++) {
    for (int c = 0; c < 4; c++) temp[c][3 - r] = board[r][c];
  }
  memcpy(board, temp, sizeof(board));
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_AUDIO, OUTPUT);
  tft.init();
  tft.setRotation(1);
  touch.begin();

  memset(board, 0, sizeof(board));
  spawnRandom();
  spawnRandom();
  drawBoard();
}

void loop() {
  if (touch.available()) {
    uint8_t g = touch.data.gestureID;
    bool moved = false;
    // 0x03=Left, 0x04=Right, 0x01=Up, 0x02=Down
    if (g == 0x03) { // Left
      moved = slideLeft();
    } else if (g == 0x04) { // Right
      rotateBoard(); rotateBoard();
      moved = slideLeft();
      rotateBoard(); rotateBoard();
    } else if (g == 0x01) { // Up
      rotateBoard(); rotateBoard(); rotateBoard();
      moved = slideLeft();
      rotateBoard();
    } else if (g == 0x02) { // Down
      rotateBoard();
      moved = slideLeft();
      rotateBoard(); rotateBoard(); rotateBoard();
    }

    if (moved) {
      tone(PIN_AUDIO, 1800, 20);
      spawnRandom();
      drawBoard();
      delay(150);
    }
  }
}
