// =========================================================================
//  Project 51 — CYD Cydintosh 1984 Retro Mac Emulator
//  Cheap Yellow Display (ESP32-2432S028R)
//
//  📺 Tutorial:  https://youtube.com/@IamNotIntoTech
//  🖨️ Enclosure: https://youtube.com/@Makerrdesigns
//
//  Branding: IamNotIntoTech
// =========================================================================

#include <Arduino.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include <SPI.h>

// =========================================================================
//  HARDWARE PINS & CALIBRATION (CYD 2.8" ESP32-2432S028R)
// =========================================================================
#define TOUCH_CS   33
#define TOUCH_CLK  25
#define TOUCH_MISO 39
#define TOUCH_MOSI 32
#define TOUCH_IRQ  36

#define LED_R      4
#define LED_G      16
#define LED_B      17

#define BUZZER_PIN 26
#define LDR_PIN    34

#define TS_MIN_X 240
#define TS_MAX_X 3800
#define TS_MIN_Y 240
#define TS_MAX_Y 3800

// Classic 1-Bit Macintosh Monochrome Palette mapped to 16-bit 565
#define C_WHITE  0xFFFF  // 1-bit Paper White
#define C_BLACK  0x0000  // 1-bit CRT Black
#define C_GRAY   0x8410  // 50% Desktop Dither
#define C_BEIGE  0xEF7B  // Vintage 1984 Mac Case Beige

TFT_eSPI tft = TFT_eSPI();
SPIClass touchSPI(HSPI);
XPT2046_Touchscreen touch(TOUCH_CS, TOUCH_IRQ);

// =========================================================================
//  SYSTEM & WINDOW MANAGER STATE
// =========================================================================
enum AppMode {
    APP_DESKTOP,
    APP_MACPAINT,
    APP_CALCULATOR,
    APP_ABOUT_MAC
};

AppMode activeApp = APP_DESKTOP;

// Calculator state
String calcDisplay = "0";
float calcAccumulator = 0.0f;
char calcPendingOp = '\0';
bool calcClearOnNext = false;

// MacPaint state
uint8_t currentTool = 0; // 0=Pencil, 1=Brush, 2=Eraser
#define CANVAS_X 16
#define CANVAS_Y 52
#define CANVAS_W 240
#define CANVAS_H 150

unsigned long lastTouchTime = 0;
int lastTouchX = -1;
int lastTouchY = -1;

// Forward declarations
void playMacChime();
void playClick();
void drawDesktop();
void drawMenuBar();
void drawWindowFrame(int x, int y, int w, int h, const char* title);
void openAboutMac();
void openCalculator();
void openMacPaint();
void handleTouch();

// =========================================================================
//  AUDIO ENGINE — 1984 MAC CHIMES
// =========================================================================
void playTone(int freq, int durationMs) {
    tone(BUZZER_PIN, freq, durationMs);
}

void playClick() {
    playTone(2400, 15);
}

void playMacChime() {
    // Iconic F-Major ascending vintage boot chord arpeggio
    playTone(349, 120); // F4
    delay(120);
    playTone(440, 120); // A4
    delay(120);
    playTone(523, 140); // C5
    delay(140);
    playTone(698, 400); // F5 (Sustained)
    delay(400);
}

// =========================================================================
//  BITMAP ICONS (Happy Mac, HD, Trash, Paint, Calc)
// =========================================================================
void drawHappyMac(int x, int y) {
    // 24x28 pixel Happy Mac icon
    tft.drawRect(x, y, 24, 28, C_BLACK);
    tft.fillRect(x + 2, y + 2, 20, 24, C_WHITE);
    // Bezel
    tft.drawRect(x + 3, y + 3, 18, 14, C_BLACK);
    // Eyes
    tft.drawPixel(x + 8, y + 7, C_BLACK);
    tft.drawPixel(x + 16, y + 7, C_BLACK);
    // Smiling mouth
    tft.drawPixel(x + 8, y + 12, C_BLACK);
    tft.drawLine(x + 9, y + 13, x + 15, y + 13, C_BLACK);
    tft.drawPixel(x + 16, y + 12, C_BLACK);
    // Floppy slot
    tft.drawFastHLine(x + 6, y + 22, 12, C_BLACK);
}

void drawTrashCan(int x, int y) {
    // Classic 1984 Trash can
    tft.fillRect(x, y, 26, 30, C_WHITE);
    tft.drawRect(x + 2, y + 4, 22, 2, C_BLACK); // Lid
    tft.drawFastHLine(x + 8, y + 2, 10, C_BLACK); // Handle
    tft.drawLine(x + 4, y + 6, x + 6, y + 26, C_BLACK); // Left wall
    tft.drawLine(x + 22, y + 6, x + 20, y + 26, C_BLACK); // Right wall
    tft.drawFastHLine(x + 6, y + 26, 14, C_BLACK); // Bottom
    // Vertical slats
    tft.drawFastVLine(x + 10, y + 8, 16, C_BLACK);
    tft.drawFastVLine(x + 13, y + 8, 16, C_BLACK);
    tft.drawFastVLine(x + 16, y + 8, 16, C_BLACK);
}

void drawDiskDrive(int x, int y) {
    // Classic Floppy Drive
    tft.fillRect(x, y, 28, 24, C_WHITE);
    tft.drawRect(x, y, 28, 24, C_BLACK);
    tft.drawFastHLine(x + 4, y + 10, 20, C_BLACK); // Slot
    tft.drawRect(x + 4, y + 16, 6, 4, C_BLACK);   // Eject button
    tft.fillRect(x + 20, y + 16, 4, 3, C_BLACK);  // LED
}

// =========================================================================
//  WINDOW & DESKTOP RENDERING
// =========================================================================
void drawDitheredDesktop() {
    // 50% checkerboard dither pattern: alternating pixels
    for (int y = 20; y < 240; y += 2) {
        for (int x = 0; x < 320; x += 2) {
            tft.drawPixel(x, y, C_WHITE);
            tft.drawPixel(x + 1, y, C_GRAY);
            tft.drawPixel(x, y + 1, C_GRAY);
            tft.drawPixel(x + 1, y + 1, C_WHITE);
        }
    }
}

void drawMenuBar() {
    tft.fillRect(0, 0, 320, 18, C_WHITE);
    tft.drawFastHLine(0, 18, 320, C_BLACK);

    // Apple symbol 
    tft.fillCircle(10, 8, 4, C_BLACK);
    tft.drawPixel(10, 3, C_BLACK);

    tft.setTextColor(C_BLACK, C_WHITE);
    tft.drawString("File", 30, 4, 1);
    tft.drawString("Edit", 65, 4, 1);
    tft.drawString("View", 100, 4, 1);
    tft.drawString("Special", 135, 4, 1);

    // Channel Branding in menu bar!
    tft.drawString("IamNotIntoTech", 225, 4, 1);
}

void drawWindowFrame(int x, int y, int w, int h, const char* title) {
    // Drop shadow
    tft.fillRect(x + 3, y + 3, w, h, C_BLACK);
    // Window Body
    tft.fillRect(x, y, w, h, C_WHITE);
    tft.drawRect(x, y, w, h, C_BLACK);

    // Titlebar
    tft.fillRect(x + 1, y + 1, w - 2, 16, C_WHITE);
    tft.drawFastHLine(x, y + 17, w, C_BLACK);

    // Striped horizontal lines across titlebar
    for (int ly = y + 4; ly <= y + 14; ly += 2) {
        tft.drawFastHLine(x + 20, ly, w - 40, C_BLACK);
    }

    // Title string with white background mask
    int textW = strlen(title) * 6 + 10;
    int textX = x + (w - textW) / 2;
    tft.fillRect(textX, y + 3, textW, 12, C_WHITE);
    tft.setTextColor(C_BLACK, C_WHITE);
    tft.drawString(title, textX + 5, y + 4, 1);

    // Classic Close Box (top left)
    tft.drawRect(x + 5, y + 4, 10, 10, C_BLACK);
}

void drawDesktop() {
    drawDitheredDesktop();
    drawMenuBar();

    // Desktop Icon 1: Macintosh HD (Top Right)
    drawDiskDrive(280, 26);
    tft.setTextColor(C_BLACK, C_WHITE);
    tft.drawString("Mac HD", 274, 52, 1);

    // Desktop Icon 2: MacPaint
    tft.fillRect(280, 72, 26, 26, C_WHITE);
    tft.drawRect(280, 72, 26, 26, C_BLACK);
    tft.drawLine(284, 94, 298, 76, C_BLACK); // Paintbrush
    tft.drawString("Paint", 280, 100, 1);

    // Desktop Icon 3: Calculator
    tft.fillRect(280, 118, 26, 26, C_WHITE);
    tft.drawRect(280, 118, 26, 26, C_BLACK);
    tft.drawFastHLine(284, 124, 18, C_BLACK);
    tft.drawPixel(286, 132, C_BLACK);
    tft.drawPixel(294, 132, C_BLACK);
    tft.drawString("Calc", 282, 146, 1);

    // Desktop Icon 4: Trash (Bottom Right)
    drawTrashCan(280, 180);
    tft.drawString("Trash", 278, 212, 1);

    // Welcome Window on Desktop
    drawWindowFrame(18, 40, 240, 160, "Welcome to Macintosh");
    drawHappyMac(30, 60);

    tft.setTextColor(C_BLACK, C_WHITE);
    tft.drawString("Macintosh 128K (1984)", 64, 62, 2);
    tft.drawString("System Software 6.0.8", 64, 82, 1);
    tft.drawString("Motorola 68000 @ 8 MHz", 64, 96, 1);
    tft.drawString("CYD 2.8\" Retro Edition", 64, 110, 1);

    // Branding banner inside window
    tft.drawFastHLine(26, 130, 224, C_BLACK);
    tft.setTextColor(C_BLACK, C_WHITE);
    tft.drawString("YouTube: @IamNotIntoTech", 34, 140, 1);
    tft.drawString("3D Cases: @Makerrdesigns", 34, 154, 1);
    tft.drawString("Tap icons on right to launch apps!", 34, 172, 1);
}

// =========================================================================
//  MINI APPS (MacPaint, Calculator, About Mac)
// =========================================================================
void openAboutMac() {
    activeApp = APP_ABOUT_MAC;
    drawWindowFrame(24, 34, 272, 170, "About the Macintosh");
    drawHappyMac(40, 60);

    tft.setTextColor(C_BLACK, C_WHITE);
    tft.drawString("Macintosh System Software", 80, 58, 2);
    tft.drawString("Finder: 6.1  System: 6.0.8", 80, 80, 1);
    tft.drawString("Total Memory: 128 KB", 80, 96, 1);
    tft.drawString("Largest Unused Block: 64 KB", 80, 110, 1);

    tft.drawFastHLine(36, 130, 248, C_BLACK);
    tft.drawString("Hardware: ESP32-2432S028R", 40, 140, 1);
    tft.drawString("Produced by IamNotIntoTech", 40, 156, 1);
    tft.drawString("[Tap Close Box to Exit]", 75, 178, 1);
}

void openCalculator() {
    activeApp = APP_CALCULATOR;
    drawWindowFrame(60, 28, 200, 195, "Calculator");

    // Display Screen (Recessed black with white text)
    tft.fillRect(72, 48, 176, 26, C_BLACK);
    tft.drawRect(70, 46, 180, 30, C_BLACK);
    tft.setTextColor(C_WHITE, C_BLACK);
    tft.drawString(calcDisplay, 80, 52, 2);

    // Button Matrix
    const char* keys[4][4] = {
        {"7", "8", "9", "/"},
        {"4", "5", "6", "*"},
        {"1", "2", "3", "-"},
        {"C", "0", "=", "+"}
    };

    for (int r = 0; r < 4; r++) {
        for (int c = 0; c < 4; c++) {
            int bx = 74 + (c * 44);
            int by = 84 + (r * 32);
            tft.fillRect(bx, by, 38, 26, C_WHITE);
            tft.drawRect(bx, by, 38, 26, C_BLACK);
            // Bevel
            tft.drawFastHLine(bx + 1, by + 1, 36, C_GRAY);
            tft.drawFastVLine(bx + 1, by + 1, 24, C_GRAY);

            tft.setTextColor(C_BLACK, C_WHITE);
            tft.drawString(keys[r][c], bx + 14, by + 6, 2);
        }
    }
}

void openMacPaint() {
    activeApp = APP_MACPAINT;
    drawWindowFrame(8, 24, 304, 206, "MacPaint — Untitled");

    // Drawing Canvas
    tft.fillRect(CANVAS_X, CANVAS_Y, CANVAS_W, CANVAS_H, C_WHITE);
    tft.drawRect(CANVAS_X - 1, CANVAS_Y - 1, CANVAS_W + 2, CANVAS_H + 2, C_BLACK);

    // Tool Palette (Right column)
    const char* tools[3] = {"Pencil", "Brush", "Eraser"};
    for (int i = 0; i < 3; i++) {
        int ty = 56 + (i * 36);
        tft.fillRect(264, ty, 42, 28, (currentTool == i) ? C_BLACK : C_WHITE);
        tft.drawRect(264, ty, 42, 28, C_BLACK);
        tft.setTextColor((currentTool == i) ? C_WHITE : C_BLACK, (currentTool == i) ? C_BLACK : C_WHITE);
        tft.drawString(tools[i], 268, ty + 8, 1);
    }

    // Clear Button
    tft.fillRect(264, 168, 42, 28, C_WHITE);
    tft.drawRect(264, 168, 42, 28, C_BLACK);
    tft.setTextColor(C_BLACK, C_WHITE);
    tft.drawString("Clear", 270, 176, 1);
}

// =========================================================================
//  TOUCH CONTROLLER & DRIVER
// =========================================================================
void handleTouch() {
    if (!touch.touched()) {
        lastTouchX = -1;
        lastTouchY = -1;
        return;
    }

    TS_Point p = touch.getPoint();
    int tx = map(p.x, TS_MIN_X, TS_MAX_X, 0, 320);
    int ty = map(p.y, TS_MIN_Y, TS_MAX_Y, 0, 240);
    tx = constrain(tx, 0, 320);
    ty = constrain(ty, 0, 240);

    // If MacPaint active and drawing on canvas: stream continuous strokes!
    if (activeApp == APP_MACPAINT && tx >= CANVAS_X && tx <= CANVAS_X + CANVAS_W && ty >= CANVAS_Y && ty <= CANVAS_Y + CANVAS_H) {
        if (currentTool == 0) { // Pencil
            if (lastTouchX > 0 && lastTouchY > 0) tft.drawLine(lastTouchX, lastTouchY, tx, ty, C_BLACK);
            else tft.drawPixel(tx, ty, C_BLACK);
        } else if (currentTool == 1) { // Brush (thicker)
            tft.fillCircle(tx, ty, 3, C_BLACK);
        } else if (currentTool == 2) { // Eraser
            tft.fillCircle(tx, ty, 6, C_WHITE);
        }
        lastTouchX = tx;
        lastTouchY = ty;
        return;
    }

    // Debounce for buttons & window controls
    unsigned long now = millis();
    if (now - lastTouchTime < 250) return;
    lastTouchTime = now;

    playClick();

    // Menu Bar touch:  opens About Mac
    if (ty <= 18) {
        if (tx <= 24) openAboutMac();
        return;
    }

    // Close box handling for open windows
    if (activeApp != APP_DESKTOP) {
        // Close boxes are always top-left of their window
        int closeX = (activeApp == APP_MACPAINT) ? 13 : ((activeApp == APP_CALCULATOR) ? 65 : 29);
        int closeY = (activeApp == APP_MACPAINT) ? 28 : ((activeApp == APP_CALCULATOR) ? 32 : 38);
        if (tx >= closeX - 5 && tx <= closeX + 15 && ty >= closeY - 5 && ty <= closeY + 15) {
            activeApp = APP_DESKTOP;
            drawDesktop();
            return;
        }
    }

    // DESKTOP ICONS TAP
    if (activeApp == APP_DESKTOP) {
        // Mac HD (About Mac)
        if (tx >= 270 && ty >= 24 && ty <= 64) openAboutMac();
        // MacPaint
        else if (tx >= 270 && ty >= 70 && ty <= 112) openMacPaint();
        // Calculator
        else if (tx >= 270 && ty >= 116 && ty <= 158) openCalculator();
        // Trash
        else if (tx >= 270 && ty >= 170 && ty <= 220) {
            playMacChime(); // Trash plays funny retro tone
        }
        return;
    }

    // CALCULATOR KEYBOARD TAP
    if (activeApp == APP_CALCULATOR) {
        const char* keys[4][4] = {
            {"7", "8", "9", "/"},
            {"4", "5", "6", "*"},
            {"1", "2", "3", "-"},
            {"C", "0", "=", "+"}
        };
        for (int r = 0; r < 4; r++) {
            for (int c = 0; c < 4; c++) {
                int bx = 74 + (c * 44);
                int by = 84 + (r * 32);
                if (tx >= bx && tx <= bx + 38 && ty >= by && ty <= by + 26) {
                    char key = keys[r][c][0];
                    if (key >= '0' && key <= '9') {
                        if (calcDisplay == "0" || calcClearOnNext) {
                            calcDisplay = String(key);
                            calcClearOnNext = false;
                        } else if (calcDisplay.length() < 9) {
                            calcDisplay += key;
                        }
                    } else if (key == 'C') {
                        calcDisplay = "0";
                        calcAccumulator = 0;
                        calcPendingOp = '\0';
                    } else if (key == '=') {
                        float cur = calcDisplay.toFloat();
                        if (calcPendingOp == '+') calcAccumulator += cur;
                        else if (calcPendingOp == '-') calcAccumulator -= cur;
                        else if (calcPendingOp == '*') calcAccumulator *= cur;
                        else if (calcPendingOp == '/') calcAccumulator = (cur != 0) ? (calcAccumulator / cur) : 0;
                        calcDisplay = String(calcAccumulator, 1);
                        if (calcDisplay.endsWith(".0")) calcDisplay = calcDisplay.substring(0, calcDisplay.length() - 2);
                        calcPendingOp = '\0';
                        calcClearOnNext = true;
                    } else { // Operator (+, -, *, /)
                        calcAccumulator = calcDisplay.toFloat();
                        calcPendingOp = key;
                        calcClearOnNext = true;
                    }
                    openCalculator();
                    return;
                }
            }
        }
    }

    // MACPAINT TOOLS TAP
    if (activeApp == APP_MACPAINT) {
        for (int i = 0; i < 3; i++) {
            int ty_btn = 56 + (i * 36);
            if (tx >= 264 && tx <= 306 && ty >= ty_btn && ty <= ty_btn + 28) {
                currentTool = i;
                openMacPaint();
                return;
            }
        }
        // Clear canvas button
        if (tx >= 264 && tx <= 306 && ty >= 168 && ty <= 196) {
            tft.fillRect(CANVAS_X, CANVAS_Y, CANVAS_W, CANVAS_H, C_WHITE);
            return;
        }
    }
}

// =========================================================================
//  SETUP & MAIN LOOP
// =========================================================================
void setup() {
    Serial.begin(115200);

    // Warm beige power LED (Red + Green dim = amber/beige power glow)
    pinMode(LED_R, OUTPUT);
    pinMode(LED_G, OUTPUT);
    pinMode(LED_B, OUTPUT);
    analogWrite(LED_R, 255 - 180);
    analogWrite(LED_G, 255 - 120);
    analogWrite(LED_B, 255 - 0);

    pinMode(BUZZER_PIN, OUTPUT);
    digitalWrite(BUZZER_PIN, LOW);

    tft.init();
    tft.setRotation(1); // Landscape 320x240

    // 1984 Macintosh Boot Sequence
    tft.fillScreen(C_GRAY);
    drawHappyMac(148, 90);
    playMacChime();
    delay(1200);

    // Initialize Touch Controller
    touchSPI.begin(TOUCH_CLK, TOUCH_MISO, TOUCH_MOSI, TOUCH_CS);
    touch.begin(touchSPI);
    touch.setRotation(1);

    drawDesktop();
}

void loop() {
    handleTouch();
    delay(10);
}
