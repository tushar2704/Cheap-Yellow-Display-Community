/**
 * ============================================================================
 * Project 66 — CYD F1 Pit Wall Telemetry Dashboard
 * Hardware: Cheap Yellow Display (ESP32-2432S028R 2.8" Resistive Touch)
 * 
 * Features:
 *   - Professional Formula 1 Pit Wall Race Strategy & Timing Console
 *   - 4 Interactive Touch Navigation Views:
 *       1. TIMING LEADERBOARD:
 *          • Top 6 Driver intervals (VER, NOR, LEC, HAM, PIA, RUS)
 *          • Gap deltas, compound badges ([S], [M], [H]), and sector color blocks
 *          • Purple fastest lap indicator
 *       2. TIRE STRATEGY & DEGRADATION:
 *          • 4-corner tire thermal heatmap (°C) & wear percentage
 *          • Stint lap age counter & pit window delta calculator ("BOX THIS LAP")
 *       3. WEATHER & TRACK CONDITIONS:
 *          • Track temperature, ambient air temp, humidity, and wind vector
 *          • Rain probability forecast arc gauge (Rain in 12 min)
 *       4. RACE CONTROL & FLAG SIMULATOR:
 *          • Interactive flag triggers: GREEN, VSC, SAFETY CAR, RED FLAG
 *          • Live acoustic pit siren and synchronized RGB flag strobe
 *   - Onboard Hardware Integration:
 *       • Active-low RGB LED flag status indicator (Green/Yellow/Orange/Crimson)
 *       • GPIO 26 Piezo pit radio chimes & safety car siren
 *       • GPIO 34 LDR ambient light sensor
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
#define C_RED         0xF800
#define C_GREEN       0x07E0
#define C_YELLOW      0xFFE0
#define C_CYAN        0x07FF
#define C_MAGENTA     0xF81F
#define C_PURPLE      0x780F
#define C_ORANGE      0xFD20
#define C_GOLD        0xFEA0
#define C_DARKGREY    0x2104
#define C_MIDGREY     0x632C
#define C_LIGHTGREY   0xBDD7
#define C_NAVY        0x000F
#define C_TAB_BG      0x18C3

// --- HARDWARE INSTANCES ---
TFT_eSPI tft = TFT_eSPI();
SPIClass touchSPI(HSPI);
XPT2046_Touchscreen touch(XPT2046_CS, XPT2046_IRQ);

// --- DASHBOARD STATE ---
enum DashboardView {
    VIEW_LEADERBOARD = 0,
    VIEW_TIRE_STRATEGY = 1,
    VIEW_WEATHER = 2,
    VIEW_RACE_CONTROL = 3,
    VIEW_COUNT = 4
};

const char* VIEW_NAMES[VIEW_COUNT] = {
    "TIMING", "TIRES", "METEO", "CONTROL"
};

int currentView = VIEW_LEADERBOARD;

// Race status
enum RaceFlag {
    FLAG_GREEN = 0,
    FLAG_VSC = 1,
    FLAG_SAFETY_CAR = 2,
    FLAG_RED = 3
};
int raceFlag = FLAG_GREEN;

int currentLap = 44;
int totalLaps = 57;
bool drsActive = true;
float safetyCarDelta = -0.42f;

// Driver Leaderboard
struct DriverTiming {
    const char* code;
    const char* gap;
    const char* tire;
    uint16_t tireColor;
    const char* lastLap;
    bool fastestLap;
};

DriverTiming grid[6] = {
    { "VER", "LEADER", "M", C_YELLOW, "1:21.432", false },
    { "NOR", "+1.842", "M", C_YELLOW, "1:21.610", false },
    { "LEC", "+3.205", "H", C_WHITE,  "1:21.890", false },
    { "HAM", "+6.411", "H", C_WHITE,  "1:22.040", false },
    { "PIA", "+8.930", "S", C_RED,    "1:21.214", true  },
    { "RUS", "+11.24", "M", C_YELLOW, "1:22.450", false }
};

// Tire degradation values
int tireWear[4] = { 68, 65, 72, 70 }; // FL, FR, RL, RR (%)
int tireTemp[4] = { 104, 106, 112, 110 }; // °C
int stintLaps = 18;

// Telemetry refresh timer
unsigned long lastUpdate = 0;
unsigned long lastFlagBlink = 0;
bool flagBlinkState = false;

// --- HARDWARE HELPERS ---
void setRgbLed(uint8_t r, uint8_t g, uint8_t b) {
    analogWrite(LED_RED_PIN, 255 - r);
    analogWrite(LED_GREEN_PIN, 255 - g);
    analogWrite(LED_BLUE_PIN, 255 - b);
}

void playRadioChime() {
    tone(BUZZER_PIN, 1200, 40); delay(60);
    tone(BUZZER_PIN, 1800, 80);
}

void playSiren() {
    for (int f = 800; f < 1600; f += 80) {
        tone(BUZZER_PIN, f, 10);
        delay(8);
    }
}

// --- BOOT SCREEN ---
void showBootSequence() {
    tft.fillScreen(C_BLACK);
    setRgbLed(0, 0, 0);

    // Checkered flag banner graphic
    for (int y = 0; y < 16; y += 8) {
        for (int x = 0; x < 320; x += 16) {
            bool white = ((x / 16) + (y / 8)) % 2 == 0;
            tft.fillRect(x, y, 16, 8, white ? C_WHITE : C_BLACK);
        }
    }

    tft.setTextColor(C_GOLD, C_BLACK);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("IamNotIntoTech", 160, 50, 4);

    tft.setTextColor(C_RED, C_BLACK);
    tft.drawString("FORMULA 1 PIT WALL TELEMETRY", 160, 85, 2);

    tft.drawRect(40, 115, 240, 50, C_DARKGREY);
    tft.fillRect(42, 117, 236, 46, C_NAVY);
    tft.setTextColor(C_CYAN, C_NAVY);
    tft.drawString("RACE CONTROL & STRATEGY ENGINE", 160, 140, 2);

    tft.setTextColor(C_LIGHTGREY, C_BLACK);
    tft.drawString("Connecting to Pit Lane Timing Bus...", 160, 190, 2);

    setRgbLed(0, 255, 0);
    playRadioChime();
    delay(1200);

    setRgbLed(0, 0, 0);
    tft.fillScreen(C_BLACK);
}

// --- TOP STATUS HEADER ---
void renderHeader() {
    // Header Bar (Y: 0 to 24)
    uint16_t flagColor = C_GREEN;
    const char* flagText = "TRACK CLEAR";
    if (raceFlag == FLAG_VSC) {
        flagColor = C_YELLOW;
        flagText = "VSC ACTIVE";
    } else if (raceFlag == FLAG_SAFETY_CAR) {
        flagColor = C_ORANGE;
        flagText = "SAFETY CAR";
    } else if (raceFlag == FLAG_RED) {
        flagColor = C_RED;
        flagText = "RED FLAG";
    }

    tft.fillRect(0, 0, 80, 24, flagColor);
    tft.setTextColor(C_BLACK, flagColor);
    tft.setTextDatum(MC_DATUM);
    tft.drawString(flagText, 40, 12, 1);

    // Center branding
    tft.fillRect(80, 0, 140, 24, C_NAVY);
    tft.setTextColor(C_GOLD, C_NAVY);
    tft.drawString("IamNotIntoTech PIT", 150, 12, 2);

    // Lap counter & DRS
    tft.fillRect(220, 0, 100, 24, C_DARKGREY);
    tft.setTextColor(C_WHITE, C_DARKGREY);
    char lapBuf[32];
    snprintf(lapBuf, sizeof(lapBuf), "L%d/%d DRS", currentLap, totalLaps);
    tft.drawString(lapBuf, 270, 12, 2);
    tft.drawFastHLine(0, 24, 320, C_MIDGREY);
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

// --- VIEW 1: TIMING LEADERBOARD ---
void renderLeaderboard() {
    tft.fillRect(0, 25, 320, 183, C_BLACK);

    // Columns: POS | DRV | GAP | TYRE | LAST LAP | S1 S2 S3
    tft.fillRect(0, 25, 320, 16, C_DARKGREY);
    tft.setTextColor(C_LIGHTGREY, C_DARKGREY);
    tft.setTextDatum(ML_DATUM);
    tft.drawString("POS", 6, 33, 1);
    tft.drawString("DRV", 34, 33, 1);
    tft.drawString("GAP", 74, 33, 1);
    tft.drawString("TYR", 130, 33, 1);
    tft.drawString("LAST LAP", 175, 33, 1);
    tft.drawString("SECTORS", 255, 33, 1);

    for (int i = 0; i < 6; i++) {
        int y = 46 + (i * 26);
        DriverTiming& d = grid[i];

        // Background alternating strip
        if (i % 2 == 1) {
            tft.fillRect(0, y - 2, 320, 24, 0x0841);
        }

        // Pos numeral
        tft.setTextDatum(ML_DATUM);
        tft.setTextColor(i == 0 ? C_GOLD : C_WHITE, (i % 2 == 1) ? 0x0841 : C_BLACK);
        tft.drawString(String(i + 1).c_str(), 8, y + 8, 2);

        // Driver code
        tft.drawString(d.code, 34, y + 8, 2);

        // Gap
        tft.setTextColor(C_CYAN, (i % 2 == 1) ? 0x0841 : C_BLACK);
        tft.drawString(d.gap, 74, y + 8, 2);

        // Tire badge
        tft.fillRect(130, y + 1, 20, 18, d.tireColor);
        tft.setTextColor(C_BLACK, d.tireColor);
        tft.setTextDatum(MC_DATUM);
        tft.drawString(d.tire, 140, y + 10, 2);

        // Last lap time
        tft.setTextDatum(ML_DATUM);
        tft.setTextColor(d.fastestLap ? C_PURPLE : C_WHITE, (i % 2 == 1) ? 0x0841 : C_BLACK);
        tft.drawString(d.lastLap, 175, y + 8, 2);

        // Sector boxes (S1, S2, S3)
        uint16_t s1 = (i == 4) ? C_PURPLE : C_GREEN;
        uint16_t s2 = (i == 0 || i == 4) ? C_PURPLE : C_GREEN;
        uint16_t s3 = (i == 1) ? C_YELLOW : C_GREEN;
        tft.fillRect(255, y + 4, 16, 12, s1);
        tft.fillRect(275, y + 4, 16, 12, s2);
        tft.fillRect(295, y + 4, 16, 12, s3);
    }
}

// --- VIEW 2: TIRE STRATEGY & DEGRADATION ---
void renderTireStrategy() {
    tft.fillRect(0, 25, 320, 183, C_BLACK);

    // Stint header
    tft.setTextColor(C_GOLD, C_BLACK);
    tft.setTextDatum(TL_DATUM);
    tft.drawString("STINT STRATEGY: MEDIUM COMPOUND", 12, 32, 2);
    char stintBuf[32];
    snprintf(stintBuf, sizeof(stintBuf), "Tire Age: %d Laps / Projected 32", stintLaps);
    tft.setTextColor(C_LIGHTGREY, C_BLACK);
    tft.drawString(stintBuf, 12, 50, 1);

    // Car silhouette & 4-corner tires
    int carCx = 90;
    int carCy = 125;
    tft.fillRect(carCx - 14, carCy - 40, 28, 80, C_DARKGREY); // Chassis
    tft.drawRect(carCx - 14, carCy - 40, 28, 80, C_WHITE);
    tft.fillRect(carCx - 8, carCy - 15, 16, 30, C_CYAN);     // Cockpit

    // 4 Tire Blocks: FL, FR, RL, RR
    int tx[4] = { carCx - 36, carCx + 18, carCx - 36, carCx + 18 };
    int ty[4] = { carCy - 38, carCy - 38, carCy + 20, carCy + 20 };
    const char* tLabels[4] = { "FL", "FR", "RL", "RR" };

    for (int i = 0; i < 4; i++) {
        uint16_t heatCol = (tireTemp[i] > 108) ? C_RED : C_YELLOW;
        tft.fillRect(tx[i], ty[i], 18, 22, heatCol);
        tft.drawRect(tx[i], ty[i], 18, 22, C_WHITE);
        tft.setTextColor(C_BLACK, heatCol);
        tft.setTextDatum(MC_DATUM);
        tft.drawString(tLabels[i], tx[i] + 9, ty[i] + 11, 1);
    }

    // Telemetry stats on the right (X: 160 to 310)
    tft.setTextColor(C_WHITE, C_BLACK);
    tft.setTextDatum(TL_DATUM);
    tft.drawString("TIRE THERMALS & WEAR", 160, 70, 2);

    for (int i = 0; i < 4; i++) {
        int y = 92 + (i * 18);
        char buf[32];
        snprintf(buf, sizeof(buf), "%s: %dC  Wear: %d%%", tLabels[i], tireTemp[i], tireWear[i]);
        tft.drawString(buf, 160, y, 1);

        // Small wear bar
        int barW = map(tireWear[i], 0, 100, 0, 45);
        tft.fillRect(265, y, 45, 8, C_DARKGREY);
        tft.fillRect(265, y, barW, 8, (tireWear[i] > 70) ? C_RED : C_GREEN);
    }

    // Box This Lap Strategy Callout Banner
    tft.fillRect(10, 172, 300, 28, C_RED);
    tft.drawRect(10, 172, 300, 28, C_WHITE);
    tft.setTextColor(C_WHITE, C_RED);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("BOX THIS LAP: DELTA -22.4s TO P4 (NOR)", 160, 186, 2);
}

// --- VIEW 3: WEATHER & TRACK CONDITIONS ---
void renderWeather() {
    tft.fillRect(0, 25, 320, 183, C_BLACK);

    tft.setTextColor(C_GOLD, C_BLACK);
    tft.setTextDatum(TL_DATUM);
    tft.drawString("CIRCUIT WEATHER & TRACK TELEMETRY", 12, 32, 2);

    // 4 Telemetry Cards (2x2 grid)
    // Card 1: Track Temp
    tft.fillRect(15, 55, 140, 55, C_DARKGREY);
    tft.drawRect(15, 55, 140, 55, C_MIDGREY);
    tft.setTextColor(C_LIGHTGREY, C_DARKGREY);
    tft.drawString("TRACK TEMP", 25, 62, 1);
    tft.setTextColor(C_ORANGE, C_DARKGREY);
    tft.drawString("38.4 C", 25, 78, 4);

    // Card 2: Air Temp
    tft.fillRect(165, 55, 140, 55, C_DARKGREY);
    tft.drawRect(165, 55, 140, 55, C_MIDGREY);
    tft.setTextColor(C_LIGHTGREY, C_DARKGREY);
    tft.drawString("AIR TEMP & HUMIDITY", 175, 62, 1);
    tft.setTextColor(C_CYAN, C_DARKGREY);
    tft.drawString("24.1 C", 175, 78, 4);

    // Card 3: Wind Vector
    tft.fillRect(15, 118, 140, 55, C_DARKGREY);
    tft.drawRect(15, 118, 140, 55, C_MIDGREY);
    tft.setTextColor(C_LIGHTGREY, C_DARKGREY);
    tft.drawString("WIND SPEED", 25, 125, 1);
    tft.setTextColor(C_WHITE, C_DARKGREY);
    tft.drawString("14 km/h", 25, 141, 4);

    // Card 4: Rain Radar
    tft.fillRect(165, 118, 140, 55, C_NAVY);
    tft.drawRect(165, 118, 140, 55, C_CYAN);
    tft.setTextColor(C_CYAN, C_NAVY);
    tft.drawString("PRECIPITATION FORECAST", 175, 125, 1);
    tft.setTextColor(C_YELLOW, C_NAVY);
    tft.drawString("RAIN IN 12 MIN", 175, 145, 2);

    // Track grip status
    tft.setTextColor(C_GREEN, C_BLACK);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("Track Status: DRY / HIGH RUBBER EVOLUTION", 160, 190, 2);
}

// --- VIEW 4: RACE CONTROL & FLAG SIMULATOR ---
void renderRaceControl() {
    tft.fillRect(0, 25, 320, 183, C_BLACK);

    tft.setTextColor(C_GOLD, C_BLACK);
    tft.setTextDatum(TL_DATUM);
    tft.drawString("FIA RACE CONTROL SIMULATOR", 12, 32, 2);

    tft.setTextColor(C_LIGHTGREY, C_BLACK);
    tft.drawString("Tap flag button to trigger simulated session condition:", 12, 50, 1);

    // 4 Flag Buttons
    // 1. Green Flag
    tft.fillRect(15, 70, 140, 48, C_GREEN);
    tft.drawRect(15, 70, 140, 48, C_WHITE);
    tft.setTextColor(C_BLACK, C_GREEN);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("GREEN FLAG", 85, 94, 2);

    // 2. VSC Flag
    tft.fillRect(165, 70, 140, 48, C_YELLOW);
    tft.drawRect(165, 70, 140, 48, C_BLACK);
    tft.setTextColor(C_BLACK, C_YELLOW);
    tft.drawString("VSC (VIRTUAL SC)", 235, 94, 2);

    // 3. Full Safety Car
    tft.fillRect(15, 126, 140, 48, C_ORANGE);
    tft.drawRect(15, 126, 140, 48, C_WHITE);
    tft.setTextColor(C_BLACK, C_ORANGE);
    tft.drawString("SAFETY CAR (SC)", 85, 150, 2);

    // 4. Red Flag
    tft.fillRect(165, 126, 140, 48, C_RED);
    tft.drawRect(165, 126, 140, 48, C_WHITE);
    tft.setTextColor(C_WHITE, C_RED);
    tft.drawString("RED FLAG (STOP)", 235, 150, 2);

    // Status description
    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(C_CYAN, C_BLACK);
    tft.drawString("Simulates FIA Race Director telemetry chimes & LEDs", 160, 192, 1);
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
            playRadioChime();
            renderHeader();
            renderNavTabs();
            switch (currentView) {
                case VIEW_LEADERBOARD: renderLeaderboard(); break;
                case VIEW_TIRE_STRATEGY: renderTireStrategy(); break;
                case VIEW_WEATHER: renderWeather(); break;
                case VIEW_RACE_CONTROL: renderRaceControl(); break;
            }
            delay(150);
            return;
        }
    }

    // If on Race Control View, check flag buttons
    if (currentView == VIEW_RACE_CONTROL && ty >= 70 && ty <= 175) {
        if (tx >= 15 && tx <= 155 && ty <= 118) {
            // Green Flag
            raceFlag = FLAG_GREEN;
            setRgbLed(0, 255, 0);
            tone(BUZZER_PIN, 1400, 100);
        } else if (tx >= 165 && tx <= 305 && ty <= 118) {
            // VSC
            raceFlag = FLAG_VSC;
            setRgbLed(255, 200, 0);
            tone(BUZZER_PIN, 900, 150);
        } else if (tx >= 15 && tx <= 155 && ty >= 126) {
            // Safety Car
            raceFlag = FLAG_SAFETY_CAR;
            setRgbLed(255, 120, 0);
            playSiren();
        } else if (tx >= 165 && tx <= 305 && ty >= 126) {
            // Red Flag
            raceFlag = FLAG_RED;
            setRgbLed(255, 0, 0);
            playSiren();
        }
        renderHeader();
        delay(180);
    }
}

// --- SETUP ---
void setup() {
    Serial.begin(115200);

    // Initialize RGB LED
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

    // Boot sequence
    showBootSequence();

    // Initial render
    renderHeader();
    renderNavTabs();
    renderLeaderboard();

    setRgbLed(0, 200, 0); // Green flag normal
    lastUpdate = millis();
}

// --- MAIN LOOP ---
void loop() {
    // Touch input handling
    handleTouch();

    // Flag flashing for Yellow / Red / SC
    if (raceFlag != FLAG_GREEN) {
        if (millis() - lastFlagBlink >= 400) {
            lastFlagBlink = millis();
            flagBlinkState = !flagBlinkState;
            if (flagBlinkState) {
                if (raceFlag == FLAG_VSC) setRgbLed(255, 200, 0);
                else if (raceFlag == FLAG_SAFETY_CAR) setRgbLed(255, 100, 0);
                else if (raceFlag == FLAG_RED) setRgbLed(255, 0, 0);
            } else {
                setRgbLed(0, 0, 0);
            }
        }
    } else {
        setRgbLed(0, 200, 0);
    }

    // Telemetry subtle drift / dynamic lap simulation every 4 seconds
    if (millis() - lastUpdate >= 4000) {
        lastUpdate = millis();
        // Dynamic tire wear & heat tick
        tireTemp[random(0, 4)] = 104 + random(-2, 8);
        if (currentView == VIEW_TIRE_STRATEGY) {
            renderTireStrategy();
        }
    }

    delay(20);
}

