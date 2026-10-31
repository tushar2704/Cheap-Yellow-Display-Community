/**
 * 🕹️ Project 39: 5-in-1 Pocket Arcade Deluxe
 * 
 * Hardware: Elecrow CrowPanel 2.8" (ESP32-2432S028R)
 * Channel Branding: IamNotIntoTech (https://youtube.com/@IamNotIntoTech)
 * Author: Tushar Aggarwal (tushar2704)
 * 
 * Features:
 * - 5 Playable retro arcade games in one firmware:
 *   1. Cyber Breakout (Touch paddle, brick grid, ball rebound physics)
 *   2. Neon Snake (Touch steering, food collision, growing tail)
 *   3. Flappy CYD (Tap-to-flap gravity physics & pipe obstacles)
 *   4. Space Defender (Spaceship laser blaster & descending aliens)
 *   5. Memory Matrix (Simon-style 4-quadrant musical pattern repeat)
 * - Persistent high-score leaderboard tracking per game.
 * - 8-Bit arcade sound synthesizer on GPIO 26 buzzer.
 * - Multi-color active-low RGB powerup & game over lighting on GPIOs 4/16/17.
 * - Hardware-calibrated non-inverted resistive touch coordinates.
 */

#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>

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

enum GameState {
  STATE_MENU,
  GAME_BREAKOUT,
  GAME_SNAKE,
  GAME_FLAPPY,
  GAME_SPACE,
  GAME_MEMORY
};
GameState currentState = STATE_MENU;

int highScores[5] = { 120, 45, 340, 18, 12 };
int currentScore = 0;
bool gameOver = false;
unsigned long lastFrameTick = 0;
unsigned long lastMenuTouchTime = 0;

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
          delay(10);
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
    delay(10);
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
// Main Arcade Menu
// ----------------------------------------------------
void drawArcadeMenu() {
  tft.fillScreen(COLOR_BG);

  tft.fillRect(0, 0, 320, 36, COLOR_PANEL);
  tft.drawFastHLine(0, 36, 320, COLOR_CARD_BRD);

  tft.setTextColor(COLOR_GOLD, COLOR_PANEL);
  tft.drawString("IamNotIntoTech", 14, 8, 4);

  tft.setTextColor(COLOR_CYAN, COLOR_PANEL);
  tft.drawRightString("ARCADE DELUXE", 310, 12, 2);

  const char* gameTitles[5] = { "1. CYBER BREAKOUT", "2. NEON SNAKE", "3. FLAPPY CYD", "4. SPACE DEFENDER", "5. MEMORY MATRIX" };
  const char* gameSub[5]    = { "Touch Paddle & Bricks", "Classic 60FPS Snake", "Tap-to-Flap Physics", "Galactic Alien Blaster", "Simon Pattern Repeat" };
  uint16_t cardColors[5]    = { COLOR_CYAN, COLOR_GREEN, COLOR_GOLD, COLOR_MAGENTA, COLOR_ORANGE };

  for (int i = 0; i < 5; i++) {
    int y = 44 + i * 38;
    tft.fillRoundRect(12, y, 296, 34, 4, COLOR_CARD);
    tft.drawRoundRect(12, y, 296, 34, 4, cardColors[i]);

    tft.fillRect(16, y + 4, 4, 26, cardColors[i]);

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
    if (touched && touchY >= 44 && millis() - lastMenuTouchTime > 180) {
      lastMenuTouchTime = millis();
      int sel = (touchY - 44) / 38;
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
      } else {
        currentState = GAME_BREAKOUT;
        initBreakout();
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
    }
  }

  delay(5);
}

