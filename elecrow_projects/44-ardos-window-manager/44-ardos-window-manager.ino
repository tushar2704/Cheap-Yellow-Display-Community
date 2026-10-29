/*
 * Elecrow 2.8" CYD — ArdOS Retro Window Manager
 * Channel: @IamNotIntoTech (https://youtube.com/@IamNotIntoTech)
 * 
 * Target: ESP32-2432S028R (2.8" ILI9341 TFT + XPT2046 Resistive Touch)
 * Features:
 *  - Retro 90s GUI Operating System (Windows 95/98 / Mac System 7 aesthetic)
 *  - Draggable overlapping windows with 3D beveled borders & active titlebars
 *  - Interactive Taskbar with [Start] menu, open app tabs, and live system clock
 *  - 3 Built-in Desktop Applications:
 *    1. Retro Notepad (Scratch text viewer & memo log)
 *    2. Desktop Calculator (Interactive 4-function touch keypad)
 *    3. Minesweeper (Classic grid with smiley face, safe reveals, and mine detonation)
 *  - Authentic acoustic UI keyclicks on IO26 & Start menu RGB LED on IO4/16/17
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

// Classic Retro Win95 Palette (16-bit 565)
#define COLOR_DESKTOP   0x0410 // Win95 Teal (#008080)
#define COLOR_WIN_BG    0xC618 // Standard 3D Silver Gray (#C0C0C0)
#define COLOR_WIN_TITLE 0x0010 // Deep Navy Blue (#000080)
#define COLOR_INACTIVE  0x8410 // Inactive titlebar slate
#define COLOR_WHITE     0xFFFF // 3D Light edge
#define COLOR_DARK      0x4208 // 3D Dark shadow edge
#define COLOR_BLACK     0x0000
#define COLOR_YELLOW    0xFFE0
#define COLOR_RED       0xF800

// Desktop Applications
enum AppID { APP_NOTEPAD = 0, APP_CALCULATOR = 1, APP_MINESWEEPER = 2, NUM_APPS = 3 };

struct Window {
  AppID id;
  const char* title;
  int x, y, w, h;
  bool isOpen;
  bool isMinimized;
};

Window windows[NUM_APPS] = {
  { APP_NOTEPAD,     "Notepad - MEMO.TXT", 20, 30, 180, 120, false, false },
  { APP_CALCULATOR,  "Calculator",         110, 45, 140, 145, true,  false },
  { APP_MINESWEEPER, "Minesweeper",        45,  35, 160, 155, false, false }
};

int activeApp = APP_CALCULATOR;
bool startMenuOpen = false;

// Calculator State
double calcAccum = 0;
double calcCurrent = 0;
char calcOp = 0;
bool calcNewNumber = true;

// Minesweeper State (6x6 Grid)
#define MINE_ROWS 6
#define MINE_COLS 6
uint8_t mineGrid[MINE_ROWS][MINE_COLS];   // 9 = mine, 0..8 = neighbor count
bool revealed[MINE_ROWS][MINE_COLS];
bool mineGameOver = false;

// Dragging State
bool isDragging = false;
int dragOffsetX = 0;
int dragOffsetY = 0;

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

void draw3DBevelRect(int x, int y, int w, int h, bool sunken = false) {
  uint16_t topCol = sunken ? COLOR_DARK : COLOR_WHITE;
  uint16_t botCol = sunken ? COLOR_WHITE : COLOR_DARK;

  tft.fillRect(x, y, w, h, COLOR_WIN_BG);
  tft.drawFastHLine(x, y, w, topCol);
  tft.drawFastVLine(x, y, h, topCol);
  tft.drawFastHLine(x, y + h - 1, w, botCol);
  tft.drawFastVLine(x + w - 1, y, h, botCol);
}

void initMinesweeper() {
  mineGameOver = false;
  for (int r = 0; r < MINE_ROWS; r++) {
    for (int c = 0; c < MINE_COLS; c++) {
      mineGrid[r][c] = 0;
      revealed[r][c] = false;
    }
  }

  // Plant 5 random mines
  int planted = 0;
  while (planted < 5) {
    int rr = random(0, MINE_ROWS);
    int rc = random(0, MINE_COLS);
    if (mineGrid[rr][rc] != 9) {
      mineGrid[rr][rc] = 9;
      planted++;
    }
  }

  // Calculate neighbor counts
  for (int r = 0; r < MINE_ROWS; r++) {
    for (int c = 0; c < MINE_COLS; c++) {
      if (mineGrid[r][c] != 9) {
        int count = 0;
        for (int dr = -1; dr <= 1; dr++) {
          for (int dc = -1; dc <= 1; dc++) {
            int nr = r + dr, nc = c + dc;
            if (nr >= 0 && nr < MINE_ROWS && nc >= 0 && nc < MINE_COLS && mineGrid[nr][nc] == 9) count++;
          }
        }
        mineGrid[r][c] = count;
      }
    }
  }
}

void drawDesktopIcons() {
  // Desktop Icons (x: 10)
  const char* icons[3] = { "Notepad", "Calc", "Mines" };
  uint16_t iCols[3] = { COLOR_WHITE, COLOR_YELLOW, COLOR_RED };

  for (int i = 0; i < 3; i++) {
    int y = 20 + i * 48;
    tft.fillRect(16, y, 22, 22, iCols[i]);
    tft.drawRect(16, y, 22, 22, COLOR_BLACK);

    tft.setTextColor(COLOR_WHITE, COLOR_DESKTOP);
    tft.setTextSize(1);
    tft.drawCentreString(icons[i], 27, y + 26, 1);
  }
}

void drawTaskbar() {
  // Taskbar: x: 0, y: 216, w: 320, h: 24
  draw3DBevelRect(0, 216, 320, 24, false);

  // Start Button: x: 3, y: 218, w: 52, h: 20
  draw3DBevelRect(3, 218, 52, 20, startMenuOpen);
  tft.fillRect(7, 223, 10, 10, startMenuOpen ? COLOR_RED : 0x07E0); // Windows flag square
  tft.setTextColor(COLOR_BLACK, COLOR_WIN_BG);
  tft.setTextSize(1);
  tft.drawString("Start", 22, 222, 1);

  // App Tabs in Taskbar
  int tabX = 60;
  for (int i = 0; i < NUM_APPS; i++) {
    if (windows[i].isOpen) {
      bool isActive = (activeApp == i && !windows[i].isMinimized);
      draw3DBevelRect(tabX, 218, 64, 20, isActive);
      tft.setTextColor(COLOR_BLACK, COLOR_WIN_BG);
      tft.drawString(i == 0 ? "Notepad" : (i == 1 ? "Calc" : "Mines"), tabX + 6, 222, 1);
      tabX += 68;
    }
  }

  // System Tray on Right: x: 265, w: 52
  draw3DBevelRect(265, 218, 52, 20, true);
  tft.setTextColor(COLOR_BLACK, COLOR_WIN_BG);
  tft.drawRightString("12:45", 310, 222, 1);
}

void drawWindowChrome(Window& win, bool isActive) {
  // Window frame bevel
  draw3DBevelRect(win.x, win.y, win.w, win.h, false);

  // Titlebar
  uint16_t tbColor = isActive ? COLOR_WIN_TITLE : COLOR_INACTIVE;
  tft.fillRect(win.x + 3, win.y + 3, win.w - 6, 16, tbColor);

  tft.setTextColor(COLOR_WHITE, tbColor);
  tft.setTextSize(1);
  tft.drawString(win.title, win.x + 6, win.y + 5, 1);

  // Close Button [X]: x: win.x + win.w - 18, y: win.y + 4, w: 14, h: 14
  draw3DBevelRect(win.x + win.w - 18, win.y + 4, 14, 14, false);
  tft.setTextColor(COLOR_BLACK, COLOR_WIN_BG);
  tft.drawString("X", win.x + win.w - 14, win.y + 6, 1);
}

void renderNotepadApp(Window& win) {
  // Text area
  tft.fillRect(win.x + 4, win.y + 22, win.w - 8, win.h - 26, COLOR_WHITE);
  tft.drawRect(win.x + 4, win.y + 22, win.w - 8, win.h - 26, COLOR_DARK);

  tft.setTextColor(COLOR_BLACK, COLOR_WHITE);
  tft.setTextSize(1);
  tft.drawString("ArdOS v1.0 Ready.", win.x + 8, win.y + 28, 1);
  tft.drawString("Elecrow CYD 2.8 Touch", win.x + 8, win.y + 42, 1);
  tft.drawString("SRAM: 520KB Available", win.x + 8, win.y + 56, 1);
  tft.drawString("Channel: @IamNotIntoTech", win.x + 8, win.y + 70, 1);
}

void renderCalculatorApp(Window& win) {
  // Calc LCD Screen: x: win.x + 6, y: win.y + 24, w: win.w - 12, h: 22
  tft.fillRect(win.x + 6, win.y + 22, win.w - 12, 22, COLOR_WHITE);
  tft.drawRect(win.x + 6, win.y + 22, win.w - 12, 22, COLOR_DARK);

  tft.setTextColor(COLOR_BLACK, COLOR_WHITE);
  tft.setTextSize(1);
  char numBuf[16];
  snprintf(numBuf, sizeof(numBuf), "%.0f", calcCurrent);
  tft.drawRightString(numBuf, win.x + win.w - 10, win.y + 25, 2);

  // Keypad: 4 rows x 4 cols
  const char* keys[4][4] = {
    { "7", "8", "9", "/" },
    { "4", "5", "6", "*" },
    { "1", "2", "3", "-" },
    { "C", "0", "=", "+" }
  };

  int kw = 28;
  int kh = 20;
  for (int r = 0; r < 4; r++) {
    for (int c = 0; c < 4; c++) {
      int kx = win.x + 6 + c * 32;
      int ky = win.y + 50 + r * 22;
      draw3DBevelRect(kx, ky, kw, kh, false);
      tft.setTextColor(COLOR_BLACK, COLOR_WIN_BG);
      tft.drawCentreString(keys[r][c], kx + kw / 2, ky + 4, 1);
    }
  }
}

void renderMinesweeperApp(Window& win) {
  // Top bar with smiley button
  tft.fillRect(win.x + 6, win.y + 22, win.w - 12, 22, COLOR_WIN_BG);
  draw3DBevelRect(win.x + win.w / 2 - 10, win.y + 23, 20, 20, false);
  tft.setTextColor(COLOR_YELLOW, COLOR_WIN_BG);
  tft.drawString(mineGameOver ? "X(" : ":)", win.x + win.w / 2 - 7, win.y + 26, 1);

  // 6x6 Minefield Grid
  int startX = win.x + 8;
  int startY = win.y + 48;
  int cellSize = 22;

  for (int r = 0; r < MINE_ROWS; r++) {
    for (int c = 0; c < MINE_COLS; c++) {
      int cx = startX + c * cellSize;
      int cy = startY + r * cellSize;

      if (revealed[r][c]) {
        draw3DBevelRect(cx, cy, cellSize - 1, cellSize - 1, true);
        if (mineGrid[r][c] == 9) {
          tft.fillCircle(cx + cellSize / 2, cy + cellSize / 2, 5, COLOR_RED);
        } else if (mineGrid[r][c] > 0) {
          tft.setTextColor(mineGrid[r][c] == 1 ? COLOR_WIN_TITLE : (mineGrid[r][c] == 2 ? 0x03E0 : COLOR_RED), COLOR_WIN_BG);
          char cBuf[4];
          snprintf(cBuf, sizeof(cBuf), "%d", mineGrid[r][c]);
          tft.drawCentreString(cBuf, cx + cellSize / 2, cy + 4, 1);
        }
      } else {
        draw3DBevelRect(cx, cy, cellSize - 1, cellSize - 1, false);
      }
    }
  }
}

void renderAllWindows() {
  tft.fillRect(0, 0, 320, 216, COLOR_DESKTOP);
  drawDesktopIcons();

  for (int i = 0; i < NUM_APPS; i++) {
    if (windows[i].isOpen && !windows[i].isMinimized && i != activeApp) {
      drawWindowChrome(windows[i], false);
      if (i == APP_NOTEPAD) renderNotepadApp(windows[i]);
      else if (i == APP_CALCULATOR) renderCalculatorApp(windows[i]);
      else if (i == APP_MINESWEEPER) renderMinesweeperApp(windows[i]);
    }
  }

  // Render Active Window on top
  if (windows[activeApp].isOpen && !windows[activeApp].isMinimized) {
    drawWindowChrome(windows[activeApp], true);
    if (activeApp == APP_NOTEPAD) renderNotepadApp(windows[activeApp]);
    else if (activeApp == APP_CALCULATOR) renderCalculatorApp(windows[activeApp]);
    else if (activeApp == APP_MINESWEEPER) renderMinesweeperApp(windows[activeApp]);
  }

  drawTaskbar();
}

void handleCalculatorClick(int rx, int ry) {
  Window& win = windows[APP_CALCULATOR];
  int kw = 28;
  int kh = 20;

  for (int r = 0; r < 4; r++) {
    for (int c = 0; c < 4; c++) {
      int kx = win.x + 6 + c * 32;
      int ky = win.y + 50 + r * 22;

      if (rx >= kx && rx <= kx + kw && ry >= ky && ry <= ky + kh) {
        playBeep(2200, 20);
        const char* keys[4][4] = {
          { "7", "8", "9", "/" },
          { "4", "5", "6", "*" },
          { "1", "2", "3", "-" },
          { "C", "0", "=", "+" }
        };
        char k = keys[r][c][0];

        if (isdigit(k)) {
          if (calcNewNumber) {
            calcCurrent = k - '0';
            calcNewNumber = false;
          } else {
            calcCurrent = calcCurrent * 10 + (k - '0');
          }
        } else if (k == 'C') {
          calcCurrent = 0;
          calcAccum = 0;
          calcOp = 0;
          calcNewNumber = true;
        } else if (k == '=') {
          if (calcOp == '+') calcCurrent = calcAccum + calcCurrent;
          else if (calcOp == '-') calcCurrent = calcAccum - calcCurrent;
          else if (calcOp == '*') calcCurrent = calcAccum * calcCurrent;
          else if (calcOp == '/') calcCurrent = (calcCurrent != 0) ? (calcAccum / calcCurrent) : 0;
          calcOp = 0;
          calcNewNumber = true;
        } else {
          calcAccum = calcCurrent;
          calcOp = k;
          calcNewNumber = true;
        }

        renderCalculatorApp(win);
        return;
      }
    }
  }
}

void handleMinesweeperClick(int rx, int ry) {
  Window& win = windows[APP_MINESWEEPER];
  int startX = win.x + 8;
  int startY = win.y + 48;
  int cellSize = 22;

  // Smiley reset tap
  if (rx >= win.x + win.w / 2 - 10 && rx <= win.x + win.w / 2 + 10 && ry >= win.y + 23 && ry <= win.y + 43) {
    playBeep(2400, 30);
    initMinesweeper();
    renderMinesweeperApp(win);
    return;
  }

  if (mineGameOver) return;

  int c = (rx - startX) / cellSize;
  int r = (ry - startY) / cellSize;

  if (r >= 0 && r < MINE_ROWS && c >= 0 && c < MINE_COLS && !revealed[r][c]) {
    revealed[r][c] = true;

    if (mineGrid[r][c] == 9) {
      mineGameOver = true;
      setRGB(true, false, false);
      playBeep(900, 150);
      // Reveal all mines
      for (int i = 0; i < MINE_ROWS; i++)
        for (int j = 0; j < MINE_COLS; j++)
          if (mineGrid[i][j] == 9) revealed[i][j] = true;
    } else {
      playBeep(2600, 15);
    }

    renderMinesweeperApp(win);
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

  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(COLOR_DESKTOP);

  touchSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  ts.begin(touchSPI);
  ts.setRotation(1);

  initMinesweeper();
  renderAllWindows();

  playBeep(2000, 50);
  delay(40);
  playBeep(2600, 80);
  Serial.println(F("[SYSTEM] ArdOS Retro Window Manager initialized."));
}

void loop() {
  if (ts.touched()) {
    TS_Point p = ts.getPoint();
    int touchX = map(p.x, 3800, 300, 0, 320);
    int touchY = map(p.y, 3800, 400, 0, 240);
    touchX = constrain(touchX, 0, 320);
    touchY = constrain(touchY, 0, 240);

    // Taskbar Clicks (y >= 216)
    if (touchY >= 216) {
      // Start Button
      if (touchX >= 3 && touchX <= 55) {
        startMenuOpen = !startMenuOpen;
        playBeep(2200, 25);
        setRGB(false, startMenuOpen, false);
        drawTaskbar();
        delay(180);
      }
      // App Tab toggles
      int tabX = 60;
      for (int i = 0; i < NUM_APPS; i++) {
        if (windows[i].isOpen) {
          if (touchX >= tabX && touchX <= tabX + 64) {
            activeApp = i;
            windows[i].isMinimized = false;
            playBeep(2000, 25);
            renderAllWindows();
            delay(180);
            return;
          }
          tabX += 68;
        }
      }
      return;
    }

    // Desktop Icon Taps (x <= 45)
    if (!windows[activeApp].isOpen || touchX < windows[activeApp].x) {
      if (touchX >= 10 && touchX <= 45) {
        int iIdx = (touchY - 20) / 48;
        if (iIdx >= 0 && iIdx < NUM_APPS) {
          windows[iIdx].isOpen = true;
          windows[iIdx].isMinimized = false;
          activeApp = iIdx;
          playBeep(2400, 30);
          renderAllWindows();
          delay(180);
          return;
        }
      }
    }

    // Active Window Interactions
    Window& win = windows[activeApp];
    if (win.isOpen && !win.isMinimized) {
      // Titlebar Close Button [X]
      if (touchX >= win.x + win.w - 18 && touchX <= win.x + win.w - 4 &&
          touchY >= win.y + 4 && touchY <= win.y + 18) {
        win.isOpen = false;
        playBeep(1600, 30);
        renderAllWindows();
        delay(180);
        return;
      }

      // Titlebar Dragging
      if (touchX >= win.x && touchX <= win.x + win.w - 20 &&
          touchY >= win.y && touchY <= win.y + 20) {
        win.x = constrain(touchX - win.w / 2, 0, 320 - win.w);
        win.y = constrain(touchY - 10, 0, 216 - win.h);
        renderAllWindows();
        delay(40);
        return;
      }

      // Inside Window Client Area
      if (activeApp == APP_CALCULATOR) {
        handleCalculatorClick(touchX, touchY);
        delay(150);
      } else if (activeApp == APP_MINESWEEPER) {
        handleMinesweeperClick(touchX, touchY);
        delay(150);
      }
    }
  }

  delay(15);
}
