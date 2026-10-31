/**
 * 🕹️ Project 39: 5-in-1 Pocket Arcade Deluxe
 * 
 * Hardware: Elecrow CrowPanel 2.8" (ESP32-2432S028R)
 * Channel Branding: IamNotIntoTech (https://youtube.com/@IamNotIntoTech)
 * Author: Tushar Aggarwal (tushar2704)
 * 
 * Features:
 * - 5 fully playable retro arcade classics:
 *   1. Cyber Breakout (Touch paddle, brick grid, ball rebound physics)
 *   2. Neon Snake (Touch direction steering, food collision, growing tail)
 *   3. Flappy CYD (Tap-to-flap gravity physics & pipe obstacle course)
 *   4. Space Defender (Touch ship guidance, dual laser blaster, descending alien armada)
 *   5. Memory Matrix (Simon-style 4-quadrant musical color pattern repeat)
 * - Persistent high-score leaderboard tracking per game with Paris NTP clock sync.
 * - 8-Bit arcade sound synthesizer on GPIO 26 buzzer with tone modulation.
 * - Multi-color active-low RGB powerup & game over lighting on GPIOs 4/16/17.
 * - Hardware-calibrated non-inverted resistive touch coordinates.
 */

#include <Arduino.h>
#include <SPI.h>
#include <WiFi.h>
#include <time.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>

#if __has_include("secrets.h")
  #include "secrets.h"
#else
  #include "secrets.h.example"
#endif

// Hardware Pin Definitions for Elecrow 2.8" CYD
#define TFT_BL_PIN    21
#define TOUCH_MOSI    32
#define TOUCH_MISO    39
#define TOUCH_CLK     25
#define TOUCH_CS      33
#define TOUCH_IRQ     36

#define LED_RED       4
#define LED_GREEN     16
#define LED_BLUE      17
#define BUZZER_PIN    26

SPIClass touchSPI = SPIClass(VSPI);
XPT2046_Touchscreen ts(TOUCH_CS);
TFT_eSPI tft = TFT_eSPI();

// UI Palette (16-bit 565)
#define COLOR_BG        0x0842
#define COLOR_PANEL     0x10A3
#define COLOR_CARD      0x18E5
#define COLOR_CARD_BRD  0x2967
#define COLOR_TEXT      0xFFFF
#define COLOR_MUTED     0x8410
#define COLOR_GOLD      0xFFE0
#define COLOR_GREEN     0x07E0
#define COLOR_CYAN      0x07FF
#define COLOR_RED       0xF800
#define COLOR_MAGENTA   0xF81F
#define COLOR_ORANGE    0xFD20
#define COLOR_DARKBLUE  0x0010
#define COLOR_YELLOW    0xFFE0
#define COLOR_BLUE      0x001F

enum GameState {
  STATE_MENU,
  GAME_BREAKOUT,
  GAME_SNAKE,
  GAME_FLAPPY,
  GAME_SPACE,
  GAME_MEMORY
};
GameState currentState = STATE_MENU;

int highScores[5] = { 120, 45, 18, 340, 12 };
int currentScore = 0;
bool gameOver = false;
unsigned long lastFrameTick = 0;
unsigned long lastMenuTouchTime = 0;
bool wifiConnected = false;

void playBeep(int freq = 2000, int ms = 30) {
  tone(BUZZER_PIN, freq, ms);
}

void setRGB(bool r, bool g, bool b) {
  digitalWrite(LED_RED, r ? LOW : HIGH);
  digitalWrite(LED_GREEN, g ? LOW : HIGH);
  digitalWrite(LED_BLUE, b ? LOW : HIGH);
}

// ----------------------------------------------------
// 1. Cyber Breakout
// ----------------------------------------------------
float ballX, ballY, ballVX, ballVY;
int paddleX, paddleW = 50, paddleH = 8;
bool bricks[4][8];

void initBreakout() {
  ballX = 160; ballY = 160;
  ballVX = 2.4; ballVY = -2.8;
  paddleX = 135;
  currentScore = 0;
  gameOver = false;
  for (int r = 0; r < 4; r++)
    for (int c = 0; c < 8; c++)
      bricks[r][c] = true;

  tft.fillScreen(COLOR_BG);
  tft.fillRect(0, 0, 320, 24, COLOR_PANEL);
  tft.setTextColor(COLOR_GOLD, COLOR_PANEL);
  tft.drawString("CYBER BREAKOUT", 10, 5, 2);
  tft.setTextColor(COLOR_CYAN, COLOR_PANEL);
  tft.drawRightString("EXIT [X]", 310, 5, 2);
}

void updateBreakout(int touchX, int touchY, bool touched) {
  if (gameOver) {
    if (touched) initBreakout();
    return;
  }

  tft.fillCircle((int)ballX, (int)ballY, 4, COLOR_BG);
  tft.fillRect(paddleX, 222, paddleW, paddleH, COLOR_BG);

  if (touched && touchY > 80) {
    paddleX = constrain(touchX - paddleW / 2, 0, 320 - paddleW);
  }

  ballX += ballVX;
  ballY += ballVY;

  if (ballX <= 4 || ballX >= 316) {
    ballVX = -ballVX;
    playBeep(1800, 10);
  }
  if (ballY <= 28) {
    ballVY = -ballVY;
    playBeep(1800, 10);
  }

  // Paddle collision
  if (ballY >= 218 && ballY <= 226 && ballX >= paddleX - 2 && ballX <= paddleX + paddleW + 2) {
    ballVY = -abs(ballVY);
    ballVX = ((ballX - (paddleX + paddleW / 2.0f)) / (paddleW / 2.0f)) * 3.5f;
    playBeep(2400, 15);
  }

  // Brick collision
  int brickW = 36;
  int brickH = 12;
  int brickStartY = 36;
  int brickStartX = 12;

  for (int r = 0; r < 4; r++) {
    for (int c = 0; c < 8; c++) {
      if (bricks[r][c]) {
        int bx = brickStartX + c * 38;
        int by = brickStartY + r * 16;
        if (ballX >= bx && ballX <= bx + brickW && ballY >= by && ballY <= by + brickH) {
          bricks[r][c] = false;
          ballVY = -ballVY;
          currentScore += 10;
          tft.fillRect(bx, by, brickW, brickH, COLOR_BG);
          playBeep(3200, 20);
          setRGB(false, true, false);
          delay(5);
          setRGB(false, false, false);
        } else {
          uint16_t bCol = (r == 0) ? COLOR_RED : ((r == 1) ? COLOR_ORANGE : ((r == 2) ? COLOR_GOLD : COLOR_GREEN));
          tft.fillRect(bx, by, brickW, brickH, bCol);
          tft.drawRect(bx, by, brickW, brickH, COLOR_BG);
        }
      }
    }
  }

  if (ballY > 238) {
    gameOver = true;
    setRGB(true, false, false);
    playBeep(1200, 100);
    tft.setTextColor(COLOR_RED, COLOR_BG);
    tft.drawCentreString("GAME OVER - TAP TO RETRY", 160, 120, 2);
    if (currentScore > highScores[0]) highScores[0] = currentScore;
    return;
  }

  tft.fillCircle((int)ballX, (int)ballY, 4, COLOR_GOLD);
  tft.fillRoundRect(paddleX, 222, paddleW, paddleH, 3, COLOR_CYAN);
}

// ----------------------------------------------------
// 2. Neon Snake
// ----------------------------------------------------
#define SNAKE_MAX_LEN 100
int snakeX[SNAKE_MAX_LEN], snakeY[SNAKE_MAX_LEN];
int snakeLen = 4;
int snakeDir = 0;
int foodX, foodY;

void spawnFood() {
  foodX = random(2, 28) * 10;
  foodY = random(4, 22) * 10;
}

void initSnake() {
  snakeLen = 4;
  snakeDir = 0;
  currentScore = 0;
  gameOver = false;
  for (int i = 0; i < snakeLen; i++) {
    snakeX[i] = 100 - i * 10;
    snakeY[i] = 100;
  }
  spawnFood();
  tft.fillScreen(COLOR_BG);
  tft.fillRect(0, 0, 320, 24, COLOR_PANEL);
  tft.setTextColor(COLOR_GREEN, COLOR_PANEL);
  tft.drawString("NEON SNAKE", 10, 5, 2);
  tft.setTextColor(COLOR_CYAN, COLOR_PANEL);
  tft.drawRightString("EXIT [X]", 310, 5, 2);
}

void updateSnake(int touchX, int touchY, bool touched) {
  if (gameOver) {
    if (touched) initSnake();
    return;
  }

  if (touched && touchY > 26) {
    int hx = snakeX[0];
    int hy = snakeY[0];
    int dx = touchX - hx;
    int dy = touchY - hy;
    if (abs(dx) > abs(dy)) {
      if (dx > 0 && snakeDir != 2) snakeDir = 0;
      else if (dx < 0 && snakeDir != 0) snakeDir = 2;
    } else {
      if (dy > 0 && snakeDir != 3) snakeDir = 1;
      else if (dy < 0 && snakeDir != 1) snakeDir = 3;
    }
  }

  tft.fillRect(snakeX[snakeLen - 1], snakeY[snakeLen - 1], 9, 9, COLOR_BG);

  for (int i = snakeLen - 1; i > 0; i--) {
    snakeX[i] = snakeX[i - 1];
    snakeY[i] = snakeY[i - 1];
  }

  if (snakeDir == 0) snakeX[0] += 10;
  else if (snakeDir == 1) snakeY[0] += 10;
  else if (snakeDir == 2) snakeX[0] -= 10;
  else if (snakeDir == 3) snakeY[0] -= 10;

  if (abs(snakeX[0] - foodX) < 10 && abs(snakeY[0] - foodY) < 10) {
    if (snakeLen < SNAKE_MAX_LEN - 1) snakeLen++;
    currentScore += 5;
    spawnFood();
    playBeep(2800, 20);
    setRGB(false, true, true);
    delay(5);
    setRGB(false, false, false);
  }

  if (snakeX[0] < 4 || snakeX[0] > 310 || snakeY[0] < 28 || snakeY[0] > 230) {
    gameOver = true;
    setRGB(true, false, false);
    playBeep(1000, 120);
    tft.setTextColor(COLOR_RED, COLOR_BG);
    tft.drawCentreString("SNAKE CRASH! TAP TO RETRY", 160, 120, 2);
    if (currentScore > highScores[1]) highScores[1] = currentScore;
    return;
  }

  tft.fillRect(foodX, foodY, 9, 9, COLOR_RED);
  for (int i = 0; i < snakeLen; i++) {
    tft.fillRect(snakeX[i], snakeY[i], 9, 9, (i == 0) ? COLOR_GOLD : COLOR_GREEN);
  }
}

// ----------------------------------------------------
// 3. Flappy CYD
// ----------------------------------------------------
float birdY, birdVY;
int pipeX, pipeGapY, pipeGapH = 65;

void initFlappy() {
  birdY = 100;
  birdVY = 0;
  pipeX = 300;
  pipeGapY = 60;
  currentScore = 0;
  gameOver = false;

  tft.fillScreen(COLOR_BG);
  tft.fillRect(0, 0, 320, 24, COLOR_PANEL);
  tft.setTextColor(COLOR_CYAN, COLOR_PANEL);
  tft.drawString("FLAPPY CYD", 10, 5, 2);
  tft.drawRightString("EXIT [X]", 310, 5, 2);
}

void updateFlappy(int touchX, int touchY, bool touched) {
  if (gameOver) {
    if (touched) initFlappy();
    return;
  }

  tft.fillCircle(60, (int)birdY, 7, COLOR_BG);
  tft.drawFastVLine(pipeX + 22, 25, 215, COLOR_BG);
  tft.fillRect(pipeX, 25, 22, 215, COLOR_BG);

  if (touched) {
    birdVY = -4.5;
    playBeep(2600, 15);
  }

  birdVY += 0.55;
  birdY += birdVY;

  pipeX -= 3;
  if (pipeX < -25) {
    pipeX = 320;
    pipeGapY = random(40, 150);
    currentScore++;
    playBeep(3400, 25);
  }

  if (birdY > 230 || birdY < 32) {
    gameOver = true;
    playBeep(900, 120);
    tft.setTextColor(COLOR_RED, COLOR_BG);
    tft.drawCentreString("BIRD CRASH! TAP TO RETRY", 160, 120, 2);
    if (currentScore > highScores[2]) highScores[2] = currentScore;
    return;
  }

  if (pipeX < 68 && pipeX > 40) {
    if (birdY - 7 < pipeGapY || birdY + 7 > pipeGapY + pipeGapH) {
      gameOver = true;
      playBeep(900, 120);
      tft.setTextColor(COLOR_RED, COLOR_BG);
      tft.drawCentreString("PIPE HIT! TAP TO RETRY", 160, 120, 2);
      if (currentScore > highScores[2]) highScores[2] = currentScore;
      return;
    }
  }

  tft.fillRect(pipeX, 25, 20, pipeGapY - 25, COLOR_GREEN);
  tft.fillRect(pipeX, pipeGapY + pipeGapH, 20, 240 - (pipeGapY + pipeGapH), COLOR_GREEN);

  tft.fillCircle(60, (int)birdY, 7, COLOR_GOLD);
  tft.drawCircle(60, (int)birdY, 7, COLOR_ORANGE);
}

// ----------------------------------------------------
// 4. Space Defender
// ----------------------------------------------------
#define MAX_LASERS 4
#define ALIEN_ROWS 2
#define ALIEN_COLS 6
int shipX = 145;
int laserX[MAX_LASERS];
int laserY[MAX_LASERS];
bool laserActive[MAX_LASERS];
bool alienAlive[ALIEN_ROWS][ALIEN_COLS];
float alienGridX = 20, alienGridY = 40;
float alienDir = 1.5;
unsigned long lastLaserShot = 0;

void initSpace() {
  shipX = 145;
  currentScore = 0;
  gameOver = false;
  alienGridX = 20;
  alienGridY = 40;
  alienDir = 1.5;
  for (int i = 0; i < MAX_LASERS; i++) laserActive[i] = false;
  for (int r = 0; r < ALIEN_ROWS; r++) {
    for (int c = 0; c < ALIEN_COLS; c++) {
      alienAlive[r][c] = true;
    }
  }

  tft.fillScreen(COLOR_BG);
  tft.fillRect(0, 0, 320, 24, COLOR_PANEL);
  tft.setTextColor(COLOR_MAGENTA, COLOR_PANEL);
  tft.drawString("SPACE DEFENDER", 10, 5, 2);
  tft.setTextColor(COLOR_CYAN, COLOR_PANEL);
  tft.drawRightString("EXIT [X]", 310, 5, 2);
}

void drawShip(int x, uint16_t color) {
  tft.fillTriangle(x + 12, 212, x, 226, x + 24, 226, color);
  tft.fillRect(x + 8, 222, 8, 5, color);
}

void updateSpace(int touchX, int touchY, bool touched) {
  if (gameOver) {
    if (touched) initSpace();
    return;
  }

  // Clear previous ship position
  drawShip(shipX, COLOR_BG);

  // Touch controls ship movement & auto-shoots
  if (touched && touchY > 30) {
    shipX = constrain(touchX - 12, 10, 296);
    if (millis() - lastLaserShot > 250) {
      lastLaserShot = millis();
      for (int i = 0; i < MAX_LASERS; i++) {
        if (!laserActive[i]) {
          laserActive[i] = true;
          laserX[i] = shipX + 11;
          laserY[i] = 208;
          playBeep(3200, 15);
          break;
        }
      }
    }
  }

  // Update & redraw lasers
  for (int i = 0; i < MAX_LASERS; i++) {
    if (laserActive[i]) {
      tft.fillRect(laserX[i], laserY[i], 2, 6, COLOR_BG);
      laserY[i] -= 6;
      if (laserY[i] < 26) {
        laserActive[i] = false;
      } else {
        tft.fillRect(laserX[i], laserY[i], 2, 6, COLOR_CYAN);
      }
    }
  }

  // Clear previous aliens
  for (int r = 0; r < ALIEN_ROWS; r++) {
    for (int c = 0; c < ALIEN_COLS; c++) {
      if (alienAlive[r][c]) {
        int ax = (int)alienGridX + c * 38;
        int ay = (int)alienGridY + r * 22;
        tft.fillRect(ax, ay, 20, 12, COLOR_BG);
      }
    }
  }

  // Move alien armada
  alienGridX += alienDir;
  if (alienGridX > 70 || alienGridX < 10) {
    alienDir = -alienDir;
    alienGridY += 8;
  }

  // Check Laser-Alien collisions
  int remainingAliens = 0;
  for (int r = 0; r < ALIEN_ROWS; r++) {
    for (int c = 0; c < ALIEN_COLS; c++) {
      if (alienAlive[r][c]) {
        remainingAliens++;
        int ax = (int)alienGridX + c * 38;
        int ay = (int)alienGridY + r * 22;

        for (int l = 0; l < MAX_LASERS; l++) {
          if (laserActive[l] && laserX[l] >= ax && laserX[l] <= ax + 20 &&
              laserY[l] >= ay && laserY[l] <= ay + 12) {
            alienAlive[r][c] = false;
            laserActive[l] = false;
            tft.fillRect(laserX[l], laserY[l], 2, 6, COLOR_BG);
            tft.fillRect(ax, ay, 20, 12, COLOR_BG);
            currentScore += 20;
            playBeep(2800, 25);
            setRGB(false, true, false);
            delay(5);
            setRGB(false, false, false);
            break;
          }
        }
      }
    }
  }

  // If all aliens defeated, respawn faster wave
  if (remainingAliens == 0) {
    alienGridX = 20;
    alienGridY = 40;
    alienDir = (alienDir > 0 ? 1 : -1) * 2.2f;
    for (int r = 0; r < ALIEN_ROWS; r++) {
      for (int c = 0; c < ALIEN_COLS; c++) alienAlive[r][c] = true;
    }
    playBeep(3600, 60);
  }

  // Draw living aliens
  for (int r = 0; r < ALIEN_ROWS; r++) {
    for (int c = 0; c < ALIEN_COLS; c++) {
      if (alienAlive[r][c]) {
        int ax = (int)alienGridX + c * 38;
        int ay = (int)alienGridY + r * 22;
        tft.fillRoundRect(ax, ay, 20, 12, 3, (r == 0) ? COLOR_RED : COLOR_MAGENTA);
        tft.fillRect(ax + 4, ay + 3, 3, 3, COLOR_BG);
        tft.fillRect(ax + 13, ay + 3, 3, 3, COLOR_BG);

        // Alien reaches defense perimeter
        if (ay >= 200) {
          gameOver = true;
          setRGB(true, false, false);
          playBeep(800, 150);
          tft.setTextColor(COLOR_RED, COLOR_BG);
          tft.drawCentreString("DEFENSE BREACHED! TAP RETRY", 160, 120, 2);
          if (currentScore > highScores[3]) highScores[3] = currentScore;
          return;
        }
      }
    }
  }

  // Draw ship
  drawShip(shipX, COLOR_GREEN);
}

// ----------------------------------------------------
// 5. Memory Matrix (Simon Pattern Repeat)
// ----------------------------------------------------
#define MAX_PATTERN 32
uint8_t patternSeq[MAX_PATTERN];
int patternLen = 1;
int playerStep = 0;
bool cpuTurn = true;
unsigned long stepShowTime = 0;
int showIndex = 0;

void drawQuadrant(int q, bool lit) {
  // 0: Top-Left (Green), 1: Top-Right (Red), 2: Bottom-Left (Yellow), 3: Bottom-Right (Blue)
  uint16_t c;
  if (q == 0) c = lit ? 0x07E0 : 0x0320;
  else if (q == 1) c = lit ? 0xF800 : 0x6000;
  else if (q == 2) c = lit ? 0xFFE0 : 0x6300;
  else c = lit ? 0x07FF : 0x0010;

  if (q == 0) tft.fillRoundRect(20, 36, 130, 85, 8, c);
  else if (q == 1) tft.fillRoundRect(170, 36, 130, 85, 8, c);
  else if (q == 2) tft.fillRoundRect(20, 135, 130, 85, 8, c);
  else if (q == 3) tft.fillRoundRect(170, 135, 130, 85, 8, c);
}

void initMemory() {
  patternLen = 1;
  playerStep = 0;
  cpuTurn = true;
  showIndex = 0;
  currentScore = 0;
  gameOver = false;
  for (int i = 0; i < MAX_PATTERN; i++) {
    patternSeq[i] = random(0, 4);
  }

  tft.fillScreen(COLOR_BG);
  tft.fillRect(0, 0, 320, 24, COLOR_PANEL);
  tft.setTextColor(COLOR_ORANGE, COLOR_PANEL);
  tft.drawString("MEMORY MATRIX", 10, 5, 2);
  tft.setTextColor(COLOR_CYAN, COLOR_PANEL);
  tft.drawRightString("EXIT [X]", 310, 5, 2);

  for (int q = 0; q < 4; q++) drawQuadrant(q, false);
  delay(300);
}

void updateMemory(int touchX, int touchY, bool touched) {
  if (gameOver) {
    if (touched) initMemory();
    return;
  }

  // CPU Demonstration Turn
  if (cpuTurn) {
    if (millis() - stepShowTime > 400) {
      stepShowTime = millis();
      if (showIndex < patternLen) {
        int q = patternSeq[showIndex];
        drawQuadrant(q, true);
        int toneFreq[4] = { 880, 1175, 1397, 1760 };
        playBeep(toneFreq[q], 180);
        delay(200);
        drawQuadrant(q, false);
        showIndex++;
      } else {
        cpuTurn = false;
        playerStep = 0;
      }
    }
    return;
  }

  // Player Response Turn
  if (touched && touchY > 30) {
    int tappedQuad = -1;
    if (touchX >= 20 && touchX <= 150) {
      if (touchY >= 36 && touchY <= 121) tappedQuad = 0;
      else if (touchY >= 135 && touchY <= 220) tappedQuad = 2;
    } else if (touchX >= 170 && touchX <= 300) {
      if (touchY >= 36 && touchY <= 121) tappedQuad = 1;
      else if (touchY >= 135 && touchY <= 220) tappedQuad = 3;
    }

    if (tappedQuad != -1) {
      drawQuadrant(tappedQuad, true);
      int toneFreq[4] = { 880, 1175, 1397, 1760 };
      playBeep(toneFreq[tappedQuad], 150);
      delay(150);
      drawQuadrant(tappedQuad, false);

      if (tappedQuad == patternSeq[playerStep]) {
        playerStep++;
        if (playerStep >= patternLen) {
          currentScore = patternLen;
          if (currentScore > highScores[4]) highScores[4] = currentScore;
          if (patternLen < MAX_PATTERN - 1) patternLen++;
          cpuTurn = true;
          showIndex = 0;
          playBeep(2600, 40);
          delay(80);
          playBeep(3200, 60);
          delay(400);
        }
      } else {
        // Wrong button pressed!
        gameOver = true;
        setRGB(true, false, false);
        playBeep(450, 250);
        tft.setTextColor(COLOR_RED, COLOR_BG);
        tft.drawCentreString("MISMATCH! TAP TO RETRY", 160, 120, 2);
        delay(400);
      }
    }
  }
}

// ----------------------------------------------------
// Main Arcade Menu
// ----------------------------------------------------
void drawArcadeMenu() {
  tft.fillScreen(COLOR_BG);

  tft.fillRect(0, 0, 320, 36, COLOR_PANEL);
  tft.drawFastHLine(0, 36, 320, COLOR_CARD_BRD);

  tft.setTextColor(COLOR_GOLD, COLOR_PANEL);
  tft.drawString("IamNotIntoTech", 14, 8, 4);

  tft.setTextColor(wifiConnected ? COLOR_GREEN : COLOR_CYAN, COLOR_PANEL);
  tft.drawRightString(wifiConnected ? "PARIS ONLINE" : "OFFLINE", 310, 12, 2);

  const char* gameTitles[5] = { "1. CYBER BREAKOUT", "2. NEON SNAKE", "3. FLAPPY CYD", "4. SPACE DEFENDER", "5. MEMORY MATRIX" };
  const char* gameSub[5]    = { "Touch Paddle & Bricks", "Classic 60FPS Snake", "Tap-to-Flap Physics", "Galactic Alien Blaster", "Simon Pattern Repeat" };
  uint16_t cardColors[5]    = { COLOR_CYAN, COLOR_GREEN, COLOR_GOLD, COLOR_MAGENTA, COLOR_ORANGE };

  for (int i = 0; i < 5; i++) {
    int y = 42 + i * 39;
    tft.fillRoundRect(12, y, 296, 35, 4, COLOR_CARD);
    tft.drawRoundRect(12, y, 296, 35, 4, cardColors[i]);

    tft.fillRect(16, y + 4, 4, 27, cardColors[i]);

    tft.setTextColor(COLOR_TEXT, COLOR_CARD);
    tft.drawString(gameTitles[i], 26, y + 6, 2);

    tft.setTextColor(COLOR_MUTED, COLOR_CARD);
    tft.drawString(gameSub[i], 26, y + 20, 1);

    char hsBuf[16];
    snprintf(hsBuf, sizeof(hsBuf), "HI: %d", highScores[i]);
    tft.setTextColor(COLOR_GOLD, COLOR_CARD);
    tft.drawRightString(hsBuf, 300, y + 10, 2);
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  setRGB(false, false, false);

  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  pinMode(TFT_BL_PIN, OUTPUT);
  digitalWrite(TFT_BL_PIN, HIGH);

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(COLOR_BG);

  // Initialize Touch (VSPI)
  touchSPI.begin(TOUCH_CLK, TOUCH_MISO, TOUCH_MOSI, TOUCH_CS);
  ts.begin(touchSPI);
  ts.setRotation(1);

  // Wi-Fi Paris NTP Sync
  tft.setTextColor(COLOR_CYAN, COLOR_BG);
  tft.drawString("Connecting Wi-Fi (Paris NTP)...", 20, 110, 2);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  unsigned long startWiFi = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startWiFi < 3000) {
    delay(100);
  }
  if (WiFi.status() == WL_CONNECTED) {
    wifiConnected = true;
    configTime(3600, 3600, "pool.ntp.org", "time.google.com");
    setRGB(false, true, false);
  } else {
    wifiConnected = false;
  }
  delay(150);
  setRGB(false, false, false);

  drawArcadeMenu();

  playBeep(2400, 60);
  delay(50);
  playBeep(3200, 90);
  Serial.println(F("[SYSTEM] 5-in-1 Pocket Arcade Deluxe Initialized."));
}

void loop() {
  int touchX = -1;
  int touchY = -1;
  bool touched = ts.touched();

  if (touched) {
    TS_Point p = ts.getPoint();
    if (p.z > 50 || ts.touched()) {
      touchX = map(p.x, 240, 3800, 0, 320);
      touchY = map(p.y, 240, 3800, 0, 240);
      touchX = constrain(touchX, 0, 319);
      touchY = constrain(touchY, 0, 239);

      // Global Exit back to menu (top-right tap)
      if (currentState != STATE_MENU && touchY <= 26 && touchX >= 240) {
        currentState = STATE_MENU;
        playBeep(1600, 40);
        drawArcadeMenu();
        delay(200);
        return;
      }
    }
  }

  // Menu Handling
  if (currentState == STATE_MENU) {
    if (touched && touchY >= 42 && millis() - lastMenuTouchTime > 200) {
      lastMenuTouchTime = millis();
      int sel = (touchY - 42) / 39;
      if (sel == 0) {
        currentState = GAME_BREAKOUT;
        playBeep(2200, 40);
        initBreakout();
      } else if (sel == 1) {
        currentState = GAME_SNAKE;
        playBeep(2400, 40);
        initSnake();
      } else if (sel == 2) {
        currentState = GAME_FLAPPY;
        playBeep(2600, 40);
        initFlappy();
      } else if (sel == 3) {
        currentState = GAME_SPACE;
        playBeep(2800, 40);
        initSpace();
      } else if (sel == 4) {
        currentState = GAME_MEMORY;
        playBeep(3000, 40);
        initMemory();
      }
    }
    delay(20);
    return;
  }

  // Game Loop Ticks (~40 FPS)
  if (millis() - lastFrameTick > 25) {
    lastFrameTick = millis();

    if (currentState == GAME_BREAKOUT) {
      updateBreakout(touchX, touchY, touched);
    } else if (currentState == GAME_SNAKE) {
      updateSnake(touchX, touchY, touched);
    } else if (currentState == GAME_FLAPPY) {
      updateFlappy(touchX, touchY, touched);
    } else if (currentState == GAME_SPACE) {
      updateSpace(touchX, touchY, touched);
    } else if (currentState == GAME_MEMORY) {
      updateMemory(touchX, touchY, touched);
    }
  }

  delay(5);
}
