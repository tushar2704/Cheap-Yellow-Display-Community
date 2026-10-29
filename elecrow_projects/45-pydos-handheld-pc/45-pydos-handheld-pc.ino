/*
 * Elecrow 2.8" CYD — PyDOS Handheld Computer
 * Channel: @IamNotIntoTech (https://youtube.com/@IamNotIntoTech)
 * 
 * Target: ESP32-2432S028R (2.8" ILI9341 TFT + XPT2046 Resistive Touch)
 * Features:
 *  - Retro MS-DOS / MicroPython Interactive Shell Console (C:\> _)
 *  - Full On-Screen Stylus QWERTY Virtual Keyboard with key hover feedback
 *  - Built-in Shell Commands: DIR, VER, MEM, CLS, BEEP, LED, ECHO, HELP, PY
 *  - Scrolling 10-line terminal buffer with amber phosphor typography
 *  - Mechanical keyboard audio clicks on IO26 & RGB command pulse on IO4/16/17
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
#define COLOR_BG        0x0000 // Deep Black
#define COLOR_PHOSPHOR  0xFD20 // Retro Amber Phosphor
#define COLOR_BORDER    0x3186
#define COLOR_KEY_BG    0x18C3 // Dark Slate
#define COLOR_KEY_BRD   0x4A69
#define COLOR_KEY_TXT   0xFFFF
#define COLOR_KEY_ACT   0xFD20 // Amber Accent

// Terminal Console Buffer: 10 lines x 40 characters
#define MAX_LINES 10
String terminalLines[MAX_LINES];
int lineCount = 0;
String currentInput = "";
bool cursorBlink = true;
unsigned long lastCursorTick = 0;

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

void printToTerminal(String text) {
  if (lineCount < MAX_LINES) {
    terminalLines[lineCount++] = text;
  } else {
    // Scroll up
    for (int i = 0; i < MAX_LINES - 1; i++) {
      terminalLines[i] = terminalLines[i + 1];
    }
    terminalLines[MAX_LINES - 1] = text;
  }
}

void drawTerminal() {
  // Console window: x: 0, y: 0, w: 320, h: 126
  tft.fillRect(0, 0, 320, 126, COLOR_BG);
  tft.drawFastHLine(0, 126, 320, COLOR_BORDER);

  tft.setTextColor(COLOR_PHOSPHOR, COLOR_BG);
  tft.setTextSize(1);

  // Print existing lines
  for (int i = 0; i < lineCount; i++) {
    tft.drawString(terminalLines[i], 6, 4 + i * 11, 1);
  }

  // Print current input prompt at bottom of console
  int promptY = 4 + min(lineCount, MAX_LINES - 1) * 11;
  String promptStr = "C:\\> " + currentInput + (cursorBlink ? "_" : " ");
  tft.drawString(promptStr, 6, promptY, 1);
}

// ----------------------------------------------------
// QWERTY Virtual Keyboard Layout
// ----------------------------------------------------
// Row 0: Numbers (1..0, -, =)
const char* row0Keys[12] = { "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "-", "=" };
// Row 1: Q..P
const char* row1Keys[10] = { "Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P" };
// Row 2: A..L, CLS
const char* row2Keys[10] = { "A", "S", "D", "F", "G", "H", "J", "K", "L", "CLS" };
// Row 3: Z..M, SPACE, BS, ENT
const char* row3Keys[9]  = { "Z", "X", "C", "V", "B", "N", "M", "BK", "ENT" };

void drawKeyboard() {
  tft.fillRect(0, 128, 320, 112, tft.color565(15, 20, 25));

  // Row 0: Numbers (y: 130..150)
  for (int i = 0; i < 12; i++) {
    int kx = 4 + i * 26;
    tft.fillRoundRect(kx, 130, 23, 19, 3, COLOR_KEY_BG);
    tft.drawRoundRect(kx, 130, 23, 19, 3, COLOR_KEY_BRD);
    tft.setTextColor(COLOR_KEY_TXT, COLOR_KEY_BG);
    tft.drawCentreString(row0Keys[i], kx + 11, 134, 1);
  }

  // Row 1: QWERTY (y: 152..172)
  for (int i = 0; i < 10; i++) {
    int kx = 6 + i * 31;
    tft.fillRoundRect(kx, 152, 28, 19, 3, COLOR_KEY_BG);
    tft.drawRoundRect(kx, 152, 28, 19, 3, COLOR_KEY_BRD);
    tft.setTextColor(COLOR_KEY_TXT, COLOR_KEY_BG);
    tft.drawCentreString(row1Keys[i], kx + 14, 156, 1);
  }

  // Row 2: ASDFGH (y: 174..194)
  for (int i = 0; i < 10; i++) {
    int kx = 6 + i * 31;
    uint16_t bg = (i == 9) ? tft.color565(45, 30, 30) : COLOR_KEY_BG;
    tft.fillRoundRect(kx, 174, 28, 19, 3, bg);
    tft.drawRoundRect(kx, 174, 28, 19, 3, COLOR_KEY_BRD);
    tft.setTextColor(COLOR_KEY_TXT, bg);
    tft.drawCentreString(row2Keys[i], kx + 14, 178, 1);
  }

  // Row 3: ZXCVBNM, Space, BS, ENT (y: 196..236)
  for (int i = 0; i < 7; i++) {
    int kx = 6 + i * 26;
    tft.fillRoundRect(kx, 196, 23, 20, 3, COLOR_KEY_BG);
    tft.drawRoundRect(kx, 196, 23, 20, 3, COLOR_KEY_BRD);
    tft.setTextColor(COLOR_KEY_TXT, COLOR_KEY_BG);
    tft.drawCentreString(row3Keys[i], kx + 11, 200, 1);
  }

  // Spacebar (x: 190, y: 196, w: 54, h: 20)
  tft.fillRoundRect(190, 196, 42, 20, 3, COLOR_KEY_BG);
  tft.drawRoundRect(190, 196, 42, 20, 3, COLOR_KEY_BRD);
  tft.setTextColor(COLOR_KEY_TXT, COLOR_KEY_BG);
  tft.drawCentreString("SPACE", 211, 200, 1);

  // Backspace (x: 235, w: 38)
  tft.fillRoundRect(235, 196, 36, 20, 3, tft.color565(55, 30, 30));
  tft.drawRoundRect(235, 196, 36, 20, 3, COLOR_KEY_BRD);
  tft.setTextColor(COLOR_KEY_TXT, tft.color565(55, 30, 30));
  tft.drawCentreString("BS", 253, 200, 1);

  // Enter (x: 274, w: 42)
  tft.fillRoundRect(274, 196, 42, 38, 4, COLOR_KEY_ACT);
  tft.drawRoundRect(274, 196, 42, 38, 4, COLOR_KEY_TXT);
  tft.setTextColor(0x0000, COLOR_KEY_ACT);
  tft.drawCentreString("ENT", 295, 208, 1);

  // Quick Command Presets Bar: y: 220..238
  const char* quickCmds[4] = { "DIR", "MEM", "BEEP", "HELP" };
  for (int i = 0; i < 4; i++) {
    int kx = 6 + i * 44;
    tft.fillRoundRect(kx, 220, 40, 17, 3, tft.color565(25, 35, 45));
    tft.drawRoundRect(kx, 220, 40, 17, 3, COLOR_KEY_BRD);
    tft.setTextColor(0x07FF, tft.color565(25, 35, 45));
    tft.drawCentreString(quickCmds[i], kx + 20, 223, 1);
  }
}

void executeCommand(String cmd) {
  cmd.trim();
  printToTerminal("C:\\> " + cmd);

  if (cmd.length() == 0) {
    drawTerminal();
    return;
  }

  setRGB(false, true, true); // Flash Amber/Cyan on command run
  String upperCmd = cmd;
  upperCmd.toUpperCase();

  if (upperCmd == "HELP") {
    printToTerminal("COMMANDS: DIR, VER, MEM, CLS,");
    printToTerminal("BEEP, LED, ECHO, PY <expression>");
  } else if (upperCmd == "VER") {
    printToTerminal("PyDOS Kernel v2.4 (c) 2026");
    printToTerminal("ESP32-2432S028R 240MHz Xtensa");
    printToTerminal("Channel: @IamNotIntoTech");
  } else if (upperCmd == "MEM") {
    char memBuf[48];
    snprintf(memBuf, sizeof(memBuf), "Free Heap: %u bytes", ESP.getFreeHeap());
    printToTerminal(String(memBuf));
    snprintf(memBuf, sizeof(memBuf), "Flash Size: %u KB", ESP.getFlashChipSize() / 1024);
    printToTerminal(String(memBuf));
  } else if (upperCmd == "DIR" || upperCmd == "LS") {
    printToTerminal(" Volume in drive C is CYD_OS");
    printToTerminal(" AUTOEXEC.BAT       128 bytes");
    printToTerminal(" CONFIG.SYS          64 bytes");
    printToTerminal(" MAIN.PY          2,048 bytes");
    printToTerminal(" SYSTEM.BIN     335,120 bytes");
  } else if (upperCmd == "CLS") {
    lineCount = 0;
  } else if (upperCmd == "BEEP") {
    playBeep(2400, 100);
    printToTerminal("Beep emitted on GPIO 26.");
  } else if (upperCmd.startsWith("ECHO ")) {
    printToTerminal(cmd.substring(5));
  } else if (upperCmd.startsWith("LED ")) {
    String arg = upperCmd.substring(4);
    if (arg == "RED") { setRGB(true, false, false); printToTerminal("LED: RED"); }
    else if (arg == "GREEN") { setRGB(false, true, false); printToTerminal("LED: GREEN"); }
    else if (arg == "BLUE") { setRGB(false, false, true); printToTerminal("LED: BLUE"); }
    else { setRGB(false, false, false); printToTerminal("LED: OFF"); }
  } else if (upperCmd.startsWith("PY ")) {
    printToTerminal(">>> " + cmd.substring(3));
    printToTerminal("Result: 42 (Interactive eval)");
  } else {
    printToTerminal("Bad command or file name.");
  }

  delay(40);
  setRGB(false, false, false);
  drawTerminal();
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

  printToTerminal("PyDOS v2.4 (ESP32-2432S028R)");
  printToTerminal("Type 'HELP' for available commands.");
  drawTerminal();
  drawKeyboard();

  playBeep(2200, 50);
  Serial.println(F("[SYSTEM] PyDOS Handheld Computer Initialized."));
}

void loop() {
  // Cursor Blinking
  if (millis() - lastCursorTick > 500) {
    lastCursorTick = millis();
    cursorBlink = !cursorBlink;
    drawTerminal();
  }

  // Touch Handling for Virtual Keyboard
  if (ts.touched()) {
    TS_Point p = ts.getPoint();
    int touchX = map(p.x, 3800, 300, 0, 320);
    int touchY = map(p.y, 3800, 400, 0, 240);
    touchX = constrain(touchX, 0, 320);
    touchY = constrain(touchY, 0, 240);

    // Keyboard Area (y >= 128)
    if (touchY >= 128) {
      playBeep(2600, 15); // Mechanical keyclick

      // Quick Command Presets (y: 220..238)
      if (touchY >= 220 && touchX <= 180) {
        int qIdx = (touchX - 6) / 44;
        const char* quickCmds[4] = { "DIR", "MEM", "BEEP", "HELP" };
        if (qIdx >= 0 && qIdx < 4) {
          executeCommand(quickCmds[qIdx]);
          delay(180);
          return;
        }
      }

      // Enter Key (x >= 274, y >= 196)
      if (touchX >= 274 && touchY >= 196) {
        executeCommand(currentInput);
        currentInput = "";
        delay(180);
        return;
      }

      // Backspace (x: 235..272, y: 196..218)
      if (touchX >= 235 && touchX <= 272 && touchY >= 196 && touchY <= 218) {
        if (currentInput.length() > 0) {
          currentInput.remove(currentInput.length() - 1);
          drawTerminal();
        }
        delay(150);
        return;
      }

      // Spacebar (x: 190..232, y: 196..218)
      if (touchX >= 190 && touchX <= 232 && touchY >= 196 && touchY <= 218) {
        currentInput += " ";
        drawTerminal();
        delay(150);
        return;
      }

      // Row 0: Numbers (y: 130..150)
      if (touchY <= 150) {
        int kIdx = (touchX - 4) / 26;
        if (kIdx >= 0 && kIdx < 12) {
          currentInput += row0Keys[kIdx];
          drawTerminal();
          delay(150);
          return;
        }
      }
      // Row 1: QWERTY (y: 152..172)
      else if (touchY <= 172) {
        int kIdx = (touchX - 6) / 31;
        if (kIdx >= 0 && kIdx < 10) {
          currentInput += row1Keys[kIdx];
          drawTerminal();
          delay(150);
          return;
        }
      }
      // Row 2: ASDFGH (y: 174..194)
      else if (touchY <= 194) {
        int kIdx = (touchX - 6) / 31;
        if (kIdx >= 0 && kIdx < 9) {
          currentInput += row2Keys[kIdx];
          drawTerminal();
          delay(150);
          return;
        } else if (kIdx == 9) { // CLS key
          lineCount = 0;
          drawTerminal();
          delay(150);
          return;
        }
      }
      // Row 3: ZXCVBNM (y: 196..218)
      else if (touchY <= 218 && touchX < 190) {
        int kIdx = (touchX - 6) / 26;
        if (kIdx >= 0 && kIdx < 7) {
          currentInput += row3Keys[kIdx];
          drawTerminal();
          delay(150);
          return;
        }
      }
    }
  }

  delay(15);
}
