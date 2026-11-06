/*
 * ============================================================
 *  Project #47 — Antigravity Vibe Monitor
 *  Cheap Yellow Display (ESP32-2432S028R) — 2.8" ILI9341
 * ============================================================
 *  Displays live Google Antigravity (Gemini AI coding tool)
 *  session metrics pulled from your local machine via a small
 *  Python bridge (bridge/monitor.py).
 *
 *  Screens
 *    0 — Live Session Dashboard
 *    1 — Streak & History
 *    2 — Standby / Clock
 *
 *  Tap the display to cycle through screens.
 * ============================================================
 */

// ===== USER CONFIGURATION =====
const char* WIFI_SSID     = "YOUR_WIFI_SSID";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";
const char* BRIDGE_HOST   = "192.168.1.100"; // IP of your Mac/PC running bridge/monitor.py
const int   BRIDGE_PORT   = 5047;
// ==============================

#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <NTPClient.h>
#include <WiFiUDP.h>

// ---------- Display constants ----------
#define TFT_WIDTH   320
#define TFT_HEIGHT  240

#define COL_BG      0x0003   // Deep blue  #001a33
#define COL_CYAN    0x07FF   // Bright cyan
#define COL_WHITE   0xFFFF
#define COL_YELLOW  0xFFE0
#define COL_RED     0xF800
#define COL_DIM     0x4208   // Dark gray
#define COL_GREEN   0x07E0
#define COL_ORANGE  0xFD20
#define COL_DARKBLUE 0x000F  // Dark navy for accents

// ---------- Touch pins ----------
#define TOUCH_CS   33
#define TOUCH_CLK  25
#define TOUCH_MISO 39
#define TOUCH_MOSI 32
#define TOUCH_IRQ  36

// ---------- LED pins (Active LOW) ----------
#define LED_R  4
#define LED_G  16
#define LED_B  17

// ---------- Touch calibration ----------
#define TOUCH_X_MIN 240
#define TOUCH_X_MAX 3800
#define TOUCH_Y_MIN 240
#define TOUCH_Y_MAX 3800

// ---------- Global State ----------
struct AgyData {
    bool   valid             = false;
    int    conversations_today = 0;
    int    tool_calls_today  = 0;
    long   tokens_in         = 0;
    long   tokens_out        = 0;
    int    subagents_spawned = 0;
    int    active_minutes    = 0;
    int    streak_days       = 0;
    int    streak_best       = 0;
    String model             = "gemini-2.5-pro";
    String last_activity     = "unknown";
    String status            = "unknown";
};

AgyData agy;

int           currentScreen   = 0;       // 0=dashboard 1=streak 2=standby
unsigned long lastFetch       = 0;
unsigned long lastTouch       = 0;
unsigned long lastLedToggle   = 0;
bool          ledState        = false;
bool          bridgeReachable = false;

const unsigned long FETCH_INTERVAL = 30000UL;  // 30 s

// ---------- Hardware objects ----------
TFT_eSPI tft;

SPIClass touchSPI(HSPI);
XPT2046_Touchscreen ts(TOUCH_CS, TOUCH_IRQ);

WiFiUDP   ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 0, 60000);

// ============================================================
//  LED helpers
// ============================================================

/** Set RGB LED channels. Active LOW: true = LED on. */
void setLED(bool r, bool g, bool b) {
    digitalWrite(LED_R, r ? LOW : HIGH);
    digitalWrite(LED_G, g ? LOW : HIGH);
    digitalWrite(LED_B, b ? LOW : HIGH);
}

/**
 * Set LED colour by state:
 *   0 = cyan  (active, low usage)
 *   1 = yellow (active, high usage)
 *   2 = blue  (idle)
 *   3 = red   (error / no bridge)
 */
void setLEDColor(int state) {
    switch (state) {
        case 0: setLED(false, true,  true);  break; // cyan  (G+B on)
        case 1: setLED(true,  true,  false); break; // yellow(R+G on)
        case 2: setLED(false, false, true);  break; // blue  (B on)
        case 3: setLED(true,  false, false); break; // red   (R on)
        default: setLED(false, false, false); break;
    }
}

void ledsOff() { setLED(false, false, false); }

// ============================================================
//  Network / data fetch
// ============================================================

bool fetchData() {
    if (WiFi.status() != WL_CONNECTED) return false;

    HTTPClient http;
    char url[128];
    snprintf(url, sizeof(url), "http://%s:%d/api/antigravity", BRIDGE_HOST, BRIDGE_PORT);

    http.begin(url);
    http.setTimeout(8000);
    int code = http.GET();

    if (code != 200) {
        http.end();
        bridgeReachable = false;
        return false;
    }

    String payload = http.getString();
    http.end();

    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, payload);
    if (err) {
        bridgeReachable = false;
        return false;
    }

    bridgeReachable              = true;
    agy.valid                    = true;
    agy.status                   = doc["status"] | "unknown";
    agy.model                    = doc["model"]  | "gemini-2.5-pro";
    agy.last_activity            = doc["last_activity"] | "unknown";

    JsonObject session           = doc["session"];
    agy.conversations_today      = session["conversations_today"] | 0;
    agy.tool_calls_today         = session["tool_calls_today"]    | 0;
    agy.tokens_in                = session["tokens_in"]           | 0;
    agy.tokens_out               = session["tokens_out"]          | 0;
    agy.subagents_spawned        = session["subagents_spawned"]   | 0;
    agy.active_minutes           = session["active_minutes"]      | 0;

    JsonObject streak            = doc["streak"];
    agy.streak_days              = streak["days"] | 0;
    agy.streak_best              = streak["best"] | 0;

    return true;
}

// ============================================================
//  Drawing helpers
// ============================================================

void drawProgressBar(int x, int y, int w, int h, float pct, uint16_t fillColor) {
    if (pct < 0.0f) pct = 0.0f;
    if (pct > 1.0f) pct = 1.0f;

    // Background
    tft.fillRect(x, y, w, h, COL_BG);
    // Outline
    tft.drawRect(x, y, w, h, COL_DIM);
    // Fill
    int fillW = (int)((w - 2) * pct);
    if (fillW > 0) {
        tft.fillRect(x + 1, y + 1, fillW, h - 2, fillColor);
    }
}

/** Format large numbers: 48200 → "48.2K" */
String fmtNum(long n) {
    if (n >= 1000000L) {
        float m = n / 1000000.0f;
        char buf[12];
        snprintf(buf, sizeof(buf), "%.1fM", m);
        return String(buf);
    }
    if (n >= 1000L) {
        float k = n / 1000.0f;
        char buf[12];
        snprintf(buf, sizeof(buf), "%.1fK", k);
        return String(buf);
    }
    return String((int)n);
}

/** Format minutes: 94 → "1h 34m" */
String fmtMins(int mins) {
    if (mins <= 0) return "0m";
    if (mins < 60) {
        char buf[8];
        snprintf(buf, sizeof(buf), "%dm", mins);
        return String(buf);
    }
    char buf[12];
    snprintf(buf, sizeof(buf), "%dh %dm", mins / 60, mins % 60);
    return String(buf);
}

// ============================================================
//  Screen 0 — Live Session Dashboard
// ============================================================

void drawScreen0() {
    tft.fillScreen(COL_BG);

    // ---- Header ----
    tft.setTextSize(1);
    tft.setTextColor(COL_CYAN, COL_BG);
    tft.setTextDatum(TL_DATUM);
    tft.drawString("* ANTIGRAVITY", 6, 6, 2);

    // Live / Offline indicator
    if (bridgeReachable && agy.valid) {
        tft.setTextColor(COL_GREEN, COL_BG);
        tft.setTextDatum(TR_DATUM);
        tft.drawString("* LIVE", TFT_WIDTH - 6, 6, 2);
    } else {
        tft.setTextColor(COL_RED, COL_BG);
        tft.setTextDatum(TR_DATUM);
        tft.drawString("o OFFLINE", TFT_WIDTH - 6, 6, 2);
    }

    // Separator
    tft.drawFastHLine(0, 24, TFT_WIDTH, COL_CYAN);

    // ---- Metric rows ----
    struct MetricRow {
        const char* label;
        long        value;
        long        maxVal;
        uint16_t    barColor;
    };

    MetricRow rows[] = {
        { "TOKENS IN",  agy.tokens_in,         100000L, COL_CYAN   },
        { "TOKENS OUT", agy.tokens_out,          50000L, COL_GREEN  },
        { "TOOL CALLS", agy.tool_calls_today,      200L, COL_YELLOW },
        { "SUBAGENTS",  agy.subagents_spawned,      20L, COL_ORANGE },
    };

    int rowH   = 36;
    int startY = 30;

    for (int i = 0; i < 4; i++) {
        int y = startY + i * rowH;

        // Label
        tft.setTextDatum(TL_DATUM);
        tft.setTextColor(COL_WHITE, COL_BG);
        tft.drawString(rows[i].label, 6, y + 2, 1);

        // Value string (right-aligned)
        String valStr = fmtNum(rows[i].value);
        tft.setTextDatum(TR_DATUM);
        tft.setTextColor(rows[i].barColor, COL_BG);
        tft.drawString(valStr, TFT_WIDTH - 6, y + 2, 2);

        // Progress bar
        float pct = (rows[i].maxVal > 0) ? (float)rows[i].value / (float)rows[i].maxVal : 0.0f;
        int barX = 6;
        int barW = TFT_WIDTH - 70;
        drawProgressBar(barX, y + 18, barW, 10, pct, rows[i].barColor);

        // Separator tick
        if (i < 3) {
            tft.drawFastHLine(0, y + rowH - 1, TFT_WIDTH, 0x0820); // very dim line
        }
    }

    // ---- Footer ----
    int footerY = startY + 4 * rowH + 2;
    tft.drawFastHLine(0, footerY, TFT_WIDTH, COL_CYAN);

    tft.setTextDatum(TL_DATUM);
    tft.setTextColor(COL_DIM, COL_BG);

    String sessionStr = "SESSION: " + fmtMins(agy.active_minutes);
    tft.drawString(sessionStr, 6, footerY + 4, 1);

    // Model (right side)
    String modelShort = agy.model;
    if (modelShort.length() > 14) modelShort = modelShort.substring(0, 14);
    tft.setTextDatum(TR_DATUM);
    tft.drawString("MODEL: " + modelShort, TFT_WIDTH - 6, footerY + 4, 1);

    // Hint
    tft.setTextDatum(BC_DATUM);
    tft.setTextColor(0x2104, COL_BG); // very dim
    tft.drawString("TAP TO CYCLE SCREENS", TFT_WIDTH / 2, TFT_HEIGHT - 2, 1);
}

// ============================================================
//  Screen 1 — Streak & History
// ============================================================

void drawScreen1() {
    tft.fillScreen(COL_BG);

    // ---- Header ----
    tft.setTextDatum(TC_DATUM);
    tft.setTextColor(COL_CYAN, COL_BG);
    tft.drawString("STREAK & HISTORY", TFT_WIDTH / 2, 6, 2);

    tft.drawFastHLine(0, 24, TFT_WIDTH, COL_CYAN);

    // ---- Large streak display ----
    char streakBuf[32];
    snprintf(streakBuf, sizeof(streakBuf), "STREAK: %d DAYS", agy.streak_days);
    tft.setTextDatum(TC_DATUM);
    tft.setTextColor(COL_YELLOW, COL_BG);
    tft.drawString(String(streakBuf), TFT_WIDTH / 2, 32, 2);

    // Fire emoji simulation: coloured squares
    int fireX = TFT_WIDTH / 2 - 70;
    tft.fillRect(fireX - 20, 30, 10, 18, COL_RED);
    tft.fillRect(fireX - 20 + 3, 26, 4, 6, COL_YELLOW);

    // Best streak
    char bestBuf[32];
    snprintf(bestBuf, sizeof(bestBuf), "Best: %d days", agy.streak_best);
    tft.setTextDatum(TC_DATUM);
    tft.setTextColor(COL_DIM, COL_BG);
    tft.drawString(String(bestBuf), TFT_WIDTH / 2, 54, 1);

    tft.drawFastHLine(0, 68, TFT_WIDTH, COL_DIM);

    // ---- Today stats ----
    int y = 74;
    tft.setTextDatum(TL_DATUM);
    tft.setTextColor(COL_WHITE, COL_BG);

    char buf[64];
    snprintf(buf, sizeof(buf), "TODAY: %d convos  %d tools",
             agy.conversations_today, agy.tool_calls_today);
    tft.drawString(String(buf), 10, y, 1);

    y += 16;
    snprintf(buf, sizeof(buf), "SUBAGENTS: %d   Session: %s",
             agy.subagents_spawned, fmtMins(agy.active_minutes).c_str());
    tft.drawString(String(buf), 10, y, 1);

    y += 16;
    snprintf(buf, sizeof(buf), "LAST: %s", agy.last_activity.c_str());
    tft.setTextColor(COL_DIM, COL_BG);
    tft.drawString(String(buf), 10, y, 1);

    y += 20;
    tft.drawFastHLine(0, y, TFT_WIDTH, COL_DIM);
    y += 6;

    // Status badge
    tft.setTextDatum(TC_DATUM);
    if (agy.status == "active") {
        tft.fillRoundRect(TFT_WIDTH / 2 - 50, y, 100, 18, 4, COL_GREEN);
        tft.setTextColor(COL_BG, COL_GREEN);
        tft.drawString("STATUS: ACTIVE", TFT_WIDTH / 2, y + 3, 1);
    } else if (agy.status == "idle") {
        tft.fillRoundRect(TFT_WIDTH / 2 - 50, y, 100, 18, 4, COL_DIM);
        tft.setTextColor(COL_WHITE, COL_DIM);
        tft.drawString("STATUS: IDLE", TFT_WIDTH / 2, y + 3, 1);
    } else {
        tft.fillRoundRect(TFT_WIDTH / 2 - 50, y, 100, 18, 4, COL_RED);
        tft.setTextColor(COL_WHITE, COL_RED);
        tft.drawString("STATUS: " + agy.status, TFT_WIDTH / 2, y + 3, 1);
    }

    // Token summary
    y += 26;
    tft.setTextDatum(TL_DATUM);
    tft.setTextColor(COL_DIM, COL_BG);
    snprintf(buf, sizeof(buf), "IN: %s   OUT: %s",
             fmtNum(agy.tokens_in).c_str(), fmtNum(agy.tokens_out).c_str());
    tft.drawString(String(buf), 10, y, 1);

    // ---- Bottom nav ----
    tft.drawFastHLine(0, TFT_HEIGHT - 18, TFT_WIDTH, COL_DIM);
    tft.setTextDatum(BC_DATUM);
    tft.setTextColor(COL_DIM, COL_BG);
    tft.drawString("[<]  TAP TO CYCLE  [>]", TFT_WIDTH / 2, TFT_HEIGHT - 2, 1);
}

// ============================================================
//  Screen 2 — Standby / Clock
// ============================================================

void drawScreen2() {
    tft.fillScreen(COL_BG);

    // Dim border around clock area
    tft.drawRect(4, 4, TFT_WIDTH - 8, TFT_HEIGHT - 8, COL_DIM);
    tft.drawRect(5, 5, TFT_WIDTH - 10, TFT_HEIGHT - 10, 0x0820);

    // Title dim
    tft.setTextDatum(TC_DATUM);
    tft.setTextColor(COL_DIM, COL_BG);
    tft.drawString("ANTIGRAVITY", TFT_WIDTH / 2, 12, 2);

    // Thin separator
    tft.drawFastHLine(20, 30, TFT_WIDTH - 40, COL_DIM);

    // ---- Large time ----
    // Build HH:MM from NTP
    timeClient.update();
    unsigned long epoch    = timeClient.getEpochTime();
    int hh = (epoch % 86400L) / 3600;
    int mm = (epoch % 3600)   / 60;

    char timeBuf[6];
    snprintf(timeBuf, sizeof(timeBuf), "%02d:%02d", hh, mm);

    tft.setTextDatum(TC_DATUM);
    tft.setTextColor(COL_CYAN, COL_BG);
    tft.drawString(String(timeBuf), TFT_WIDTH / 2, 50, 7); // big font size

    // ---- Date ----
    // Day-of-week and date from epoch
    const char* days[]   = {"Sunday","Monday","Tuesday","Wednesday","Thursday","Friday","Saturday"};
    const char* months[] = {"Jan","Feb","Mar","Apr","May","Jun",
                            "Jul","Aug","Sep","Oct","Nov","Dec"};

    // days since epoch
    unsigned long daySinceEpoch = epoch / 86400L;
    int dow  = (daySinceEpoch + 4) % 7; // 1970-01-01 was Thursday (4)

    // Simple date computation
    unsigned long remaining = epoch;
    int year = 1970;
    while (true) {
        bool leap = (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0));
        unsigned long secsInYear = leap ? 31622400UL : 31536000UL;
        if (remaining < secsInYear) break;
        remaining -= secsInYear;
        year++;
    }
    int monthDays[] = {31,28,31,30,31,30,31,31,30,31,30,31};
    bool isLeap = (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0));
    if (isLeap) monthDays[1] = 29;
    int month = 0;
    for (month = 0; month < 12; month++) {
        unsigned long secsInMonth = (unsigned long)monthDays[month] * 86400UL;
        if (remaining < secsInMonth) break;
        remaining -= secsInMonth;
    }
    int day = (int)(remaining / 86400L) + 1;

    char dateBuf[32];
    snprintf(dateBuf, sizeof(dateBuf), "%s, %s %d", days[dow], months[month], day);

    tft.setTextDatum(TC_DATUM);
    tft.setTextColor(COL_WHITE, COL_BG);
    tft.drawString(String(dateBuf), TFT_WIDTH / 2, 155, 2);

    // ---- Thin divider ----
    tft.drawFastHLine(20, 175, TFT_WIDTH - 40, COL_DIM);

    // ---- Last session summary ----
    char summBuf[48];
    snprintf(summBuf, sizeof(summBuf), "Last: %d convos today", agy.conversations_today);
    tft.setTextDatum(TC_DATUM);
    tft.setTextColor(COL_DIM, COL_BG);
    tft.drawString(String(summBuf), TFT_WIDTH / 2, 183, 1);

    snprintf(summBuf, sizeof(summBuf), "Tools: %d   Subagents: %d",
             agy.tool_calls_today, agy.subagents_spawned);
    tft.drawString(String(summBuf), TFT_WIDTH / 2, 198, 1);

    // Tap hint
    tft.setTextDatum(BC_DATUM);
    tft.setTextColor(0x2104, COL_BG);
    tft.drawString("TAP TO CYCLE SCREENS", TFT_WIDTH / 2, TFT_HEIGHT - 6, 1);
}

// ============================================================
//  Splash / status screens
// ============================================================

void drawConnecting() {
    tft.fillScreen(COL_BG);
    tft.setTextDatum(TC_DATUM);
    tft.setTextColor(COL_CYAN, COL_BG);
    tft.drawString("ANTIGRAVITY", TFT_WIDTH / 2, 60, 4);
    tft.setTextColor(COL_WHITE, COL_BG);
    tft.drawString("VIBE MONITOR", TFT_WIDTH / 2, 100, 2);
    tft.drawFastHLine(40, 118, TFT_WIDTH - 80, COL_CYAN);
    tft.setTextColor(COL_DIM, COL_BG);
    tft.drawString("Connecting to WiFi...", TFT_WIDTH / 2, 128, 1);
}

void drawError(String msg) {
    tft.fillScreen(COL_BG);
    // Red border
    tft.drawRect(2, 2, TFT_WIDTH - 4, TFT_HEIGHT - 4, COL_RED);
    tft.drawRect(3, 3, TFT_WIDTH - 6, TFT_HEIGHT - 6, COL_RED);

    tft.setTextDatum(TC_DATUM);
    tft.setTextColor(COL_RED, COL_BG);
    tft.drawString("ERROR", TFT_WIDTH / 2, 50, 4);
    tft.setTextColor(COL_WHITE, COL_BG);
    tft.drawString(msg, TFT_WIDTH / 2, 110, 1);
    tft.setTextColor(COL_DIM, COL_BG);
    tft.drawString("Check bridge/monitor.py", TFT_WIDTH / 2, 130, 1);
    tft.drawString("and BRIDGE_HOST config", TFT_WIDTH / 2, 148, 1);
}

// ============================================================
//  Screen dispatch
// ============================================================

void drawCurrentScreen() {
    switch (currentScreen) {
        case 0: drawScreen0(); break;
        case 1: drawScreen1(); break;
        case 2: drawScreen2(); break;
        default: currentScreen = 0; drawScreen0(); break;
    }
}

// ============================================================
//  Touch handling
// ============================================================

void handleTouch() {
    if (!ts.tirqTouched()) return;
    if (!ts.touched())     return;

    unsigned long now = millis();
    if (now - lastTouch < 500UL) return; // debounce 500 ms
    lastTouch = now;

    TS_Point p = ts.getPoint();

    // Calibrate
    int tx = map(p.x, TOUCH_X_MIN, TOUCH_X_MAX, 0, TFT_WIDTH);
    int ty = map(p.y, TOUCH_Y_MIN, TOUCH_Y_MAX, 0, TFT_HEIGHT);

    // Any touch cycles the screen
    (void)tx; (void)ty;
    currentScreen = (currentScreen + 1) % 3;
    drawCurrentScreen();
}

// ============================================================
//  LED update logic
// ============================================================

void updateLED() {
    unsigned long now = millis();

    if (!bridgeReachable || !agy.valid) {
        // Red blink
        if (now - lastLedToggle > 500UL) {
            lastLedToggle = now;
            ledState = !ledState;
            if (ledState) setLEDColor(3); else ledsOff();
        }
        return;
    }

    if (agy.status == "active") {
        if (agy.tool_calls_today >= 50) {
            // Yellow pulse every 1 s
            if (now - lastLedToggle > 1000UL) {
                lastLedToggle = now;
                ledState = !ledState;
                if (ledState) setLEDColor(1); else ledsOff();
            }
        } else {
            // Cyan breathe every 2 s
            if (now - lastLedToggle > 2000UL) {
                lastLedToggle = now;
                ledState = !ledState;
                if (ledState) setLEDColor(0); else ledsOff();
            }
        }
    } else {
        // Blue solid
        setLEDColor(2);
    }
}

// ============================================================
//  setup()
// ============================================================

void setup() {
    Serial.begin(115200);
    Serial.println("[AGY Monitor] Booting...");

    // ---- LED pins ----
    pinMode(LED_R, OUTPUT);
    pinMode(LED_G, OUTPUT);
    pinMode(LED_B, OUTPUT);
    // Active LOW — HIGH = off
    digitalWrite(LED_R, HIGH);
    digitalWrite(LED_G, HIGH);
    digitalWrite(LED_B, HIGH);

    // ---- TFT ----
    tft.init();
    tft.setRotation(1);
    tft.fillScreen(COL_BG);

    // ---- Touch ----
    touchSPI.begin(TOUCH_CLK, TOUCH_MISO, TOUCH_MOSI, TOUCH_CS);
    ts.begin(touchSPI);
    ts.setRotation(1);

    // ---- WiFi ----
    drawConnecting();
    setLEDColor(2); // blue while connecting

    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    unsigned long wifiStart = millis();
    int dotX = 80;
    while (WiFi.status() != WL_CONNECTED && millis() - wifiStart < 20000UL) {
        delay(400);
        // Animate dots
        tft.setTextColor(COL_CYAN, COL_BG);
        tft.setTextDatum(TL_DATUM);
        tft.drawString(".", dotX, 150, 2);
        dotX += 10;
        if (dotX > 240) {
            tft.fillRect(80, 148, 200, 20, COL_BG);
            dotX = 80;
        }
    }

    if (WiFi.status() != WL_CONNECTED) {
        drawError("WiFi failed");
        setLEDColor(3);
        // Continue without WiFi — display error but don't hang
        return;
    }

    Serial.print("[AGY Monitor] WiFi connected: ");
    Serial.println(WiFi.localIP());

    // ---- NTP ----
    timeClient.begin();
    timeClient.update();

    // ---- Initial fetch ----
    setLEDColor(0);
    bool ok = fetchData();
    if (!ok) {
        Serial.println("[AGY Monitor] Initial fetch failed — bridge not reachable yet");
    }

    lastFetch = millis();
    drawCurrentScreen();
    Serial.println("[AGY Monitor] Ready.");
}

// ============================================================
//  loop()
// ============================================================

void loop() {
    // Touch
    handleTouch();

    unsigned long now = millis();

    // Periodic data fetch
    if (now - lastFetch >= FETCH_INTERVAL) {
        lastFetch = now;
        bool ok = fetchData();
        Serial.printf("[AGY Monitor] Fetch %s\n", ok ? "OK" : "FAIL");
        // Redraw current screen with fresh data (avoid redrawing clock too often)
        drawCurrentScreen();
    }

    // Clock screen: refresh every 30 s automatically
    if (currentScreen == 2 && now - lastFetch >= 30000UL) {
        drawScreen2();
    }

    // LED update
    updateLED();

    delay(20); // small yield
}
