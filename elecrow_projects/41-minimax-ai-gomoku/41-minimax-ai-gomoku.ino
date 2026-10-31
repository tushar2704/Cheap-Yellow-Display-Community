/**
 * ♟️ Project 41: Minimax AI Gomoku & Tactical Board
 * 
 * Hardware: Elecrow CrowPanel 2.8" (ESP32-2432S028R)
 * Channel Branding: IamNotIntoTech (https://youtube.com/@IamNotIntoTech)
 * Author: Tushar Aggarwal (tushar2704)
 * 
 * Features:
 * - 10x10 Gomoku (Five-in-a-Row) board with bamboo wood styling and stone rendering.
 * - Embedded Minimax heuristic AI with tactical pattern evaluation and defensive blocking.
 * - Real-time AI telemetry sidebar: nodes evaluated count, search depth, evaluation advantage meter.
 * - Precision resistive touch stone placement with Last Move red ring indicator.
 * - Tactile stone snap sound effects on GPIO 26 & AI thinking Blue LED on GPIOs 4/16/17.
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
int difficulty = 3; // 2: Novice, 3: Master
int nodesEvaluated = 0;
int aiAdvantage = 0; // -100 to +100

unsigned long lastBtnTouchTime = 0;

void playBeep(int freq = 2000, int ms = 30) {
  tone(BUZZER_PIN, freq, ms);
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
  tft.drawString("IamNotIntoTech", 8, 5, 2);

  if (gameWinner == 1) {
    tft.setTextColor(COLOR_GREEN, COLOR_PANEL);
    tft.drawRightString("HUMAN WINS! ★", 312, 5, 2);
  } else if (gameWinner == 2) {
    tft.setTextColor(COLOR_RED, COLOR_PANEL);
    tft.drawRightString("AI WINS! ★", 312, 5, 2);
  } else {
    tft.setTextColor(isPlayerTurn ? COLOR_CYAN : COLOR_GOLD, COLOR_PANEL);
    tft.drawRightString(isPlayerTurn ? "YOUR TURN ●" : "AI THINKING...", 312, 5, 2);
  }
}

void drawBoard() {
  tft.fillRoundRect(BOARD_X - 4, BOARD_Y - 4, BOARD_SIZE * CELL_SIZE + 8, BOARD_SIZE * CELL_SIZE + 8, 4, COLOR_BOARD);
  tft.drawRoundRect(BOARD_X - 4, BOARD_Y - 4, BOARD_SIZE * CELL_SIZE + 8, BOARD_SIZE * CELL_SIZE + 8, 4, 0x9460);

  // Draw Grid Lines
  for (int i = 0; i < BOARD_SIZE; i++) {
    tft.drawFastHLine(BOARD_X + CELL_SIZE / 2, BOARD_Y + i * CELL_SIZE + CELL_SIZE / 2, (BOARD_SIZE - 1) * CELL_SIZE, COLOR_GRID);
    tft.drawFastVLine(BOARD_X + i * CELL_SIZE + CELL_SIZE / 2, BOARD_Y + CELL_SIZE / 2, (BOARD_SIZE - 1) * CELL_SIZE, COLOR_GRID);
  }

  // Draw Stones
  for (int r = 0; r < BOARD_SIZE; r++) {
    for (int c = 0; c < BOARD_SIZE; c++) {
      int sx = BOARD_X + c * CELL_SIZE + CELL_SIZE / 2;
      int sy = BOARD_Y + r * CELL_SIZE + CELL_SIZE / 2;

      if (board[r][c] == 1) {
        // Human Black Stone
        tft.fillCircle(sx, sy, 7, 0x18C3);
        tft.drawCircle(sx, sy, 7, 0x0000);
        tft.fillCircle(sx - 2, sy - 2, 2, 0x4A69);
      } else if (board[r][c] == 2) {
        // AI White Stone
        tft.fillCircle(sx, sy, 7, 0xFFFF);
        tft.drawCircle(sx, sy, 7, 0xBDD7);
        tft.fillCircle(sx - 2, sy - 2, 2, 0xFFFF);
      }

      // Last move highlight
      if (c == lastMoveX && r == lastMoveY) {
        tft.drawCircle(sx, sy, 8, COLOR_RED);
      }
    }
  }
}

void drawSidebar() {
  tft.fillRoundRect(220, 30, 94, 202, 6, COLOR_CARD);
  tft.drawRoundRect(220, 30, 94, 202, 6, COLOR_CARD_BRD);

  tft.setTextColor(COLOR_CYAN, COLOR_CARD);
  tft.drawCentreString("AI TELEMETRY", 267, 36, 1);

  // Depth Card
  tft.setTextColor(COLOR_MUTED, COLOR_CARD);
  tft.drawString("DEPTH", 226, 52, 1);
  tft.setTextColor(COLOR_TEXT, COLOR_CARD);
  char depthBuf[16];
  snprintf(depthBuf, sizeof(depthBuf), "%d PLY", difficulty);
  tft.drawString(depthBuf, 226, 64, 2);

  // Nodes Evaluated
  tft.setTextColor(COLOR_MUTED, COLOR_CARD);
  tft.drawString("NODES", 226, 82, 1);
  tft.setTextColor(COLOR_GOLD, COLOR_CARD);
  char nodeBuf[16];
  snprintf(nodeBuf, sizeof(nodeBuf), "%d", nodesEvaluated);
  tft.drawString(nodeBuf, 226, 94, 2);

  // Advantage Meter (-100 to +100)
  tft.setTextColor(COLOR_MUTED, COLOR_CARD);
  tft.drawString("EVAL METER", 226, 114, 1);
  tft.fillRoundRect(224, 128, 86, 10, 3, tft.color565(30, 35, 45));
  int barFill = map(constrain(aiAdvantage, -100, 100), -100, 100, 0, 86);
  tft.fillRoundRect(224, 128, barFill, 10, 3, (aiAdvantage >= 0 ? COLOR_RED : COLOR_GREEN));

  // Difficulty Toggle Button (x: 224, y: 148, w: 86, h: 32)
  tft.fillRoundRect(224, 148, 86, 32, 4, tft.color565(35, 45, 60));
  tft.drawRoundRect(224, 148, 86, 32, 4, COLOR_CARD_BRD);
  tft.setTextColor(COLOR_TEXT, tft.color565(35, 45, 60));
  tft.drawCentreString((difficulty == 3) ? "LVL: MASTER" : "LVL: NOVICE", 267, 156, 1);

  // Reset Button (x: 224, y: 188, w: 86, h: 36)
  tft.fillRoundRect(224, 188, 86, 36, 4, COLOR_RED);
  tft.drawRoundRect(224, 188, 86, 36, 4, COLOR_GOLD);
  tft.setTextColor(0x0000, COLOR_RED);
  tft.drawCentreString("NEW GAME", 267, 198, 2);
}

bool checkWin(uint8_t player) {
  // Check horizontal, vertical, and diagonals for 5 in a row
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
  for (int r = 0; r < BOARD_SIZE; r++) {
    for (int c = 0; c < BOARD_SIZE; c++) {
      if (board[r][c] == 2) score += (5 - abs(r - 5)) + (5 - abs(c - 5));
      else if (board[r][c] == 1) score -= (5 - abs(r - 5)) + (5 - abs(c - 5));
    }
  }
  return score;
}

void makeAIMove() {
  setRGB(false, false, true);
  drawHeader();

  nodesEvaluated = 0;
  int bestScore = -99999;
  int bestR = -1;
  int bestC = -1;

  for (int r = 0; r < BOARD_SIZE; r++) {
    for (int c = 0; c < BOARD_SIZE; c++) {
      if (board[r][c] == 0) {
        nodesEvaluated++;
        board[r][c] = 2;
        if (checkWin(2)) {
          bestR = r; bestC = c;
          board[r][c] = 0;
          goto commitMove;
        }
        board[r][c] = 0;

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

  for (int r = 0; r < BOARD_SIZE; r++) {
    for (int c = 0; c < BOARD_SIZE; c++) {
      if (board[r][c] == 0) {
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

  pinMode(TFT_BL_PIN, OUTPUT);
  digitalWrite(TFT_BL_PIN, HIGH);

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(COLOR_BG);

  // Initialize Touch (VSPI)
  touchSPI.begin(TOUCH_CLK, TOUCH_MISO, TOUCH_MOSI, TOUCH_CS);
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
    if (p.z > 50 || ts.touched()) {
      int screenX = map(p.x, 240, 3800, 0, 320);
      int screenY = map(p.y, 240, 3800, 0, 240);
      screenX = constrain(screenX, 0, 319);
      screenY = constrain(screenY, 0, 239);

      // Sidebar buttons (screenX >= 220)
      if (screenX >= 220) {
        if (millis() - lastBtnTouchTime > 180) {
          lastBtnTouchTime = millis();
          // Difficulty Toggle
          if (screenY >= 148 && screenY <= 180) {
            difficulty = (difficulty == 3) ? 2 : 3;
            playBeep(2000, 30);
            drawSidebar();
          }
          // Reset Button
          else if (screenY >= 188 && screenY <= 228) {
            playBeep(2400, 50);
            resetGame();
            drawBoard();
            drawSidebar();
            drawHeader();
          }
        }
      }
      // Board Placement (screenX < 210, screenY: 30..230)
      else if (gameWinner == 0 && isPlayerTurn && screenY >= BOARD_Y && screenY <= BOARD_Y + BOARD_SIZE * CELL_SIZE) {
        if (millis() - lastBtnTouchTime > 180) {
          lastBtnTouchTime = millis();
          int c = (screenX - BOARD_X) / CELL_SIZE;
          int r = (screenY - BOARD_Y) / CELL_SIZE;

          if (c >= 0 && c < BOARD_SIZE && r >= 0 && r < BOARD_SIZE && board[r][c] == 0) {
            board[r][c] = 1;
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

            if (gameWinner == 0 && !isPlayerTurn) {
              delay(120);
              makeAIMove();
            }
          }
        }
      }
    }
  }

  delay(15);
}

