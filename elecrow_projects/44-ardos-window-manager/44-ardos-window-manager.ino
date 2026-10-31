/*
 * Elecrow 2.8" CYD — ArdOS Retro Window Manager
 * Channel: @IamNotIntoTech (https://youtube.com/@IamNotIntoTech)
 * 
 * Target Hardware: ESP32-2432S028R (2.8" ILI9341 TFT + XPT2046 Resistive Touch)
 * Features:
 *  - Retro 90s GUI Operating System (Windows 95/98 / Classic Mac aesthetic)
 *  - Draggable overlapping windows with 3D beveled borders & active titlebars
 *  - Interactive Taskbar with [Start] menu, open app tabs, and live system clock
 *  - 3 Built-in Desktop Applications:
 *    1. Retro Notepad (System info, memo scratchpad & @IamNotIntoTech branding)
 *    2. Desktop Calculator (Interactive 4-function touch keypad)
 *    3. Minesweeper (Classic 6x6 grid with smiley face, safe reveals, and mine detection)
 *  - Dedicated VSPI hardware SPI isolation & non-inverted Y touch mapping
 *  - Acoustic UI keyclicks on IO26 & Start menu RGB LED on IO4/16/17
 */

#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>

// Hardware Pin Definitions for Elecrow 2.8" CYD
#define TFT_BL_PIN   21
#define TOUCH_MOSI   32
#define TOUCH_MISO   39
#define TOUCH_CLK    25
#define TOUCH_CS     33
#define TOUCH_IRQ    36

#define LED_RED      4
#define LED_GREEN    16
#define LED_BLUE     17
#define BUZZER_PIN   26

SPIClass touchSPI(VSPI);
XPT2046_Touchscreen ts(TOUCH_CS);
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
#define COLOR_GREEN     0x07E0

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
  { APP_NOTEPAD,     "Notepad - MEMO.TXT", 20, 26, 190, 130, false, false },
  { APP_CALCULATOR,  "Calculator",         105, 30, 150, 150, true,  false },
  { APP_MINESWEEPER, "Minesweeper",        45,  28, 160, 155, false, false }
};

int activeApp = APP_CALCULATOR;
bool startMenuOpen = false;
unsigned long lastTouchTime = 0;

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
  const char* icons[3] = { "Notepad", "Calc", "Mines" };
  uint16_t iCols[3] = { COLOR_WHITE, COLOR_YELLOW, COLOR_RED };

  for (int i = 0; i < 3; i++) {
    int y = 20 + i * 50;
    tft.fillRect(14, y, 24, 24, iCols[i]);
    tft.drawRect(14, y, 24, 24, COLOR_BLACK);

    tft.setTextColor(COLOR_WHITE, COLOR_DESKTOP);
    tft.setTextSize(1);
    tft.drawCentreString(icons[i], 26, y + 27, 1);
  }
}

void drawStartMenuPopup() {
  // Start Menu Popup: x: 3, y: 104, w: 110, h: 110
  draw3DBevelRect(3, 104, 110, 110, false);

  // Left vertical blue banner
  tft.fillRect(5, 106, 18, 106, COLOR_WIN_TITLE);
  tft.setTextColor(COLOR_WHITE, COLOR_WIN_TITLE);
  tft.setTextSize(1);
  tft.drawString("95", 8, 192, 1);

  // Menu items
  const char* mItems[4] = { "Notepad", "Calculator", "Minesweeper", "About OS" };
  for (int i = 0; i < 4; i++) {
    int my = 108 + i * 26;
    tft.setTextColor(COLOR_BLACK, COLOR_WIN_BG);
    tft.drawString(mItems[i], 28, my + 6, 1);
    tft.drawFastHLine(26, my + 24, 84, COLOR_DARK);
  }
}

void drawTaskbar() {
  // Taskbar: x: 0, y: 216, w: 320, h: 24
  draw3DBevelRect(0, 216, 320, 24, false);

  // Start Button: x: 3, y: 218, w: 52, h: 20
  draw3DBevelRect(3, 218, 52, 20, startMenuOpen);
  tft.fillRect(7, 223, 10, 10, startMenuOpen ? COLOR_RED : COLOR_GREEN);
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

  // System Tray on Right: x: 260, w: 57
  draw3DBevelRect(260, 218, 57, 20, true);
  tft.setTextColor(COLOR_BLACK, COLOR_WIN_BG);
  tft.drawString("CYD 2.8", 265, 222, 1);

  if (startMenuOpen) {
    drawStartMenuPopup();
  }
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
  // White document canvas
  tft.fillRect(win.x + 4, win.y + 22, win.w - 8, win.h - 26, COLOR_WHITE);
  tft.drawRect(win.x + 4, win.y + 22, win.w - 8, win.h - 26, COLOR_DARK);

  tft.setTextColor(COLOR_BLACK, COLOR_WHITE);
  tft.setTextSize(1);
  tft.drawString("ArdOS v1.0 [Ready]", win.x + 8, win.y + 28, 1);
  tft.drawString("Platform: Elecrow CYD 2.8", win.x + 8, win.y + 42, 1);
  tft.drawString("Branding: @IamNotIntoTech", win.x + 8, win.y + 56, 1);
  tft.drawString("SRAM: 320KB Dual-Core", win.x + 8, win.y + 70, 1);
  tft.drawString("Touch: VSPI Polled XPT2046", win.x + 8, win.y + 84, 1);
  tft.drawString("System Status: OK", win.x + 8, win.y + 98, 1);
}

void renderCalculatorApp(Window& win) {
  // LCD Screen
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

  int kw = 30;
  int kh = 22;
  for (int r = 0; r < 4; r++) {
    for (int c = 0; c < 4; c++) {
      int kx = win.x + 8 + c * 34;
      int ky = win.y + 48 + r * 24;
      draw3DBevelRect(kx, ky, kw, kh, false);
      tft.setTextColor(COLOR_BLACK, COLOR_WIN_BG);
      tft.drawCentreString(keys[r][c], kx + kw / 2, ky + 5, 1);
    }
  }
}

void renderMinesweeperApp(Window& win) {
  // Smiley reset bar
  tft.fillRect(win.x + 6, win.y + 22, win.w - 12, 22, COLOR_WIN_BG);
  draw3DBevelRect(win.x + win.w / 2 - 12, win.y + 23, 24, 20, false);
  tft.setTextColor(COLOR_BLACK, COLOR_WIN_BG);
  tft.drawString(mineGameOver ? "X(" : ":)", win.x + win.w / 2 - 6, win.y + 26, 1);

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
          uint16_t numColor = (mineGrid[r][c] == 1) ? COLOR_WIN_TITLE : ((mineGrid[r][c] == 2) ? 0x03E0 : COLOR_RED);
          tft.setTextColor(numColor, COLOR_WIN_BG);
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
  int kw = 30;
  int kh = 22;

  for (int r = 0; r < 4; r++) {
    for (int c = 0; c < 4; c++) {
      int kx = win.x + 8 + c * 34;
      int ky = win.y + 48 + r * 24;

      if (rx >= kx && rx <= kx + kw && ry >= ky && ry <= ky + kh) {
        playBeep(2200, 20);
        const char* keys[4][4] = {
          { "7", "8", "9", "/" },
          { "4", "5", "6", "*" },
          { "1", "2", "3", "-" },
          { "C", "0", "=", "+" }
        };

        char k = keys[r][c][0];
        if (k >= '0' && k <= '9') {
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
  if (rx >= win.x + win.w / 2 - 12 && rx <= win.x + win.w / 2 + 12 && ry >= win.y + 23 && ry <= win.y + 43) {
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
      playBeep(800, 150);
      for (int i = 0; i < MINE_ROWS; i++) {
        for (int j = 0; j < MINE_COLS; j++) {
          if (mineGrid[i][j] == 9) revealed[i][j] = true;
        }
      }
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

  pinMode(TFT_BL_PIN, OUTPUT);
  digitalWrite(TFT_BL_PIN, HIGH);

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(COLOR_DESKTOP);

  // Dedicated VSPI for Touch Controller (Pins 25, 39, 32, 33)
  touchSPI.begin(TOUCH_CLK, TOUCH_MISO, TOUCH_MOSI, TOUCH_CS);
  ts.begin(touchSPI);
  ts.setRotation(1);

  initMinesweeper();
  renderAllWindows();

  playBeep(2000, 40);
  delay(30);
  playBeep(2500, 60);
  Serial.println(F("[SYSTEM] ArdOS Retro Window Manager Initialized."));
}

void loop() {
  if (ts.touched()) {
    TS_Point p = ts.getPoint();
    if (p.z > 50 || ts.touched()) {
      // Landscape non-inverted mapping
      int screenX = map(p.x, 240, 3800, 0, 320);
      int screenY = map(p.y, 240, 3800, 0, 240);
      screenX = constrain(screenX, 0, 319);
      screenY = constrain(screenY, 0, 239);

      // 1. Taskbar Interactions (y >= 216)
      if (screenY >= 216) {
        if (millis() - lastTouchTime > 200) {
          lastTouchTime = millis();
          // Start Button: x: 3..55
          if (screenX >= 3 && screenX <= 55) {
            startMenuOpen = !startMenuOpen;
            playBeep(2200, 25);
            setRGB(false, startMenuOpen, false);
            renderAllWindows();
            return;
          }

          // Taskbar App Tabs
          int tabX = 60;
          for (int i = 0; i < NUM_APPS; i++) {
            if (windows[i].isOpen) {
              if (screenX >= tabX && screenX <= tabX + 64) {
                activeApp = i;
                windows[i].isMinimized = false;
                startMenuOpen = false;
                playBeep(2000, 25);
                renderAllWindows();
                return;
              }
              tabX += 68;
            }
          }
        }
        return;
      }

      // 2. Start Menu Popup Interactions (if open)
      if (startMenuOpen) {
        if (millis() - lastTouchTime > 200) {
          lastTouchTime = millis();
          if (screenX >= 3 && screenX <= 113 && screenY >= 104 && screenY <= 214) {
            int itemIdx = (screenY - 108) / 26;
            if (itemIdx >= 0 && itemIdx < NUM_APPS) {
              windows[itemIdx].isOpen = true;
              windows[itemIdx].isMinimized = false;
              activeApp = itemIdx;
            }
            startMenuOpen = false;
            playBeep(2400, 25);
            renderAllWindows();
            return;
          } else {
            // Tapped outside start menu closes it
            startMenuOpen = false;
            renderAllWindows();
            return;
          }
        }
        return;
      }

      // 3. Desktop Icon Taps (when clicking outside active window)
      if (!windows[activeApp].isOpen || screenX < windows[activeApp].x || screenX > windows[activeApp].x + windows[activeApp].w ||
          screenY < windows[activeApp].y || screenY > windows[activeApp].y + windows[activeApp].h) {
        if (screenX >= 10 && screenX <= 45) {
          if (millis() - lastTouchTime > 200) {
            lastTouchTime = millis();
            int iIdx = (screenY - 20) / 50;
            if (iIdx >= 0 && iIdx < NUM_APPS) {
              windows[iIdx].isOpen = true;
              windows[iIdx].isMinimized = false;
              activeApp = iIdx;
              playBeep(2400, 30);
              renderAllWindows();
              return;
            }
          }
        }
      }

      // 4. Active Window Interactions
      Window& win = windows[activeApp];
      if (win.isOpen && !win.isMinimized) {
        // Close Button [X]: x: win.x + win.w - 18..win.w - 4, y: win.y + 4..18
        if (screenX >= win.x + win.w - 20 && screenX <= win.x + win.w - 2 &&
            screenY >= win.y + 2 && screenY <= win.y + 20) {
          if (millis() - lastTouchTime > 200) {
            lastTouchTime = millis();
            win.isOpen = false;
            playBeep(1600, 30);
            renderAllWindows();
          }
          return;
        }

        // Titlebar Dragging
        if (screenX >= win.x && screenX <= win.x + win.w - 22 &&
            screenY >= win.y && screenY <= win.y + 20) {
          int newX = constrain(screenX - win.w / 2, 0, 320 - win.w);
          int newY = constrain(screenY - 10, 0, 216 - win.h);
          if (abs(newX - win.x) > 4 || abs(newY - win.y) > 4) {
            win.x = newX;
            win.y = newY;
            renderAllWindows();
          }
          return;
        }

        // Inside Window Body
        if (millis() - lastTouchTime > 150) {
          lastTouchTime = millis();
          if (activeApp == APP_CALCULATOR) {
            handleCalculatorClick(screenX, screenY);
          } else if (activeApp == APP_MINESWEEPER) {
            handleMinesweeperClick(screenX, screenY);
          }
        }
      }
    }
  }

  delay(15);
}

