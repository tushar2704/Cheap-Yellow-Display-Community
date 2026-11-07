// =========================================================================
//  Project 57 — CYD WLED Matrix & Lightstrip Network Controller
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
#include <ArduinoJson.h>

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

// 16-bit 565 Colors
#define C_BLACK       0x0000
#define C_WHITE       0xFFFF
#define C_BG          0x0821  // Deep charcoal/slate
#define C_CARD        0x10A2  // Slightly elevated card
#define C_CARD_BORDER 0x2945
#define C_GRAY        0x73AE
#define C_DARKGRAY    0x3186
#define C_GREEN       0x07E0
#define C_CYAN        0x07FF
#define C_YELLOW      0xFFE0
#define C_RED         0xF800
#define C_MAGENTA     0xF81F
#define C_ORANGE      0xFD20
#define C_BLUE        0x001F

TFT_eSPI tft = TFT_eSPI();
SPIClass touchSPI(HSPI);
XPT2046_Touchscreen touch(TOUCH_CS, TOUCH_IRQ);

// =========================================================================
//  WLED STATE & EFFECTS
// =========================================================================
bool powerState = true;
uint8_t brightness = 192; // 0..255
uint8_t effectSpeed = 128; // 0..255
uint8_t effectIntensity = 180; // 0..255

// Primary Color
uint8_t curR = 0;
uint8_t curG = 255;
uint8_t curB = 200;

enum WledEffect {
    FX_SOLID,
    FX_RAINBOW,
    FX_FIRE,
    FX_CYBERPUNK,
    FX_AURORA,
    FX_SCANNER
};

const char* FX_NAMES[6] = { "Solid", "Rainbow", "Fire", "Cyber", "Aurora", "Scanner" };
int currentFx = FX_RAINBOW;

// Virtual 16x5 LED Matrix Preview Canvas
const int MATRIX_W = 16;
const int MATRIX_H = 5;
uint16_t matrixPixels[MATRIX_H][MATRIX_W];
float fxPhase = 0;

// Color Swatches
const uint16_t SWATCHES[8] = {
    0xF800, // Red
    0xFD20, // Orange
    0xFFE0, // Yellow
    0x07E0, // Green
    0x07FF, // Cyan
    0x001F, // Blue
    0xF81F, // Pink/Magenta
    0xFFFF  // Warm White
};

// =========================================================================
//  SOUND & LED OUTPUT
// =========================================================================
void playBeep(int freq = 1600, int ms = 15) {
    tone(BUZZER_PIN, freq, ms);
}

void setRGB(uint8_t r, uint8_t g, uint8_t b) {
    if (!powerState) {
        analogWrite(LED_R, 255);
        analogWrite(LED_G, 255);
        analogWrite(LED_B, 255);
        return;
    }
    // Scale by brightness
    uint8_t sr = (r * brightness) / 255;
    uint8_t sg = (g * brightness) / 255;
    uint8_t sb = (b * brightness) / 255;
    analogWrite(LED_R, 255 - sr);
    analogWrite(LED_G, 255 - sg);
    analogWrite(LED_B, 255 - sb);
}

uint16_t rgb565(uint8_t r, uint8_t g, uint8_t b) {
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

// =========================================================================
//  UI DRAWING ROUTINES
// =========================================================================
void drawHeader() {
    tft.fillRect(0, 0, 320, 26, C_BG);

    // Title / Brand
    tft.setTextColor(C_CYAN, C_BG);
    tft.setTextDatum(TL_DATUM);
    tft.drawString("IamNotIntoTech", 6, 6, 2);
    tft.setTextColor(C_WHITE, C_BG);
    tft.drawString("WLED", 126, 6, 2);

    // Strip Profile Chip
    tft.fillRoundRect(180, 4, 80, 18, 9, C_CARD);
    tft.drawRoundRect(180, 4, 80, 18, 9, C_CARD_BORDER);
    tft.setTextColor(C_GREEN, C_CARD);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("DESK 144", 220, 13, 1);

    // Power Button (X: 268, Y: 3, W: 46, H: 20)
    uint16_t pBg = powerState ? C_GREEN : C_DARKGRAY;
    tft.fillRoundRect(268, 3, 46, 20, 4, pBg);
    tft.setTextColor(powerState ? C_BLACK : C_WHITE, pBg);
    tft.drawString(powerState ? "ON" : "OFF", 291, 13, 2);

    tft.drawFastHLine(0, 26, 320, C_CARD_BORDER);
}

void drawMatrixPreview() {
    int px = 8;
    int py = 32;
    int pw = 168;
    int ph = 60;

    // Outer Preview Bezel
    tft.fillRoundRect(px, py, pw, ph, 6, C_CARD);
    tft.drawRoundRect(px, py, pw, ph, 6, C_CARD_BORDER);

    // Draw 16x5 simulated LED dots
    int dotR = 3;
    int stepX = (pw - 12) / MATRIX_W;
    int stepY = (ph - 12) / MATRIX_H;

    for (int y = 0; y < MATRIX_H; y++) {
        for (int x = 0; x < MATRIX_W; x++) {
            int cx = px + 8 + (x * stepX);
            int cy = py + 8 + (y * stepY);
            uint16_t col = powerState ? matrixPixels[y][x] : C_BLACK;
            tft.fillCircle(cx, cy, dotR, col);
        }
    }
}

void drawSliders() {
    int sx = 186;
    int sw = 126;

    // 1. Brightness Slider (Y: 32..54)
    tft.setTextColor(C_GRAY, C_BG);
    tft.setTextDatum(TL_DATUM);
    char bBuf[24];
    snprintf(bBuf, sizeof(bBuf), "BRI: %d%%", (brightness * 100) / 255);
    tft.drawString(bBuf, sx, 32, 1);

    tft.fillRoundRect(sx, 44, sw, 8, 4, C_CARD);
    int bFill = map(brightness, 0, 255, 0, sw);
    tft.fillRoundRect(sx, 44, bFill, 8, 4, C_YELLOW);
    tft.fillCircle(sx + bFill, 48, 5, C_WHITE);

    // 2. Speed Slider (Y: 58..80)
    char sBuf[24];
    snprintf(sBuf, sizeof(sBuf), "SPEED: %d%%", (effectSpeed * 100) / 255);
    tft.drawString(sBuf, sx, 58, 1);

    tft.fillRoundRect(sx, 70, sw, 8, 4, C_CARD);
    int sFill = map(effectSpeed, 0, 255, 0, sw);
    tft.fillRoundRect(sx, 70, sFill, 8, 4, C_CYAN);
    tft.fillCircle(sx + sFill, 74, 5, C_WHITE);

    // 3. Intensity Slider (Y: 84..106)
    char iBuf[24];
    snprintf(iBuf, sizeof(iBuf), "INTENSITY: %d%%", (effectIntensity * 100) / 255);
    tft.drawString(iBuf, sx, 84, 1);

    tft.fillRoundRect(sx, 96, sw, 8, 4, C_CARD);
    int iFill = map(effectIntensity, 0, 255, 0, sw);
    tft.fillRoundRect(sx, 96, iFill, 8, 4, C_MAGENTA);
    tft.fillCircle(sx + iFill, 100, 5, C_WHITE);
}

void drawColorPalette() {
    int cx = 8;
    int cy = 98;
    int cw = 168;
    int ch = 136;

    tft.fillRoundRect(cx, cy, cw, ch, 6, C_CARD);
    tft.drawRoundRect(cx, cy, cw, ch, 6, C_CARD_BORDER);

    tft.setTextColor(C_GRAY, C_CARD);
    tft.setTextDatum(TL_DATUM);
    tft.drawString("COLOR PALETTE", cx + 8, cy + 6, 1);

    // 8 Swatch Buttons (2 rows of 4)
    int swW = 32;
    int swH = 22;
    for (int i = 0; i < 8; i++) {
        int r = i / 4;
        int c = i % 4;
        int bx = cx + 8 + (c * (swW + 7));
        int by = cy + 20 + (r * (swH + 6));
        tft.fillRoundRect(bx, by, swW, swH, 4, SWATCHES[i]);
        tft.drawRoundRect(bx, by, swW, swH, 4, C_WHITE);
    }

    // Continuous Spectrum Gradient Bar (Y: cy + 74, H: 20)
    int specY = cy + 74;
    int specW = cw - 16;
    for (int x = 0; x < specW; x++) {
        float hue = (float)x / specW;
        // Fast HSV to 565 rainbow
        int h6 = (int)(hue * 6.0);
        float f = (hue * 6.0) - h6;
        uint8_t q = (1.0 - f) * 255;
        uint8_t t = f * 255;
        uint16_t col;
        switch (h6 % 6) {
            case 0: col = rgb565(255, t, 0); break;
            case 1: col = rgb565(q, 255, 0); break;
            case 2: col = rgb565(0, 255, t); break;
            case 3: col = rgb565(0, q, 255); break;
            case 4: col = rgb565(t, 0, 255); break;
            default: col = rgb565(255, 0, q); break;
        }
        tft.drawFastVLine(cx + 8 + x, specY, 16, col);
    }
    tft.drawRect(cx + 8, specY, specW, 16, C_WHITE);

    // Active Color Pill & RGB Hex (Y: cy + 98)
    uint16_t activeCol = rgb565(curR, curG, curB);
    tft.fillRoundRect(cx + 8, cy + 98, 40, 28, 4, activeCol);
    tft.drawRoundRect(cx + 8, cy + 98, 40, 28, 4, C_WHITE);

    tft.setTextColor(C_WHITE, C_CARD);
    tft.setTextDatum(TL_DATUM);
    char hexBuf[24];
    snprintf(hexBuf, sizeof(hexBuf), "R:%d G:%d B:%d", curR, curG, curB);
    tft.drawString(hexBuf, cx + 54, cy + 102, 1);
    char hexVal[16];
    snprintf(hexVal, sizeof(hexVal), "#%02X%02X%02X", curR, curG, curB);
    tft.setTextColor(C_CYAN, C_CARD);
    tft.drawString(hexVal, cx + 54, cy + 114, 2);
}

void drawEffectButtons() {
    int fxX = 186;
    int fxY = 114;
    int fxW = 126;
    int btnH = 18;

    tft.setTextColor(C_GRAY, C_BG);
    tft.setTextDatum(TL_DATUM);
    tft.drawString("EFFECT PRESETS", fxX, fxY, 1);

    for (int i = 0; i < 6; i++) {
        int by = fxY + 12 + (i * (btnH + 3));
        bool isAct = (currentFx == i);
        uint16_t bBg = isAct ? C_CYAN : C_CARD;
        uint16_t bFg = isAct ? C_BLACK : C_WHITE;

        tft.fillRoundRect(fxX, by, fxW, btnH, 4, bBg);
        tft.drawRoundRect(fxX, by, fxW, btnH, 4, isAct ? C_WHITE : C_CARD_BORDER);
        tft.setTextColor(bFg, bBg);
        tft.setTextDatum(MC_DATUM);
        tft.drawString(FX_NAMES[i], fxX + (fxW / 2), by + (btnH / 2), 1);
    }
}

// =========================================================================
//  EFFECT ENGINE (PROCEDURAL SIMULATION)
// =========================================================================
void updateEffects() {
    float spd = (effectSpeed / 255.0) * 0.2 + 0.02;
    fxPhase += spd;

    for (int y = 0; y < MATRIX_H; y++) {
        for (int x = 0; x < MATRIX_W; x++) {
            uint16_t pixCol = C_BLACK;

            switch (currentFx) {
                case FX_SOLID:
                    pixCol = rgb565(curR, curG, curB);
                    break;

                case FX_RAINBOW: {
                    float h = fmod(fxPhase + ((float)x / MATRIX_W) + ((float)y / MATRIX_H * 0.2), 1.0);
                    int h6 = (int)(h * 6.0);
                    float f = (h * 6.0) - h6;
                    uint8_t q = (1.0 - f) * 255;
                    uint8_t t = f * 255;
                    switch (h6 % 6) {
                        case 0: pixCol = rgb565(255, t, 0); break;
                        case 1: pixCol = rgb565(q, 255, 0); break;
                        case 2: pixCol = rgb565(0, 255, t); break;
                        case 3: pixCol = rgb565(0, q, 255); break;
                        case 4: pixCol = rgb565(t, 0, 255); break;
                        default: pixCol = rgb565(255, 0, q); break;
                    }
                    break;
                }

                case FX_FIRE: {
                    int heat = random(100, 255) - (y * 35);
                    heat = constrain(heat, 0, 255);
                    pixCol = rgb565(heat, heat / 3, 0);
                    break;
                }

                case FX_CYBERPUNK: {
                    int step = (int)(fxPhase * 8.0) % MATRIX_W;
                    if (x == step || x == (step + 1) % MATRIX_W) {
                        pixCol = C_CYAN;
                    } else if (x == (step + 8) % MATRIX_W) {
                        pixCol = C_MAGENTA;
                    } else {
                        pixCol = 0x0810;
                    }
                    break;
                }

                case FX_AURORA: {
                    float wave = sin(fxPhase + (x * 0.3) + (y * 0.5));
                    uint8_t gVal = (uint8_t)((wave + 1.0) * 127);
                    pixCol = rgb565(0, gVal, 255 - gVal);
                    break;
                }

                case FX_SCANNER: {
                    int scanPos = (int)(sin(fxPhase * 2.0) * (MATRIX_W / 2) + (MATRIX_W / 2));
                    int dist = abs(x - scanPos);
                    if (dist == 0) pixCol = rgb565(curR, curG, curB);
                    else if (dist == 1) pixCol = rgb565(curR / 2, curG / 2, curB / 2);
                    else pixCol = 0x1082;
                    break;
                }
            }
            matrixPixels[y][x] = pixCol;
        }
    }

    // Mirror center LED onto hardware RGB LED
    if (powerState) {
        uint16_t centerPix = matrixPixels[2][8];
        uint8_t r = (centerPix >> 11) << 3;
        uint8_t g = ((centerPix >> 5) & 0x3F) << 2;
        uint8_t b = (centerPix & 0x1F) << 3;
        setRGB(r, g, b);
    }
}

// =========================================================================
//  TOUCH INPUT PROCESSING
// =========================================================================
void handleTouch() {
    if (!touch.touched()) return;

    TS_Point p = touch.getPoint();
    int tx = map(p.x, TS_MIN_X, TS_MAX_X, 0, 320);
    int ty = map(p.y, TS_MIN_Y, TS_MAX_Y, 0, 240);
    tx = constrain(tx, 0, 319);
    ty = constrain(ty, 0, 239);

    // 1. Power Button (X: 268..314, Y: 0..26)
    if (tx >= 268 && tx <= 314 && ty <= 26) {
        powerState = !powerState;
        playBeep(powerState ? 2000 : 800, 25);
        drawHeader();
        setRGB(curR, curG, curB);
        delay(250);
        return;
    }

    // 2. Sliders (X: 186..312)
    if (tx >= 186 && tx <= 312) {
        int sw = 126;
        int tapOffset = tx - 186;
        tapOffset = constrain(tapOffset, 0, sw);

        // Brightness (Y: 32..54)
        if (ty >= 32 && ty <= 54) {
            brightness = map(tapOffset, 0, sw, 0, 255);
            playBeep(1400, 15);
            drawSliders();
            delay(100);
            return;
        }

        // Speed (Y: 58..80)
        if (ty >= 58 && ty <= 80) {
            effectSpeed = map(tapOffset, 0, sw, 0, 255);
            playBeep(1600, 15);
            drawSliders();
            delay(100);
            return;
        }

        // Intensity (Y: 84..106)
        if (ty >= 84 && ty <= 106) {
            effectIntensity = map(tapOffset, 0, sw, 0, 255);
            playBeep(1800, 15);
            drawSliders();
            delay(100);
            return;
        }
    }

    // 3. Effect Presets (X: 186..312, Y: 124..234)
    if (tx >= 186 && tx <= 312 && ty >= 124 && ty <= 234) {
        int btnH = 18;
        int idx = (ty - 126) / (btnH + 3);
        if (idx >= 0 && idx < 6) {
            currentFx = idx;
            playBeep(1600 + (idx * 100), 20);
            drawEffectButtons();
            delay(200);
            return;
        }
    }

    // 4. Color Palette Interactions (X: 8..176, Y: 98..234)
    if (tx >= 8 && tx <= 176 && ty >= 98 && ty <= 234) {
        // Swatches (Y: 118..166)
        if (ty >= 118 && ty <= 166) {
            int swW = 39;
            int swH = 28;
            int c = (tx - 16) / swW;
            int r = (ty - 118) / swH;
            int sIdx = r * 4 + c;
            if (sIdx >= 0 && sIdx < 8) {
                uint16_t sc = SWATCHES[sIdx];
                curR = (sc >> 11) << 3;
                curG = ((sc >> 5) & 0x3F) << 2;
                curB = (sc & 0x1F) << 3;
                playBeep(1800, 20);
                drawColorPalette();
                delay(200);
                return;
            }
        }

        // Spectrum bar (Y: 172..192)
        if (ty >= 172 && ty <= 192) {
            int specW = 152;
            int ox = tx - 16;
            ox = constrain(ox, 0, specW);
            float hue = (float)ox / specW;
            int h6 = (int)(hue * 6.0);
            float f = (hue * 6.0) - h6;
            uint8_t q = (1.0 - f) * 255;
            uint8_t t = f * 255;
            switch (h6 % 6) {
                case 0: curR = 255; curG = t; curB = 0; break;
                case 1: curR = q; curG = 255; curB = 0; break;
                case 2: curR = 0; curG = 255; curB = t; break;
                case 3: curR = 0; curG = q; curB = 255; break;
                case 4: curR = t; curG = 0; curB = 255; break;
                default: curR = 255; curG = 0; curB = q; break;
            }
            playBeep(1500, 15);
            drawColorPalette();
            delay(150);
            return;
        }
    }
}

// =========================================================================
//  SETUP & MAIN LOOP
// =========================================================================
void setup() {
    Serial.begin(115200);
    delay(200);

    pinMode(LED_R, OUTPUT);
    pinMode(LED_G, OUTPUT);
    pinMode(LED_B, OUTPUT);
    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(LDR_PIN, INPUT);

    setRGB(0, 0, 0);

    // Init Display & Touch
    tft.init();
    tft.setRotation(1); // Landscape 320x240
    tft.fillScreen(C_BG);

    touchSPI.begin(TOUCH_CLK, TOUCH_MISO, TOUCH_MOSI, TOUCH_CS);
    touch.begin(touchSPI);
    touch.setRotation(1);

    // Boot Screen
    tft.setTextColor(C_CYAN, C_BG);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("IamNotIntoTech", 160, 90, 4);
    tft.setTextColor(C_WHITE, C_BG);
    tft.drawString("WLED MATRIX & LIGHTSTRIP REMOTE", 160, 130, 2);
    tft.setTextColor(C_GREEN, C_BG);
    tft.drawString("Discovering WLED UDP Nodes...", 160, 160, 1);

    playBeep(1200, 60);
    delay(80);
    playBeep(1800, 80);
    delay(700);

    tft.fillScreen(C_BG);

    drawHeader();
    drawSliders();
    drawColorPalette();
    drawEffectButtons();

    setRGB(curR, curG, curB);
}

void loop() {
    handleTouch();

    // Refresh LED matrix simulation at ~30 FPS
    static unsigned long lastFxUpdate = 0;
    if (millis() - lastFxUpdate >= 33) {
        lastFxUpdate = millis();
        updateEffects();
        drawMatrixPreview();
    }

    delay(15);
}

