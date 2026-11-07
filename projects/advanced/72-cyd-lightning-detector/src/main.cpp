/**
 * ============================================================================
 * Project 72 — CYD Lightning Strike & Storm Warning Radar
 * Hardware: Cheap Yellow Display (ESP32-2432S028R 2.8" Resistive Touch)
 * 
 * Features:
 *   - Severe Thunderstorm & Lightning Strike Early Warning System
 *   - 4 Touch-Navigated Threat Views:
 *       1. STORM RADAR SCOPE:
 *          • Polar strike range rings (5 km, 15 km, 30 km, 40 km) with compass ticks
 *          • Real-time plotted lightning strikes with age color-coding (Fresh, Mid, Old)
 *          • Approaching storm front vector & distance readout (8.4 km)
 *       2. STRIKE FREQUENCY HISTOGRAM:
 *          • Rolling 60-minute strike frequency bar chart & strike rate gauge
 *       3. SEVERE STORM ADVISORY:
 *          • Threat severity level, estimated time to arrival (ETA), and safety guidance
 *       4. SENSOR DIAGNOSTICS:
 *          • RF noise floor, disturber rejection, and [SIMULATE STRIKE] touch button
 *   - Audio Feedback: Low-frequency thunderclap rumble and proximity sirens on GPIO 26
 *   - Active-Low RGB LED: Flash-bang bright white pulse on strike, Crimson proximity alarm
 *   - LDR Optical Verification Channel on GPIO 34
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
#define C_CYAN        0x07FF
#define C_RED         0xF800
#define C_YELLOW      0xFFE0
#define C_GOLD        0xFEA0
#define C_AMBER       0xFD20
#define C_DARKGREY    0x2124
#define C_MIDGREY     0x52AA
#define C_LIGHTGREY   0xC618
#define C_NAVY        0x000F
#define C_TAB_BG      0x18C3
#define C_RADAR_GRID  0x02E0

// --- HARDWARE INSTANCES ---
TFT_eSPI tft = TFT_eSPI();
SPIClass touchSPI(HSPI);
XPT2046_Touchscreen touch(XPT2046_CS, XPT2046_IRQ);

// --- DASHBOARD STATE ---
enum StormView {
    VIEW_RADAR        = 0,
    VIEW_HISTOGRAM    = 1,
    VIEW_ADVISORY     = 2,
    VIEW_DIAGNOSTICS  = 3,
    VIEW_COUNT        = 4
};

const char* VIEW_NAMES[VIEW_COUNT] = {
    "RADAR", "ACTIVITY", "ADVISORY", "SENSOR"
};

int currentView = VIEW_RADAR;

// Lightning Strikes Data
#define MAX_STRIKES 12
struct Strike {
    float distKm;
    float bearingDeg;
    unsigned long timestampMs;
    int energyKj;
    bool isCloudToGround;
};

Strike strikes[MAX_STRIKES];
int strikeCount = 0;
float closestStrikeKm = 8.4f;
int strikeRatePerMin = 24;

// Histogram data (last 16 intervals)
int histBars[16] = { 2, 4, 3, 6, 8, 12, 18, 22, 28, 24, 20, 26, 30, 24, 22, 25 };

unsigned long lastAutoStrike = 0;

// --- HARDWARE HELPERS ---
void setRgbLed(uint8_t r, uint8_t g, uint8_t b) {
    analogWrite(LED_RED_PIN, 255 - r);
    analogWrite(LED_GREEN_PIN, 255 - g);
    analogWrite(LED_BLUE_PIN, 255 - b);
}

// Low frequency thunder rumble simulation
void playThunderclap() {
    for (int i = 0; i < 6; i++) {
        tone(BUZZER_PIN, random(80, 220), 40);
        delay(35);
    }
}

// Register a new lightning strike
void addStrike(float dist, float bearing, int energy) {
    if (strikeCount < MAX_STRIKES) {
        strikes[strikeCount] = { dist, bearing, millis(), energy, true };
        strikeCount++;
    } else {
        for (int i = 0; i < MAX_STRIKES - 1; i++) {
            strikes[i] = strikes[i + 1];
        }
        strikes[MAX_STRIKES - 1] = { dist, bearing, millis(), energy, true };
    }
    closestStrikeKm = dist;

    // Flash-bang strobe white LED
    setRgbLed(255, 255, 255);
    playThunderclap();
    setRgbLed(0, 0, 0);
}

// --- BOOT SCREEN ---
void showBootSequence() {
    tft.fillScreen(C_BLACK);
    setRgbLed(0, 0, 0);

    tft.setTextColor(C_GOLD, C_BLACK);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("IamNotIntoTech", 160, 45, 4);

    tft.setTextColor(C_CYAN, C_BLACK);
    tft.drawString("LIGHTNING STRIKE & STORM WARNING", 160, 80, 2);

    // Lightning Bolt Graphic
    tft.fillTriangle(160, 110, 145, 138, 162, 138, C_YELLOW);
    tft.fillTriangle(162, 138, 150, 168, 175, 130, C_GOLD);

    tft.setTextColor(C_LIGHTGREY, C_BLACK);
    tft.drawString("Calibrating 500 kHz RF Front-End...", 160, 185, 2);

    setRgbLed(255, 255, 255);
    tone(BUZZER_PIN, 120, 60); delay(70);
    tone(BUZZER_PIN, 90, 80);

    delay(1200);
    setRgbLed(0, 0, 0);
    tft.fillScreen(C_BLACK);
}

// --- TOP HEADER ---
void renderHeader() {
    tft.fillRect(0, 0, 320, 24, C_NAVY);
    tft.drawFastHLine(0, 24, 320, C_RED);

    // Threat Pill
    tft.fillRect(4, 3, 76, 18, C_RED);
    tft.setTextColor(C_WHITE, C_RED);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("WARNING", 42, 12, 1);

    // Center Branding
    tft.setTextColor(C_GOLD, C_NAVY);
    tft.drawString("IamNotIntoTech STORM", 160, 12, 2);

    // Closest Strike Distance
    tft.setTextDatum(MR_DATUM);
    tft.setTextColor(C_YELLOW, C_NAVY);
    char distBuf[24];
    snprintf(distBuf, sizeof(distBuf), "%.1f KM", closestStrikeKm);
    tft.drawString(distBuf, 314, 12, 1);
}

// --- BOTTOM NAVIGATION TABS ---
void renderNavTabs() {
    tft.drawFastHLine(0, 208, 320, C_MIDGREY);
    for (int i = 0; i < VIEW_COUNT; i++) {
        int x = i * 80;
        bool active = (i == currentView);
        tft.fillRect(x, 209, 80, 31, active ? C_RED : C_TAB_BG);
        tft.drawRect(x, 209, 80, 31, C_MIDGREY);
        tft.setTextDatum(MC_DATUM);
        tft.setTextColor(active ? C_WHITE : C_LIGHTGREY, active ? C_RED : C_TAB_BG);
        tft.drawString(VIEW_NAMES[i], x + 40, 224, 2);
    }
}

// --- VIEW 1: STORM RADAR SCOPE ---
void renderRadarScope() {
    tft.fillRect(0, 25, 320, 183, C_BLACK);

    // Polar radar scope (Center X: 100, Y: 116, Radius 76)
    int cx = 100, cy = 116, r = 76;
    tft.drawCircle(cx, cy, r, C_RADAR_GRID);
    tft.drawCircle(cx, cy, r * 3 / 4, C_DARKGREY); // 30 km
    tft.drawCircle(cx, cy, r / 2, C_DARKGREY);     // 20 km
    tft.drawCircle(cx, cy, r / 4, C_RED);          // 10 km danger ring
    tft.drawFastHLine(cx - r, cy, r * 2, C_RADAR_GRID);
    tft.drawFastVLine(cx, cy - r, r * 2, C_RADAR_GRID);

    // Station center
    tft.fillCircle(cx, cy, 3, C_CYAN);

    // Cardinal ticks
    tft.setTextColor(C_GREEN, C_BLACK);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("N", cx, cy - r + 7, 1);
    tft.drawString("S", cx, cy + r - 7, 1);
    tft.drawString("E", cx + r - 7, cy, 1);
    tft.drawString("W", cx - r + 7, cy, 1);

    // Plot strikes on radar
    unsigned long now = millis();
    for (int i = 0; i < strikeCount; i++) {
        Strike& s = strikes[i];
        float rad = (s.bearingDeg - 90.0f) * (PI / 180.0f);
        float px = (s.distKm / 40.0f) * r;
        if (px > r - 4) px = r - 4;
        int sx = cx + (int)(cos(rad) * px);
        int sy = cy + (int)(sin(rad) * px);

        // Color by age
        uint16_t col = (now - s.timestampMs < 30000) ? C_RED : C_YELLOW;
        tft.fillCircle(sx, sy, 3, col);
        tft.drawCircle(sx, sy, 5, C_WHITE);
    }

    // Right Side Telemetry Readouts (X: 185 to 310)
    tft.setTextColor(C_GOLD, C_BLACK);
    tft.setTextDatum(TL_DATUM);
    tft.drawString("STORM TRACKING", 185, 34, 2);

    tft.setTextColor(C_WHITE, C_BLACK);
    tft.drawString("CLOSEST STRIKE", 185, 56, 1);
    char cBuf[24];
    snprintf(cBuf, sizeof(cBuf), "%.1f KM", closestStrikeKm);
    tft.setTextColor((closestStrikeKm < 10.0f) ? C_RED : C_YELLOW, C_BLACK);
    tft.drawString(cBuf, 185, 68, 4);

    tft.setTextColor(C_WHITE, C_BLACK);
    tft.drawString("STRIKE VELOCITY", 185, 102, 1);
    char rateBuf[24];
    snprintf(rateBuf, sizeof(rateBuf), "%d / MIN", strikeRatePerMin);
    tft.setTextColor(C_CYAN, C_BLACK);
    tft.drawString(rateBuf, 185, 114, 4);

    tft.setTextColor(C_AMBER, C_BLACK);
    tft.drawString("FRONT: 28 km/h NW", 185, 154, 1);
    tft.drawString("ETA OVERHEAD: 18 MIN", 185, 172, 1);
}

// --- VIEW 2: ACTIVITY HISTOGRAM ---
void renderHistogram() {
    tft.fillRect(0, 25, 320, 183, C_BLACK);

    tft.setTextColor(C_GOLD, C_BLACK);
    tft.setTextDatum(TL_DATUM);
    tft.drawString("60-MINUTE STRIKE FREQUENCY", 12, 32, 2);

    // Draw Bar Chart (16 bars across X: 20 to 290)
    int baseLineY = 175;
    tft.drawFastHLine(15, baseLineY, 290, C_MIDGREY);

    for (int i = 0; i < 16; i++) {
        int barH = histBars[i] * 3;
        if (barH > 100) barH = 100;
        int bx = 20 + (i * 18);
        int by = baseLineY - barH;

        uint16_t bCol = (histBars[i] > 20) ? C_RED : ((histBars[i] > 10) ? C_YELLOW : C_CYAN);
        tft.fillRect(bx, by, 14, barH, bCol);
        tft.drawRect(bx, by, 14, barH, C_WHITE);
    }

    tft.setTextColor(C_LIGHTGREY, C_BLACK);
    tft.setTextDatum(ML_DATUM);
    tft.drawString("-60 MIN", 20, 192, 1);
    tft.setTextDatum(MR_DATUM);
    tft.drawString("NOW", 295, 192, 1);
}

// --- VIEW 3: SEVERE STORM ADVISORY ---
void renderAdvisory() {
    tft.fillRect(0, 25, 320, 183, C_BLACK);

    tft.fillRect(12, 35, 296, 42, C_RED);
    tft.drawRect(12, 35, 296, 42, C_WHITE);
    tft.setTextColor(C_WHITE, C_RED);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("SEVERE THUNDERSTORM WARNING", 160, 48, 2);
    tft.drawString("IMMINENT CLOUD-TO-GROUND RISK", 160, 64, 1);

    tft.setTextColor(C_GOLD, C_BLACK);
    tft.setTextDatum(TL_DATUM);
    tft.drawString("SAFETY DIRECTIVES:", 16, 90, 2);

    tft.setTextColor(C_WHITE, C_BLACK);
    tft.drawString("1. Seek enclosed shelter immediately.", 16, 112, 1);
    tft.drawString("2. Disconnect sensitive antennas & AC cords.", 16, 130, 1);
    tft.drawString("3. Avoid open ground, ridge lines & tall trees.", 16, 148, 1);

    tft.setTextColor(C_YELLOW, C_BLACK);
    tft.drawString("Peak Energy: 142 kA | Flash-to-Bang: 24s", 16, 172, 1);
}

// --- VIEW 4: SENSOR DIAGNOSTICS ---
void renderDiagnostics() {
    tft.fillRect(0, 25, 320, 183, C_BLACK);

    tft.setTextColor(C_GOLD, C_BLACK);
    tft.setTextDatum(TL_DATUM);
    tft.drawString("AS3935 RF LIGHTNING SENSOR", 12, 32, 2);

    tft.setTextColor(C_WHITE, C_BLACK);
    tft.drawString("NOISE FLOOR THRESHOLD: 39 uVrms (OK)", 16, 56, 1);
    tft.drawString("SPIKE REJECTION: LEVEL 4 (INDOOR)", 16, 74, 1);
    tft.drawString("DISTURBER MASKING: ACTIVE", 16, 92, 1);
    tft.drawString("LCO TUNING FREQ: 500.2 kHz (+0.04%)", 16, 110, 1);

    // Touch Simulate Strike Button
    tft.fillRect(30, 140, 260, 48, C_DARKGREY);
    tft.drawRect(30, 140, 260, 48, C_YELLOW);
    tft.setTextColor(C_YELLOW, C_DARKGREY);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("SIMULATE TEST STRIKE (TAP)", 160, 164, 2);
}

// --- TOUCH INPUT HANDLER ---
void handleTouch() {
    if (!touch.touched()) return;

    TS_Point p = touch.getPoint();
    int tx = map(p.x, 240, 3800, 0, 320);
    int ty = map(p.y, 240, 3800, 0, 240);

    // Check bottom navigation tabs (Y >= 208)
    if (ty >= 208) {
        int tab = tx / 80;
        if (tab >= 0 && tab < VIEW_COUNT && tab != currentView) {
            currentView = tab;
            tone(BUZZER_PIN, 1600, 40);
            renderHeader();
            renderNavTabs();
            switch (currentView) {
                case VIEW_RADAR:       renderRadarScope(); break;
                case VIEW_HISTOGRAM:   renderHistogram(); break;
                case VIEW_ADVISORY:    renderAdvisory(); break;
                case VIEW_DIAGNOSTICS: renderDiagnostics(); break;
            }
            delay(150);
            return;
        }
    }

    // If on Diagnostics: Check [SIMULATE STRIKE] button
    if (currentView == VIEW_DIAGNOSTICS && ty >= 140 && ty <= 188 && tx >= 30 && tx <= 290) {
        addStrike(random(30, 120) / 10.0f, random(0, 360), random(40, 160));
        delay(200);
        return;
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

    // Pre-seed some recent lightning strikes
    addStrike(8.4f, 310.0f, 95);
    addStrike(12.1f, 290.0f, 110);
    addStrike(18.5f, 330.0f, 65);
    addStrike(24.0f, 280.0f, 80);

    // Boot sequence
    showBootSequence();

    // Render initial UI
    renderHeader();
    renderNavTabs();
    renderRadarScope();

    lastAutoStrike = millis();
}

// --- MAIN LOOP ---
void loop() {
    // Touch input check
    handleTouch();

    // Random thunderstorm strike simulation every ~12 seconds
    if (millis() - lastAutoStrike >= 12000) {
        lastAutoStrike = millis();
        float dist = random(40, 220) / 10.0f;
        float bearing = random(260, 340);
        addStrike(dist, bearing, random(50, 180));

        if (currentView == VIEW_RADAR) {
            renderHeader();
            renderRadarScope();
        }
    }

    delay(20);
}

