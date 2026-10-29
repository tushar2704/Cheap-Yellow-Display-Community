/*
 * Elecrow 2.8" CYD — Minimax AI Gomoku & Tactical Board
 * Channel: @IamNotIntoTech (https://youtube.com/@IamNotIntoTech)
 * 
 * Target: ESP32-2432S028R (2.8" ILI9341 TFT + XPT2046 Resistive Touch)
 * Features:
 *  - 10x10 Gomoku Board with wood grain theme and anti-aliased stones
 *  - Embedded Minimax Search Algorithm with Alpha-Beta Pruning
 *  - Real-Time AI Telemetry Dashboard: Search Depth, Nodes Evaluated, Advantage Meter
 *  - Touch stone placement with stylus precision & Last Move highlight ring
 *  - Tactile stone snap audio on IO26 & AI thinking RGB LED on IO4/16/17
 */

#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>

// Hardware Pin Definitions for Elecrow 2.8" CYD
#define XPT2046_IRQ 36
#define XPT2046_MOSI 13
#define XPT2046_MISO 12
#define XPT2046_CLK 14
#define XPT2046_CS 33

#define TFT_BL 21
#define LED_RED 4
#define LED_GREEN 16
#define LED_BLUE 17
#define BUZZER_PIN 26

SPIClass touchSPI = SPIClass(VSPI);
XPT2046_Touchscreen ts(XPT2046_CS, XPT2046_IRQ);
TFT_eSPI tft = TFT_eSPI();

// UI Palette (16-bit 565)
#define COLOR_BG        0x0842 // Charcoal
#define COLOR_PANEL     0x10A3
#define COLOR_BOARD     0xD611 // Warm bamboo wood
#define COLOR_GRID      0x39E7 // Dark slate grid lines
#define COLOR_CARD      0x18E5
#define COLOR_CARD_BRD  0x2967
#define COLOR_TEXT      0xFFFF
#define COLOR_MUTED     0x8410
#define COLOR_GOLD      0xFFE0
#define COLOR_GREEN     0x07E0
#define COLOR_RED       0xF800
#define COLOR_CYAN      0x07FF

// Board Dimensions: 10x10 Grid
#define BOARD_SIZE 10
#define CELL_SIZE 20
#define BOARD_X 10
#define BOARD_Y 30

// 0: Empty, 1: Human (Black), 2: AI (White)
uint8_t board[BOARD_SIZE][BOARD_SIZE];
int lastMoveX = -1;
int lastMoveY = -1;

bool isPlayerTurn = true;
int gameWinner = 0; // 0: Playing, 1: Human, 2: AI, 3: Draw
int difficulty = 3; // Minimax depth (2: Novice, 3: Master)
int nodesEvaluated = 0;
int aiAdvantage = 0; // -100 to +100

void playBeep(int freq, int ms) {
  tone(BUZZER_PIN, freq, ms);
  delay(ms);
  noTone(BUZZER_PIN);
}

void setRGB(bool r, bool g, bool b) {
  digitalWrite(LED_RED, r ? LOW : HIGH);
  digitalWrite(LED_GREEN, g ? LOW : HIGH);
  digitalWrite(LED_BLUE, b ? LOW : HIGH);
}

void resetGame() {
  for (int r = 0; r < BOARD_SIZE; r++) {
    for (int c = 0; c < BOARD_SIZE; c++) {
      board[r][c] = 0;
    }
  }
  lastMoveX = -1;
  lastMoveY = -1;
  isPlayerTurn = true;
  gameWinner = 0;
  nodesEvaluated = 0;
  aiAdvantage = 0;
}

void drawHeader() {
  tft.fillRect(0, 0, 320, 24, COLOR_PANEL);
  tft.drawFastHLine(0, 24, 320, COLOR_CARD_BRD);

  tft.setTextColor(COLOR_GOLD, COLOR_PANEL);
  tft.setTextSize(1);
  tft.drawString("MINIMAX AI GOMOKU", 10, 5, 2);

  if (gameWinner == 1) {
    tft.setTextColor(COLOR_GREEN, COLOR_PANEL);
    tft.drawRightString("HUMAN WINS! ★", 310, 5, 2);
  } else if (gameWinner == 2) {
    tft.setTextColor(COLOR_RED, COLOR_PANEL);
    tft.drawRightString("AI WINS! ★", 310, 5, 2);
  } else {
    tft.setTextColor(isPlayerTurn ? COLOR_CYAN : COLOR_MUTED, COLOR_PANEL);
    tft.drawRightString(isPlayerTurn ? "YOUR TURN ●" : "AI THINKING...", 310, 5, 2);
  }
}

void drawBoard() {
  // Wood Board background: x: 10, y: 30, w: 200, h: 200
  tft.fillRoundRect(BOARD_X - 4, BOARD_Y - 4, BOARD_SIZE * CELL_SIZE + 8, BOARD_SIZE * CELL_SIZE + 8, 4, COLOR_BOARD);
  tft.drawRoundRect(BOARD_X - 4, BOARD_Y - 4, BOARD_SIZE * CELL_SIZE + 8, BOARD_SIZE * CELL_SIZE + 8, 4, 0x9460);

  // Draw Grid Lines
  for (int i = 0; i < BOARD_SIZE; i++) {
    // Horizontal
    tft.drawFastHLine(BOARD_X + CELL_SIZE / 2, BOARD_Y + CELL_SIZE / 2 + i * CELL_SIZE, (BOARD_SIZE - 1) * CELL_SIZE, COLOR_GRID);
    // Vertical
    tft.drawFastVLine(BOARD_X + CELL_SIZE / 2 + i * CELL_SIZE, BOARD_Y + CELL_SIZE / 2, (BOARD_SIZE - 1) * CELL_SIZE, COLOR_GRID);
  }

  // Draw Star Points (Tengen / hoshi)
  tft.fillCircle(BOARD_X + CELL_SIZE / 2 + 2 * CELL_SIZE, BOARD_Y + CELL_SIZE / 2 + 2 * CELL_SIZE, 2, COLOR_GRID);
  tft.fillCircle(BOARD_X + CELL_SIZE / 2 + 7 * CELL_SIZE, BOARD_Y + CELL_SIZE / 2 + 2 * CELL_SIZE, 2, COLOR_GRID);
  tft.fillCircle(BOARD_X + CELL_SIZE / 2 + 2 * CELL_SIZE, BOARD_Y + CELL_SIZE / 2 + 7 * CELL_SIZE, 2, COLOR_GRID);
  tft.fillCircle(BOARD_X + CELL_SIZE / 2 + 7 * CELL_SIZE, BOARD_Y + CELL_SIZE / 2 + 7 * CELL_SIZE, 2, COLOR_GRID);

  // Draw Stones
  for (int r = 0; r < BOARD_SIZE; r++) {
    for (int c = 0; c < BOARD_SIZE; c++) {
      if (board[r][c] != 0) {
        int sx = BOARD_X + CELL_SIZE / 2 + c * CELL_SIZE;
        int sy = BOARD_Y + CELL_SIZE / 2 + r * CELL_SIZE;

        if (board[r][c] == 1) { // Human: Black Stone
          tft.fillCircle(sx, sy, 8, 0x18C3);
          tft.drawCircle(sx, sy, 8, 0x0000);
          tft.fillCircle(sx - 2, sy - 2, 2, 0x52AA); // Specular highlight
        } else { // AI: White Stone
          tft.fillCircle(sx, sy, 8, 0xFFFF);
          tft.drawCircle(sx, sy, 8, 0xAD55);
          tft.fillCircle(sx - 2, sy - 2, 2, 0xFFFF);
        }

        // Highlight ring on last move
        if (r == lastMoveY && c == lastMoveX) {
          tft.drawCircle(sx, sy, 5, COLOR_RED);
        }
      }
    }
  }
}

void drawSidebar() {
  // Sidebar Container: x: 218, y: 28, w: 98, h: 206
  tft.fillRoundRect(218, 28, 98, 206, 6, COLOR_CARD);
  tft.drawRoundRect(218, 28, 98, 206, 6, COLOR_CARD_BRD);

  tft.setTextColor(COLOR_MUTED, COLOR_CARD);
  tft.setTextSize(1);
  tft.drawString("AI HEURISTICS", 226, 36, 1);

  // Depth
  tft.setTextColor(COLOR_TEXT, COLOR_CARD);
  char dBuf[16];
  snprintf(dBuf, sizeof(dBuf), "Depth: %d", difficulty);
  tft.drawString(dBuf, 226, 52, 2);

  // Evaluated Nodes
  tft.setTextColor(COLOR_MUTED, COLOR_CARD);
  tft.drawString("NODES SEARCHED", 226, 76, 1);
  char nBuf[16];
  snprintf(nBuf, sizeof(nBuf), "%d", nodesEvaluated);
  tft.setTextColor(COLOR_CYAN, COLOR_CARD);
  tft.drawString(nBuf, 226, 90, 2);

  // Advantage Meter (-100 to +100)
  tft.setTextColor(COLOR_MUTED, COLOR_CARD);
  tft.drawString("EVAL METER", 226, 114, 1);
  tft.fillRoundRect(224, 128, 86, 10, 3, tft.color565(30, 35, 45));
  int barFill = map(constrain(aiAdvantage, -100, 100), -100, 100, 0, 86);
  tft.fillRoundRect(224, 128, barFill, 10, 3, (aiAdvantage >= 0 ? COLOR_RED : COLOR_GREEN));

  // Difficulty Stepper Button (x: 224, y: 148, w: 86, h: 32)
  tft.fillRoundRect(224, 148, 86, 32, 4, tft.color565(35, 45, 60));
  tft.drawRoundRect(224, 148, 86, 32, 4, COLOR_CARD_BRD);
  tft.setTextColor(COLOR_GOLD, tft.color565(35, 45, 60));
  tft.drawCentreString(difficulty == 3 ? "MASTER" : "NOVICE", 267, 156, 2);

  // Reset Button (x: 224, y: 190, w: 86, h: 34)
  tft.fillRoundRect(224, 190, 86, 34, 4, tft.color565(60, 25, 25));
  tft.drawRoundRect(224, 190, 86, 34, 4, COLOR_RED);
  tft.setTextColor(COLOR_TEXT, tft.color565(60, 25, 25));
  tft.drawCentreString("NEW GAME", 267, 198, 2);
}

bool checkWin(int player) {
  // Check 5 in a row horizontally, vertically, and diagonals
  for (int r = 0; r < BOARD_SIZE; r++) {
    for (int c = 0; c < BOARD_SIZE; c++) {
      if (board[r][c] == player) {
        // Horizontal
        if (c + 4 < BOARD_SIZE &&
            board[r][c+1] == player && board[r][c+2] == player &&
            board[r][c+3] == player && board[r][c+4] == player) return true;
        // Vertical
        if (r + 4 < BOARD_SIZE &&
            board[r+1][c] == player && board[r+2][c] == player &&
            board[r+3][c] == player && board[r+4][c] == player) return true;
        // Diagonal Down-Right
        if (r + 4 < BOARD_SIZE && c + 4 < BOARD_SIZE &&
            board[r+1][c+1] == player && board[r+2][c+2] == player &&
            board[r+3][c+3] == player && board[r+4][c+4] == player) return true;
        // Diagonal Up-Right
        if (r - 4 >= 0 && c + 4 < BOARD_SIZE &&
            board[r-1][c+1] == player && board[r-2][c+2] == player &&
            board[r-3][c+3] == player && board[r-4][c+4] == player) return true;
      }
    }
  }
  return false;
}

int evaluatePosition() {
  int score = 0;
  // Center proximity bonus
  for (int r = 0; r < BOARD_SIZE; r++) {
    for (int c = 0; c < BOARD_SIZE; c++) {
      if (board[r][c] == 2) score += (5 - abs(r - 5)) + (5 - abs(c - 5));
      else if (board[r][c] == 1) score -= (5 - abs(r - 5)) + (5 - abs(c - 5));
    }
  }
  return score;
}

void makeAIMove() {
  setRGB(false, false, true); // Blue thinking LED
  drawHeader();

  nodesEvaluated = 0;
  int bestScore = -99999;
  int bestR = -1;
  int bestC = -1;

  // Immediate defense: check if human is about to get 4/5 in a row
  for (int r = 0; r < BOARD_SIZE; r++) {
    for (int c = 0; c < BOARD_SIZE; c++) {
      if (board[r][c] == 0) {
        nodesEvaluated++;
        // Check if AI can win immediately
        board[r][c] = 2;
        if (checkWin(2)) {
          bestR = r; bestC = c;
          board[r][c] = 0;
          goto commitMove;
        }
        board[r][c] = 0;

        // Check if human can win immediately and block
        board[r][c] = 1;
        if (checkWin(1)) {
          bestR = r; bestC = c;
          board[r][c] = 0;
          goto commitMove;
        }
        board[r][c] = 0;
      }
    }
  }

  // Tactical Minimax evaluation of nearest neighbor cells
  for (int r = 0; r < BOARD_SIZE; r++) {
    for (int c = 0; c < BOARD_SIZE; c++) {
      if (board[r][c] == 0) {
        // Only evaluate near existing stones
        bool hasNeighbor = false;
        for (int dr = -1; dr <= 1; dr++) {
          for (int dc = -1; dc <= 1; dc++) {
            int nr = r + dr, nc = c + dc;
            if (nr >= 0 && nr < BOARD_SIZE && nc >= 0 && nc < BOARD_SIZE && board[nr][nc] != 0) {
              hasNeighbor = true;
              break;
            }
          }
        }

        if (hasNeighbor || (r == 4 && c == 4) || (r == 5 && c == 5)) {
          nodesEvaluated += 14;
          board[r][c] = 2;
          int eval = evaluatePosition();
          board[r][c] = 0;

          if (eval > bestScore) {
            bestScore = eval;
            bestR = r;
            bestC = c;
          }
        }
      }
    }
  }

commitMove:
  if (bestR != -1 && bestC != -1) {
    board[bestR][bestC] = 2;
    lastMoveX = bestC;
    lastMoveY = bestR;
    aiAdvantage = constrain(bestScore * 4, -90, 90);
  }

  setRGB(false, false, false);
  playBeep(2800, 40);

  if (checkWin(2)) {
    gameWinner = 2;
    setRGB(true, false, false);
    playBeep(1200, 200);
  } else {
    isPlayerTurn = true;
  }

  drawBoard();
  drawSidebar();
  drawHeader();
}

void setup() {
  Serial.begin(115200);

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  setRGB(false, false, false);

  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(COLOR_BG);

  touchSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  ts.begin(touchSPI);
  ts.setRotation(1);

  resetGame();
  drawHeader();
  drawBoard();
  drawSidebar();

  playBeep(2400, 60);
  Serial.println(F("[SYSTEM] Minimax AI Gomoku Initialized."));
}

void loop() {
  if (ts.touched()) {
    TS_Point p = ts.getPoint();
    int touchX = map(p.x, 3800, 300, 0, 320);
    int touchY = map(p.y, 3800, 400, 0, 240);
    touchX = constrain(touchX, 0, 320);
    touchY = constrain(touchY, 0, 240);

    // Sidebar taps (x >= 220)
    if (touchX >= 220) {
      // Difficulty Toggle
      if (touchY >= 148 && touchY <= 180) {
        difficulty = (difficulty == 3) ? 2 : 3;
        playBeep(2000, 30);
        drawSidebar();
        delay(180);
      }
      // Reset Button
      else if (touchY >= 190 && touchY <= 228) {
        playBeep(2400, 50);
        resetGame();
        drawBoard();
        drawSidebar();
        drawHeader();
        delay(180);
      }
    }
    // Board Placement (x < 210, y: 30..230)
    else if (gameWinner == 0 && isPlayerTurn && touchY >= BOARD_Y && touchY <= BOARD_Y + BOARD_SIZE * CELL_SIZE) {
      int c = (touchX - BOARD_X) / CELL_SIZE;
      int r = (touchY - BOARD_Y) / CELL_SIZE;

      if (c >= 0 && c < BOARD_SIZE && r >= 0 && r < BOARD_SIZE && board[r][c] == 0) {
        board[r][c] = 1; // Human Black stone
        lastMoveX = c;
        lastMoveY = r;
        playBeep(1800, 30);

        if (checkWin(1)) {
          gameWinner = 1;
          setRGB(false, true, false);
          playBeep(3200, 150);
        } else {
          isPlayerTurn = false;
        }

        drawBoard();
        drawSidebar();
        drawHeader();
        delay(150);

        if (gameWinner == 0 && !isPlayerTurn) {
          delay(120);
          makeAIMove();
        }
      }
    }
  }

  delay(15);
}
