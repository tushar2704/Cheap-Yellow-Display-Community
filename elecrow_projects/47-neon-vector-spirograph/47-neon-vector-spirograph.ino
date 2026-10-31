/*
 * Project 47: Neon Vector Spirograph & Mandala Generator
 * Board: Elecrow ESP32-2432S028R (2.8" ILI9341 TFT + XPT2046 Resistive Touch)
 * Channel: @IamNotIntoTech (https://youtube.com/@IamNotIntoTech)
 * 
 * Hardware Pinouts:
 *  - ILI9341 TFT SPI: MOSI=13, MISO=12, SCK=14, CS=15, DC=2, BL=21
 *  - XPT2046 Touch VSPI: MOSI=32, MISO=39, CLK=25, CS=33
 *  - RGB LED: IO4 (Red), IO16 (Green), IO17 (Blue) - Active LOW
 *  - Piezo Speaker: IO26 (Buzzer tone)
 *  - LDR Light Sensor: IO34 (ADC1)
 */

#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include <math.h>

#define TFT_BL_PIN    21
#define TOUCH_MOSI    32
#define TOUCH_MISO    39
#define TOUCH_CLK     25
#define TOUCH_CS      33
#define TOUCH_IRQ     36

#define RGB_RED_PIN   4
#define RGB_GREEN_PIN 16
#define RGB_BLUE_PIN  17
#define SPEAKER_PIN   26
#define LDR_PIN       34

SPIClass touchSPI(VSPI);
XPT2046_Touchscreen ts(TOUCH_CS);
TFT_eSPI tft = TFT_eSPI();

enum EngineMode {
    MODE_AUTO_SPIRO = 0,
    MODE_TOUCH_MANDALA = 1
};
EngineMode currentMode = MODE_AUTO_SPIRO;

// Mathematical Spirograph Parameters (Hypotrochoid)
float paramR = 95.0f; // Outer Fixed Circle
float paramr = 57.0f; // Rolling Inner Circle
float paramd = 62.0f; // Pen Distance
float theta = 0.0f;
float thetaStep = 0.038f;
int prevPlotX = -1, prevPlotY = -1;
uint32_t stepCounter = 0;

// Mandala Touch Parameters
uint8_t symmetryFold = 8; // 4x, 6x, 8x, 12x
int prevTouchX = -1, prevTouchY = -1;
unsigned long lastTouchTime = 0;

// Color Themes
enum ColorPalette {
    PALETTE_NEON_RAINBOW = 0,
    PALETTE_CYBERPUNK = 1,
    PALETTE_FIRE = 2,
    PALETTE_EMERALD = 3
};
ColorPalette activePalette = PALETTE_NEON_RAINBOW;
float colorHue = 0.0f;

// Presets for Auto Spirograph
struct SpiroPreset {
    float R, r, d;
    const char* name;
};

const SpiroPreset presets[] = {
    { 96.0f, 60.0f, 54.0f, "Hypotrochoid 8P" },
    { 104.0f, 40.0f, 68.0f, "Complex Star 13" },
    { 90.0f, 54.0f, 42.0f, "Harmonic Rose" },
    { 110.0f, 70.0f, 85.0f, "Double Bloom" },
    { 84.0f, 28.0f, 45.0f, "Classic Trifoil" }
};
uint8_t currentPresetIdx = 0;
const uint8_t totalPresets = sizeof(presets) / sizeof(presets[0]);

void setRGB(bool r, bool g, bool b) {
    digitalWrite(RGB_RED_PIN, r ? LOW : HIGH);
    digitalWrite(RGB_GREEN_PIN, g ? LOW : HIGH);
    digitalWrite(RGB_BLUE_PIN, b ? LOW : HIGH);
}

void playToneBeep(uint16_t freq, uint16_t durationMs) {
    tone(SPEAKER_PIN, freq, durationMs);
    delay(durationMs);
    noTone(SPEAKER_PIN);
}

// Convert HSV (0..360, 0..1, 0..1) to RGB565
uint16_t hsvToRGB565(float h, float s, float v) {
    while (h >= 360.0f) h -= 360.0f;
    while (h < 0.0f) h += 360.0f;
    
    float c = v * s;
    float x = c * (1.0f - fabsf(fmodf(h / 60.0f, 2.0f) - 1.0f));
    float m = v - c;
    
    float r = 0, g = 0, b = 0;
    if (h < 60)       { r = c; g = x; b = 0; }
    else if (h < 120) { r = x; g = c; b = 0; }
    else if (h < 180) { r = 0; g = c; b = x; }
    else if (h < 240) { r = 0; g = x; b = c; }
    else if (h < 300) { r = x; g = 0; b = c; }
    else              { r = c; g = 0; b = x; }

    uint8_t red = (uint8_t)((r + m) * 31.0f);
    uint8_t grn = (uint8_t)((g + m) * 63.0f);
    uint8_t blu = (uint8_t)((b + m) * 31.0f);

    return (red << 11) | (grn << 5) | blu;
}

uint16_t getPaletteColor(float progress) {
    switch (activePalette) {
        case PALETTE_NEON_RAINBOW:
            return hsvToRGB565(fmodf(progress * 360.0f, 360.0f), 1.0f, 1.0f);
        case PALETTE_CYBERPUNK:
            return (fmodf(progress, 0.2f) < 0.1f) ? 0x07FF : 0xF81F;
        case PALETTE_FIRE:
            return hsvToRGB565(fmodf(progress * 60.0f, 60.0f), 1.0f, 1.0f);
        case PALETTE_EMERALD:
            return hsvToRGB565(85.0f + fmodf(progress * 70.0f, 70.0f), 1.0f, 1.0f);
        default:
            return TFT_WHITE;
    }
}

void drawUIStatic() {
    // Header
    tft.fillRect(0, 0, 320, 24, 0x18C3);
    tft.drawFastHLine(0, 24, 320, 0x4208);
    tft.setTextColor(TFT_WHITE, 0x18C3);
    tft.setTextSize(1);
    tft.drawString("@IamNotIntoTech", 8, 8, 1);

    // Mode Toggle (Top right)
    tft.fillRect(195, 2, 120, 20, 0x032E);
    tft.drawRect(195, 2, 120, 20, 0x05BF);
    tft.setTextColor(TFT_CYAN, 0x032E);
    tft.drawCentreString(currentMode == MODE_AUTO_SPIRO ? "MANDALA MODE" : "AUTO SPIRO", 255, 6, 1);

    // Bottom Navigation Bar
    tft.fillRect(0, 216, 320, 24, 0x18C3);
    tft.drawFastHLine(0, 215, 320, 0x4208);

    if (currentMode == MODE_AUTO_SPIRO) {
        // Next Preset button
        tft.fillRect(6, 218, 64, 20, 0x2124);
        tft.drawRect(6, 218, 64, 20, 0x52AA);
        tft.setTextColor(TFT_WHITE, 0x2124);
        tft.drawCentreString("PRESET", 38, 222, 1);

        // Palette cycle
        tft.fillRect(74, 218, 68, 20, 0x2124);
        tft.drawRect(74, 218, 68, 20, 0x52AA);
        tft.setTextColor(TFT_WHITE, 0x2124);
        tft.drawCentreString("PALETTE", 108, 222, 1);

        // Clear screen button
        tft.fillRect(146, 218, 54, 20, 0x8000);
        tft.drawRect(146, 218, 54, 20, TFT_RED);
        tft.setTextColor(TFT_WHITE, 0x8000);
        tft.drawCentreString("CLEAR", 173, 222, 1);

        // Current Preset Name
        tft.setTextColor(TFT_YELLOW, 0x18C3);
        tft.drawRightString(presets[currentPresetIdx].name, 314, 222, 1);
    } else {
        // Symmetry Fold button (4x, 6x, 8x, 12x)
        tft.fillRect(6, 218, 70, 20, 0x2124);
        tft.drawRect(6, 218, 70, 20, 0x52AA);
        tft.setTextColor(TFT_WHITE, 0x2124);
        char sBuf[16];
        snprintf(sBuf, sizeof(sBuf), "SYM: %dx", symmetryFold);
        tft.drawCentreString(sBuf, 41, 222, 1);

        // Palette cycle
        tft.fillRect(80, 218, 68, 20, 0x2124);
        tft.drawRect(80, 218, 68, 20, 0x52AA);
        tft.setTextColor(TFT_WHITE, 0x2124);
        tft.drawCentreString("PALETTE", 114, 222, 1);

        // Clear Screen
        tft.fillRect(152, 218, 54, 20, 0x8000);
        tft.drawRect(152, 218, 54, 20, TFT_RED);
        tft.setTextColor(TFT_WHITE, 0x8000);
        tft.drawCentreString("CLEAR", 179, 222, 1);

        tft.setTextColor(0x9E7F, 0x18C3);
        tft.drawRightString("Stylus Mandala", 314, 222, 1);
    }
}

void loadPreset(uint8_t idx) {
    currentPresetIdx = idx % totalPresets;
    paramR = presets[currentPresetIdx].R;
    paramr = presets[currentPresetIdx].r;
    paramd = presets[currentPresetIdx].d;
    theta = 0.0f;
    prevPlotX = -1;
    prevPlotY = -1;
    stepCounter = 0;
    
    tft.fillRect(0, 25, 320, 190, TFT_BLACK);
    drawUIStatic();
}

void drawMandalaStroke(int x, int y, int px, int py, uint16_t color) {
    const float cx = 160.0f;
    const float cy = 120.0f;
    float angleStep = 6.2831853f / symmetryFold;

    float dx0 = (float)(px - cx);
    float dy0 = (float)(py - cy);
    float dx1 = (float)(x - cx);
    float dy1 = (float)(y - cy);

    for (int i = 0; i < symmetryFold; i++) {
        float rot = i * angleStep;
        float cosR = cosf(rot);
        float sinR = sinf(rot);

        int rx0 = (int)(cx + dx0 * cosR - dy0 * sinR);
        int ry0 = (int)(cy + dx0 * sinR + dy0 * cosR);
        int rx1 = (int)(cx + dx1 * cosR - dy1 * sinR);
        int ry1 = (int)(cy + dx1 * sinR + dy1 * cosR);

        if (ry0 >= 25 && ry0 <= 214 && ry1 >= 25 && ry1 <= 214) {
            tft.drawLine(rx0, ry0, rx1, ry1, color);
            tft.drawLine(rx0 + 1, ry0, rx1 + 1, ry1, color);
        }
    }
}

void stepSpirograph() {
    const float cx = 160.0f;
    const float cy = 120.0f;
    
    for (int k = 0; k < 4; k++) {
        float diff = paramR - paramr;
        float ratio = diff / paramr;
        float x = diff * cosf(theta) + paramd * cosf(ratio * theta);
        float y = diff * sinf(theta) - paramd * sinf(ratio * theta);

        int plotX = (int)(cx + x);
        int plotY = (int)(cy + y);

        if (plotY >= 25 && plotY <= 214 && plotX >= 0 && plotX < 320) {
            if (prevPlotX >= 0) {
                uint16_t color = getPaletteColor(stepCounter * 0.0018f);
                tft.drawLine(prevPlotX, prevPlotY, plotX, plotY, color);
            }
            prevPlotX = plotX;
            prevPlotY = plotY;
        }

        theta += thetaStep;
        stepCounter++;
    }

    // LED RGB cycle
    colorHue += 1.5f;
    if (colorHue >= 360.0f) colorHue = 0.0f;
    bool r = (colorHue < 120.0f);
    bool g = (colorHue >= 60.0f && colorHue < 240.0f);
    bool b = (colorHue >= 180.0f);
    setRGB(r, g, b);
}

void handleTouch() {
    if (!ts.touched()) {
        prevTouchX = -1;
        prevTouchY = -1;
        return;
    }

    TS_Point p = ts.getPoint();
    if (p.z > 50 || ts.touched()) {
        int touchX = map(p.x, 240, 3800, 0, 320);
        int touchY = map(p.y, 240, 3800, 0, 240);
        touchX = constrain(touchX, 0, 319);
        touchY = constrain(touchY, 0, 239);

        // Top Bar Mode Switch (x >= 195, y <= 24)
        if (touchY <= 24 && touchX >= 195) {
            if (millis() - lastTouchTime > 250) {
                lastTouchTime = millis();
                currentMode = (currentMode == MODE_AUTO_SPIRO) ? MODE_TOUCH_MANDALA : MODE_AUTO_SPIRO;
                playToneBeep(2200, 40);
                tft.fillRect(0, 25, 320, 190, TFT_BLACK);
                drawUIStatic();
                prevTouchX = -1;
                prevTouchY = -1;
            }
            return;
        }

        // Bottom Navigation Bar (y >= 215)
        if (touchY >= 215) {
            if (millis() - lastTouchTime > 200) {
                lastTouchTime = millis();
                if (currentMode == MODE_AUTO_SPIRO) {
                    // Preset Button (6..70)
                    if (touchX >= 6 && touchX <= 70) {
                        currentPresetIdx = (currentPresetIdx + 1) % totalPresets;
                        loadPreset(currentPresetIdx);
                        playToneBeep(1400, 30);
                        return;
                    }
                    // Palette Button (74..142)
                    if (touchX >= 74 && touchX <= 142) {
                        activePalette = (ColorPalette)((activePalette + 1) % 4);
                        playToneBeep(1800, 30);
                        drawUIStatic();
                        return;
                    }
                    // Clear Button (146..206)
                    if (touchX >= 146 && touchX <= 206) {
                        tft.fillRect(0, 25, 320, 190, TFT_BLACK);
                        prevPlotX = -1;
                        prevPlotY = -1;
                        theta = 0.0f;
                        playToneBeep(800, 40);
                        return;
                    }
                } else {
                    // Symmetry Fold Cycle (6..76)
                    if (touchX >= 6 && touchX <= 76) {
                        if (symmetryFold == 4) symmetryFold = 6;
                        else if (symmetryFold == 6) symmetryFold = 8;
                        else if (symmetryFold == 8) symmetryFold = 12;
                        else symmetryFold = 4;
                        playToneBeep(1600, 30);
                        drawUIStatic();
                        return;
                    }
                    // Palette Cycle (80..148)
                    if (touchX >= 80 && touchX <= 148) {
                        activePalette = (ColorPalette)((activePalette + 1) % 4);
                        playToneBeep(1800, 30);
                        drawUIStatic();
                        return;
                    }
                    // Clear Canvas (152..212)
                    if (touchX >= 152 && touchX <= 212) {
                        tft.fillRect(0, 25, 320, 190, TFT_BLACK);
                        playToneBeep(800, 40);
                        return;
                    }
                }
            }
            return;
        }

        // Canvas drawing interaction (Mandala Mode)
        if (currentMode == MODE_TOUCH_MANDALA && touchY >= 25 && touchY <= 214) {
            if (prevTouchX >= 0 && prevTouchY >= 25 && prevTouchY <= 214) {
                uint16_t strokeColor = getPaletteColor((float)millis() * 0.001f);
                drawMandalaStroke(touchX, touchY, prevTouchX, prevTouchY, strokeColor);
            }
            prevTouchX = touchX;
            prevTouchY = touchY;
        }
    }
}

void setup() {
    Serial.begin(115200);

    pinMode(RGB_RED_PIN, OUTPUT);
    pinMode(RGB_GREEN_PIN, OUTPUT);
    pinMode(RGB_BLUE_PIN, OUTPUT);
    pinMode(LDR_PIN, INPUT);
    setRGB(false, false, false);

    pinMode(SPEAKER_PIN, OUTPUT);
    digitalWrite(SPEAKER_PIN, LOW);

    pinMode(TFT_BL_PIN, OUTPUT);
    digitalWrite(TFT_BL_PIN, HIGH);

    tft.init();
    tft.setRotation(1);
    tft.fillScreen(TFT_BLACK);

    // Dedicated VSPI for Touch Controller (Pins 25, 39, 32, 33)
    touchSPI.begin(TOUCH_CLK, TOUCH_MISO, TOUCH_MOSI, TOUCH_CS);
    ts.begin(touchSPI);
    ts.setRotation(1);

    drawUIStatic();
    playToneBeep(1750, 70);
    Serial.println(F("[SYSTEM] Neon Vector Spirograph & Mandala Generator Initialized."));
}

void loop() {
    handleTouch();

    if (currentMode == MODE_AUTO_SPIRO) {
        stepSpirograph();
    }

    delay(8);
}

