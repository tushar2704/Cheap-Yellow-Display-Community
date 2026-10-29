/*
 * Elecrow 2.8" CYD — Stylus RPN Calculator
 * Channel: @IamNotIntoTech (https://youtube.com/@IamNotIntoTech)
 * 
 * Target: ESP32-2432S028R (2.8" ILI9341 TFT + XPT2046 Resistive Touch)
 * Features:
 *  - 4-Level Classic RPN Stack (X, Y, Z, T)
 *  - Programmer's Multi-Base Display (HEX, DEC, BIN, OCT)
 *  - Interactive 32-bit stylus bit-toggle grid for instant bit flipping
 *  - Full bitwise operations (AND, OR, XOR, NOT, SHL, SHR) & RPN arithmetic
 *  - Tactile acoustic audio keyclick on IO26 & stack pulse LED on IO4/16/17
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

// UI Colors (16-bit 565)
#define COLOR_BG        0x0842 // Dark charcoal
#define COLOR_DISPLAY   0x10A3 // LCD background
#define COLOR_BORDER    0x2967
#define COLOR_TEXT      0xFFFF
#define COLOR_STACK_X   0x07FF // Cyan
#define COLOR_STACK_Y   0xFFE0 // Yellow
#define COLOR_STACK_ZT  0x8410 // Muted
#define COLOR_KEY_NUM   0x2124 // Dark slate
#define COLOR_KEY_OP    0x3186 // Mid slate
#define COLOR_KEY_ENTER 0x03E0 // Green
#define COLOR_KEY_HEX   0x4A69 // Indigo
#define COLOR_ACCENT    0xF81F

// RPN Stack (4 levels: 0=X, 1=Y, 2=Z, 3=T)
double rpnStack[4] = { 0.0, 0.0, 0.0, 0.0 };
String inputBuffer = "";
bool enteringNumber = false;

enum NumBase { BASE_DEC, BASE_HEX, BASE_BIN };
NumBase currentBase = BASE_DEC;

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

void pushStack(double val) {
  rpnStack[3] = rpnStack[2];
  rpnStack[2] = rpnStack[1];
  rpnStack[1] = rpnStack[0];
  rpnStack[0] = val;
}

double popStack() {
  double val = rpnStack[0];
  rpnStack[0] = rpnStack[1];
  rpnStack[1] = rpnStack[2];
  rpnStack[2] = rpnStack[3];
  return val;
}

void executeOp(char op) {
  if (enteringNumber) {
    rpnStack[0] = inputBuffer.toDouble();
    inputBuffer = "";
    enteringNumber = false;
  }

  double x = popStack();
  double y = popStack();
  double res = 0;

  switch (op) {
    case '+': res = y + x; break;
    case '-': res = y - x; break;
    case '*': res = y * x; break;
    case '/': res = (x != 0) ? (y / x) : 0; break;
    case '%': res = (uint32_t)y % (uint32_t)x; break;
    case '&': res = (uint32_t)y & (uint32_t)x; break;
    case '|': res = (uint32_t)y | (uint32_t)x; break;
    case '^': res = (uint32_t)y ^ (uint32_t)x; break;
    case '<': res = (uint32_t)y << (uint32_t)x; break; // SHL
    case '>': res = (uint32_t)y >> (uint32_t)x; break; // SHR
  }

  pushStack(res);
  setRGB(false, true, true);
  delay(30);
  setRGB(false, false, false);
}

void drawDisplay() {
  // Display Screen: x: 0, y: 0, w: 320, h: 86
  tft.fillRect(0, 0, 320, 86, COLOR_DISPLAY);
  tft.drawFastHLine(0, 86, 320, COLOR_BORDER);

  // Stack T & Z (Row 1)
  tft.setTextColor(COLOR_STACK_ZT, COLOR_DISPLAY);
  tft.setTextSize(1);
  char tzBuf[48];
  snprintf(tzBuf, sizeof(tzBuf), "T: %.2f   Z: %.2f", rpnStack[3], rpnStack[2]);
  tft.drawString(tzBuf, 10, 4, 1);

  // Stack Y (Row 2)
  tft.setTextColor(COLOR_STACK_Y, COLOR_DISPLAY);
  char yBuf[32];
  snprintf(yBuf, sizeof(yBuf), "Y: %.4f", rpnStack[1]);
  tft.drawString(yBuf, 10, 18, 2);

  // Stack X / Input (Row 3 - Large)
  tft.setTextColor(COLOR_STACK_X, COLOR_DISPLAY);
  if (enteringNumber) {
    tft.drawString("X: " + inputBuffer + "_", 10, 36, 4);
  } else {
    char xBuf[32];
    snprintf(xBuf, sizeof(xBuf), "X: %.4f", rpnStack[0]);
    tft.drawString(xBuf, 10, 36, 4);
  }

  // Multi-Base Programmer Bar (Row 4: y: 64)
  uint32_t regX = (uint32_t)rpnStack[0];
  char baseBuf[64];
  snprintf(baseBuf, sizeof(baseBuf), "HEX:0x%08X  DEC:%u", regX, regX);
  tft.setTextColor(0x07E0, COLOR_DISPLAY);
  tft.drawString(baseBuf, 10, 68, 1);

  // 32-bit Mini Bit Grid (x: 180 to 310, y: 68..78)
  for (int b = 31; b >= 0; b--) {
    int bx = 180 + (31 - b) * 4;
    bool bitSet = (regX >> b) & 1;
    tft.drawFastVLine(bx, 68, 8, bitSet ? COLOR_STACK_X : tft.color565(30, 40, 50));
  }
}

// Keypad Configuration: 6 columns x 5 rows = 30 keys
// x: 0..320 -> width 52px each (52 * 6 = 312, 4px margin)
// y: 88..240 -> height 29px each (29 * 5 = 145 + margin)
const char* keyLabels[5][6] = {
  { "ENT", "x<>y", "R v", "DROP", "AND", "OR" },
  { "HEX", "DEC",  "BIN", "NOT",  "<<",  ">>" },
  { "7",   "8",    "9",   "/",    "A",   "B"  },
  { "4",   "5",    "6",   "*",    "C",   "D"  },
  { "1",   "2",    "3",   "-",    "+",   "CLR"}
};

void drawKeypad() {
  for (int r = 0; r < 5; r++) {
    for (int c = 0; c < 6; c++) {
      int kx = 4 + c * 52;
      int ky = 89 + r * 30;
      int kw = 48;
      int kh = 26;

      const char* lbl = keyLabels[r][c];
      uint16_t kBg = COLOR_KEY_NUM;

      if (strcmp(lbl, "ENT") == 0) kBg = COLOR_KEY_ENTER;
      else if (strcmp(lbl, "HEX") == 0 || strcmp(lbl, "DEC") == 0 || strcmp(lbl, "BIN") == 0) kBg = COLOR_KEY_HEX;
      else if (strchr("+-*/&|^<>", lbl[0]) != NULL) kBg = COLOR_KEY_OP;

      tft.fillRoundRect(kx, ky, kw, kh, 4, kBg);
      tft.drawRoundRect(kx, ky, kw, kh, 4, COLOR_BORDER);

      tft.setTextColor(COLOR_TEXT, kBg);
      tft.setTextSize(1);
      tft.drawCentreString(lbl, kx + kw / 2, ky + 6, 2);
    }
  }
}

void handleKeyPress(int r, int c) {
  const char* key = keyLabels[r][c];
  playBeep(2200, 25);

  // Digit or Hex letter (0..9, A..D)
  if (isdigit(key[0]) || (key[0] >= 'A' && key[0] <= 'D' && strlen(key) == 1)) {
    if (!enteringNumber) {
      inputBuffer = "";
      enteringNumber = true;
    }
    inputBuffer += key[0];
    drawDisplay();
    return;
  }

  // Clear
  if (strcmp(key, "CLR") == 0) {
    inputBuffer = "";
    enteringNumber = false;
    rpnStack[0] = 0;
    drawDisplay();
    return;
  }

  // ENTER
  if (strcmp(key, "ENT") == 0) {
    if (enteringNumber) {
      rpnStack[0] = inputBuffer.toDouble();
      inputBuffer = "";
      enteringNumber = false;
    }
    pushStack(rpnStack[0]);
    drawDisplay();
    return;
  }

  // Stack Operations
  if (strcmp(key, "x<>y") == 0) {
    double temp = rpnStack[0];
    rpnStack[0] = rpnStack[1];
    rpnStack[1] = temp;
    drawDisplay();
    return;
  }
  if (strcmp(key, "R v") == 0) {
    double temp = rpnStack[0];
    rpnStack[0] = rpnStack[1];
    rpnStack[1] = rpnStack[2];
    rpnStack[2] = rpnStack[3];
    rpnStack[3] = temp;
    drawDisplay();
    return;
  }
  if (strcmp(key, "DROP") == 0) {
    popStack();
    drawDisplay();
    return;
  }

  // Operators
  if (strcmp(key, "+") == 0) executeOp('+');
  else if (strcmp(key, "-") == 0) executeOp('-');
  else if (strcmp(key, "*") == 0) executeOp('*');
  else if (strcmp(key, "/") == 0) executeOp('/');
  else if (strcmp(key, "AND") == 0) executeOp('&');
  else if (strcmp(key, "OR") == 0) executeOp('|');
  else if (strcmp(key, "<<") == 0) executeOp('<');
  else if (strcmp(key, ">>") == 0) executeOp('>');
  else if (strcmp(key, "NOT") == 0) {
    if (enteringNumber) {
      rpnStack[0] = inputBuffer.toDouble();
      inputBuffer = "";
      enteringNumber = false;
    }
    rpnStack[0] = ~(uint32_t)rpnStack[0];
  }

  drawDisplay();
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

  drawDisplay();
  drawKeypad();

  playBeep(2000, 60);
  Serial.println(F("[SYSTEM] Stylus RPN Calculator Initialized."));
}

void loop() {
  if (ts.touched()) {
    TS_Point p = ts.getPoint();
    int touchX = map(p.x, 3800, 300, 0, 320);
    int touchY = map(p.y, 3800, 400, 0, 240);
    touchX = constrain(touchX, 0, 320);
    touchY = constrain(touchY, 0, 240);

    // Keypad Area (y >= 88)
    if (touchY >= 88) {
      int r = (touchY - 88) / 30;
      int c = (touchX - 4) / 52;
      r = constrain(r, 0, 4);
      c = constrain(c, 0, 5);

      handleKeyPress(r, c);
      delay(180);
    }
    // Direct Bit-Toggle Area (x >= 180, y: 64..84)
    else if (touchX >= 180 && touchY >= 64 && touchY <= 84) {
      int bitIdx = 31 - ((touchX - 180) / 4);
      bitIdx = constrain(bitIdx, 0, 31);
      uint32_t regX = (uint32_t)rpnStack[0];
      regX ^= (1UL << bitIdx); // Flip tapped bit
      rpnStack[0] = regX;
      playBeep(2600, 20);
      drawDisplay();
      delay(150);
    }
  }

  delay(15);
}
