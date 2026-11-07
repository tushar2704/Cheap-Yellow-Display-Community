// =========================================================================
//  Project 58 — CYD Home Assistant Quick-Tiles Glass Panel
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

// 16-bit 565 Colors (Modern Dark Glassmorphic Palette)
#define C_BLACK       0x0000
#define C_WHITE       0xFFFF
#define C_BG          0x0842  // Deep dark obsidian slate
#define C_GLASS       0x18C4  // Translucent dark card surface
#define C_GLASS_HIGH  0x2966  // Card highlight / active state
#define C_BORDER      0x31C8  // Subtle frosted border
#define C_GRAY        0x8410
#define C_DARKGRAY    0x3186
#define C_CYAN        0x07FF
#define C_GREEN       0x07E0
#define C_YELLOW      0xFFE0
#define C_ORANGE      0xFD20
#define C_RED         0xF800
#define C_BLUE        0x001F
#define C_PURPLE      0x915C

TFT_eSPI tft = TFT_eSPI();
SPIClass touchSPI(HSPI);
XPT2046_Touchscreen touch(TOUCH_CS, TOUCH_IRQ);

// =========================================================================
//  SMART HOME STATE
// =========================================================================
enum PanelTab {
    TAB_HOME,
    TAB_LIGHTS,
    TAB_CLIMATE,
    TAB_ENERGY
};

int currentTab = TAB_HOME;

// Clock & Environment
int clockHours = 14;
int clockMins = 28;
int clockSecs = 45;
unsigned long lastSecTick = 0;
float currentTemp = 21.8;
int currentHumidity = 48;

// Climate State
float targetTemp = 22.5;
enum HvacMode { HVAC_HEAT, HVAC_COOL, HVAC_AUTO, HVAC_ECO, HVAC_OFF };
int currentHvac = HVAC_HEAT;

// Security State
enum AlarmState { ALARM_DISARMED, ALARM_ARMED_HOME, ALARM_ARMED_AWAY };
int currentAlarm = ALARM_ARMED_HOME;

// Lights State (4 Tiles)
struct LightTile {
    const char* name;
    bool state;
    uint8_t brightness; // 0..100
};

LightTile lights[4] = {
    { "Living Room", true, 80 },
    { "Desk Strip",  true, 100 },
    { "Kitchen Pod", false, 50 },
    { "Porch Light", true, 60 }
};

// Energy State
float solarKw = 3.42;
float homeKw = 1.85;
int batterySoc = 84;

// =========================================================================
//  SOUND & LED
// =========================================================================
void playClick(int freq = 1600, int ms = 15) {
    tone(BUZZER_PIN, freq, ms);
}

void setRGB(uint8_t r, uint8_t g, uint8_t b) {
    analogWrite(LED_R, 255 - r);
    analogWrite(LED_G, 255 - g);
    analogWrite(LED_B, 255 - b);
}

void updateStatusLed() {
    if (currentAlarm == ALARM_ARMED_AWAY) {
        setRGB(200, 0, 0); // Red alert
    } else if (currentHvac == HVAC_HEAT) {
        setRGB(200, 80, 0); // Warm Amber
    } else if (currentHvac == HVAC_COOL) {
        setRGB(0, 100, 255); // Cool Blue
    } else {
        setRGB(0, 180, 60); // Eco Green
    }
}

// =========================================================================
//  UI CHROME: HEADER & TAB BAR
// =========================================================================
void drawHeader() {
    tft.fillRect(0, 0, 320, 24, C_BG);

    // Brand Title
    tft.setTextColor(C_CYAN, C_BG);
    tft.setTextDatum(TL_DATUM);
    tft.drawString("IamNotIntoTech", 6, 5, 2);
    tft.setTextColor(C_WHITE, C_BG);
    tft.drawString("HOME", 126, 5, 2);

    // Live Digital Clock
    char clkBuf[16];
    snprintf(clkBuf, sizeof(clkBuf), "%02d:%02d:%02d", clockHours, clockMins, clockSecs);
    tft.setTextColor(C_YELLOW, C_BG);
    tft.setTextDatum(MC_DATUM);
    tft.drawString(clkBuf, 196, 12, 2);

    // Online HA Chip
    tft.fillRoundRect(248, 3, 68, 18, 9, C_GLASS);
    tft.drawRoundRect(248, 3, 68, 18, 9, C_BORDER);
    tft.setTextColor(C_GREEN, C_GLASS);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("ONLINE", 282, 12, 1);

    tft.drawFastHLine(0, 24, 320, C_BORDER);
}

void drawTabBar() {
    int ty = 212;
    int th = 28;
    int tw = 320 / 4;

    tft.fillRect(0, ty, 320, th, C_BG);
    tft.drawFastHLine(0, ty, 320, C_BORDER);

    const char* tabNames[4] = { "HOME", "LIGHTS", "CLIMATE", "ENERGY" };

    for (int i = 0; i < 4; i++) {
        int tx = i * tw;
        bool isAct = (currentTab == i);
        uint16_t tBg = isAct ? C_GLASS_HIGH : C_BG;
        uint16_t tFg = isAct ? C_CYAN : C_GRAY;

        if (isAct) {
            tft.fillRoundRect(tx + 2, ty + 2, tw - 4, th - 4, 4, tBg);
            tft.drawFastHLine(tx + 8, ty + th - 2, tw - 16, C_CYAN);
        }
        tft.setTextColor(tFg, tBg);
        tft.setTextDatum(MC_DATUM);
        tft.drawString(tabNames[i], tx + (tw / 2), ty + (th / 2), 2);
    }
}

// =========================================================================
//  TAB 0: HOME OVERVIEW
// =========================================================================
void drawTabHome() {
    tft.fillRect(0, 25, 320, 186, C_BG);

    // Left Card: Climate Quick Dial (X: 8, Y: 28, W: 148, H: 178)
    tft.fillRoundRect(8, 28, 148, 178, 8, C_GLASS);
    tft.drawRoundRect(8, 28, 148, 178, 8, C_BORDER);

    tft.setTextColor(C_GRAY, C_GLASS);
    tft.setTextDatum(TL_DATUM);
    tft.drawString("THERMOSTAT", 16, 36, 1);

    // Current Temp Big Dial
    int cx = 82;
    int cy = 94;
    tft.drawCircle(cx, cy, 38, C_DARKGRAY);
    tft.drawCircle(cx, cy, 37, C_ORANGE);

    tft.setTextColor(C_WHITE, C_GLASS);
    tft.setTextDatum(MC_DATUM);
    char curTempBuf[16];
    snprintf(curTempBuf, sizeof(curTempBuf), "%.1f C", targetTemp);
    tft.drawString(curTempBuf, cx, cy - 2, 4);

    tft.setTextColor(C_GRAY, C_GLASS);
    char actTempBuf[24];
    snprintf(actTempBuf, sizeof(actTempBuf), "Actual: %.1f C", currentTemp);
    tft.drawString(actTempBuf, cx, cy + 18, 1);

    // Target +/- Buttons (X: 16..148, Y: 146..174)
    tft.fillRoundRect(18, 144, 56, 26, 4, C_GLASS_HIGH);
    tft.drawRoundRect(18, 144, 56, 26, 4, C_CYAN);
    tft.setTextColor(C_CYAN, C_GLASS_HIGH);
    tft.drawString("- 0.5", 46, 157, 2);

    tft.fillRoundRect(90, 144, 56, 26, 4, C_GLASS_HIGH);
    tft.drawRoundRect(90, 144, 56, 26, 4, C_ORANGE);
    tft.setTextColor(C_ORANGE, C_GLASS_HIGH);
    tft.drawString("+ 0.5", 118, 157, 2);

    tft.setTextColor(C_ORANGE, C_GLASS);
    tft.drawString("HVAC: HEATING", 82, 188, 1);

    // Right Top Card: Security Alarm (X: 164, Y: 28, W: 148, H: 84)
    tft.fillRoundRect(164, 28, 148, 84, 8, C_GLASS);
    tft.drawRoundRect(164, 28, 148, 84, 8, C_BORDER);

    tft.setTextColor(C_GRAY, C_GLASS);
    tft.setTextDatum(TL_DATUM);
    tft.drawString("SECURITY SYSTEM", 172, 36, 1);

    uint16_t almCol = (currentAlarm == ALARM_DISARMED) ? C_GRAY : (currentAlarm == ALARM_ARMED_HOME ? C_GREEN : C_RED);
    const char* almTxt = (currentAlarm == ALARM_DISARMED) ? "DISARMED" : (currentAlarm == ALARM_ARMED_HOME ? "ARMED HOME" : "ARMED AWAY");
    tft.fillRoundRect(172, 50, 132, 28, 6, almCol);
    tft.setTextColor(C_BLACK, almCol);
    tft.setTextDatum(MC_DATUM);
    tft.drawString(almTxt, 238, 64, 2);

    tft.setTextColor(C_GRAY, C_GLASS);
    tft.drawString("Tap to toggle mode", 238, 94, 1);

    // Right Bottom Card: Quick Scene Buttons (X: 164, Y: 120, W: 148, H: 86)
    tft.fillRoundRect(164, 120, 148, 86, 8, C_GLASS);
    tft.drawRoundRect(164, 120, 148, 86, 8, C_BORDER);

    tft.setTextColor(C_GRAY, C_GLASS);
    tft.setTextDatum(TL_DATUM);
    tft.drawString("QUICK SCENES", 172, 128, 1);

    // 4 scene buttons (Relax, Movie, Bright, Off)
    const char* scNames[4] = { "Relax", "Movie", "Bright", "Off" };
    uint16_t scCols[4] = { C_ORANGE, C_PURPLE, C_YELLOW, C_GRAY };
    for (int i = 0; i < 4; i++) {
        int r = i / 2;
        int c = i % 2;
        int bx = 172 + (c * 68);
        int by = 142 + (r * 28);
        tft.fillRoundRect(bx, by, 62, 24, 4, C_GLASS_HIGH);
        tft.drawRoundRect(bx, by, 62, 24, 4, scCols[i]);
        tft.setTextColor(scCols[i], C_GLASS_HIGH);
        tft.setTextDatum(MC_DATUM);
        tft.drawString(scNames[i], bx + 31, by + 12, 1);
    }
}

// =========================================================================
//  TAB 1: LIGHTS QUICK TILES (2x2 GRID)
// =========================================================================
void drawTabLights() {
    tft.fillRect(0, 25, 320, 186, C_BG);

    int cardW = 148;
    int cardH = 86;

    for (int i = 0; i < 4; i++) {
        int r = i / 2;
        int c = i % 2;
        int cx = 8 + (c * (cardW + 8));
        int cy = 28 + (r * (cardH + 6));

        bool on = lights[i].state;
        uint16_t cardBg = on ? C_GLASS_HIGH : C_GLASS;
        uint16_t accentCol = on ? C_YELLOW : C_BORDER;

        tft.fillRoundRect(cx, cy, cardW, cardH, 8, cardBg);
        tft.drawRoundRect(cx, cy, cardW, cardH, 8, accentCol);

        // Bulb / Icon
        tft.fillCircle(cx + 22, cy + 24, 10, on ? C_YELLOW : C_DARKGRAY);
        tft.drawCircle(cx + 22, cy + 24, 10, C_WHITE);

        // Name
        tft.setTextColor(C_WHITE, cardBg);
        tft.setTextDatum(TL_DATUM);
        tft.drawString(lights[i].name, cx + 40, cy + 12, 2);

        // State & Brightness text
        tft.setTextColor(on ? C_YELLOW : C_GRAY, cardBg);
        char stBuf[24];
        if (on) snprintf(stBuf, sizeof(stBuf), "ON • %d%%", lights[i].brightness);
        else snprintf(stBuf, sizeof(stBuf), "OFF");
        tft.drawString(stBuf, cx + 40, cy + 30, 1);

        // Dimmer Bar at bottom of tile
        int bw = cardW - 24;
        tft.fillRoundRect(cx + 12, cy + 56, bw, 6, 3, C_DARKGRAY);
        if (on) {
            int fill = (bw * lights[i].brightness) / 100;
            tft.fillRoundRect(cx + 12, cy + 56, fill, 6, 3, C_YELLOW);
            tft.fillCircle(cx + 12 + fill, cy + 59, 4, C_WHITE);
        }

        // Tap toggle hint
        tft.setTextColor(C_GRAY, cardBg);
        tft.setTextDatum(BR_DATUM);
        tft.drawString("TAP TO TOGGLE", cx + cardW - 8, cy + cardH - 4, 1);
    }
}

// =========================================================================
//  TAB 2: CLIMATE MASTER THERMOSTAT
// =========================================================================
void drawTabClimate() {
    tft.fillRect(0, 25, 320, 186, C_BG);

    // Master Thermostat Card
    tft.fillRoundRect(8, 28, 304, 178, 8, C_GLASS);
    tft.drawRoundRect(8, 28, 304, 178, 8, C_BORDER);

    // Big Center Dial (Radius 46)
    int cx = 110;
    int cy = 100;
    tft.drawCircle(cx, cy, 46, C_DARKGRAY);
    tft.drawCircle(cx, cy, 45, C_ORANGE);
    tft.drawCircle(cx, cy, 44, C_ORANGE);

    tft.setTextColor(C_WHITE, C_GLASS);
    tft.setTextDatum(MC_DATUM);
    char tBuf[16];
    snprintf(tBuf, sizeof(tBuf), "%.1f C", targetTemp);
    tft.drawString(tBuf, cx, cy - 4, 4);

    tft.setTextColor(C_GRAY, C_GLASS);
    char actBuf[32];
    snprintf(actBuf, sizeof(actBuf), "Room: %.1f C • %d%% RH", currentTemp, currentHumidity);
    tft.drawString(actBuf, cx, cy + 22, 1);

    // Large Temperature Buttons (+ / -)
    // Minus Button (Center: 32, 100)
    tft.fillCircle(34, 100, 18, C_GLASS_HIGH);
    tft.drawCircle(34, 100, 18, C_CYAN);
    tft.setTextColor(C_CYAN, C_GLASS_HIGH);
    tft.drawString("-", 34, 100, 4);

    // Plus Button (Center: 186, 100)
    tft.fillCircle(186, 100, 18, C_GLASS_HIGH);
    tft.drawCircle(186, 100, 18, C_ORANGE);
    tft.setTextColor(C_ORANGE, C_GLASS_HIGH);
    tft.drawString("+", 186, 100, 4);

    // Right Mode Selection Stack (X: 216, Y: 36, W: 88)
    const char* mNames[4] = { "HEAT", "COOL", "AUTO", "ECO" };
    uint16_t mCols[4] = { C_ORANGE, C_CYAN, C_GREEN, C_YELLOW };

    for (int i = 0; i < 4; i++) {
        int by = 38 + (i * 38);
        bool isAct = (currentHvac == i);
        uint16_t bBg = isAct ? mCols[i] : C_GLASS_HIGH;
        uint16_t bFg = isAct ? C_BLACK : C_WHITE;

        tft.fillRoundRect(216, by, 86, 30, 4, bBg);
        tft.drawRoundRect(216, by, 86, 30, 4, isAct ? C_WHITE : C_BORDER);
        tft.setTextColor(bFg, bBg);
        tft.drawString(mNames[i], 259, by + 15, 2);
    }
}

// =========================================================================
//  TAB 3: ENERGY & SOLAR DASHBOARD
// =========================================================================
void drawTabEnergy() {
    tft.fillRect(0, 25, 320, 186, C_BG);

    // Solar Generation Card (Top Left)
    tft.fillRoundRect(8, 28, 148, 86, 8, C_GLASS);
    tft.drawRoundRect(8, 28, 148, 86, 8, C_YELLOW);
    tft.setTextColor(C_YELLOW, C_GLASS);
    tft.setTextDatum(TL_DATUM);
    tft.drawString("SOLAR ROOF", 16, 34, 2);
    tft.setTextColor(C_WHITE, C_GLASS);
    char solBuf[16];
    snprintf(solBuf, sizeof(solBuf), "%.2f kW", solarKw);
    tft.drawString(solBuf, 16, 56, 4);
    tft.setTextColor(C_GRAY, C_GLASS);
    tft.drawString("Peak: 4.8 kW today", 16, 88, 1);

    // Home Load Card (Top Right)
    tft.fillRoundRect(164, 28, 148, 86, 8, C_GLASS);
    tft.drawRoundRect(164, 28, 148, 86, 8, C_CYAN);
    tft.setTextColor(C_CYAN, C_GLASS);
    tft.setTextDatum(TL_DATUM);
    tft.drawString("HOME LOAD", 172, 34, 2);
    tft.setTextColor(C_WHITE, C_GLASS);
    char loadBuf[16];
    snprintf(loadBuf, sizeof(loadBuf), "%.2f kW", homeKw);
    tft.drawString(loadBuf, 172, 56, 4);
    tft.setTextColor(C_GREEN, C_GLASS);
    tft.drawString("NET EXPORT: 1.57 kW", 172, 88, 1);

    // Home Battery Card (Bottom Span: X: 8, Y: 120, W: 304, H: 86)
    tft.fillRoundRect(8, 120, 304, 86, 8, C_GLASS);
    tft.drawRoundRect(8, 120, 304, 86, 8, C_BORDER);

    tft.setTextColor(C_WHITE, C_GLASS);
    tft.setTextDatum(TL_DATUM);
    tft.drawString("HOME BATTERY STORAGE", 18, 128, 2);

    tft.setTextDatum(TR_DATUM);
    tft.setTextColor(C_GREEN, C_GLASS);
    char socBuf[16];
    snprintf(socBuf, sizeof(socBuf), "%d%% SOC", batterySoc);
    tft.drawString(socBuf, 302, 128, 2);

    // Battery Progress Gauge
    int barW = 280;
    tft.fillRoundRect(18, 150, barW, 14, 6, C_DARKGRAY);
    int bFill = (barW * batterySoc) / 100;
    tft.fillRoundRect(18, 150, bFill, 14, 6, C_GREEN);

    tft.setTextColor(C_GRAY, C_GLASS);
    tft.setTextDatum(TL_DATUM);
    tft.drawString("Status: CHARGING • 13.5 kWh Capacity • 9.2 hrs to Full", 18, 174, 1);
}

// =========================================================================
//  ACTIVE TAB RENDER ROUTER
// =========================================================================
void renderCurrentTab() {
    drawTabBar();
    switch (currentTab) {
        case TAB_HOME:    drawTabHome(); break;
        case TAB_LIGHTS:  drawTabLights(); break;
        case TAB_CLIMATE: drawTabClimate(); break;
        case TAB_ENERGY:  drawTabEnergy(); break;
    }
    updateStatusLed();
}

// =========================================================================
//  TOUCH INPUT HANDLING
// =========================================================================
void handleTouch() {
    if (!touch.touched()) return;

    TS_Point p = touch.getPoint();
    int tx = map(p.x, TS_MIN_X, TS_MAX_X, 0, 320);
    int ty = map(p.y, TS_MIN_Y, TS_MAX_Y, 0, 240);
    tx = constrain(tx, 0, 319);
    ty = constrain(ty, 0, 239);

    // 1. Bottom Tab Bar Touch (Y: 210..240)
    if (ty >= 210) {
        int tw = 320 / 4;
        int tIdx = tx / tw;
        if (tIdx >= 0 && tIdx < 4 && tIdx != currentTab) {
            currentTab = tIdx;
            playClick(1800, 20);
            renderCurrentTab();
            delay(250);
            return;
        }
    }

    // 2. Tab Specific Touches
    if (currentTab == TAB_HOME) {
        // Temperature +/- Buttons
        if (ty >= 140 && ty <= 175) {
            if (tx >= 18 && tx <= 74) {
                targetTemp -= 0.5;
                playClick(1400, 15);
                drawTabHome();
                delay(180);
                return;
            }
            if (tx >= 90 && tx <= 146) {
                targetTemp += 0.5;
                playClick(1800, 15);
                drawTabHome();
                delay(180);
                return;
            }
        }

        // Security State Toggle (X: 172..304, Y: 45..85)
        if (tx >= 172 && tx <= 304 && ty >= 45 && ty <= 85) {
            currentAlarm = (currentAlarm + 1) % 3;
            playClick(2200, 30);
            drawTabHome();
            updateStatusLed();
            delay(250);
            return;
        }

        // Quick Scene Buttons (X: 172..304, Y: 135..200)
        if (tx >= 164 && ty >= 135 && ty <= 200) {
            playClick(2000, 25);
            // Flash LED cyan
            setRGB(0, 255, 255);
            delay(150);
            updateStatusLed();
            return;
        }
    } else if (currentTab == TAB_LIGHTS) {
        // 4 Light Tiles
        int cardW = 148;
        int cardH = 86;
        for (int i = 0; i < 4; i++) {
            int r = i / 2;
            int c = i % 2;
            int cx = 8 + (c * (cardW + 8));
            int cy = 28 + (r * (cardH + 6));

            if (tx >= cx && tx <= cx + cardW && ty >= cy && ty <= cy + cardH) {
                lights[i].state = !lights[i].state;
                playClick(lights[i].state ? 2000 : 1000, 20);
                drawTabLights();
                delay(220);
                return;
            }
        }
    } else if (currentTab == TAB_CLIMATE) {
        // Plus / Minus Buttons
        if (abs(tx - 34) < 26 && abs(ty - 100) < 26) {
            targetTemp -= 0.5;
            playClick(1400, 15);
            drawTabClimate();
            delay(180);
            return;
        }
        if (abs(tx - 186) < 26 && abs(ty - 100) < 26) {
            targetTemp += 0.5;
            playClick(1800, 15);
            drawTabClimate();
            delay(180);
            return;
        }

        // Mode buttons (X: 216..302)
        if (tx >= 216 && tx <= 302) {
            for (int i = 0; i < 4; i++) {
                int by = 38 + (i * 38);
                if (ty >= by && ty <= by + 30) {
                    currentHvac = i;
                    playClick(1800, 20);
                    drawTabClimate();
                    updateStatusLed();
                    delay(200);
                    return;
                }
            }
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

    // Boot Sequence
    tft.setTextColor(C_CYAN, C_BG);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("IamNotIntoTech", 160, 90, 4);
    tft.setTextColor(C_WHITE, C_BG);
    tft.drawString("SMART HOME GLASS PANEL", 160, 130, 2);
    tft.setTextColor(C_GREEN, C_BG);
    tft.drawString("Connecting to Home Assistant API...", 160, 160, 1);

    playClick(1200, 50);
    delay(70);
    playClick(1800, 80);
    delay(700);

    tft.fillScreen(C_BG);

    drawHeader();
    renderCurrentTab();
}

void loop() {
    handleTouch();

    // 1-second clock & environment telemetry update
    unsigned long now = millis();
    if (now - lastSecTick >= 1000) {
        lastSecTick = now;
        clockSecs++;
        if (clockSecs >= 60) {
            clockSecs = 0;
            clockMins++;
            if (clockMins >= 60) {
                clockMins = 0;
                clockHours = (clockHours + 1) % 24;
            }
        }
        drawHeader();

        // Slow temperature micro-drift simulation
        if (random(0, 15) == 0) {
            currentTemp += (random(-10, 11) / 100.0);
            if (currentTab == TAB_HOME) drawTabHome();
            if (currentTab == TAB_CLIMATE) drawTabClimate();
        }
    }

    delay(20);
}

