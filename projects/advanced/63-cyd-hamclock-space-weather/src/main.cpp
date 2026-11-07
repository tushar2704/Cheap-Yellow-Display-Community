// =========================================================================
//  Project 63 — CYD HamClock Space Weather Station
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
#define C_BG          0x0821  // Deep space obsidian slate
#define C_CARD        0x10A2  // Slightly elevated card
#define C_CARD_BORDER 0x2124
#define C_OCEAN_DAY   0x098F  // Day Ocean Blue
#define C_OCEAN_NIGHT 0x0108  // Night Ocean Deep Blue
#define C_LAND_DAY    0x2384  // Day Continent Olive
#define C_LAND_NIGHT  0x0941  // Night Continent Dark
#define C_TERMINATOR  0xFDE0  // Solar greyline gold
#define C_YELLOW      0xFFE0
#define C_GREEN       0x07E0
#define C_CYAN        0x07FF
#define C_ALERT_RED   0xF800
#define C_GRAY        0x73AE
#define C_DARKGRAY    0x2965

TFT_eSPI tft = TFT_eSPI();
SPIClass touchSPI(HSPI);
XPT2046_Touchscreen touch(TOUCH_CS, TOUCH_IRQ);

// =========================================================================
//  SPACE WEATHER & CLOCK STATE
// =========================================================================
// Clocks
int utcH = 14;
int utcM = 28;
int utcS = 45;
int localH = 9;
int localM = 28;
int localS = 45;
bool is24hFormat = true;
bool morseAudioEnabled = true;

// Space Weather Metrics
int sfiFlux = 168;         // Solar Flux Index (>150 is excellent DX)
int ssnCount = 142;        // Sunspot number
float kpIndex = 2.3;       // Geomagnetic Kp-index (0..9)
const char* xRayClass = "C 1.4"; // Solar flare class

// HF Propagation Status
const char* BANDS[4] = { "10m: GOOD", "15m: EXC", "20m: GOOD", "40m: FAIR" };

// Solar Terminator Simulation
float sunLonOffset = 0;
unsigned long lastSecTick = 0;

// =========================================================================
//  SOUND & LED OUTPUT
// =========================================================================
void playTouchClick() {
    tone(BUZZER_PIN, 1800, 15);
}

// Plays "DX" in Morse Code (-..  -..-)
void playMorseDX() {
    if (!morseAudioEnabled) return;
    int dot = 50;
    // D: - . .
    tone(BUZZER_PIN, 1200, dot * 3); delay(dot * 4);
    tone(BUZZER_PIN, 1200, dot);     delay(dot * 2);
    tone(BUZZER_PIN, 1200, dot);     delay(dot * 4);
    // X: - . . -
    tone(BUZZER_PIN, 1200, dot * 3); delay(dot * 4);
    tone(BUZZER_PIN, 1200, dot);     delay(dot * 2);
    tone(BUZZER_PIN, 1200, dot);     delay(dot * 2);
    tone(BUZZER_PIN, 1200, dot * 3); delay(dot * 4);
}

void setRGB(uint8_t r, uint8_t g, uint8_t b) {
    analogWrite(LED_R, 255 - r);
    analogWrite(LED_G, 255 - g);
    analogWrite(LED_B, 255 - b);
}

void updateKpLed() {
    if (kpIndex >= 5.0) {
        setRGB(255, 0, 0); // Geomagnetic Storm Red!
    } else if (kpIndex >= 4.0) {
        setRGB(255, 180, 0); // Unsettled Yellow
    } else {
        setRGB(0, 180, 60); // Quiet Green
    }
}

// =========================================================================
//  UI DRAWING ROUTINES
// =========================================================================
void drawHeader() {
    tft.fillRect(0, 0, 320, 24, C_BG);

    // Title / Brand
    tft.setTextColor(C_CYAN, C_BG);
    tft.setTextDatum(TL_DATUM);
    tft.drawString("IamNotIntoTech", 6, 5, 2);
    tft.setTextColor(C_YELLOW, C_BG);
    tft.drawString("HAMCLOCK", 126, 5, 2);

    // Grid locator badge
    tft.fillRoundRect(204, 3, 56, 18, 4, C_CARD);
    tft.drawRoundRect(204, 3, 56, 18, 4, C_CARD_BORDER);
    tft.setTextColor(C_GREEN, C_CARD);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("FN31pr", 232, 12, 1);

    // Solar cycle chip
    tft.fillRoundRect(264, 3, 50, 18, 4, C_CARD);
    tft.drawRoundRect(264, 3, 50, 18, 4, C_CARD_BORDER);
    tft.setTextColor(C_WHITE, C_CARD);
    tft.drawString("SOL 25", 289, 12, 1);

    tft.drawFastHLine(0, 24, 320, C_CARD_BORDER);
}

// --- WORLD MAP WITH DAY / NIGHT SOLAR TERMINATOR ---
void drawWorldMap() {
    int mx = 8;
    int my = 28;
    int mw = 184;
    int mh = 104;

    tft.fillRoundRect(mx, my, mw, mh, 6, C_CARD);
    tft.drawRoundRect(mx, my, mw, mh, 6, C_CARD_BORDER);

    // Draw Ocean Background
    int subSolarX = mx + (int)((fmod(sunLonOffset + 180.0, 360.0) / 360.0) * mw);

    // Render day/night split ocean background
    for (int x = 0; x < mw - 8; x++) {
        int px = mx + 4 + x;
        int distFromSun = abs(px - subSolarX);
        if (distFromSun > (mw / 2)) distFromSun = mw - distFromSun;
        bool isDay = (distFromSun < (mw / 4));
        tft.drawFastVLine(px, my + 4, mh - 8, isDay ? C_OCEAN_DAY : C_OCEAN_NIGHT);
    }

    // Simplified Continental Vector Silhouettes
    // North America (X: 18..55, Y: 12..45)
    tft.fillRect(mx + 16, my + 14, 34, 22, C_LAND_DAY);
    tft.drawRect(mx + 16, my + 14, 34, 22, C_LAND_NIGHT);

    // South America (X: 38..62, Y: 46..84)
    tft.fillRect(mx + 38, my + 44, 22, 34, C_LAND_DAY);
    tft.drawRect(mx + 38, my + 44, 22, 34, C_LAND_NIGHT);

    // Europe (X: 78..102, Y: 12..34)
    tft.fillRect(mx + 76, my + 14, 24, 18, C_LAND_DAY);
    tft.drawRect(mx + 76, my + 14, 24, 18, C_LAND_NIGHT);

    // Africa (X: 76..106, Y: 36..76)
    tft.fillRect(mx + 76, my + 36, 28, 38, C_LAND_DAY);
    tft.drawRect(mx + 76, my + 36, 28, 38, C_LAND_NIGHT);

    // Asia (X: 104..162, Y: 12..50)
    tft.fillRect(mx + 104, my + 14, 52, 32, C_LAND_DAY);
    tft.drawRect(mx + 104, my + 14, 52, 32, C_LAND_NIGHT);

    // Australia (X: 136..162, Y: 60..86)
    tft.fillRect(mx + 136, my + 60, 24, 20, C_LAND_DAY);
    tft.drawRect(mx + 136, my + 60, 24, 20, C_LAND_NIGHT);

    // Solar Terminator (Golden Sinusoidal Greyline Projection)
    for (int x = 0; x < mw - 8; x++) {
        int px = mx + 4 + x;
        float angle = ((float)(x + (int)sunLonOffset) / mw) * 2 * PI;
        int termY = my + (mh / 2) + (int)(sin(angle) * 32.0);
        if (termY >= my + 4 && termY < my + mh - 4) {
            tft.drawPixel(px, termY, C_TERMINATOR);
            tft.drawPixel(px, termY + 1, C_TERMINATOR);
        }
    }

    // Subsolar point (Golden Sun Disc)
    int sunY = my + (mh / 2) + (int)(sin(((subSolarX - mx) / (float)mw) * 2 * PI) * 16.0);
    sunY = constrain(sunY, my + 8, my + mh - 8);
    tft.fillCircle(subSolarX, sunY, 4, C_YELLOW);
    tft.drawCircle(subSolarX, sunY, 4, C_WHITE);

    // Auroral Oval Ring at North Pole
    tft.drawCircle(mx + (mw / 2), my + 12, 14, C_GREEN);

    tft.setTextColor(C_GRAY, C_CARD);
    tft.setTextDatum(BL_DATUM);
    tft.drawString("GREYLINE TERMINATOR", mx + 8, my + mh - 4, 1);
}

// --- DUAL CLOCKS PANEL (UTC & LOCAL) ---
void drawClocks() {
    int cx = 198;
    int cy = 28;
    int cw = 114;
    int ch = 104;

    tft.fillRoundRect(cx, cy, cw, ch, 6, C_CARD);
    tft.drawRoundRect(cx, cy, cw, ch, 6, C_CARD_BORDER);

    // UTC / ZULU Time Header
    tft.setTextColor(C_GRAY, C_CARD);
    tft.setTextDatum(TL_DATUM);
    tft.drawString("UTC / ZULU CLOCK", cx + 8, cy + 6, 1);

    // Big UTC Clock
    char utcBuf[24];
    snprintf(utcBuf, sizeof(utcBuf), "%02d:%02d:%02d", utcH, utcM, utcS);
    tft.setTextColor(C_YELLOW, C_CARD);
    tft.setTextDatum(MC_DATUM);
    tft.drawString(utcBuf, cx + (cw / 2), cy + 28, 4);

    // Local Clock
    tft.setTextColor(C_GRAY, C_CARD);
    tft.setTextDatum(TL_DATUM);
    tft.drawString("LOCAL TIME (EST)", cx + 8, cy + 50, 1);

    char locBuf[24];
    snprintf(locBuf, sizeof(locBuf), "%02d:%02d:%02d", localH, localM, localS);
    tft.setTextColor(C_CYAN, C_CARD);
    tft.setTextDatum(MC_DATUM);
    tft.drawString(locBuf, cx + (cw / 2), cy + 70, 4);

    // Solar Ephemeris markers
    tft.setTextColor(C_GRAY, C_CARD);
    tft.setTextDatum(BC_DATUM);
    tft.drawString("NOON: 12:14  MID: 00:14", cx + (cw / 2), cy + ch - 4, 1);
}

// --- SPACE WEATHER TELEMETRY CARDS (SFI, SSN, Kp, X-RAY) ---
void drawSpaceWeather() {
    int sy = 138;
    int cardW = 73;
    int cardH = 46;
    int spacing = 6;
    int startX = 8;

    // 1. SFI (Solar Flux Index)
    int x1 = startX;
    tft.fillRoundRect(x1, sy, cardW, cardH, 4, C_CARD);
    tft.drawRoundRect(x1, sy, cardW, cardH, 4, C_CARD_BORDER);
    tft.setTextColor(C_GRAY, C_CARD);
    tft.setTextDatum(TL_DATUM);
    tft.drawString("SOLAR SFI", x1 + 6, sy + 4, 1);
    char sfiBuf[16];
    snprintf(sfiBuf, sizeof(sfiBuf), "%d", sfiFlux);
    tft.setTextColor(C_GREEN, C_CARD);
    tft.drawString(sfiBuf, x1 + 6, sy + 18, 4);
    tft.setTextColor(C_GRAY, C_CARD);
    tft.drawString("sfu (DX: EXC)", x1 + 6, sy + 36, 1);

    // 2. SSN (Sunspot Number)
    int x2 = x1 + cardW + spacing;
    tft.fillRoundRect(x2, sy, cardW, cardH, 4, C_CARD);
    tft.drawRoundRect(x2, sy, cardW, cardH, 4, C_CARD_BORDER);
    tft.setTextColor(C_GRAY, C_CARD);
    tft.drawString("SUNSPOTS", x2 + 6, sy + 4, 1);
    char ssnBuf[16];
    snprintf(ssnBuf, sizeof(ssnBuf), "%d", ssnCount);
    tft.setTextColor(C_YELLOW, C_CARD);
    tft.drawString(ssnBuf, x2 + 6, sy + 18, 4);
    tft.setTextColor(C_GRAY, C_CARD);
    tft.drawString("Spots (Sol 25)", x2 + 6, sy + 36, 1);

    // 3. Kp Geomagnetic Index
    int x3 = x2 + cardW + spacing;
    tft.fillRoundRect(x3, sy, cardW, cardH, 4, C_CARD);
    tft.drawRoundRect(x3, sy, cardW, cardH, 4, C_CARD_BORDER);
    tft.setTextColor(C_GRAY, C_CARD);
    tft.drawString("Kp INDEX", x3 + 6, sy + 4, 1);
    char kpBuf[16];
    snprintf(kpBuf, sizeof(kpBuf), "%.1f", kpIndex);
    uint16_t kpCol = (kpIndex < 3.0) ? C_GREEN : (kpIndex < 5.0 ? C_YELLOW : C_ALERT_RED);
    tft.setTextColor(kpCol, C_CARD);
    tft.drawString(kpBuf, x3 + 6, sy + 18, 4);
    tft.setTextColor(C_GRAY, C_CARD);
    tft.drawString(kpIndex < 3.0 ? "Quiet Field" : "Unsettled", x3 + 6, sy + 36, 1);

    // 4. X-Ray Solar Flare Class
    int x4 = x3 + cardW + spacing;
    tft.fillRoundRect(x4, sy, cardW, cardH, 4, C_CARD);
    tft.drawRoundRect(x4, sy, cardW, cardH, 4, C_CARD_BORDER);
    tft.setTextColor(C_GRAY, C_CARD);
    tft.drawString("X-RAY FLUX", x4 + 6, sy + 4, 1);
    tft.setTextColor(C_CYAN, C_CARD);
    tft.drawString(xRayClass, x4 + 6, sy + 18, 4);
    tft.setTextColor(C_GRAY, C_CARD);
    tft.drawString("Normal HF", x4 + 6, sy + 36, 1);

    // Lower Band Propagation Strip (Y: 190, H: 20)
    int py = 188;
    int pw = 304;
    tft.fillRoundRect(startX, py, pw, 22, 4, C_CARD);
    tft.drawRoundRect(startX, py, pw, 22, 4, C_CARD_BORDER);

    int bandStep = pw / 4;
    for (int i = 0; i < 4; i++) {
        tft.setTextColor(C_GREEN, C_CARD);
        tft.setTextDatum(MC_DATUM);
        tft.drawString(BANDS[i], startX + (i * bandStep) + (bandStep / 2), py + 11, 1);
    }
}

void drawFooter() {
    int by = 214;
    tft.fillRect(0, by, 320, 26, C_BG);
    tft.drawFastHLine(0, by, 320, C_CARD_BORDER);

    tft.setTextColor(C_GRAY, C_BG);
    tft.setTextDatum(TL_DATUM);
    tft.drawString("NOAA Space Weather Telemetry", 8, by + 6, 2);

    tft.setTextColor(morseAudioEnabled ? C_GREEN : C_GRAY, C_BG);
    tft.setTextDatum(TR_DATUM);
    tft.drawString(morseAudioEnabled ? "MORSE: ON" : "MORSE: OFF", 312, by + 6, 2);
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

    // 1. Morse Audio Toggle (Footer Touch, Y >= 210)
    if (ty >= 210 && tx >= 220) {
        morseAudioEnabled = !morseAudioEnabled;
        playTouchClick();
        drawFooter();
        delay(250);
        return;
    }

    // 2. Play Morse Code test (Tap Clocks, X >= 198, Y: 28..134)
    if (tx >= 198 && ty >= 28 && ty <= 134) {
        playMorseDX();
        delay(250);
        return;
    }

    // 3. Force space weather update / rescan
    if (ty < 210) {
        playTouchClick();
        sfiFlux += random(-2, 3);
        ssnCount += random(-1, 2);
        drawSpaceWeather();
        delay(200);
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

    // Space Weather Boot Screen
    tft.setTextColor(C_CYAN, C_BG);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("IamNotIntoTech", 160, 80, 4);
    tft.setTextColor(C_YELLOW, C_BG);
    tft.drawString("HAMCLOCK & SPACE WEATHER STATION", 160, 120, 2);
    tft.setTextColor(C_WHITE, C_BG);
    tft.drawString("Connecting to NOAA Space Weather & Solar Indices...", 160, 150, 1);

    playTouchClick();
    delay(100);
    tone(BUZZER_PIN, 1200, 60);
    delay(600);

    tft.fillScreen(C_BG);

    drawHeader();
    drawWorldMap();
    drawClocks();
    drawSpaceWeather();
    drawFooter();

    updateKpLed();
}

void loop() {
    handleTouch();

    unsigned long now = millis();

    // 1-second clock advance & solar terminator rotation
    if (now - lastSecTick >= 1000) {
        lastSecTick = now;

        utcS++;
        localS++;

        if (utcS >= 60) {
            utcS = 0;
            utcM++;
            if (utcM >= 60) {
                utcM = 0;
                utcH = (utcH + 1) % 24;
                // Play hourly Morse Code DX chime!
                playMorseDX();
            }
        }

        if (localS >= 60) {
            localS = 0;
            localM++;
            if (localM >= 60) {
                localM = 0;
                localH = (localH + 1) % 24;
            }
        }

        // Advance sun longitude
        sunLonOffset += 0.2;
        if (sunLonOffset >= 360.0) sunLonOffset = 0;

        drawClocks();

        // Refresh map every 30 seconds
        if (utcS % 30 == 0) {
            drawWorldMap();
        }
    }

    delay(20);
}
