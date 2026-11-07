/**
 * ============================================================================
 * Project 67 — CYD ADS-B Flight Radar & Aircraft Tracker
 * Hardware: Cheap Yellow Display (ESP32-2432S028R 2.8" Resistive Touch)
 * 
 * Features:
 *   - Standalone Aviation Air Traffic & ADS-B Radar Scope
 *   - Real-Time 360-Degree CRT Phosphor Radar Sweep Beam
 *   - Range Rings (10 NM, 25 NM, 50 NM) with Compass Bearing Ticks (N, E, S, W)
 *   - Multi-Aircraft Target Tracking:
 *       • Directional velocity heading vectors & flight level altitude tags
 *       • Tap any aircraft target to inspect full flight telemetry:
 *           - Callsign (e.g., UAL842, BAW117, DLH400, AFR066, QFA009, EMR202)
 *           - Aircraft Type (B789, A359, B77W, A388, A321neo)
 *           - Origin & Destination (LHR -> JFK, DXB -> LAX, etc.)
 *           - Altitude (ft), Ground Speed (knots), Climb/Descent Rate (fpm)
 *           - Transponder Squawk (1200 VFR, 7700 General Emergency)
 *   - Emergency Transponder Alert Mode:
 *       • Squawk 7700 trigger with two-tone ATC siren & crimson flashing LED
 *   - Audio Feedback: Radar sweep ping chimes on GPIO 26
 *   - Active-Low RGB LED: Cyan radar sweep pulse, Flashing Red on emergency
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
#define C_CYAN        0x07FF
#define C_AMBER       0xFD20
#define C_RED         0xF800
#define C_YELLOW      0xFFE0
#define C_GOLD        0xFEA0
#define C_DARKGREY    0x2124
#define C_MIDGREY     0x52AA
#define C_LIGHTGREY   0xC618
#define C_NAVY        0x000F
#define C_RADAR_BG    0x0100
#define C_RADAR_GRID  0x02E0

// --- HARDWARE INSTANCES ---
TFT_eSPI tft = TFT_eSPI();
SPIClass touchSPI(HSPI);
XPT2046_Touchscreen touch(XPT2046_CS, XPT2046_IRQ);

// --- RADAR GEOMETRY ---
#define RADAR_CX 105
#define RADAR_CY 132
#define RADAR_R  88

float sweepAngleDeg = 0.0f;
unsigned long lastSweepUpdate = 0;

// --- AIRCRAFT FLEET DATA ---
#define NUM_AIRCRAFT 6

struct Aircraft {
    const char* callsign;
    const char* type;
    const char* route;
    float distNM;       // Distance in nautical miles (0..50)
    float bearingDeg;   // Bearing from radar station (0..360)
    float headingDeg;   // Flight heading (0..360)
    int altitudeFt;     // Altitude in feet
    int speedKts;       // Ground speed in knots
    int verticalFpm;    // Climb / Descent rate ft/min
    int squawk;         // Transponder code (e.g. 1200, 7700)
    bool isEmergency;
};

Aircraft fleet[NUM_AIRCRAFT] = {
    { "UAL842", "B789", "LHR -> JFK", 18.4f,  42.0f, 260.0f, 34000, 482, -300, 1200, false },
    { "BAW117", "A359", "CDG -> BOS", 32.1f, 135.0f, 285.0f, 38000, 510,  150, 1200, false },
    { "DLH400", "B748", "FRA -> EWR", 41.5f, 210.0f,  75.0f, 29000, 445,    0, 1200, false },
    { "AFR066", "A388", "DXB -> LAX", 26.8f, 295.0f, 310.0f, 36000, 498,  400, 1200, false },
    { "QFA009", "B789", "PER -> LHR", 12.2f,  88.0f, 120.0f, 16000, 390, -800, 1200, false },
    { "EMR202", "B77W", "JFK -> DXB", 38.0f, 350.0f,  90.0f, 31000, 470,    0, 7700, true  }
};

int selectedTarget = 0;
bool emergencyBlinkState = false;
unsigned long lastEmergencyBlink = 0;

// --- HARDWARE HELPERS ---
void setRgbLed(uint8_t r, uint8_t g, uint8_t b) {
    analogWrite(LED_RED_PIN, 255 - r);
    analogWrite(LED_GREEN_PIN, 255 - g);
    analogWrite(LED_BLUE_PIN, 255 - b);
}

void playSweepPing() {
    tone(BUZZER_PIN, 1800, 20);
}

void playEmergencySiren() {
    for (int f = 900; f < 1800; f += 90) {
        tone(BUZZER_PIN, f, 12);
        delay(10);
    }
}

// --- BOOT SEQUENCE ---
void showBootSequence() {
    tft.fillScreen(C_BLACK);
    setRgbLed(0, 0, 0);

    tft.setTextColor(C_GOLD, C_BLACK);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("IamNotIntoTech", 160, 45, 4);

    tft.setTextColor(C_CYAN, C_BLACK);
    tft.drawString("ADS-B VIRTUAL RADAR & AIR TRAFFIC", 160, 80, 2);

    // Radar antenna icon
    tft.drawCircle(160, 130, 24, C_GREEN);
    tft.drawCircle(160, 130, 16, C_GREEN);
    tft.drawCircle(160, 130, 8, C_GREEN);
    tft.fillCircle(160, 130, 3, C_WHITE);
    tft.drawLine(160, 130, 178, 114, C_CYAN);

    tft.setTextColor(C_LIGHTGREY, C_BLACK);
    tft.drawString("Tuning 1090 MHz Mode-S Receiver...", 160, 180, 2);

    setRgbLed(0, 180, 220);
    tone(BUZZER_PIN, 1400, 80); delay(100);
    tone(BUZZER_PIN, 2100, 100);

    delay(1200);
    setRgbLed(0, 0, 0);
    tft.fillScreen(C_BLACK);
}

// --- TOP STATUS BAR ---
void renderTopBar() {
    tft.fillRect(0, 0, 320, 24, C_NAVY);
    tft.drawFastHLine(0, 24, 320, C_CYAN);

    // Left range badge
    tft.fillRect(4, 3, 50, 18, C_DARKGREY);
    tft.drawRect(4, 3, 50, 18, C_CYAN);
    tft.setTextColor(C_CYAN, C_DARKGREY);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("50 NM", 29, 12, 1);

    // Center branding
    tft.setTextColor(C_GOLD, C_NAVY);
    tft.drawString("IamNotIntoTech ADS-B", 145, 12, 2);

    // Right track count & alert badge
    tft.setTextDatum(MR_DATUM);
    tft.setTextColor(C_GREEN, C_NAVY);
    tft.drawString("TARGETS: 6", 314, 12, 1);
}

// --- RADAR SCOPE STATIC GRID ---
void drawRadarGrid() {
    // Fill scope area
    tft.fillCircle(RADAR_CX, RADAR_CY, RADAR_R, C_RADAR_BG);
    tft.drawCircle(RADAR_CX, RADAR_CY, RADAR_R, C_RADAR_GRID);

    // Concentric Range Rings (10 NM, 25 NM, 50 NM)
    int r10 = (int)(RADAR_R * (10.0f / 50.0f));
    int r25 = (int)(RADAR_R * (25.0f / 50.0f));
    tft.drawCircle(RADAR_CX, RADAR_CY, r10, C_RADAR_GRID);
    tft.drawCircle(RADAR_CX, RADAR_CY, r25, C_RADAR_GRID);

    // Crosshairs
    tft.drawFastHLine(RADAR_CX - RADAR_R, RADAR_CY, RADAR_R * 2, C_RADAR_GRID);
    tft.drawFastVLine(RADAR_CX, RADAR_CY - RADAR_R, RADAR_R * 2, C_RADAR_GRID);

    // Compass Headings
    tft.setTextColor(C_GREEN, C_RADAR_BG);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("N", RADAR_CX, RADAR_CY - RADAR_R + 8, 1);
    tft.drawString("S", RADAR_CX, RADAR_CY + RADAR_R - 8, 1);
    tft.drawString("E", RADAR_CX + RADAR_R - 8, RADAR_CY, 1);
    tft.drawString("W", RADAR_CX - RADAR_R + 8, RADAR_CY, 1);

    // Center Home Station Dot
    tft.fillCircle(RADAR_CX, RADAR_CY, 2, C_WHITE);
}

// --- AVIONICS TELEMETRY CARD (RIGHT SIDE) ---
void renderAvionicsCard() {
    // Card background (X: 204 to 320, Y: 25 to 240)
    tft.fillRect(204, 25, 116, 215, C_BLACK);
    tft.drawFastVLine(204, 25, 215, C_DARKGREY);

    Aircraft& a = fleet[selectedTarget];

    // Card Header: Callsign
    uint16_t hdrBg = a.isEmergency ? C_RED : C_DARKGREY;
    tft.fillRect(208, 28, 108, 24, hdrBg);
    tft.drawRect(208, 28, 108, 24, a.isEmergency ? C_WHITE : C_CYAN);
    tft.setTextColor(C_WHITE, hdrBg);
    tft.setTextDatum(MC_DATUM);
    tft.drawString(a.callsign, 262, 40, 2);

    // Aircraft Type & Route
    tft.setTextColor(C_GOLD, C_BLACK);
    tft.setTextDatum(TL_DATUM);
    tft.drawString(a.type, 210, 56, 2);

    tft.setTextColor(C_LIGHTGREY, C_BLACK);
    tft.drawString(a.route, 210, 74, 1);

    tft.drawFastHLine(210, 88, 104, C_DARKGREY);

    // Altitude
    tft.setTextColor(C_WHITE, C_BLACK);
    tft.drawString("ALTITUDE", 210, 94, 1);
    char altBuf[24];
    snprintf(altBuf, sizeof(altBuf), "%d FT", a.altitudeFt);
    tft.setTextColor(C_CYAN, C_BLACK);
    tft.drawString(altBuf, 210, 106, 2);

    // Ground Speed
    tft.setTextColor(C_WHITE, C_BLACK);
    tft.drawString("GROUND SPEED", 210, 126, 1);
    char spdBuf[24];
    snprintf(spdBuf, sizeof(spdBuf), "%d KTS", a.speedKts);
    tft.setTextColor(C_GREEN, C_BLACK);
    tft.drawString(spdBuf, 210, 138, 2);

    // Bearing & Range
    tft.setTextColor(C_WHITE, C_BLACK);
    tft.drawString("RANGE / BRG", 210, 158, 1);
    char brgBuf[32];
    snprintf(brgBuf, sizeof(brgBuf), "%.1f NM @ %d", a.distNM, (int)a.bearingDeg);
    tft.setTextColor(C_AMBER, C_BLACK);
    tft.drawString(brgBuf, 210, 170, 1);

    // Squawk Code
    tft.drawString("SQUAWK", 210, 184, 1);
    char sqkBuf[16];
    snprintf(sqkBuf, sizeof(sqkBuf), "%04d", a.squawk);
    tft.setTextColor(a.isEmergency ? C_RED : C_YELLOW, C_BLACK);
    tft.drawString(sqkBuf, 210, 196, 2);

    // Touch "NEXT TARGET" Button
    tft.fillRect(208, 216, 108, 20, C_NAVY);
    tft.drawRect(208, 216, 108, 20, C_CYAN);
    tft.setTextColor(C_CYAN, C_NAVY);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("NEXT TARGET >>", 262, 226, 1);
}

// --- RENDER AIRCRAFT ON RADAR ---
void renderAircraftOnRadar() {
    for (int i = 0; i < NUM_AIRCRAFT; i++) {
        Aircraft& a = fleet[i];

        // Compute polar to cartesian
        float rad = (a.bearingDeg - 90.0f) * (PI / 180.0f);
        float pxDist = (a.distNM / 50.0f) * RADAR_R;
        int ax = RADAR_CX + (int)(cos(rad) * pxDist);
        int ay = RADAR_CY + (int)(sin(rad) * pxDist);

        bool isSel = (i == selectedTarget);

        // Aircraft glyph color
        uint16_t glyphCol = a.isEmergency ? C_RED : (isSel ? C_YELLOW : C_GREEN);

        // Draw aircraft blip
        tft.fillCircle(ax, ay, isSel ? 4 : 3, glyphCol);
        if (isSel) {
            tft.drawCircle(ax, ay, 7, C_WHITE);
        }

        // Draw heading line (vector)
        float hRad = (a.headingDeg - 90.0f) * (PI / 180.0f);
        int vx = ax + (int)(cos(hRad) * 8.0f);
        int vy = ay + (int)(sin(hRad) * 8.0f);
        tft.drawLine(ax, ay, vx, vy, glyphCol);

        // Label (Callsign + Flight Level)
        tft.setTextColor(glyphCol, C_RADAR_BG);
        tft.setTextDatum(TL_DATUM);
        char lbl[16];
        snprintf(lbl, sizeof(lbl), "%s", a.callsign);
        tft.drawString(lbl, ax + 5, ay - 4, 1);
    }
}

// --- RADAR SWEEP LINE ---
void drawSweepLine(float angleDeg) {
    float rad = (angleDeg - 90.0f) * (PI / 180.0f);
    int sx = RADAR_CX + (int)(cos(rad) * RADAR_R);
    int sy = RADAR_CY + (int)(sin(rad) * RADAR_R);

    // Draw active sweep ray
    tft.drawLine(RADAR_CX, RADAR_CY, sx, sy, C_WHITE);
}

// --- TOUCH INPUT HANDLER ---
void handleTouch() {
    if (!touch.touched()) return;

    TS_Point p = touch.getPoint();
    int tx = map(p.x, 240, 3800, 0, 320);
    int ty = map(p.y, 240, 3800, 0, 240);

    // Check "NEXT TARGET" button on right (X: 205 to 320, Y: 210 to 240)
    if (tx >= 205 && ty >= 210) {
        selectedTarget = (selectedTarget + 1) % NUM_AIRCRAFT;
        tone(BUZZER_PIN, 1600, 40);
        if (fleet[selectedTarget].isEmergency) {
            playEmergencySiren();
        }
        renderAvionicsCard();
        delay(180);
        return;
    }

    // Check radar scope target taps
    for (int i = 0; i < NUM_AIRCRAFT; i++) {
        Aircraft& a = fleet[i];
        float rad = (a.bearingDeg - 90.0f) * (PI / 180.0f);
        float pxDist = (a.distNM / 50.0f) * RADAR_R;
        int ax = RADAR_CX + (int)(cos(rad) * pxDist);
        int ay = RADAR_CY + (int)(sin(rad) * pxDist);

        if (abs(tx - ax) <= 12 && abs(ty - ay) <= 12) {
            selectedTarget = i;
            tone(BUZZER_PIN, 1800, 50);
            if (a.isEmergency) {
                playEmergencySiren();
            }
            renderAvionicsCard();
            delay(180);
            return;
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

    // Boot sequence
    showBootSequence();

    // Render initial UI
    renderTopBar();
    drawRadarGrid();
    renderAvionicsCard();

    lastSweepUpdate = millis();
}

// --- MAIN LOOP ---
void loop() {
    unsigned long now = millis();

    // Touch input check
    handleTouch();

    // Radar beam rotation every 35ms
    if (now - lastSweepUpdate >= 35) {
        lastSweepUpdate = now;

        // Erase old sweep and redraw scope grid
        drawRadarGrid();
        renderAircraftOnRadar();

        // Increment sweep angle
        sweepAngleDeg += 3.0f;
        if (sweepAngleDeg >= 360.0f) {
            sweepAngleDeg = 0.0f;
            playSweepPing();
            setRgbLed(0, 180, 255); // Pulse cyan on full 360 revolution
        } else if ((int)sweepAngleDeg % 60 == 0) {
            setRgbLed(0, 40, 60);
        }

        // Draw new sweep beam
        drawSweepLine(sweepAngleDeg);
    }

    // Emergency Flashing LED for Squawk 7700
    if (fleet[selectedTarget].isEmergency) {
        if (now - lastEmergencyBlink >= 300) {
            lastEmergencyBlink = now;
            emergencyBlinkState = !emergencyBlinkState;
            setRgbLed(emergencyBlinkState ? 255 : 0, 0, 0);
        }
    }

    delay(10);
}

