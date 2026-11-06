// ============================================================
//  48 — Claude Code Vibe Monitor
//  ESP32-2432S028R (CYD 2.8") + ILI9341 + XPT2046
//  Reads Claude Code token metrics from local Python bridge
// ============================================================

#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <NTPClient.h>
#include <WiFiUDP.h>

// ===== USER CONFIGURATION =====
const char* WIFI_SSID     = "YOUR_WIFI_SSID";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";
const char* BRIDGE_HOST   = "192.168.1.100"; // IP of your Mac/PC running bridge/monitor.py
const int   BRIDGE_PORT   = 5048;
// ==============================

// ----- Display dimensions -----
#define TFT_WIDTH  320
#define TFT_HEIGHT 240

// ----- Color palette — Anthropic Claude orange theme -----
#define COL_BG      0x1000   // Very dark brown-black
#define COL_ORANGE  0xFB40   // #FF6B00 bright orange
#define COL_CORAL   0xFC62   // #FF8C42 coral
#define COL_WHITE   0xFFFF
#define COL_YELLOW  0xFFE0
#define COL_RED     0xF800
#define COL_DIM     0x4208
#define COL_GREEN   0x07E0
#define COL_AMBER   0xFE60   // Warning amber
#define COL_DARKRED 0x7800
#define COL_DARKORG 0x7920

// ----- Touch pins -----
#define TOUCH_CS   33
#define TOUCH_CLK  25
#define TOUCH_MISO 39
#define TOUCH_MOSI 32
#define TOUCH_IRQ  36

// ----- RGB LED pins (Active LOW) -----
#define LED_R 4
#define LED_G 16
#define LED_B 17

// ----- Data structure -----
struct ClaudeData {
    bool valid = false;
    // 5-hour window
    float  window5h_pct         = 0.0f;
    int    window5h_reset_min   = 0;
    long   window5h_tokens_used = 0;
    long   window5h_tokens_limit = 200000;
    // 7-day window
    float  window7d_pct         = 0.0f;
    long   window7d_tokens_used = 0;
    // Current session (today)
    long   input_tokens         = 0;
    long   output_tokens        = 0;
    long   cache_read_tokens    = 0;
    long   cache_write_tokens   = 0;
    float  cost_usd             = 0.0f;
    // Activity
    int    files_edited         = 0;
    int    commands_run         = 0;
    String status               = "unknown";
};

// ----- Globals -----
TFT_eSPI tft = TFT_eSPI();
SPIClass touchSPI(HSPI);
XPT2046_Touchscreen ts(TOUCH_CS, TOUCH_IRQ);

WiFiUDP   ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 0, 60000);

ClaudeData claude;

int           currentScreen    = 0;
unsigned long lastFetch        = 0;
unsigned long lastTouch        = 0;
unsigned long lastLEDToggle    = 0;
unsigned long lastBorderPulse  = 0;
bool          ledState         = false;
bool          borderVisible    = false;
bool          bridgeReachable  = false;
bool          wifiConnected    = false;

const unsigned long FETCH_INTERVAL = 30000;

// ============================================================
//  LED helpers
// ============================================================

void setLED(bool r, bool g, bool b) {
    // Active-LOW: LOW = ON
    digitalWrite(LED_R, r ? LOW : HIGH);
    digitalWrite(LED_G, g ? LOW : HIGH);
    digitalWrite(LED_B, b ? LOW : HIGH);
}

void updateLED() {
    if (!wifiConnected || !bridgeReachable || !claude.valid) {
        // Blue dim solid — offline
        setLED(false, false, true);
        return;
    }
    if (claude.status == "idle") {
        // Green solid — idle
        setLED(false, true, false);
        return;
    }

    unsigned long now = millis();
    unsigned long interval = 2000;

    if (claude.window5h_pct > 80.0f) {
        interval = 300; // rapid red blink
    } else if (claude.window5h_pct > 50.0f) {
        interval = 800; // amber fast pulse
    } else {
        interval = 2000; // orange breathe
    }

    if (now - lastLEDToggle >= interval) {
        ledState = !ledState;
        lastLEDToggle = now;
    }

    if (!ledState) {
        setLED(false, false, false);
        return;
    }

    if (claude.window5h_pct > 80.0f) {
        setLED(true, false, false);        // red
    } else if (claude.window5h_pct > 50.0f) {
        setLED(true, true, false);         // amber (R+G)
    } else {
        setLED(true, false, false);        // orange = R on
    }
}

// ============================================================
//  Token / time formatters
// ============================================================

String formatTokens(long tokens) {
    if (tokens >= 1000000) {
        return String(tokens / 100000 / 10.0f, 1) + "M";
    } else if (tokens >= 1000) {
        return String(tokens / 100 / 10.0f, 1) + "K";
    }
    return String(tokens);
}

String formatTime(int minutes) {
    if (minutes <= 0) return "0m";
    int h = minutes / 60;
    int m = minutes % 60;
    if (h == 0) return String(m) + "m";
    return String(h) + "h " + String(m) + "m";
}

String formatCost(float cost) {
    char buf[12];
    snprintf(buf, sizeof(buf), "$%.3f", cost);
    return String(buf);
}

// ============================================================
//  Progress bar
// ============================================================

void drawProgressBar(int x, int y, int w, int h, float pct, uint16_t color) {
    // Clamp
    if (pct < 0.0f) pct = 0.0f;
    if (pct > 100.0f) pct = 100.0f;

    // Dynamic color based on percentage
    uint16_t fillColor = color;
    if (pct > 80.0f) {
        fillColor = COL_RED;
    } else if (pct > 50.0f) {
        fillColor = COL_AMBER;
    }

    // Background + border
    tft.drawRect(x, y, w, h, COL_DIM);
    tft.fillRect(x + 1, y + 1, w - 2, h - 2, COL_BG);

    // Fill
    int fillW = (int)((w - 2) * pct / 100.0f);
    if (fillW > 0) {
        tft.fillRect(x + 1, y + 1, fillW, h - 2, fillColor);
    }

    // Overlay percentage text if bar is tall enough
    if (h >= 16) {
        char buf[8];
        snprintf(buf, sizeof(buf), "%.0f%%", pct);
        tft.setTextColor(COL_WHITE);
        tft.setTextSize(1);
        int tw = strlen(buf) * 6;
        int tx = x + (w - tw) / 2;
        int ty = y + (h - 7) / 2;
        tft.setCursor(tx, ty);
        tft.print(buf);
    }
}

// ============================================================
//  Network fetch
// ============================================================

bool fetchData() {
    if (WiFi.status() != WL_CONNECTED) {
        bridgeReachable = false;
        return false;
    }
    HTTPClient http;
    char url[128];
    snprintf(url, sizeof(url), "http://%s:%d/api/claude", BRIDGE_HOST, BRIDGE_PORT);
    http.begin(url);
    http.setTimeout(8000);
    int code = http.GET();
    if (code != 200) {
        http.end();
        bridgeReachable = false;
        claude.valid = false;
        return false;
    }
    String payload = http.getString();
    http.end();

    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, payload);
    if (err) {
        bridgeReachable = false;
        claude.valid = false;
        return false;
    }

    bridgeReachable = true;
    claude.valid = true;

    // 5-hour window
    JsonObject w5 = doc["window_5h"];
    claude.window5h_pct         = w5["used_pct"]    | 0.0f;
    claude.window5h_reset_min   = w5["reset_in_min"] | 0;
    claude.window5h_tokens_used = w5["tokens_used"]  | 0L;
    claude.window5h_tokens_limit= w5["tokens_limit"] | 200000L;

    // 7-day window
    JsonObject w7 = doc["window_7d"];
    claude.window7d_pct         = w7["used_pct"]    | 0.0f;
    claude.window7d_tokens_used = w7["tokens_used"]  | 0L;

    // Session
    JsonObject sess = doc["session"];
    claude.input_tokens       = sess["input_tokens"]       | 0L;
    claude.output_tokens      = sess["output_tokens"]      | 0L;
    claude.cache_read_tokens  = sess["cache_read_tokens"]  | 0L;
    claude.cache_write_tokens = sess["cache_write_tokens"] | 0L;
    claude.cost_usd           = sess["cost_usd"]           | 0.0f;

    // Activity
    claude.files_edited = doc["files_edited"] | 0;
    claude.commands_run = doc["commands_run"] | 0;
    claude.status       = doc["status"].as<String>();

    return true;
}

// ============================================================
//  Screen 0 — 5-Hour Window Gauge (PRIMARY)
// ============================================================

void drawScreen0() {
    tft.fillScreen(COL_BG);

    // --- Header bar ---
    tft.fillRect(0, 0, TFT_WIDTH, 22, COL_DARKORG);
    tft.setTextColor(COL_WHITE);
    tft.setTextSize(1);
    tft.setCursor(6, 7);
    tft.print("* CLAUDE CODE");

    // Status indicator right side
    if (bridgeReachable && claude.valid) {
        tft.setTextColor(COL_GREEN);
        tft.setCursor(240, 7);
        tft.print("* LIVE");
    } else {
        tft.setTextColor(COL_RED);
        tft.setCursor(228, 7);
        tft.print("o OFFLINE");
    }

    // Orange separator line
    tft.drawFastHLine(0, 23, TFT_WIDTH, COL_ORANGE);

    // --- 5-HOUR WINDOW section ---
    tft.setTextColor(COL_ORANGE);
    tft.setTextSize(1);
    tft.setCursor(85, 30);
    tft.print("5-HOUR WINDOW");

    // Big progress bar
    int barX = 10, barY = 42, barW = TFT_WIDTH - 20, barH = 28;
    drawProgressBar(barX, barY, barW, barH, claude.window5h_pct, COL_ORANGE);

    // Tokens + reset info
    char tokenLine[64];
    snprintf(tokenLine, sizeof(tokenLine), "%s / %s   Resets: %s",
        formatTokens(claude.window5h_tokens_used).c_str(),
        formatTokens(claude.window5h_tokens_limit).c_str(),
        formatTime(claude.window5h_reset_min).c_str()
    );
    tft.setTextColor(COL_DIM);
    tft.setTextSize(1);
    tft.setCursor(10, 74);
    tft.print(tokenLine);

    // Separator
    tft.drawFastHLine(10, 86, TFT_WIDTH - 20, COL_DARKORG);

    // --- 7-DAY WINDOW section ---
    tft.setTextColor(COL_CORAL);
    tft.setTextSize(1);
    tft.setCursor(100, 92);
    tft.print("7-DAY WINDOW");

    int bar7X = 10, bar7Y = 104, bar7W = TFT_WIDTH - 20, bar7H = 16;
    drawProgressBar(bar7X, bar7Y, bar7W, bar7H, claude.window7d_pct, COL_CORAL);

    char w7line[48];
    snprintf(w7line, sizeof(w7line), "%s tokens used this week",
        formatTokens(claude.window7d_tokens_used).c_str());
    tft.setTextColor(COL_DIM);
    tft.setCursor(10, 124);
    tft.print(w7line);

    // Separator
    tft.drawFastHLine(10, 136, TFT_WIDTH - 20, COL_DARKORG);

    // --- Bottom status bar ---
    char bottomLine[64];
    snprintf(bottomLine, sizeof(bottomLine), "COST: %s   FILES: %d   CMDS: %d",
        formatCost(claude.cost_usd).c_str(),
        claude.files_edited,
        claude.commands_run
    );
    tft.setTextColor(COL_YELLOW);
    tft.setTextSize(1);
    int bw = strlen(bottomLine) * 6;
    tft.setCursor((TFT_WIDTH - bw) / 2, 144);
    tft.print(bottomLine);

    // Session status badge
    tft.drawRoundRect(10, 158, 80, 16, 4, COL_ORANGE);
    tft.setTextColor(COL_ORANGE);
    tft.setCursor(14, 162);
    tft.print("STATUS: ");
    tft.setTextColor(COL_WHITE);
    tft.print(claude.status);

    // Fetch time hint
    tft.setTextColor(COL_DIM);
    tft.setTextSize(1);
    tft.setCursor(6, 184);
    tft.print("Updates every 30s");

    // Hint bottom
    tft.setTextColor(COL_DIM);
    tft.setTextSize(1);
    int hw = 19 * 6;
    tft.setCursor((TFT_WIDTH - hw) / 2, 228);
    tft.print("TAP TO CYCLE SCREENS");
}

// ============================================================
//  Screen 1 — Token Economics Breakdown
// ============================================================

void drawScreen1() {
    tft.fillScreen(COL_BG);

    // Header
    tft.fillRect(0, 0, TFT_WIDTH, 22, COL_DARKORG);
    tft.setTextColor(COL_WHITE);
    tft.setTextSize(1);
    tft.setCursor(6, 7);
    tft.print("$ SESSION ECONOMICS");

    if (bridgeReachable && claude.valid) {
        tft.setTextColor(COL_GREEN);
        tft.setCursor(252, 7);
        tft.print("LIVE");
    } else {
        tft.setTextColor(COL_RED);
        tft.setCursor(228, 7);
        tft.print("OFFLINE");
    }

    tft.drawFastHLine(0, 23, TFT_WIDTH, COL_ORANGE);

    // Pricing constants (per-token)
    const float P_INPUT  = 0.000003f;
    const float P_OUTPUT = 0.000015f;
    const float P_CACHEW = 0.00000375f;
    const float P_CACHER = 0.0000003f;

    struct TokenRow {
        const char* label;
        long        tokens;
        float       cost;
        uint16_t    color;
    };

    long maxTokens = max((long)1, max(claude.input_tokens,
                         max(claude.output_tokens,
                         max(claude.cache_read_tokens, claude.cache_write_tokens))));

    TokenRow rows[4] = {
        { "INPUT  ", claude.input_tokens,       claude.input_tokens       * P_INPUT,  COL_ORANGE },
        { "OUTPUT ", claude.output_tokens,      claude.output_tokens      * P_OUTPUT, COL_CORAL  },
        { "CACHE+ ", claude.cache_write_tokens, claude.cache_write_tokens * P_CACHEW, COL_AMBER  },
        { "CACHE- ", claude.cache_read_tokens,  claude.cache_read_tokens  * P_CACHER, COL_DIM    },
    };

    int rowY = 30;
    int rowH = 36;

    for (int i = 0; i < 4; i++) {
        int y = rowY + i * rowH;

        // Label
        tft.setTextColor(rows[i].color);
        tft.setTextSize(1);
        tft.setCursor(6, y + 4);
        tft.print(rows[i].label);

        // Mini bar
        int bx = 60, bw2 = 120, bh2 = 10;
        float bpct = (maxTokens > 0) ? (float)rows[i].tokens / maxTokens * 100.0f : 0.0f;
        tft.drawRect(bx, y + 2, bw2, bh2, COL_DIM);
        int fill = (int)(bw2 * bpct / 100.0f);
        if (fill > 0) tft.fillRect(bx + 1, y + 3, fill - 1, bh2 - 2, rows[i].color);

        // Tokens
        tft.setTextColor(COL_WHITE);
        tft.setTextSize(1);
        tft.setCursor(186, y + 4);
        tft.print(formatTokens(rows[i].tokens));

        // Cost
        tft.setTextColor(COL_YELLOW);
        char cbuf[10];
        snprintf(cbuf, sizeof(cbuf), "$%.4f", rows[i].cost);
        tft.setCursor(252, y + 4);
        tft.print(cbuf);

        // Separator
        if (i < 3) tft.drawFastHLine(6, y + rowH - 4, TFT_WIDTH - 12, 0x2104);
    }

    // Total separator
    int sepY = rowY + 4 * rowH - 2;
    tft.drawFastHLine(0, sepY, TFT_WIDTH, COL_ORANGE);

    long totalTokens = claude.input_tokens + claude.output_tokens +
                       claude.cache_read_tokens + claude.cache_write_tokens;

    tft.setTextColor(COL_ORANGE);
    tft.setTextSize(1);
    tft.setCursor(6, sepY + 6);
    tft.print("TOTAL");

    tft.setTextColor(COL_WHITE);
    tft.setCursor(60, sepY + 6);
    tft.print(formatTokens(totalTokens));
    tft.print(" tokens");

    tft.setTextColor(COL_YELLOW);
    tft.setCursor(200, sepY + 6);
    tft.print(formatCost(claude.cost_usd));

    tft.setTextColor(COL_DIM);
    tft.setCursor(6, sepY + 18);
    char actLine[48];
    snprintf(actLine, sizeof(actLine), "FILES EDITED: %d   CMDS RUN: %d",
        claude.files_edited, claude.commands_run);
    tft.print(actLine);

    // Hint
    tft.setTextColor(COL_DIM);
    int hw = 19 * 6;
    tft.setCursor((TFT_WIDTH - hw) / 2, 228);
    tft.print("TAP TO CYCLE SCREENS");
}

// ============================================================
//  Screen 2 — Warning or Standby
// ============================================================

void drawScreen2() {
    if (claude.valid && claude.window5h_pct > 80.0f) {
        // WARNING screen
        tft.fillScreen(COL_BG);

        // Red border
        for (int t = 0; t < 4; t++) {
            tft.drawRect(t, t, TFT_WIDTH - 2 * t, TFT_HEIGHT - 2 * t, COL_RED);
        }

        // Big WARNING header
        tft.fillRect(0, 0, TFT_WIDTH, 22, COL_DARKRED);
        tft.setTextColor(COL_WHITE);
        tft.setTextSize(1);
        tft.setCursor(90, 7);
        tft.print("!! WARNING !!");

        // Big text
        tft.setTextColor(COL_RED);
        tft.setTextSize(2);
        tft.setCursor(42, 36);
        tft.print("! SLOW DOWN!");

        // Percentage
        tft.setTextColor(COL_AMBER);
        tft.setTextSize(3);
        char pctBuf[8];
        snprintf(pctBuf, sizeof(pctBuf), "%.0f%%", claude.window5h_pct);
        int pw = strlen(pctBuf) * 18;
        tft.setCursor((TFT_WIDTH - pw) / 2, 70);
        tft.print(pctBuf);

        tft.setTextColor(COL_WHITE);
        tft.setTextSize(1);
        tft.setCursor(70, 100);
        tft.print("of 5h window used");

        // Countdown
        tft.setTextColor(COL_CORAL);
        tft.setTextSize(2);
        String resetStr = formatTime(claude.window5h_reset_min);
        int rw = resetStr.length() * 12;
        tft.setCursor((TFT_WIDTH - rw) / 2, 118);
        tft.print(resetStr);

        tft.setTextColor(COL_DIM);
        tft.setTextSize(1);
        tft.setCursor(90, 142);
        tft.print("until window resets");

        // Tokens used bar
        drawProgressBar(10, 160, TFT_WIDTH - 20, 20, claude.window5h_pct, COL_RED);

        char tkLine[48];
        snprintf(tkLine, sizeof(tkLine), "%s / %s tokens used",
            formatTokens(claude.window5h_tokens_used).c_str(),
            formatTokens(claude.window5h_tokens_limit).c_str());
        tft.setTextColor(COL_DIM);
        tft.setTextSize(1);
        int tlen = strlen(tkLine) * 6;
        tft.setCursor((TFT_WIDTH - tlen) / 2, 184);
        tft.print(tkLine);

        tft.setTextColor(COL_DIM);
        int hw = 19 * 6;
        tft.setCursor((TFT_WIDTH - hw) / 2, 228);
        tft.print("TAP TO CYCLE SCREENS");

    } else {
        // Standby / clock screen
        tft.fillScreen(COL_BG);

        // Header
        tft.fillRect(0, 0, TFT_WIDTH, 22, COL_DARKORG);
        tft.setTextColor(COL_WHITE);
        tft.setTextSize(1);
        tft.setCursor(6, 7);
        tft.print("* CLAUDE CODE — STANDBY");

        if (!bridgeReachable) {
            tft.setTextColor(COL_RED);
            int bw = 14 * 6;
            tft.setCursor(TFT_WIDTH - bw - 4, 7);
            tft.print("BRIDGE OFFLINE");
        }

        tft.drawFastHLine(0, 23, TFT_WIDTH, COL_ORANGE);

        // NTP clock
        timeClient.update();
        String timeStr = timeClient.getFormattedTime();
        tft.setTextColor(COL_ORANGE);
        tft.setTextSize(3);
        int tw = timeStr.length() * 18;
        tft.setCursor((TFT_WIDTH - tw) / 2, 50);
        tft.print(timeStr);

        // Date (epoch → human)
        unsigned long epoch = timeClient.getEpochTime();
        time_t t = (time_t)epoch;
        struct tm* tmInfo = gmtime(&t);
        char dateBuf[24];
        strftime(dateBuf, sizeof(dateBuf), "%A, %d %b %Y", tmInfo);
        tft.setTextColor(COL_DIM);
        tft.setTextSize(1);
        int dw = strlen(dateBuf) * 6;
        tft.setCursor((TFT_WIDTH - dw) / 2, 94);
        tft.print(dateBuf);

        // Separator
        tft.drawFastHLine(10, 108, TFT_WIDTH - 20, COL_DARKORG);

        // Last session summary
        tft.setTextColor(COL_CORAL);
        tft.setTextSize(1);
        tft.setCursor(80, 116);
        tft.print("LAST SESSION SUMMARY");

        char summary1[48];
        snprintf(summary1, sizeof(summary1), "Tokens: %s in / %s out",
            formatTokens(claude.input_tokens).c_str(),
            formatTokens(claude.output_tokens).c_str());
        tft.setTextColor(COL_WHITE);
        tft.setCursor(10, 132);
        tft.print(summary1);

        char summary2[48];
        snprintf(summary2, sizeof(summary2), "Cost today: %s",
            formatCost(claude.cost_usd).c_str());
        tft.setTextColor(COL_YELLOW);
        tft.setCursor(10, 148);
        tft.print(summary2);

        char summary3[48];
        snprintf(summary3, sizeof(summary3), "Files edited: %d   Commands: %d",
            claude.files_edited, claude.commands_run);
        tft.setTextColor(COL_DIM);
        tft.setCursor(10, 164);
        tft.print(summary3);

        // Status
        tft.drawRoundRect(10, 180, 100, 16, 4,
            (claude.status == "active") ? COL_GREEN : COL_DIM);
        tft.setTextColor((claude.status == "active") ? COL_GREEN : COL_DIM);
        tft.setCursor(14, 184);
        tft.print("STATUS: ");
        tft.print(claude.status);

        tft.setTextColor(COL_DIM);
        int hw = 19 * 6;
        tft.setCursor((TFT_WIDTH - hw) / 2, 228);
        tft.print("TAP TO CYCLE SCREENS");
    }
}

// ============================================================
//  Connecting / Error screens
// ============================================================

void drawConnecting() {
    tft.fillScreen(COL_BG);
    tft.fillRect(0, 0, TFT_WIDTH, 22, COL_DARKORG);
    tft.setTextColor(COL_WHITE);
    tft.setTextSize(1);
    tft.setCursor(6, 7);
    tft.print("* CLAUDE CODE VIBE MONITOR");
    tft.drawFastHLine(0, 23, TFT_WIDTH, COL_ORANGE);

    tft.setTextColor(COL_ORANGE);
    tft.setTextSize(2);
    tft.setCursor(50, 70);
    tft.print("Connecting...");

    tft.setTextColor(COL_DIM);
    tft.setTextSize(1);
    tft.setCursor(50, 100);
    tft.print("SSID: ");
    tft.print(WIFI_SSID);

    // Animated dots row
    static int dots = 0;
    tft.setTextColor(COL_ORANGE);
    tft.setCursor(100, 130);
    for (int i = 0; i < dots % 4; i++) tft.print(".");
    dots++;
}

void drawError(String msg) {
    tft.fillScreen(COL_BG);
    tft.fillRect(0, 0, TFT_WIDTH, 22, COL_DARKRED);
    tft.setTextColor(COL_WHITE);
    tft.setTextSize(1);
    tft.setCursor(6, 7);
    tft.print("* ERROR");
    tft.drawFastHLine(0, 23, TFT_WIDTH, COL_RED);

    tft.setTextColor(COL_RED);
    tft.setTextSize(2);
    tft.setCursor(16, 60);
    tft.print("Connection Error");

    tft.setTextColor(COL_WHITE);
    tft.setTextSize(1);
    // Word-wrap the message manually
    int cursor = 0;
    int lineY = 100;
    while (cursor < (int)msg.length()) {
        int end = min((int)msg.length(), cursor + 40);
        tft.setCursor(10, lineY);
        tft.print(msg.substring(cursor, end));
        cursor = end;
        lineY += 14;
    }

    tft.setTextColor(COL_DIM);
    tft.setCursor(10, 180);
    tft.print("Check WiFi / bridge settings");
    tft.setCursor(10, 196);
    tft.print("Retrying automatically...");
}

// ============================================================
//  Touch handler
// ============================================================

void handleTouch() {
    if (!ts.tirqTouched()) return;
    if (!ts.touched()) return;

    unsigned long now = millis();
    if (now - lastTouch < 500) return;

    TS_Point p = ts.getPoint();
    int tx = map(p.x, 240, 3800, 0, 320);
    int ty = map(p.y, 240, 3800, 0, 240);

    // Accept any touch on the screen
    if (tx >= 0 && tx <= 320 && ty >= 0 && ty <= 240) {
        lastTouch = now;
        currentScreen = (currentScreen + 1) % 3;
        // Redraw immediately
        switch (currentScreen) {
            case 0: drawScreen0(); break;
            case 1: drawScreen1(); break;
            case 2: drawScreen2(); break;
        }
    }
}

// ============================================================
//  Redraw current screen
// ============================================================

void redrawCurrentScreen() {
    switch (currentScreen) {
        case 0: drawScreen0(); break;
        case 1: drawScreen1(); break;
        case 2: drawScreen2(); break;
    }
}

// ============================================================
//  setup()
// ============================================================

void setup() {
    Serial.begin(115200);
    Serial.println("[Claude Vibe Monitor] Booting...");

    // LED pins — start OFF (active LOW so HIGH = off)
    pinMode(LED_R, OUTPUT); digitalWrite(LED_R, HIGH);
    pinMode(LED_G, OUTPUT); digitalWrite(LED_G, HIGH);
    pinMode(LED_B, OUTPUT); digitalWrite(LED_B, HIGH);

    // TFT
    tft.init();
    tft.setRotation(1);
    tft.fillScreen(COL_BG);
    tft.setTextWrap(false);

    // Touch (HSPI)
    touchSPI.begin(TOUCH_CLK, TOUCH_MISO, TOUCH_MOSI, TOUCH_CS);
    ts.begin(touchSPI);
    ts.setRotation(1);

    // Show boot logo
    tft.fillRect(0, 0, TFT_WIDTH, 22, COL_DARKORG);
    tft.setTextColor(COL_WHITE);
    tft.setTextSize(1);
    tft.setCursor(6, 7);
    tft.print("* CLAUDE CODE VIBE MONITOR v1.0");
    tft.drawFastHLine(0, 23, TFT_WIDTH, COL_ORANGE);
    tft.setTextColor(COL_ORANGE);
    tft.setTextSize(2);
    tft.setCursor(28, 80);
    tft.print("Claude Code");
    tft.setTextColor(COL_CORAL);
    tft.setCursor(52, 108);
    tft.print("Vibe Monitor");
    tft.setTextColor(COL_DIM);
    tft.setTextSize(1);
    tft.setCursor(70, 142);
    tft.print("By CYD Community");

    // WiFi connect
    Serial.printf("[WiFi] Connecting to %s\n", WIFI_SSID);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    unsigned long wStart = millis();
    bool shown = false;
    while (WiFi.status() != WL_CONNECTED && millis() - wStart < 20000) {
        if (!shown) { drawConnecting(); shown = true; }
        delay(500);
        Serial.print(".");
        tft.setTextColor(COL_ORANGE);
        tft.setCursor(100 + ((millis() - wStart) / 500 % 10) * 6, 130);
        tft.print(".");
    }

    if (WiFi.status() == WL_CONNECTED) {
        wifiConnected = true;
        Serial.printf("\n[WiFi] Connected. IP: %s\n", WiFi.localIP().toString().c_str());

        // NTP
        timeClient.begin();
        timeClient.update();

        // Initial fetch
        if (!fetchData()) {
            Serial.println("[Bridge] Initial fetch failed");
        }
        drawScreen0();
    } else {
        wifiConnected = false;
        Serial.println("\n[WiFi] Connection failed!");
        drawError("WiFi connection timed out.\nCheck SSID/password in\nsrc/main.cpp");
    }
}

// ============================================================
//  loop()
// ============================================================

void loop() {
    // Touch
    handleTouch();

    unsigned long now = millis();

    // WiFi reconnect watchdog
    if (WiFi.status() != WL_CONNECTED) {
        wifiConnected = false;
        bridgeReachable = false;
    } else {
        wifiConnected = true;
    }

    // Periodic data fetch
    if (now - lastFetch >= FETCH_INTERVAL) {
        lastFetch = now;
        bool ok = fetchData();
        Serial.printf("[Fetch] %s — 5h:%.1f%% cost:$%.3f\n",
            ok ? "OK" : "FAIL",
            claude.window5h_pct,
            claude.cost_usd);
        redrawCurrentScreen();
    }

    // LED update
    updateLED();

    // Pulsing red border on screen2 when >80%
    if (currentScreen == 2 && claude.valid && claude.window5h_pct > 80.0f) {
        if (now - lastBorderPulse >= 500) {
            lastBorderPulse = now;
            borderVisible = !borderVisible;
            uint16_t borderCol = borderVisible ? COL_RED : COL_BG;
            for (int t = 0; t < 4; t++) {
                tft.drawRect(t, t, TFT_WIDTH - 2 * t, TFT_HEIGHT - 2 * t, borderCol);
            }
        }
    }

    // NTP refresh for standby screen
    if (currentScreen == 2) {
        timeClient.update();
    }

    delay(20);
}
