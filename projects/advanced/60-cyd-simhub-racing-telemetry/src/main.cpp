// =========================================================================
//  Project 60 — CYD SimHub Racing Cockpit Telemetry HUD
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
#define C_BG          0x0821  // Deep obsidian slate
#define C_CARD        0x10A2  // Carbon card background
#define C_CARD_BORDER 0x2124
#define C_GRAY        0x73AE
#define C_DARKGRAY    0x2965
#define C_GREEN       0x07E0  // Shift Green
#define C_YELLOW      0xFFE0  // Shift Yellow
#define C_RED         0xF800  // Shift Red
#define C_BLUE        0x03FF  // Redline Strobe Blue
#define C_CYAN        0x07FF
#define C_MAGENTA     0xF81F
#define C_ORANGE      0xFD20

TFT_eSPI tft = TFT_eSPI();
SPIClass touchSPI(HSPI);
XPT2046_Touchscreen touch(TOUCH_CS, TOUCH_IRQ);

// =========================================================================
//  TELEMETRY & VEHICLE STATE
// =========================================================================
int currentGear = 6;       // 0: N, -1: R, 1..8
int currentRpm = 11450;
const int MAX_RPM = 13500;
const int REDLINE_RPM = 12600;
int currentSpeed = 248;    // KM/H
bool useMph = false;
bool shiftToneEnabled = true;

// Lap Timings
float deltaSeconds = -0.342; // Negative = faster (green), positive = slower (red)
int lapMinutes = 1;
int lapSeconds = 23;
int lapMillis = 412;
int bestLapMinutes = 1;
int bestLapSeconds = 22;
int bestLapMillis = 880;

// Tires & Vehicle
int tireTempFL = 98;
int tireTempFR = 104;
int tireTempRL = 92;
int tireTempRR = 96;
float fuelLiters = 18.4;
int ersBatteryPct = 82;
bool drsAvailable = true;
bool drsActive = true;

// Simulation loop variables
float simPhase = 0;
unsigned long lastFrameTime = 0;

// =========================================================================
//  SOUND & LED OUTPUT
// =========================================================================
void playBeep(int freq = 2000, int ms = 15) {
    if (!shiftToneEnabled) return;
    tone(BUZZER_PIN, freq, ms);
}

void setRGB(uint8_t r, uint8_t g, uint8_t b) {
    analogWrite(LED_R, 255 - r);
    analogWrite(LED_G, 255 - g);
    analogWrite(LED_B, 255 - b);
}

// =========================================================================
//  SHIFT LIGHT BAR (16 LED SEGMENTS ACROSS TOP)
// =========================================================================
void drawShiftLightBar() {
    int totalLeds = 16;
    int ledW = 16;
    int ledH = 14;
    int spacing = 3;
    int startX = (320 - (totalLeds * (ledW + spacing) - spacing)) / 2;
    int startY = 4;

    int activeLeds = map(currentRpm, 6000, MAX_RPM, 0, totalLeds);
    activeLeds = constrain(activeLeds, 0, totalLeds);
    bool isRedlining = (currentRpm >= REDLINE_RPM);

    for (int i = 0; i < totalLeds; i++) {
        int x = startX + (i * (ledW + spacing));
        uint16_t col = C_DARKGRAY;

        if (isRedlining) {
            // Flash all LEDs blue/white strobe
            col = ((millis() / 60) % 2 == 0) ? C_BLUE : C_WHITE;
        } else if (i < activeLeds) {
            if (i < 5) col = C_GREEN;
            else if (i < 10) col = C_YELLOW;
            else if (i < 14) col = C_RED;
            else col = C_BLUE;
        }

        tft.fillRoundRect(x, startY, ledW, ledH, 3, col);
        tft.drawRoundRect(x, startY, ledW, ledH, 3, C_BLACK);
    }

    // Hardware shift strobe
    if (isRedlining) {
        setRGB(0, 50, 255);
        if ((millis() / 80) % 2 == 0) playBeep(2800, 15);
    } else {
        setRGB(0, 0, 0);
    }
}

// =========================================================================
//  COCKPIT PANELS
// =========================================================================
void drawSpeedometer() {
    int sx = 8;
    int sy = 24;
    int sw = 90;
    int sh = 118;

    tft.fillRoundRect(sx, sy, sw, sh, 6, C_CARD);
    tft.drawRoundRect(sx, sy, sw, sh, 6, C_CARD_BORDER);

    // Speed display
    tft.setTextColor(C_GRAY, C_CARD);
    tft.setTextDatum(TL_DATUM);
    tft.drawString(useMph ? "SPEED MPH" : "SPEED KMH", sx + 8, sy + 6, 1);

    int dispSpeed = useMph ? (int)(currentSpeed * 0.621371) : currentSpeed;
    char spdBuf[16];
    snprintf(spdBuf, sizeof(spdBuf), "%d", dispSpeed);

    tft.setTextColor(C_WHITE, C_CARD);
    tft.setTextDatum(MC_DATUM);
    tft.drawString(spdBuf, sx + (sw / 2), sy + 44, 6);

    // ERS Battery Bar (bottom of speed card)
    tft.setTextColor(C_CYAN, C_CARD);
    tft.setTextDatum(TL_DATUM);
    char ersBuf[16];
    snprintf(ersBuf, sizeof(ersBuf), "ERS %d%%", ersBatteryPct);
    tft.drawString(ersBuf, sx + 8, sy + 82, 1);

    tft.fillRoundRect(sx + 8, sy + 96, sw - 16, 8, 3, C_DARKGRAY);
    int ersFill = (ersBatteryPct * (sw - 16)) / 100;
    tft.fillRoundRect(sx + 8, sy + 96, ersFill, 8, 3, C_CYAN);
}

void drawGearBox() {
    int gx = 104;
    int gy = 24;
    int gw = 112;
    int gh = 118;

    tft.fillRoundRect(gx, gy, gw, gh, 6, C_CARD);
    tft.drawRoundRect(gx, gy, gw, gh, 6, currentRpm >= REDLINE_RPM ? C_RED : C_CYAN);

    // Large Current Gear numeral
    char gearChar[4] = "N";
    if (currentGear == -1) snprintf(gearChar, sizeof(gearChar), "R");
    else if (currentGear == 0) snprintf(gearChar, sizeof(gearChar), "N");
    else snprintf(gearChar, sizeof(gearChar), "%d", currentGear);

    tft.setTextColor(currentRpm >= REDLINE_RPM ? C_RED : C_WHITE, C_CARD);
    tft.setTextDatum(MC_DATUM);
    tft.drawString(gearChar, gx + (gw / 2), gy + 44, 7);

    // RPM readout
    char rpmBuf[24];
    snprintf(rpmBuf, sizeof(rpmBuf), "%d RPM", currentRpm);
    tft.setTextColor(C_YELLOW, C_CARD);
    tft.drawString(rpmBuf, gx + (gw / 2), gy + 94, 2);

    // DRS / Status Tag
    if (drsActive) {
        tft.fillRoundRect(gx + 12, gy + 106, gw - 24, 10, 2, C_GREEN);
        tft.setTextColor(C_BLACK, C_GREEN);
        tft.drawString("DRS ACTIVE", gx + (gw / 2), gy + 110, 1);
    } else {
        tft.fillRoundRect(gx + 12, gy + 106, gw - 24, 10, 2, C_DARKGRAY);
        tft.setTextColor(C_GRAY, C_DARKGRAY);
        tft.drawString("DRS CLOSED", gx + (gw / 2), gy + 110, 1);
    }
}

void drawDeltaPanel() {
    int dx = 222;
    int dy = 24;
    int dw = 90;
    int dh = 118;

    tft.fillRoundRect(dx, dy, dw, dh, 6, C_CARD);
    tft.drawRoundRect(dx, dy, dw, dh, 6, C_CARD_BORDER);

    // Delta Lap Time Box
    tft.setTextColor(C_GRAY, C_CARD);
    tft.setTextDatum(TL_DATUM);
    tft.drawString("DELTA LAP", dx + 8, dy + 6, 1);

    bool isFaster = (deltaSeconds <= 0);
    uint16_t dCol = isFaster ? C_GREEN : C_RED;
    char deltaBuf[24];
    snprintf(deltaBuf, sizeof(deltaBuf), "%s%.3f", isFaster ? "-" : "+", fabs(deltaSeconds));

    tft.setTextColor(dCol, C_CARD);
    tft.setTextDatum(MC_DATUM);
    tft.drawString(deltaBuf, dx + (dw / 2), dy + 32, 4);

    // Current Lap Time
    tft.setTextColor(C_GRAY, C_CARD);
    tft.setTextDatum(TL_DATUM);
    tft.drawString("LAP TIME", dx + 8, dy + 54, 1);

    char lapBuf[24];
    snprintf(lapBuf, sizeof(lapBuf), "%02d:%02d.%03d", lapMinutes, lapSeconds, lapMillis);
    tft.setTextColor(C_WHITE, C_CARD);
    tft.drawString(lapBuf, dx + 8, dy + 68, 1);

    // Best Lap
    tft.setTextColor(C_GRAY, C_CARD);
    tft.drawString("BEST LAP", dx + 8, dy + 84, 1);
    char bestBuf[24];
    snprintf(bestBuf, sizeof(bestBuf), "%02d:%02d.%03d", bestLapMinutes, bestLapSeconds, bestLapMillis);
    tft.setTextColor(C_MAGENTA, C_CARD);
    tft.drawString(bestBuf, dx + 8, dy + 98, 1);
}

// Helper to color-code tire temperature
uint16_t getTireColor(int temp) {
    if (temp < 80) return C_BLUE;         // Cold
    if (temp <= 100) return C_GREEN;      // Optimum
    if (temp <= 110) return C_ORANGE;     // Hot
    return C_RED;                         // Overheating
}

void drawTireTelemetry() {
    int tx = 8;
    int ty = 148;
    int tw = 186;
    int th = 60;

    tft.fillRoundRect(tx, ty, tw, th, 6, C_CARD);
    tft.drawRoundRect(tx, ty, tw, th, 6, C_CARD_BORDER);

    tft.setTextColor(C_GRAY, C_CARD);
    tft.setTextDatum(TL_DATUM);
    tft.drawString("TIRE TEMPS (C)", tx + 8, ty + 6, 1);

    // 4 Tire Badges
    const char* tLabels[4] = { "FL", "FR", "RL", "RR" };
    int tVals[4] = { tireTempFL, tireTempFR, tireTempRL, tireTempRR };
    int tCoords[4][2] = {
        { tx + 12, ty + 20 },
        { tx + 98, ty + 20 },
        { tx + 12, ty + 38 },
        { tx + 98, ty + 38 }
    };

    for (int i = 0; i < 4; i++) {
        uint16_t tc = getTireColor(tVals[i]);
        tft.fillRoundRect(tCoords[i][0], tCoords[i][1], 78, 16, 3, C_BG);
        tft.drawRoundRect(tCoords[i][0], tCoords[i][1], 78, 16, 3, tc);
        tft.setTextColor(tc, C_BG);
        tft.setTextDatum(TL_DATUM);
        char tb[16];
        snprintf(tb, sizeof(tb), "%s:%dC", tLabels[i], tVals[i]);
        tft.drawString(tb, tCoords[i][0] + 6, tCoords[i][1] + 2, 1);
    }
}

void drawFuelAndBias() {
    int fx = 200;
    int fy = 148;
    int fw = 112;
    int fh = 60;

    tft.fillRoundRect(fx, fy, fw, fh, 6, C_CARD);
    tft.drawRoundRect(fx, fy, fw, fh, 6, C_CARD_BORDER);

    tft.setTextColor(C_GRAY, C_CARD);
    tft.setTextDatum(TL_DATUM);
    tft.drawString("FUEL / BRAKE", fx + 8, fy + 6, 1);

    // Fuel remaining
    char fBuf[24];
    snprintf(fBuf, sizeof(fBuf), "%.1f L (7.4 Laps)", fuelLiters);
    tft.setTextColor(C_YELLOW, C_CARD);
    tft.drawString(fBuf, fx + 8, fy + 22, 1);

    // Brake bias
    tft.setTextColor(C_WHITE, C_CARD);
    tft.drawString("BIAS: 54.2% F", fx + 8, fy + 38, 1);
}

void drawFooter() {
    int by = 214;
    tft.fillRect(0, by, 320, 26, C_BG);
    tft.drawFastHLine(0, by, 320, C_CARD_BORDER);

    // Brand and circuit name
    tft.setTextColor(C_CYAN, C_BG);
    tft.setTextDatum(TL_DATUM);
    tft.drawString("IamNotIntoTech", 8, by + 6, 2);

    tft.setTextColor(C_WHITE, C_BG);
    tft.drawString("SIMHUB COCKPIT", 128, by + 6, 2);

    tft.setTextColor(shiftToneEnabled ? C_GREEN : C_GRAY, C_BG);
    tft.setTextDatum(TR_DATUM);
    tft.drawString(shiftToneEnabled ? "AUDIO: ON" : "AUDIO: MUTE", 312, by + 6, 2);
}

// =========================================================================
//  SIMULATION TELEMETRY ENGINE
// =========================================================================
void updateSimulation() {
    simPhase += 0.08;

    // RPM acceleration & shifting wave
    int cycle = (int)(simPhase * 10) % 100;
    if (cycle < 85) {
        currentRpm = map(cycle, 0, 85, 7500, MAX_RPM);
        currentSpeed = map(cycle, 0, 85, 180, 295);
        currentGear = (cycle / 15) + 2;
        currentGear = constrain(currentGear, 2, 8);
    } else {
        // Downshift & braking
        currentRpm = map(cycle, 85, 100, 11000, 7200);
        currentSpeed = map(cycle, 85, 100, 295, 175);
        currentGear = 3;
    }

    // Lap timer advancement
    lapMillis += 33;
    if (lapMillis >= 1000) {
        lapMillis -= 1000;
        lapSeconds++;
        if (lapSeconds >= 60) {
            lapSeconds = 0;
            lapMinutes++;
        }
        // Micro delta drift
        deltaSeconds += (random(-8, 9) / 100.0);
    }

    // DRS trigger on high speed
    drsActive = (currentSpeed > 240);
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

    // 1. Audio Toggle (Footer Touch, Y > 210)
    if (ty >= 210) {
        shiftToneEnabled = !shiftToneEnabled;
        playBeep(2400, 20);
        drawFooter();
        delay(250);
        return;
    }

    // 2. Speed Unit Toggle (KM/H vs MPH, X: 8..98, Y: 24..140)
    if (tx >= 8 && tx <= 98 && ty >= 24 && ty <= 140) {
        useMph = !useMph;
        playBeep(1800, 15);
        drawSpeedometer();
        delay(250);
        return;
    }

    // 3. Gear Box Manual Override Tap (X: 104..216, Y: 24..140)
    if (tx >= 104 && tx <= 216 && ty >= 24 && ty <= 140) {
        currentGear = (currentGear + 1) % 9;
        playBeep(2200, 25);
        drawGearBox();
        delay(250);
        return;
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

    // Racing Boot Sequence
    tft.setTextColor(C_CYAN, C_BG);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("IamNotIntoTech", 160, 80, 4);
    tft.setTextColor(C_YELLOW, C_BG);
    tft.drawString("SIMHUB COCKPIT TELEMETRY HUD", 160, 120, 2);
    tft.setTextColor(C_WHITE, C_BG);
    tft.drawString("Connecting to SimHub UDP Telemetry Bus...", 160, 150, 1);

    playBeep(1200, 50);
    delay(60);
    playBeep(1800, 80);
    delay(700);

    tft.fillScreen(C_BG);

    drawFooter();
}

void loop() {
    handleTouch();

    unsigned long now = millis();
    if (now - lastFrameTime >= 33) {
        lastFrameTime = now;

        updateSimulation();

        drawShiftLightBar();
        drawSpeedometer();
        drawGearBox();
        drawDeltaPanel();
        drawTireTelemetry();
        drawFuelAndBias();
    }

    delay(10);
}
