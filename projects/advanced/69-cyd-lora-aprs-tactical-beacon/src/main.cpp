/**
 * ============================================================================
 * Project 69 — CYD LoRa APRS Tactical Position Tracker & Beacon
 * Hardware: Cheap Yellow Display (ESP32-2432S028R 2.8" Resistive Touch)
 * 
 * Features:
 *   - Tactical Amateur Radio APRS (Automatic Packet Reporting System) Terminal
 *   - 4 Touch-Navigated Tactical Views:
 *       1. TACTICAL RADAR GRID:
 *          • Polar range rings with Maidenhead Grid Locator (FN31pr)
 *          • Local GPS telemetry: Latitude, Longitude, Altitude, Speed & Course
 *          • Nearby APRS stations plotted with symbols ([Car], [Base], [Hiker], [Balloon])
 *       2. PACKET SNIFFER LOG:
 *          • Decoded AX.25 / APRS raw packet monitor with timestamps & RSSI (-84 dBm)
 *       3. BEACON CONFIG DECK:
 *          • My Callsign (N0TECH-9), SSID selector, Symbol icons, and Beacon rate
 *       4. EMERGENCY SOS BEACON:
 *          • High-priority emergency beacon trigger with siren & crimson strobe
 *   - Acoustic Bell 202 AFSK Sound Synthesis:
 *       • Simulates authentic 1200 Hz mark / 2200 Hz space tones on GPIO 26
 *   - Active-Low RGB LED: Blue on TX, Green on RX, Flashing Red on SOS
 *   - LDR Ambient Light Sensor on GPIO 34
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
#define C_TAB_BG      0x18C3
#define C_GRID_LINE   0x02E0

// --- HARDWARE INSTANCES ---
TFT_eSPI tft = TFT_eSPI();
SPIClass touchSPI(HSPI);
XPT2046_Touchscreen touch(XPT2046_CS, XPT2046_IRQ);

// --- DASHBOARD STATE ---
enum AprsView {
    VIEW_MAP_GRID  = 0,
    VIEW_PACKETS   = 1,
    VIEW_CONFIG    = 2,
    VIEW_SOS       = 3,
    VIEW_COUNT     = 4
};

const char* VIEW_NAMES[VIEW_COUNT] = {
    "TACTICAL", "PACKETS", "CONFIG", "SOS"
};

int currentView = VIEW_MAP_GRID;

// My Station Telemetry
const char* myCallsign = "N0TECH-9";
const char* myGrid = "FN31pr";
float myLat = 40.7128f;
float myLon = -74.0060f;
int myAltM = 142;
int mySpeedKmh = 28;
int myCourseDeg = 45;

int beaconIntervalSec = 60;
unsigned long lastBeaconTx = 0;
bool sosActive = false;
bool sosStrobeState = false;
unsigned long lastSosStrobe = 0;

// Nearby Stations
struct AprsNode {
    const char* callsign;
    const char* symbol;
    float distKm;
    float bearingDeg;
    int rssi;
};

#define NUM_NODES 4
AprsNode nearbyNodes[NUM_NODES] = {
    { "W2XYZ-7",  "[CAR]",     4.2f,  35.0f, -74 },
    { "K3ABC-1",  "[BASE]",    8.5f, 140.0f, -88 },
    { "N1CAL-11", "[BALLOON]", 16.8f, 260.0f, -95 },
    { "KD2DEF",   "[HIKER]",   2.1f, 310.0f, -68 }
};

// Packet Log Buffer
#define MAX_LOG 6
struct PacketEntry {
    const char* src;
    const char* dst;
    const char* info;
    int rssi;
};

PacketEntry packetLog[MAX_LOG] = {
    { "W2XYZ-7", "APRS,WIDE1", "En route to summit camp", -74 },
    { "K3ABC-1", "APRS,WIDE2", "Repeater 144.390 Active", -88 },
    { "KD2DEF",  "APRS,WIDE1", "Trail checkpoint #3 OK",   -68 },
    { "N1CAL-11","APRS,WIDE2", "Alt: 28400m Batt: 3.9V",   -95 },
    { "W2XYZ-7", "APRS,WIDE1", "Speed 45kmh Heading 030", -76 },
    { "N0TECH-9","APRS,WIDE1", "IamNotIntoTech CYD Beacon",-50 }
};

// --- HARDWARE HELPERS ---
void setRgbLed(uint8_t r, uint8_t g, uint8_t b) {
    analogWrite(LED_RED_PIN, 255 - r);
    analogWrite(LED_GREEN_PIN, 255 - g);
    analogWrite(LED_BLUE_PIN, 255 - b);
}

// AX.25 Bell 202 AFSK audio simulation (1200 / 2200 Hz tones)
void playAfskBurst() {
    int tones[] = { 1200, 2200, 1200, 1200, 2200, 1200, 2200, 2200 };
    for (int i = 0; i < 8; i++) {
        tone(BUZZER_PIN, tones[i], 18);
        delay(22);
    }
}

// --- BOOT SEQUENCE ---
void showBootSequence() {
    tft.fillScreen(C_BLACK);
    setRgbLed(0, 0, 0);

    tft.setTextColor(C_GOLD, C_BLACK);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("IamNotIntoTech", 160, 45, 4);

    tft.setTextColor(C_GREEN, C_BLACK);
    tft.drawString("LoRa APRS TACTICAL POSITION BEACON", 160, 80, 2);

    // Radio antenna icon
    tft.drawCircle(160, 128, 22, C_GREEN);
    tft.drawCircle(160, 128, 14, C_CYAN);
    tft.drawFastVLine(160, 115, 30, C_WHITE);
    tft.fillCircle(160, 115, 3, C_GOLD);

    tft.setTextColor(C_LIGHTGREY, C_BLACK);
    tft.drawString("Initializing AX.25 144.390 MHz Radio...", 160, 185, 2);

    setRgbLed(0, 200, 255);
    playAfskBurst();

    delay(1200);
    setRgbLed(0, 0, 0);
    tft.fillScreen(C_BLACK);
}

// --- TOP HEADER ---
void renderHeader() {
    tft.fillRect(0, 0, 320, 24, C_NAVY);
    tft.drawFastHLine(0, 24, 320, C_GREEN);

    // Callsign badge
    tft.fillRect(4, 3, 76, 18, C_DARKGREY);
    tft.drawRect(4, 3, 76, 18, C_GREEN);
    tft.setTextColor(C_GREEN, C_DARKGREY);
    tft.setTextDatum(MC_DATUM);
    tft.drawString(myCallsign, 42, 12, 1);

    // Center branding
    tft.setTextColor(C_GOLD, C_NAVY);
    tft.drawString("IamNotIntoTech APRS", 160, 12, 2);

    // Grid locator
    tft.setTextDatum(MR_DATUM);
    tft.setTextColor(C_CYAN, C_NAVY);
    char gBuf[16];
    snprintf(gBuf, sizeof(gBuf), "GRID: %s", myGrid);
    tft.drawString(gBuf, 314, 12, 1);
}

// --- BOTTOM NAVIGATION TABS ---
void renderNavTabs() {
    tft.drawFastHLine(0, 208, 320, C_MIDGREY);
    for (int i = 0; i < VIEW_COUNT; i++) {
        int x = i * 80;
        bool active = (i == currentView);
        uint16_t bg = active ? (i == VIEW_SOS ? C_RED : C_GREEN) : C_TAB_BG;
        uint16_t textCol = active ? C_BLACK : C_LIGHTGREY;
        if (active && i == VIEW_SOS) textCol = C_WHITE;

        tft.fillRect(x, 209, 80, 31, bg);
        tft.drawRect(x, 209, 80, 31, C_MIDGREY);
        tft.setTextDatum(MC_DATUM);
        tft.setTextColor(textCol, bg);
        tft.drawString(VIEW_NAMES[i], x + 40, 224, 2);
    }
}

// --- VIEW 1: TACTICAL RADAR GRID ---
void renderTacticalGrid() {
    tft.fillRect(0, 25, 320, 183, C_BLACK);

    // Left Polar Radar Scope (Center X: 100, Y: 118, Radius 76)
    int cx = 100, cy = 118, r = 76;
    tft.fillCircle(cx, cy, r, 0x0100);
    tft.drawCircle(cx, cy, r, C_GRID_LINE);
    tft.drawCircle(cx, cy, r / 2, C_GRID_LINE);
    tft.drawFastHLine(cx - r, cy, r * 2, C_GRID_LINE);
    tft.drawFastVLine(cx, cy - r, r * 2, C_GRID_LINE);

    // Cardinal bearings
    tft.setTextColor(C_GREEN, 0x0100);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("N", cx, cy - r + 7, 1);
    tft.drawString("S", cx, cy + r - 7, 1);
    tft.drawString("E", cx + r - 7, cy, 1);
    tft.drawString("W", cx - r + 7, cy, 1);

    // My station icon at center
    tft.fillCircle(cx, cy, 3, C_CYAN);
    tft.drawCircle(cx, cy, 6, C_WHITE);

    // Plot nearby APRS nodes
    for (int i = 0; i < NUM_NODES; i++) {
        AprsNode& n = nearbyNodes[i];
        float rad = (n.bearingDeg - 90.0f) * (PI / 180.0f);
        float px = (n.distKm / 20.0f) * r;
        if (px > r - 6) px = r - 6;
        int nx = cx + (int)(cos(rad) * px);
        int ny = cy + (int)(sin(rad) * px);

        tft.fillCircle(nx, ny, 3, C_YELLOW);
        tft.setTextColor(C_YELLOW, 0x0100);
        tft.setTextDatum(TL_DATUM);
        tft.drawString(n.callsign, nx + 4, ny - 4, 1);
    }

    // Right Telemetry Box (X: 190 to 315)
    tft.setTextColor(C_GOLD, C_BLACK);
    tft.setTextDatum(TL_DATUM);
    tft.drawString("GPS TELEMETRY", 190, 32, 2);

    tft.setTextColor(C_WHITE, C_BLACK);
    tft.drawString("LATITUDE", 190, 52, 1);
    tft.setTextColor(C_CYAN, C_BLACK);
    tft.drawString("40 42.82' N", 190, 64, 2);

    tft.setTextColor(C_WHITE, C_BLACK);
    tft.drawString("LONGITUDE", 190, 84, 1);
    tft.setTextColor(C_CYAN, C_BLACK);
    tft.drawString("074 00.41' W", 190, 96, 2);

    tft.setTextColor(C_WHITE, C_BLACK);
    tft.drawString("ALT / SPEED", 190, 116, 1);
    char spdBuf[32];
    snprintf(spdBuf, sizeof(spdBuf), "%dm / %dkmh", myAltM, mySpeedKmh);
    tft.setTextColor(C_GREEN, C_BLACK);
    tft.drawString(spdBuf, 190, 128, 2);

    // 1-Tap Force Beacon Button (X: 190 to 310, Y: 155 to 195)
    tft.fillRect(190, 155, 122, 34, C_DARKGREY);
    tft.drawRect(190, 155, 122, 34, C_CYAN);
    tft.setTextColor(C_CYAN, C_DARKGREY);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("SEND BEACON", 251, 172, 2);
}

// --- VIEW 2: PACKET SNIFFER LOG ---
void renderPacketLog() {
    tft.fillRect(0, 25, 320, 183, C_BLACK);

    tft.setTextColor(C_GOLD, C_BLACK);
    tft.setTextDatum(TL_DATUM);
    tft.drawString("AX.25 APRS PACKET MONITOR", 10, 32, 2);

    for (int i = 0; i < MAX_LOG; i++) {
        int y = 52 + (i * 24);
        PacketEntry& p = packetLog[i];

        // Draw packet entry strip
        tft.fillRect(8, y, 304, 21, (i % 2 == 0) ? 0x0841 : C_BLACK);
        tft.drawRect(8, y, 304, 21, C_DARKGREY);

        // Callsign & RSSI
        tft.setTextColor(C_GREEN, (i % 2 == 0) ? 0x0841 : C_BLACK);
        tft.setTextDatum(ML_DATUM);
        tft.drawString(p.src, 14, y + 10, 1);

        tft.setTextColor(C_AMBER, (i % 2 == 0) ? 0x0841 : C_BLACK);
        tft.drawString(p.dst, 75, y + 10, 1);

        tft.setTextColor(C_WHITE, (i % 2 == 0) ? 0x0841 : C_BLACK);
        tft.drawString(p.info, 140, y + 10, 1);

        tft.setTextColor(C_CYAN, (i % 2 == 0) ? 0x0841 : C_BLACK);
        tft.setTextDatum(MR_DATUM);
        char rssiBuf[16];
        snprintf(rssiBuf, sizeof(rssiBuf), "%ddBm", p.rssi);
        tft.drawString(rssiBuf, 306, y + 10, 1);
    }
}

// --- VIEW 3: BEACON CONFIG DECK ---
void renderConfigDeck() {
    tft.fillRect(0, 25, 320, 183, C_BLACK);

    tft.setTextColor(C_GOLD, C_BLACK);
    tft.setTextDatum(TL_DATUM);
    tft.drawString("STATION BEACON CONFIGURATION", 10, 32, 2);

    // Setting 1: Station Callsign & SSID
    tft.fillRect(10, 56, 300, 36, C_DARKGREY);
    tft.drawRect(10, 56, 300, 36, C_MIDGREY);
    tft.setTextColor(C_LIGHTGREY, C_DARKGREY);
    tft.drawString("STATION CALLSIGN & SSID", 18, 62, 1);
    tft.setTextColor(C_GREEN, C_DARKGREY);
    tft.drawString(myCallsign, 18, 76, 2);

    // Setting 2: Beacon Interval
    tft.fillRect(10, 100, 300, 36, C_DARKGREY);
    tft.drawRect(10, 100, 300, 36, C_MIDGREY);
    tft.setTextColor(C_LIGHTGREY, C_DARKGREY);
    tft.drawString("BEACON BROADCAST RATE", 18, 106, 1);
    char intBuf[32];
    snprintf(intBuf, sizeof(intBuf), "Every %d Seconds (SmartBeaconing)", beaconIntervalSec);
    tft.setTextColor(C_CYAN, C_DARKGREY);
    tft.drawString(intBuf, 18, 120, 2);

    // Setting 3: Transmit Power
    tft.fillRect(10, 144, 300, 36, C_DARKGREY);
    tft.drawRect(10, 144, 300, 36, C_MIDGREY);
    tft.setTextColor(C_LIGHTGREY, C_DARKGREY);
    tft.drawString("TX POWER / FREQUENCY", 18, 150, 1);
    tft.setTextColor(C_GOLD, C_DARKGREY);
    tft.drawString("100mW (20 dBm) / 144.390 MHz", 18, 164, 2);
}

// --- VIEW 4: EMERGENCY SOS BEACON ---
void renderSosView() {
    tft.fillRect(0, 25, 320, 183, C_BLACK);

    tft.fillRect(15, 35, 290, 40, C_RED);
    tft.drawRect(15, 35, 290, 40, C_WHITE);
    tft.setTextColor(C_WHITE, C_RED);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("APRS EMERGENCY DISTRESS SYSTEM", 160, 55, 2);

    tft.setTextColor(C_WHITE, C_BLACK);
    tft.drawString("Broadcasts high-priority emergency beacon frame", 160, 88, 1);
    tft.drawString("with GPS coordinates to all digipeaters & satellites", 160, 102, 1);

    // Big Action Button
    uint16_t btnBg = sosActive ? C_YELLOW : C_RED;
    tft.fillRect(40, 120, 240, 60, btnBg);
    tft.drawRect(40, 120, 240, 60, C_WHITE);
    tft.setTextColor(sosActive ? C_BLACK : C_WHITE, btnBg);
    tft.drawString(sosActive ? "DEACTIVATE SOS" : "! TRANSMIT SOS BEACON !", 160, 150, 4);
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
                case VIEW_MAP_GRID: renderTacticalGrid(); break;
                case VIEW_PACKETS:  renderPacketLog(); break;
                case VIEW_CONFIG:   renderConfigDeck(); break;
                case VIEW_SOS:      renderSosView(); break;
            }
            delay(150);
            return;
        }
    }

    // Force Send Beacon on Tactical View (X: 190 to 310, Y: 155 to 195)
    if (currentView == VIEW_MAP_GRID && tx >= 190 && ty >= 155 && ty <= 195) {
        setRgbLed(0, 0, 255); // Blue TX
        playAfskBurst();
        setRgbLed(0, 0, 0);
        delay(180);
        return;
    }

    // SOS Toggle Button (X: 40 to 280, Y: 120 to 180)
    if (currentView == VIEW_SOS && tx >= 40 && tx <= 280 && ty >= 120 && ty <= 180) {
        sosActive = !sosActive;
        if (sosActive) {
            setRgbLed(255, 0, 0);
            tone(BUZZER_PIN, 2400, 200);
        } else {
            setRgbLed(0, 0, 0);
            tone(BUZZER_PIN, 800, 100);
        }
        renderSosView();
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

    // Boot sequence
    showBootSequence();

    // Render initial UI
    renderHeader();
    renderNavTabs();
    renderTacticalGrid();

    lastBeaconTx = millis();
}

// --- MAIN LOOP ---
void loop() {
    unsigned long now = millis();

    // Touch input check
    handleTouch();

    // Automatic beacon transmission timer
    if (now - lastBeaconTx >= (beaconIntervalSec * 1000UL)) {
        lastBeaconTx = now;
        setRgbLed(0, 0, 255); // Blue TX
        playAfskBurst();
        setRgbLed(0, 0, 0);
    }

    // Emergency SOS Strobe
    if (sosActive) {
        if (now - lastSosStrobe >= 250) {
            lastSosStrobe = now;
            sosStrobeState = !sosStrobeState;
            setRgbLed(sosStrobeState ? 255 : 0, 0, 0);
            if (sosStrobeState) {
                tone(BUZZER_PIN, 1800, 60);
            }
        }
    }

    delay(20);
}
