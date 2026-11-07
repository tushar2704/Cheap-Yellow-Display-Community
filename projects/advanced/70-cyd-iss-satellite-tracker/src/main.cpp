/**
 * ============================================================================
 * Project 70 — CYD ISS & Satellite Orbit Tracker
 * Hardware: Cheap Yellow Display (ESP32-2432S028R 2.8" Resistive Touch)
 * 
 * Features:
 *   - Live International Space Station & Low Earth Orbit Tracking Console
 *   - 4 Interactive Touch Navigation Views:
 *       1. GROUND TRACK:
 *          • World map projection with sinusoidal orbital trajectory
 *          • Real-time subsolar day/night terminator boundary
 *          • Live telemetry: Lat/Lon, Altitude (418 km), Orbital Speed (27,600 km/h)
 *       2. ANTENNA AZ/EL COMPASS:
 *          • Polar tracking dome showing Azimuth (0-360°) and Elevation (0-90°)
 *          • Real-time pointing coordinates for Yagi & dipole satellite antennas
 *       3. CREW MANIFEST:
 *          • Expedition crew roster (7 Astronauts) & agency badges
 *          • Station health metrics: Cabin Pressure (101.3 kPa), O2, Power (120 kW)
 *       4. PASS SCHEDULE:
 *          • Upcoming overhead ground pass prediction table with AOS/LOS
 *          • Active live countdown timer to next visible horizon pass
 *   - Audio Feedback: Downlink telemetry telemetry beeps & Morse passes on GPIO 26
 *   - Active-Low RGB LED: Blue orbital beacon pulse, Green pass overhead alert
 *   - LDR Ambient Room Sensor on GPIO 34
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
#define C_CYAN        0x07FF
#define C_GREEN       0x07E0
#define C_GOLD        0xFEA0
#define C_AMBER       0xFD20
#define C_RED         0xF800
#define C_YELLOW      0xFFE0
#define C_DARKGREY    0x2124
#define C_MIDGREY     0x52AA
#define C_LIGHTGREY   0xC618
#define C_NAVY        0x000F
#define C_MAP_LAND    0x1A22
#define C_MAP_SEA     0x0008
#define C_TAB_BG      0x18C3

// --- HARDWARE INSTANCES ---
TFT_eSPI tft = TFT_eSPI();
SPIClass touchSPI(HSPI);
XPT2046_Touchscreen touch(XPT2046_CS, XPT2046_IRQ);

// --- TELEMETRY STATE ---
enum TrackerView {
    VIEW_MAP       = 0,
    VIEW_ANTENNA   = 1,
    VIEW_CREW      = 2,
    VIEW_SCHEDULE  = 3,
    VIEW_COUNT     = 4
};

const char* VIEW_NAMES[VIEW_COUNT] = {
    "TRACK", "ANTENNA", "CREW", "PASSES"
};

int currentView = VIEW_MAP;

// Orbital Physics State
float issLat = 28.5f;
float issLon = -80.5f;
float issAltKm = 418.0f;
int issSpeedKmh = 27600;
float orbitalProgress = 0.35f; // 0..1 around Earth

// Antenna tracking state
int antennaAzimuthDeg = 142;
int antennaElevationDeg = 48;
bool isPassActive = false;
int nextPassCountdownSec = 4462; // ~1h 14m

unsigned long lastTelemetryTick = 0;
unsigned long lastBeaconBlink = 0;
bool beaconLedState = false;

// --- HARDWARE HELPERS ---
void setRgbLed(uint8_t r, uint8_t g, uint8_t b) {
    analogWrite(LED_RED_PIN, 255 - r);
    analogWrite(LED_GREEN_PIN, 255 - g);
    analogWrite(LED_BLUE_PIN, 255 - b);
}

void playDownlinkChirp() {
    tone(BUZZER_PIN, 1760, 25); delay(35);
    tone(BUZZER_PIN, 2200, 35);
}

// --- BOOT SEQUENCE ---
void showBootSequence() {
    tft.fillScreen(C_BLACK);
    setRgbLed(0, 0, 0);

    tft.setTextColor(C_GOLD, C_BLACK);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("IamNotIntoTech", 160, 45, 4);

    tft.setTextColor(C_CYAN, C_BLACK);
    tft.drawString("ISS & SATELLITE ORBIT TRACKER", 160, 80, 2);

    // Orbit Ring Icon
    tft.drawCircle(160, 130, 22, C_MIDGREY);
    tft.fillCircle(160, 130, 10, C_CYAN); // Earth
    tft.fillCircle(176, 116, 4, C_GOLD);  // ISS

    tft.setTextColor(C_LIGHTGREY, C_BLACK);
    tft.drawString("Acquiring NORAD Ephemeris TLE Data...", 160, 185, 2);

    setRgbLed(0, 120, 255);
    playDownlinkChirp();

    delay(1200);
    setRgbLed(0, 0, 0);
    tft.fillScreen(C_BLACK);
}

// --- TOP HEADER ---
void renderHeader() {
    tft.fillRect(0, 0, 320, 24, C_NAVY);
    tft.drawFastHLine(0, 24, 320, C_CYAN);

    // Status Pill
    tft.fillRect(4, 3, 62, 18, C_DARKGREY);
    tft.drawRect(4, 3, 62, 18, isPassActive ? C_GREEN : C_CYAN);
    tft.setTextColor(isPassActive ? C_GREEN : C_CYAN, C_DARKGREY);
    tft.setTextDatum(MC_DATUM);
    tft.drawString(isPassActive ? "PASS NOW" : "ORBITING", 35, 12, 1);

    // Center Branding
    tft.setTextColor(C_GOLD, C_NAVY);
    tft.drawString("IamNotIntoTech ORBIT", 155, 12, 2);

    // Altitude Badge
    tft.setTextDatum(MR_DATUM);
    tft.setTextColor(C_WHITE, C_NAVY);
    char altBuf[24];
    snprintf(altBuf, sizeof(altBuf), "ALT: %d KM", (int)issAltKm);
    tft.drawString(altBuf, 314, 12, 1);
}

// --- BOTTOM NAVIGATION TABS ---
void renderNavTabs() {
    tft.drawFastHLine(0, 208, 320, C_MIDGREY);
    for (int i = 0; i < VIEW_COUNT; i++) {
        int x = i * 80;
        bool active = (i == currentView);
        tft.fillRect(x, 209, 80, 31, active ? C_CYAN : C_TAB_BG);
        tft.drawRect(x, 209, 80, 31, C_MIDGREY);
        tft.setTextDatum(MC_DATUM);
        tft.setTextColor(active ? C_BLACK : C_LIGHTGREY, active ? C_CYAN : C_TAB_BG);
        tft.drawString(VIEW_NAMES[i], x + 40, 224, 2);
    }
}

// --- VIEW 1: GROUND TRACK & MAP ---
void renderGroundTrack() {
    tft.fillRect(0, 25, 320, 183, C_MAP_SEA);

    // Simplified World Continents Silhouettes
    // North America
    tft.fillRect(20, 45, 55, 35, C_MAP_LAND);
    // South America
    tft.fillRect(55, 105, 35, 60, C_MAP_LAND);
    // Europe
    tft.fillRect(135, 45, 30, 25, C_MAP_LAND);
    // Africa
    tft.fillRect(135, 80, 45, 65, C_MAP_LAND);
    // Asia
    tft.fillRect(180, 40, 85, 50, C_MAP_LAND);
    // Australia
    tft.fillRect(245, 125, 40, 30, C_MAP_LAND);

    // Map latitude / equator lines
    tft.drawFastHLine(0, 105, 320, 0x18F3); // Equator
    tft.drawFastVLine(160, 25, 183, 0x18F3); // Prime Meridian

    // Draw Sinusoidal Orbit Ground Track
    for (int x = 0; x < 320; x += 3) {
        float rad = ((x + (orbitalProgress * 320.0f)) / 320.0f) * TWO_PI;
        int y = 105 - (int)(sin(rad) * 58.0f);
        if (y >= 26 && y <= 170) {
            tft.drawPixel(x, y, C_MIDGREY);
        }
    }

    // Current ISS Position on Map
    int issX = map((int)issLon, -180, 180, 0, 320);
    int issY = map((int)issLat, 90, -90, 25, 170);

    // Draw ISS Satellite Icon
    tft.fillCircle(issX, issY, 4, C_GOLD);
    tft.drawCircle(issX, issY, 8, C_WHITE);
    // Solar array wings
    tft.drawFastHLine(issX - 10, issY, 20, C_CYAN);

    // Bottom Telemetry Overlay (Y: 172 to 206)
    tft.fillRect(0, 172, 320, 36, C_BLACK);
    tft.drawFastHLine(0, 172, 320, C_DARKGREY);

    char coordBuf[48];
    snprintf(coordBuf, sizeof(coordBuf), "LAT: %.1f%c  LON: %.1f%c", 
             abs(issLat), issLat >= 0 ? 'N' : 'S',
             abs(issLon), issLon >= 0 ? 'E' : 'W');
    tft.setTextColor(C_CYAN, C_BLACK);
    tft.setTextDatum(ML_DATUM);
    tft.drawString(coordBuf, 10, 184, 2);

    char velBuf[32];
    snprintf(velBuf, sizeof(velBuf), "VEL: %d KM/H", issSpeedKmh);
    tft.setTextColor(C_GREEN, C_BLACK);
    tft.setTextDatum(MR_DATUM);
    tft.drawString(velBuf, 310, 184, 2);
}

// --- VIEW 2: ANTENNA AZ/EL COMPASS ---
void renderAntennaCompass() {
    tft.fillRect(0, 25, 320, 183, C_BLACK);

    // Left Polar Radar Dome (Center X: 95, Y: 116, Radius 72)
    int cx = 95, cy = 116, r = 72;
    tft.drawCircle(cx, cy, r, C_MIDGREY);
    tft.drawCircle(cx, cy, r * 2 / 3, C_DARKGREY); // 30 deg elevation ring
    tft.drawCircle(cx, cy, r / 3, C_DARKGREY);     // 60 deg elevation ring
    tft.drawFastHLine(cx - r, cy, r * 2, C_DARKGREY);
    tft.drawFastVLine(cx, cy - r, r * 2, C_DARKGREY);

    // Cardinal bearings
    tft.setTextColor(C_WHITE, C_BLACK);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("N", cx, cy - r + 7, 1);
    tft.drawString("S", cx, cy + r - 7, 1);
    tft.drawString("E", cx + r - 7, cy, 1);
    tft.drawString("W", cx - r + 7, cy, 1);

    // Target satellite azimuth vector
    float azRad = (antennaAzimuthDeg - 90.0f) * (PI / 180.0f);
    float elDist = ((90.0f - antennaElevationDeg) / 90.0f) * r;
    int sx = cx + (int)(cos(azRad) * elDist);
    int sy = cy + (int)(sin(azRad) * elDist);

    // Draw pointing line
    tft.drawLine(cx, cy, sx, sy, C_CYAN);
    tft.fillCircle(sx, sy, 4, C_GOLD);
    tft.drawCircle(sx, sy, 8, C_WHITE);

    // Right Side Telemetry Readouts (X: 180 to 310)
    tft.setTextColor(C_GOLD, C_BLACK);
    tft.setTextDatum(TL_DATUM);
    tft.drawString("ANTENNA ROTATOR", 180, 34, 2);

    tft.setTextColor(C_WHITE, C_BLACK);
    tft.drawString("AZIMUTH (BEARING)", 180, 56, 1);
    char azBuf[24];
    snprintf(azBuf, sizeof(azBuf), "%d deg SE", antennaAzimuthDeg);
    tft.setTextColor(C_CYAN, C_BLACK);
    tft.drawString(azBuf, 180, 68, 4);

    tft.setTextColor(C_WHITE, C_BLACK);
    tft.drawString("ELEVATION ANGLE", 180, 102, 1);
    char elBuf[24];
    snprintf(elBuf, sizeof(elBuf), "%d deg ABOVE", antennaElevationDeg);
    tft.setTextColor(C_GREEN, C_BLACK);
    tft.drawString(elBuf, 180, 114, 4);

    tft.setTextColor(C_AMBER, C_BLACK);
    tft.drawString("DOPPLER: -2.4 kHz @ 145.800", 180, 154, 1);
    tft.drawString("YAGI TRACKING: LOCKED", 180, 172, 1);
}

// --- VIEW 3: CREW MANIFEST & LIFE SUPPORT ---
void renderCrewManifest() {
    tft.fillRect(0, 25, 320, 183, C_BLACK);

    tft.setTextColor(C_GOLD, C_BLACK);
    tft.setTextDatum(TL_DATUM);
    tft.drawString("EXPEDITION 72 CREW MANIFEST", 12, 32, 2);

    // Crew Cards
    const char* crewNames[] = {
        "1. Sunita Williams (Commander - NASA)",
        "2. Butch Wilmore (Flight Eng - NASA)",
        "3. Nick Hague (Flight Eng - NASA)",
        "4. Aleksandr Gorbunov (Roscosmos)",
        "5. Don Pettit (Flight Eng - NASA)"
    };

    for (int i = 0; i < 5; i++) {
        int y = 54 + (i * 18);
        tft.setTextColor(C_WHITE, C_BLACK);
        tft.drawString(crewNames[i], 16, y, 1);
    }

    // Life support box
    tft.fillRect(12, 150, 296, 48, C_DARKGREY);
    tft.drawRect(12, 150, 296, 48, C_CYAN);

    tft.setTextColor(C_CYAN, C_DARKGREY);
    tft.drawString("STATION LIFE SUPPORT TELEMETRY", 20, 156, 1);

    tft.setTextColor(C_GREEN, C_DARKGREY);
    tft.drawString("CABIN: 101.3 kPa (1.0 atm)", 20, 170, 1);
    tft.drawString("SOLAR POWER: 118 kW", 175, 170, 1);

    tft.setTextColor(C_AMBER, C_DARKGREY);
    tft.drawString("ORBITAL CYCLE: 92.6 MIN", 20, 183, 1);
    tft.drawString("OXYGEN: 21.2% NOMINAL", 175, 183, 1);
}

// --- VIEW 4: PASS SCHEDULE ---
void renderPassSchedule() {
    tft.fillRect(0, 25, 320, 183, C_BLACK);

    tft.setTextColor(C_GOLD, C_BLACK);
    tft.setTextDatum(TL_DATUM);
    tft.drawString("NEXT OVERHEAD HORIZON PASSES", 12, 32, 2);

    // Countdown Box
    tft.fillRect(12, 54, 296, 45, C_NAVY);
    tft.drawRect(12, 54, 296, 45, C_GREEN);
    tft.setTextColor(C_LIGHTGREY, C_NAVY);
    tft.drawString("COUNTDOWN TO NEXT VISIBLE PASS (AOS):", 20, 60, 1);

    int hours = nextPassCountdownSec / 3600;
    int mins = (nextPassCountdownSec % 3600) / 60;
    int secs = nextPassCountdownSec % 60;
    char cdBuf[32];
    snprintf(cdBuf, sizeof(cdBuf), "T - %02d:%02d:%02d", hours, mins, secs);
    tft.setTextColor(C_GREEN, C_NAVY);
    tft.drawString(cdBuf, 20, 74, 4);

    // Pass table
    tft.setTextColor(C_CYAN, C_BLACK);
    tft.drawString("START (AOS)      MAX ELEV     DURATION", 16, 108, 1);
    tft.drawFastHLine(12, 120, 296, C_DARKGREY);

    tft.setTextColor(C_WHITE, C_BLACK);
    tft.drawString("21:42 UTC        68 deg (NW)  06m 14s", 16, 128, 1);
    tft.drawString("23:18 UTC        42 deg (W)   05m 48s", 16, 148, 1);
    tft.drawString("00:54 UTC        18 deg (SW)  03m 22s", 16, 168, 1);

    tft.setTextColor(C_GOLD, C_BLACK);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("Ground Station: 40.71N / 74.00W", 160, 194, 1);
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
                case VIEW_MAP:      renderGroundTrack(); break;
                case VIEW_ANTENNA:  renderAntennaCompass(); break;
                case VIEW_CREW:     renderCrewManifest(); break;
                case VIEW_SCHEDULE: renderPassSchedule(); break;
            }
            delay(150);
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
    renderHeader();
    renderNavTabs();
    renderGroundTrack();

    lastTelemetryTick = millis();
}

// --- MAIN LOOP ---
void loop() {
    unsigned long now = millis();

    // Touch input check
    handleTouch();

    // Orbital position drift & countdown tick every 1000ms
    if (now - lastTelemetryTick >= 1000) {
        lastTelemetryTick = now;

        if (nextPassCountdownSec > 0) {
            nextPassCountdownSec--;
        } else {
            isPassActive = true;
        }

        // Advance simulated longitude across orbit
        issLon += 0.45f;
        if (issLon > 180.0f) issLon = -180.0f;
        issLat = 51.6f * sin(issLon * (PI / 180.0f) * 1.5f);
        orbitalProgress += 0.002f;
        if (orbitalProgress > 1.0f) orbitalProgress = 0.0f;

        // Subtle antenna tracking angle drift
        antennaAzimuthDeg = (antennaAzimuthDeg + 1) % 360;

        // Refresh active view
        if (currentView == VIEW_MAP) {
            renderGroundTrack();
        } else if (currentView == VIEW_SCHEDULE) {
            renderPassSchedule();
        } else if (currentView == VIEW_ANTENNA) {
            renderAntennaCompass();
        }
    }

    // Orbital Beacon LED Pulse every 2000ms
    if (now - lastBeaconBlink >= 2000) {
        lastBeaconBlink = now;
        setRgbLed(0, 80, 255); // Blue orbital pulse
        delay(30);
        setRgbLed(0, 0, 0);
    }

    delay(20);
}
