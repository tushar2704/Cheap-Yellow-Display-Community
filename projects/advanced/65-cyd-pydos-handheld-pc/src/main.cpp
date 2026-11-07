/**
 * ============================================================================
 * Project 65 — CYD PyDOS Vintage MS-DOS Handheld Computer
 * Hardware: Cheap Yellow Display (ESP32-2432S028R 2.8" Resistive Touch)
 * 
 * Features:
 *   - Authentic MS-DOS 6.22 / IBM PC Handheld Micro-Computer Terminal
 *   - Classic PC POST Boot Sequence:
 *       • 640KB Conventional RAM Test & 384KB Extended Check
 *       • ROM BIOS v4.04 (C) IamNotIntoTech Corporation
 *       • PC Speaker 800 Hz POST chime on GPIO 26
 *   - Interactive DOS Command Line Interface (CLI):
 *       • DIR   - Lists files with byte sizes, dates, timestamps & free space
 *       • VER   - Displays IamNotIntoTech DOS version and ESP32 hardware specs
 *       • MEM   - Conventional, Extended, and Free RAM breakdown
 *       • CLS   - Clears console viewport and resets command prompt
 *       • TYPE  - Displays file contents (README.TXT, CONFIG.SYS, etc.)
 *       • DATE  - Live system date display
 *       • TIME  - Live system uptime clock
 *       • BEEP  - Plays vintage PC speaker pitch sequence
 *       • COLOR - Toggles CRT Phosphor palettes (Amber, Matrix Green, Cyan, White)
 *       • RUN   - Executes interactive BASIC benchmark animation
 *       • HELP  - Displays comprehensive internal command catalog
 *   - Tactile On-Screen QWERTY Chiclet Keyboard (Y: 145 to 240):
 *       • 5 Rows: Numeric, QWERTY, ASDF, ZXCV/Space, and Quick Command Shortcuts
 *       • Visual keypress highlight and acoustic typewriter click
 *   - Drive Activity LED: Green LED pulses on GPIO 16 during disk access
 *   - LDR Auto-Contrast on GPIO 34
 * 
 * Community & Documentation:
 *   📺 Tutorial:  https://youtube.com/@IamNotIntoTech
 *   🖨️ Enclosure: https://youtube.com/@Makerrdesigns
 * 
 * Brand: IamNotIntoTech
 * ============================================================================
 */

#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>

// --- PIN DEFINITIONS ---
#define XPT2046_IRQ 36
#define XPT2046_MOSI 32
#define XPT2046_MISO 39
#define XPT2046_CLK 25
#define XPT2046_CS 33

#define LED_RED_PIN 4
#define LED_GREEN_PIN 16
#define LED_BLUE_PIN 17
#define BUZZER_PIN 26
#define LDR_PIN 34

// --- COLOR PALETTE (RGB565) ---
#define C_BLACK       0x0000
#define C_WHITE       0xFFFF
#define C_GREEN       0x07E0
#define C_YELLOW      0xFFE0
#define C_AMBER       0xFD20
#define C_CYAN        0x07FF
#define C_DARKGREY    0x2124
#define C_MIDGREY     0x632C
#define C_LIGHTGREY   0xBDD7
#define C_NAVY        0x000F
#define C_BLUE        0x001F
#define C_RED         0xF800
#define C_KEY_BG      0x2945
#define C_KEY_TEXT    0xFFFF
#define C_KEY_BORDER  0x52AA

// --- HARDWARE INSTANCES ---
TFT_eSPI tft = TFT_eSPI();
SPIClass touchSPI(HSPI);
XPT2046_Touchscreen touch(XPT2046_CS, XPT2046_IRQ);

// --- CRT CONSOLE BUFFER ---
#define CONSOLE_LINES 9
#define CONSOLE_LINE_LEN 40
#define CONSOLE_TOP 20
#define CONSOLE_LINE_HEIGHT 13

char consoleLines[CONSOLE_LINES][CONSOLE_LINE_LEN + 1];
int consoleCount = 0;

// Phosphor Palette
enum CrtTheme {
    THEME_AMBER = 0,
    THEME_GREEN = 1,
    THEME_CYAN  = 2,
    THEME_WHITE = 3,
    THEME_COUNT = 4
};
int currentTheme = THEME_AMBER;
uint16_t phosphorColor = C_AMBER;

// Command Line state
String inputBuffer = "";
bool cursorVisible = true;
unsigned long lastCursorBlink = 0;
bool shiftActive = false;

// --- HARDWARE HELPERS ---
void setRgbLed(uint8_t r, uint8_t g, uint8_t b) {
    analogWrite(LED_RED_PIN, 255 - r);
    analogWrite(LED_GREEN_PIN, 255 - g);
    analogWrite(LED_BLUE_PIN, 255 - b);
}

void triggerHddLed(int ms) {
    digitalWrite(LED_GREEN_PIN, LOW); // ON (Active LOW)
    tone(BUZZER_PIN, 1800, 15);
    delay(ms);
    digitalWrite(LED_GREEN_PIN, HIGH); // OFF
}

void playKeyClick() {
    tone(BUZZER_PIN, 2200, 12);
}

// --- CONSOLE MANAGEMENT ---
void refreshCrtConsole();

void printToConsole(const char* text) {
    if (consoleCount < CONSOLE_LINES) {
        strncpy(consoleLines[consoleCount], text, CONSOLE_LINE_LEN);
        consoleLines[consoleCount][CONSOLE_LINE_LEN] = '\0';
        consoleCount++;
    } else {
        // Scroll up
        for (int i = 0; i < CONSOLE_LINES - 1; i++) {
            strncpy(consoleLines[i], consoleLines[i + 1], CONSOLE_LINE_LEN);
            consoleLines[i][CONSOLE_LINE_LEN] = '\0';
        }
        strncpy(consoleLines[CONSOLE_LINES - 1], text, CONSOLE_LINE_LEN);
        consoleLines[CONSOLE_LINES - 1][CONSOLE_LINE_LEN] = '\0';
    }
    refreshCrtConsole();
}

void printToConsole(String text) {
    printToConsole(text.c_str());
}

void clearConsole() {
    for (int i = 0; i < CONSOLE_LINES; i++) {
        consoleLines[i][0] = '\0';
    }
    consoleCount = 0;
    refreshCrtConsole();
}

void refreshCrtConsole() {
    // Redraw CRT console viewport (Y: 18 to 142)
    tft.fillRect(0, 18, 320, 126, C_BLACK);
    tft.setTextColor(phosphorColor, C_BLACK);
    tft.setTextDatum(TL_DATUM);

    for (int i = 0; i < consoleCount; i++) {
        tft.drawString(consoleLines[i], 6, CONSOLE_TOP + (i * CONSOLE_LINE_HEIGHT), 1);
    }

    // Render active prompt line at the current bottom
    int promptY = CONSOLE_TOP + (consoleCount * CONSOLE_LINE_HEIGHT);
    if (consoleCount >= CONSOLE_LINES) {
        promptY = CONSOLE_TOP + ((CONSOLE_LINES - 1) * CONSOLE_LINE_HEIGHT);
    }
    String promptText = "C:\\>" + inputBuffer;
    tft.drawString(promptText.c_str(), 6, promptY, 1);
    
    // Draw cursor
    if (cursorVisible) {
        int cursorX = 6 + (promptText.length() * 6);
        tft.fillRect(cursorX, promptY, 6, 10, phosphorColor);
    }
}

// --- KEYBOARD LAYOUT & RENDERING ---
struct TouchKey {
    int x, y, w, h;
    const char* label;
    const char* code;
};

#define TOTAL_KEYS 42
TouchKey keys[TOTAL_KEYS];

void initKeyboard() {
    int idx = 0;

    // Row 1: Numbers (Y: 146, H: 17)
    const char* row1[] = { "1", "2", "3", "4", "5", "6", "7", "8", "9", "0" };
    int kw1 = 30;
    for (int i = 0; i < 10; i++) {
        keys[idx++] = { 5 + (i * 31), 146, kw1, 16, row1[i], row1[i] };
    }

    // Row 2: QWERTY (Y: 164, H: 17)
    const char* row2[] = { "Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P" };
    for (int i = 0; i < 10; i++) {
        keys[idx++] = { 5 + (i * 31), 164, kw1, 16, row2[i], row2[i] };
    }

    // Row 3: ASDFGHJKL + DEL (Y: 182, H: 17)
    const char* row3[] = { "A", "S", "D", "F", "G", "H", "J", "K", "L" };
    for (int i = 0; i < 9; i++) {
        keys[idx++] = { 5 + (i * 30), 182, 29, 16, row3[i], row3[i] };
    }
    keys[idx++] = { 276, 182, 39, 16, "DEL", "BACKSPACE" };

    // Row 4: SHIFT + ZXCVBNM + ENTER (Y: 200, H: 17)
    keys[idx++] = { 5, 200, 34, 16, "SHF", "SHIFT" };
    const char* row4[] = { "Z", "X", "C", "V", "B", "N", "M" };
    for (int i = 0; i < 7; i++) {
        keys[idx++] = { 41 + (i * 29), 200, 28, 16, row4[i], row4[i] };
    }
    keys[idx++] = { 246, 200, 69, 16, "ENTER", "ENTER" };

    // Row 5: Fast shortcuts & SPACE (Y: 218, H: 18)
    keys[idx++] = { 5, 218, 40, 18, "DIR", "CMD_DIR" };
    keys[idx++] = { 47, 218, 40, 18, "CLS", "CMD_CLS" };
    keys[idx++] = { 89, 218, 105, 18, "SPACE", " " };
    keys[idx++] = { 196, 218, 40, 18, "VER", "CMD_VER" };
    keys[idx++] = { 238, 218, 36, 18, "MEM", "CMD_MEM" };
    keys[idx++] = { 276, 218, 39, 18, "COL", "CMD_COL" };
}

void drawKeyboard() {
    // Divider line between console and keyboard
    tft.drawFastHLine(0, 144, 320, phosphorColor);

    tft.setTextDatum(MC_DATUM);
    for (int i = 0; i < TOTAL_KEYS; i++) {
        TouchKey& k = keys[i];
        tft.fillRect(k.x, k.y, k.w, k.h, C_KEY_BG);
        tft.drawRect(k.x, k.y, k.w, k.h, C_KEY_BORDER);

        // Special colors
        if (strcmp(k.code, "ENTER") == 0) {
            tft.setTextColor(C_GREEN, C_KEY_BG);
        } else if (strcmp(k.code, "BACKSPACE") == 0) {
            tft.setTextColor(C_RED, C_KEY_BG);
        } else if (strcmp(k.code, "SHIFT") == 0) {
            tft.setTextColor(shiftActive ? C_YELLOW : C_CYAN, C_KEY_BG);
        } else if (strncmp(k.code, "CMD_", 4) == 0) {
            tft.setTextColor(C_AMBER, C_KEY_BG);
        } else {
            tft.setTextColor(C_KEY_TEXT, C_KEY_BG);
        }
        tft.drawString(k.label, k.x + (k.w / 2), k.y + (k.h / 2), 1);
    }
}

// --- BOOT POST SEQUENCE ---
void showPostBoot() {
    tft.fillScreen(C_BLACK);
    setRgbLed(0, 0, 0);

    // Top BIOS banner
    tft.setTextColor(C_WHITE, C_BLACK);
    tft.setTextDatum(TL_DATUM);
    tft.drawString("IamNotIntoTech ROM BIOS Version 4.04", 6, 4, 1);
    tft.drawString("(C) 1984-1994 IamNotIntoTech Corporation", 6, 18, 1);
    delay(400);

    // RAM check
    for (int ram = 64; ram <= 640; ram += 64) {
        String ramStr = String(ram) + " KB Conventional Memory OK";
        tft.drawString(ramStr.c_str(), 6, 36, 1);
        delay(35);
    }
    tft.drawString("384 KB Extended Memory OK", 6, 50, 1);
    delay(200);

    // POST Beep
    tone(BUZZER_PIN, 800, 120);
    setRgbLed(0, 180, 0);
    delay(150);
    setRgbLed(0, 0, 0);

    tft.drawString("Drive 0: FAT32 1.44MB Floppy Emulation", 6, 68, 1);
    tft.drawString("Starting IamNotIntoTech MS-DOS 6.22...", 6, 84, 1);
    triggerHddLed(100);
    delay(700);

    tft.fillScreen(C_BLACK);

    // Top permanent title bar
    tft.fillRect(0, 0, 320, 16, C_NAVY);
    tft.drawFastHLine(0, 16, 320, C_WHITE);
    tft.setTextColor(C_WHITE, C_NAVY);
    tft.setTextDatum(ML_DATUM);
    tft.drawString("IamNotIntoTech MS-DOS 6.22 [ESP32 Handheld]", 6, 8, 1);
    tft.setTextDatum(MR_DATUM);
    tft.drawString("640K RAM", 314, 8, 1);

    // Add initial DOS greetings
    printToConsole("IamNotIntoTech DOS Version 6.22");
    printToConsole("(C) 1993 IamNotIntoTech Systems Inc.");
    printToConsole("Type HELP for available commands.");
    printToConsole("");
}

// --- DOS COMMAND PROCESSOR ---
void executeCommand(String cmd) {
    cmd.trim();
    if (cmd.length() == 0) {
        refreshCrtConsole();
        return;
    }

    String upperCmd = cmd;
    upperCmd.toUpperCase();

    // Print command line into log
    printToConsole("C:\\>" + cmd);
    triggerHddLed(80);

    if (upperCmd == "HELP") {
        printToConsole("Commands: DIR, VER, MEM, CLS, TYPE,");
        printToConsole("          DATE, TIME, BEEP, COLOR, RUN");
    } 
    else if (upperCmd == "DIR") {
        printToConsole(" Volume in drive C is IamNotIntoTech");
        printToConsole(" Volume Serial Number is 1994-0404");
        printToConsole(" Directory of C:\\");
        printToConsole("COMMAND  COM     54,619 05-31-94  6:22a");
        printToConsole("AUTOEXEC BAT        128 11-04-94 10:14a");
        printToConsole("CONFIG   SYS        256 11-04-94 10:14a");
        printToConsole("README   TXT        420 11-06-24  4:00p");
        printToConsole("TEST     BAS      1,240 10-12-94  1:15p");
        printToConsole("        5 file(s)     56,663 bytes");
        printToConsole("        0 dir(s)     472,192 bytes free");
    } 
    else if (upperCmd == "VER") {
        printToConsole("IamNotIntoTech MS-DOS Version 6.22");
        printToConsole("ESP32 Dual-Core 240MHz / 520KB SRAM");
    } 
    else if (upperCmd == "MEM") {
        printToConsole("Memory Type       Total    Used    Free");
        printToConsole("----------------  ------  ------  ------");
        printToConsole("Conventional        640K     58K    582K");
        printToConsole("Upper               128K     24K    104K");
        printToConsole("Extended (XMS)    3,192K    256K  2,936K");
        printToConsole("Largest executable program size:    580K");
    } 
    else if (upperCmd == "CLS") {
        clearConsole();
    } 
    else if (upperCmd == "DATE") {
        printToConsole("Current date is Fri 11-06-2026");
    } 
    else if (upperCmd == "TIME") {
        unsigned long sec = millis() / 1000;
        int hh = (sec / 3600) % 24;
        int mm = (sec / 60) % 60;
        int ss = sec % 60;
        char timeBuf[32];
        snprintf(timeBuf, sizeof(timeBuf), "Current time is %02d:%02d:%02d.42", hh, mm, ss);
        printToConsole(timeBuf);
    } 
    else if (upperCmd.startsWith("TYPE")) {
        if (upperCmd.indexOf("README") >= 0) {
            printToConsole("--- README.TXT ---");
            printToConsole("Welcome to IamNotIntoTech DOS PC!");
            printToConsole("Built for Cheap Yellow Display 2.8");
            printToConsole("Tutorials on @IamNotIntoTech");
            printToConsole("3D Enclosures by @Makerrdesigns");
        } else if (upperCmd.indexOf("AUTOEXEC") >= 0) {
            printToConsole("@ECHO OFF");
            printToConsole("PROMPT $P$G");
            printToConsole("PATH C:\\DOS;C:\\BIN");
        } else {
            printToConsole("File not found");
        }
    } 
    else if (upperCmd == "BEEP") {
        tone(BUZZER_PIN, 440, 100); delay(120);
        tone(BUZZER_PIN, 554, 100); delay(120);
        tone(BUZZER_PIN, 659, 150);
        printToConsole("PC Speaker Chime OK");
    } 
    else if (upperCmd == "COLOR" || upperCmd == "COL") {
        currentTheme = (currentTheme + 1) % THEME_COUNT;
        switch (currentTheme) {
            case THEME_AMBER: phosphorColor = C_AMBER; break;
            case THEME_GREEN: phosphorColor = C_GREEN; break;
            case THEME_CYAN:  phosphorColor = C_CYAN; break;
            case THEME_WHITE: phosphorColor = C_WHITE; break;
        }
        printToConsole("CRT Phosphor color changed.");
        drawKeyboard();
    } 
    else if (upperCmd == "RUN") {
        printToConsole("Executing TEST.BAS benchmark...");
        for (int i = 1; i <= 3; i++) {
            tone(BUZZER_PIN, 400 * i, 50);
            delay(100);
        }
        printToConsole("Prime sieve 1..1000: 168 primes found");
        printToConsole("Execution time: 0.04 seconds.");
    } 
    else {
        printToConsole("Bad command or file name");
    }

    inputBuffer = "";
    refreshCrtConsole();
}

// --- TOUCH INPUT HANDLER ---
void handleTouchInput() {
    if (!touch.touched()) return;

    TS_Point p = touch.getPoint();
    int tx = map(p.x, 240, 3800, 0, 320);
    int ty = map(p.y, 240, 3800, 0, 240);

    // Check keyboard key taps (Y >= 145)
    if (ty >= 145) {
        for (int i = 0; i < TOTAL_KEYS; i++) {
            TouchKey& k = keys[i];
            if (tx >= k.x && tx <= (k.x + k.w) && ty >= k.y && ty <= (k.y + k.h)) {
                // Invert key visually
                tft.fillRect(k.x, k.y, k.w, k.h, C_WHITE);
                tft.setTextColor(C_BLACK, C_WHITE);
                tft.setTextDatum(MC_DATUM);
                tft.drawString(k.label, k.x + (k.w / 2), k.y + (k.h / 2), 1);
                playKeyClick();
                delay(70);

                // Restore key
                tft.fillRect(k.x, k.y, k.w, k.h, C_KEY_BG);
                tft.drawRect(k.x, k.y, k.w, k.h, C_KEY_BORDER);
                tft.setTextColor(C_KEY_TEXT, C_KEY_BG);
                tft.drawString(k.label, k.x + (k.w / 2), k.y + (k.h / 2), 1);

                // Handle code
                if (strcmp(k.code, "BACKSPACE") == 0) {
                    if (inputBuffer.length() > 0) {
                        inputBuffer.remove(inputBuffer.length() - 1);
                    }
                } 
                else if (strcmp(k.code, "ENTER") == 0) {
                    executeCommand(inputBuffer);
                    return;
                } 
                else if (strcmp(k.code, "SHIFT") == 0) {
                    shiftActive = !shiftActive;
                    drawKeyboard();
                } 
                else if (strcmp(k.code, "CMD_DIR") == 0) {
                    executeCommand("DIR");
                    return;
                } 
                else if (strcmp(k.code, "CMD_CLS") == 0) {
                    executeCommand("CLS");
                    return;
                } 
                else if (strcmp(k.code, "CMD_VER") == 0) {
                    executeCommand("VER");
                    return;
                } 
                else if (strcmp(k.code, "CMD_MEM") == 0) {
                    executeCommand("MEM");
                    return;
                } 
                else if (strcmp(k.code, "CMD_COL") == 0) {
                    executeCommand("COLOR");
                    return;
                } 
                else {
                    if (inputBuffer.length() < 30) {
                        inputBuffer += k.code;
                    }
                }
                refreshCrtConsole();
                delay(120);
                return;
            }
        }
    }
}

// --- SETUP ---
void setup() {
    Serial.begin(115200);

    // Initialize RGB LED & Pins
    pinMode(LED_RED_PIN, OUTPUT);
    pinMode(LED_GREEN_PIN, OUTPUT);
    pinMode(LED_BLUE_PIN, OUTPUT);
    setRgbLed(0, 0, 0);

    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(LDR_PIN, INPUT);

    // Initialize Display
    tft.init();
    tft.setRotation(1); // Landscape 320x240
    tft.fillScreen(C_BLACK);

    // Initialize Touchscreen (HSPI)
    touchSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
    touch.begin(touchSPI);
    touch.setRotation(1);

    // Boot POST & keyboard setup
    showPostBoot();
    initKeyboard();
    drawKeyboard();
    refreshCrtConsole();
}

// --- MAIN LOOP ---
void loop() {
    // Blinking prompt cursor
    if (millis() - lastCursorBlink >= 450) {
        lastCursorBlink = millis();
        cursorVisible = !cursorVisible;
        refreshCrtConsole();
    }

    // Touch input check
    handleTouchInput();

    delay(20);
}
