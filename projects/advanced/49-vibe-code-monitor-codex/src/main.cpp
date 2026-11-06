#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include <SPI.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <NTPClient.h>
#include <WiFiUDP.h>

// ===== USER CONFIGURATION =====
const char* WIFI_SSID     = "YOUR_WIFI_SSID";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";
const char* BRIDGE_HOST   = "192.168.1.100"; // IP of your Mac/PC running bridge/monitor.py
const int   BRIDGE_PORT   = 5049;
// ==============================

// Display dimensions
#define TFT_WIDTH  320
#define TFT_HEIGHT 240

// Colors — Matrix terminal green theme (RGB565)
#define COL_BG       0x0000   // Pure black
#define COL_GREEN    0x07E0   // Bright green #00FF00
#define COL_EMERALD  0x0640   // Darker green #00C853
#define COL_WHITE    0xFFFF
#define COL_YELLOW   0xFFE0
#define COL_RED      0xF800
#define COL_DIM      0x2104   // Very dark green-gray for dim text
#define COL_CYAN     0x07FF
#define COL_MAGENTA  0xF81F   // Accent for reasoning tokens

// Touch pins (HSPI)
#define TOUCH_CS   33
#define TOUCH_CLK  25
#define TOUCH_MISO 39
#define TOUCH_MOSI 32
#define TOUCH_IRQ  36

// RGB LED pins (Active LOW)
#define LED_R 4
#define LED_G 16
#define LED_B 17

// Global objects
TFT_eSPI tft = TFT_eSPI();
SPIClass touchSPI(HSPI);
XPT2046_Touchscreen touch(TOUCH_CS, TOUCH_IRQ);
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 0, 60000);

// Data structure
struct CodexData {
    bool valid = false;
    // 5-hour window
    float window5h_pct = 0.0f;
    int   window5h_reset_min = 0;
    long  window5h_tokens_used = 0;
    // 7-day window
    float window7d_pct = 0.0f;
    // Session tokens (today)
    long  input_tokens = 0;
    long  output_tokens = 0;
    long  reasoning_tokens = 0;
    long  cached_input_tokens = 0;
    float cost_usd = 0.0f;
    float cache_hit_pct = 0.0f;
    // Activity
    int   tasks_completed = 0;
    int   patches_applied = 0;
    String model = "o4-mini";
    String status = "unknown";
    // Recent tasks (last 5)
    String recent_tasks[5];
    int task_count = 0;
};

CodexData codex;
int currentScreen = 0;         // 0=command center, 1=reasoning breakdown, 2=task history
unsigned long lastFetch = 0;
unsigned long lastTouch = 0;
unsigned long lastLEDToggle = 0;
bool ledState = false;
bool bridgeReachable = false;
const unsigned long FETCH_INTERVAL = 30000;

// Blinking cursor state for screen2 clock
unsigned long lastCursorToggle = 0;
bool cursorVisible = true;

// =====================================================================
// LED HELPERS
// =====================================================================
void setLED(bool r, bool g, bool b) {
    // Active LOW: LOW = ON
    digitalWrite(LED_R, r ? LOW : HIGH);
    digitalWrite(LED_G, g ? LOW : HIGH);
    digitalWrite(LED_B, b ? LOW : HIGH);
}

void updateLED() {
    unsigned long now = millis();

    if (!codex.valid) {
        // Solid red — bridge offline
        setLED(true, false, false);
        return;
    }

    if (codex.window5h_pct > 90.0f) {
        // Red rapid blink (200ms)
        if (now - lastLEDToggle >= 200) {
            ledState = !ledState;
            lastLEDToggle = now;
        }
        setLED(ledState, false, false);
    } else if (codex.window5h_pct > 70.0f) {
        // Yellow pulse (700ms)
        if (now - lastLEDToggle >= 700) {
            ledState = !ledState;
            lastLEDToggle = now;
        }
        if (ledState) setLED(true, true, false);
        else          setLED(false, false, false);
    } else if (codex.status == "active") {
        // Green breathe (1500ms toggle)
        if (now - lastLEDToggle >= 1500) {
            ledState = !ledState;
            lastLEDToggle = now;
        }
        if (ledState) setLED(false, true, false);
        else          setLED(false, false, false);
    } else {
        // Idle: green dim — 1s on, 3s off
        unsigned long cyclePos = (now / 1000) % 4;
        if (cyclePos == 0) setLED(false, true, false);
        else               setLED(false, false, false);
    }
}

// =====================================================================
// FORMATTING HELPERS
// =====================================================================
String formatTokens(long tokens) {
    if (tokens < 1000) {
        return String(tokens);
    } else if (tokens < 1000000) {
        float k = tokens / 1000.0f;
        char buf[16];
        snprintf(buf, sizeof(buf), "%.1fK", k);
        return String(buf);
    } else {
        float m = tokens / 1000000.0f;
        char buf[16];
        snprintf(buf, sizeof(buf), "%.1fM", m);
        return String(buf);
    }
}

String formatTime(int minutes) {
    if (minutes <= 0) return "0m";
    if (minutes < 60) {
        return String(minutes) + "m";
    }
    int h = minutes / 60;
    int m = minutes % 60;
    return String(h) + "h " + String(m) + "m";
}

String formatCost(float cost) {
    char buf[16];
    snprintf(buf, sizeof(buf), "$%.3f", cost);
    return String(buf);
}

// =====================================================================
// DRAW HELPERS
// =====================================================================
void drawProgressBar(int x, int y, int w, int h, float pct, uint16_t color) {
    // Outline
    tft.drawRect(x, y, w, h, COL_DIM);
    // Fill
    int fillW = (int)((w - 2) * constrain(pct / 100.0f, 0.0f, 1.0f));
    if (fillW > 0) {
        tft.fillRect(x + 1, y + 1, fillW, h - 2, color);
    }
    // Clear unfilled area
    tft.fillRect(x + 1 + fillW, y + 1, (w - 2) - fillW, h - 2, COL_BG);
}

void drawBridgeOfflineBanner() {
    tft.setTextColor(COL_RED, COL_BG);
    tft.setTextSize(1);
    tft.setCursor(TFT_WIDTH - 90, 2);
    tft.print("[BRIDGE OFFLINE]");
}

// =====================================================================
// DATA FETCH
// =====================================================================
bool fetchData() {
    if (WiFi.status() != WL_CONNECTED) {
        WiFi.reconnect();
        return false;
    }

    HTTPClient http;
    char url[128];
    snprintf(url, sizeof(url), "http://%s:%d/api/codex", BRIDGE_HOST, BRIDGE_PORT);

    http.begin(url);
    http.setTimeout(5000);
    int code = http.GET();

    if (code != 200) {
        http.end();
        bridgeReachable = false;
        codex.valid = false;
        return false;
    }

    String payload = http.getString();
    http.end();

    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, payload);
    if (err) {
        bridgeReachable = false;
        codex.valid = false;
        return false;
    }

    bridgeReachable = true;
    codex.valid = true;

    // 5h window
    codex.window5h_pct         = doc["window_5h"]["used_pct"]   | 0.0f;
    codex.window5h_reset_min   = doc["window_5h"]["reset_in_min"] | 0;
    codex.window5h_tokens_used = doc["window_5h"]["tokens_used"] | 0L;

    // 7d window
    codex.window7d_pct = doc["window_7d"]["used_pct"] | 0.0f;

    // Session
    codex.input_tokens        = doc["session"]["input_tokens"]        | 0L;
    codex.output_tokens       = doc["session"]["output_tokens"]       | 0L;
    codex.reasoning_tokens    = doc["session"]["reasoning_tokens"]    | 0L;
    codex.cached_input_tokens = doc["session"]["cached_input_tokens"] | 0L;
    codex.cost_usd            = doc["session"]["cost_usd"]            | 0.0f;
    codex.cache_hit_pct       = doc["session"]["cache_hit_pct"]       | 0.0f;

    // Activity
    codex.tasks_completed = doc["tasks_completed"] | 0;
    codex.patches_applied = doc["patches_applied"] | 0;
    codex.model  = doc["model"]  | "o4-mini";
    codex.status = doc["status"] | "unknown";

    // Recent tasks
    codex.task_count = 0;
    JsonArray tasks = doc["recent_tasks"];
    for (JsonVariant v : tasks) {
        if (codex.task_count >= 5) break;
        codex.recent_tasks[codex.task_count++] = v.as<String>();
    }

    return true;
}

// =====================================================================
// SCREEN 0 — CODEX COMMAND CENTER
// =====================================================================
void drawScreen0() {
    tft.fillScreen(COL_BG);

    // === HEADER ===
    tft.setTextColor(COL_GREEN, COL_BG);
    tft.setTextSize(2);
    tft.setCursor(4, 4);
    tft.print("[CODEX CLI]");

    // Status indicator
    tft.setTextSize(1);
    if (bridgeReachable && codex.valid) {
        tft.setTextColor(COL_GREEN, COL_BG);
        tft.setCursor(TFT_WIDTH - 68, 8);
        tft.print("* ONLINE");
    } else {
        tft.setTextColor(COL_RED, COL_BG);
        tft.setCursor(TFT_WIDTH - 76, 8);
        tft.print("o OFFLINE");
    }

    // Model tag (small, right aligned)
    tft.setTextColor(COL_DIM, COL_BG);
    tft.setCursor(TFT_WIDTH - 56, 18);
    tft.print(codex.model.c_str());

    // Green separator line
    tft.drawFastHLine(0, 28, TFT_WIDTH, COL_GREEN);

    int y = 34;

    // === 5-HOUR WINDOW ===
    tft.setTextColor(COL_GREEN, COL_BG);
    tft.setTextSize(1);
    tft.setCursor(4, y);
    tft.print("5-HOUR WINDOW");

    // Show percentage right-aligned
    char pctBuf[12];
    snprintf(pctBuf, sizeof(pctBuf), "%.1f%%", codex.window5h_pct);
    tft.setTextColor(codex.window5h_pct > 90.0f ? COL_RED :
                     codex.window5h_pct > 70.0f ? COL_YELLOW : COL_GREEN, COL_BG);
    tft.setCursor(TFT_WIDTH - (strlen(pctBuf) * 6) - 4, y);
    tft.print(pctBuf);

    y += 11;
    // Large progress bar (full width, 20px tall)
    uint16_t barColor = codex.window5h_pct > 90.0f ? COL_RED :
                        codex.window5h_pct > 70.0f ? COL_YELLOW : COL_GREEN;
    drawProgressBar(4, y, TFT_WIDTH - 8, 20, codex.window5h_pct, barColor);

    y += 24;
    tft.setTextColor(COL_DIM, COL_BG);
    tft.setTextSize(1);
    tft.setCursor(4, y);
    char barInfo[64];
    snprintf(barInfo, sizeof(barInfo), "%s used   Resets %s",
             formatTokens(codex.window5h_tokens_used).c_str(),
             formatTime(codex.window5h_reset_min).c_str());
    tft.print(barInfo);

    y += 14;
    tft.drawFastHLine(0, y, TFT_WIDTH, COL_DIM);
    y += 6;

    // === REASONING ROW ===
    long totalTokens = codex.input_tokens + codex.output_tokens + codex.reasoning_tokens + 1;
    float rPct = (totalTokens > 0) ? (codex.reasoning_tokens * 100.0f / totalTokens) : 0.0f;

    tft.setTextColor(COL_MAGENTA, COL_BG);
    tft.setCursor(4, y);
    tft.print("REASONING");
    drawProgressBar(72, y - 1, TFT_WIDTH - 72 - 50, 10, rPct, COL_MAGENTA);
    tft.setTextColor(COL_WHITE, COL_BG);
    tft.setCursor(TFT_WIDTH - 46, y);
    tft.print(formatTokens(codex.reasoning_tokens).c_str());

    y += 14;

    // === CACHE HIT ROW ===
    tft.setTextColor(COL_CYAN, COL_BG);
    tft.setCursor(4, y);
    tft.print("CACHE HIT");
    drawProgressBar(72, y - 1, TFT_WIDTH - 72 - 50, 10, codex.cache_hit_pct, COL_CYAN);
    tft.setTextColor(COL_WHITE, COL_BG);
    tft.setCursor(TFT_WIDTH - 46, y);
    char cacheStr[12];
    snprintf(cacheStr, sizeof(cacheStr), "%.0f%%", codex.cache_hit_pct);
    tft.print(cacheStr);

    y += 18;
    tft.drawFastHLine(0, y, TFT_WIDTH, COL_DIM);
    y += 6;

    // === TASKS / PATCHES / MODEL ===
    tft.setTextColor(COL_WHITE, COL_BG);
    tft.setTextSize(1);
    tft.setCursor(4, y);
    char taskBuf[64];
    snprintf(taskBuf, sizeof(taskBuf), "TASKS: %d  PATCHES: %d  MDL: %s",
             codex.tasks_completed, codex.patches_applied, codex.model.c_str());
    tft.print(taskBuf);

    y += 13;

    // === COST ===
    tft.setTextColor(COL_YELLOW, COL_BG);
    tft.setCursor(4, y);
    char costBuf[32];
    snprintf(costBuf, sizeof(costBuf), "COST TODAY: %s", formatCost(codex.cost_usd).c_str());
    tft.print(costBuf);

    // 7d window right side
    tft.setTextColor(COL_DIM, COL_BG);
    char w7Buf[20];
    snprintf(w7Buf, sizeof(w7Buf), "7D:%.1f%%", codex.window7d_pct);
    tft.setCursor(TFT_WIDTH - (strlen(w7Buf) * 6) - 4, y);
    tft.print(w7Buf);

    y += 16;
    tft.drawFastHLine(0, y, TFT_WIDTH, COL_DIM);
    y += 4;

    // === HINT ===
    tft.setTextColor(COL_DIM, COL_BG);
    tft.setCursor(4, y);
    tft.print("< TAP TO CYCLE SCREENS >");
}

// =====================================================================
// SCREEN 1 — REASONING TOKEN BREAKDOWN
// =====================================================================
void drawScreen1() {
    tft.fillScreen(COL_BG);

    // === HEADER ===
    tft.setTextColor(COL_GREEN, COL_BG);
    tft.setTextSize(2);
    tft.setCursor(4, 4);
    tft.print("[REASONING]");

    tft.setTextSize(1);
    if (!bridgeReachable) drawBridgeOfflineBanner();

    tft.drawFastHLine(0, 24, TFT_WIDTH, COL_GREEN);

    int y = 32;

    // Find max token value for proportional bars
    long maxTok = max(max(codex.input_tokens, codex.output_tokens),
                      max(codex.reasoning_tokens, codex.cached_input_tokens));
    if (maxTok < 1) maxTok = 1;

    int barX = 72;
    int barW = TFT_WIDTH - barX - 54;
    int barH = 12;
    int rowH = 22;

    // Helper lambda-like — just inline 4 rows
    // INPUT
    tft.setTextColor(COL_GREEN, COL_BG);
    tft.setCursor(4, y + 2);
    tft.print("INPUT");
    float pI = codex.input_tokens * 100.0f / maxTok;
    drawProgressBar(barX, y, barW, barH, pI, COL_GREEN);
    tft.setTextColor(COL_WHITE, COL_BG);
    tft.setCursor(barX + barW + 4, y + 2);
    tft.print(formatTokens(codex.input_tokens).c_str());
    y += rowH;

    // OUTPUT
    tft.setTextColor(COL_EMERALD, COL_BG);
    tft.setCursor(4, y + 2);
    tft.print("OUTPUT");
    float pO = codex.output_tokens * 100.0f / maxTok;
    drawProgressBar(barX, y, barW, barH, pO, COL_EMERALD);
    tft.setTextColor(COL_WHITE, COL_BG);
    tft.setCursor(barX + barW + 4, y + 2);
    tft.print(formatTokens(codex.output_tokens).c_str());
    y += rowH;

    // REASONING — magenta, stand out
    tft.setTextColor(COL_MAGENTA, COL_BG);
    tft.setCursor(4, y + 2);
    tft.print("REASON");
    float pR = codex.reasoning_tokens * 100.0f / maxTok;
    drawProgressBar(barX, y, barW, barH, pR, COL_MAGENTA);
    tft.setTextColor(COL_MAGENTA, COL_BG);
    tft.setCursor(barX + barW + 4, y + 2);
    tft.print(formatTokens(codex.reasoning_tokens).c_str());
    y += rowH;

    // CACHED
    tft.setTextColor(COL_CYAN, COL_BG);
    tft.setCursor(4, y + 2);
    tft.print("CACHED");
    float pC = codex.cached_input_tokens * 100.0f / maxTok;
    drawProgressBar(barX, y, barW, barH, pC, COL_CYAN);
    tft.setTextColor(COL_WHITE, COL_BG);
    tft.setCursor(barX + barW + 4, y + 2);
    tft.print(formatTokens(codex.cached_input_tokens).c_str());
    y += rowH;

    tft.drawFastHLine(0, y, TFT_WIDTH, COL_DIM);
    y += 8;

    // === EFFICIENCY ===
    uint16_t effColor = codex.cache_hit_pct >= 50.0f ? COL_YELLOW : COL_WHITE;
    tft.setTextColor(effColor, COL_BG);
    tft.setCursor(4, y);
    char effBuf[48];
    snprintf(effBuf, sizeof(effBuf), "EFFICIENCY: %.1f%% cache hit", codex.cache_hit_pct);
    tft.print(effBuf);
    y += 13;

    tft.setTextColor(COL_YELLOW, COL_BG);
    tft.setCursor(4, y);
    char costBuf[32];
    snprintf(costBuf, sizeof(costBuf), "TODAY COST: %s", formatCost(codex.cost_usd).c_str());
    tft.print(costBuf);
    y += 13;

    tft.setTextColor(COL_DIM, COL_BG);
    tft.setCursor(4, y);
    char wBuf[64];
    snprintf(wBuf, sizeof(wBuf), "WINDOW USED: %.1f%%  |  7-DAY: %.1f%%",
             codex.window5h_pct, codex.window7d_pct);
    tft.print(wBuf);
    y += 13;

    tft.setTextColor(COL_DIM, COL_BG);
    tft.setCursor(4, y);
    tft.print("< TAP TO CYCLE SCREENS >");
}

// =====================================================================
// SCREEN 2 — TASK HISTORY / STANDBY CLOCK
// =====================================================================
void drawScreen2() {
    tft.fillScreen(COL_BG);

    // === HEADER ===
    tft.setTextColor(COL_GREEN, COL_BG);
    tft.setTextSize(2);
    tft.setCursor(4, 4);
    tft.print("[RECENT TASKS]");

    tft.setTextSize(1);
    if (!bridgeReachable) drawBridgeOfflineBanner();

    tft.drawFastHLine(0, 24, TFT_WIDTH, COL_GREEN);

    if (codex.valid && codex.task_count > 0) {
        int y = 32;
        for (int i = 0; i < codex.task_count && i < 5; i++) {
            tft.setTextColor(COL_GREEN, COL_BG);
            tft.setCursor(4, y);
            tft.print("> ");

            String task = codex.recent_tasks[i];
            if (task.length() > 36) {
                task = task.substring(0, 33) + "...";
            }
            tft.setTextColor(COL_WHITE, COL_BG);
            tft.print(task.c_str());
            y += 16;
        }

        // Bottom summary
        int yBottom = TFT_HEIGHT - 26;
        tft.drawFastHLine(0, yBottom - 4, TFT_WIDTH, COL_DIM);
        tft.setTextColor(COL_EMERALD, COL_BG);
        tft.setCursor(4, yBottom);
        char sumBuf[48];
        snprintf(sumBuf, sizeof(sumBuf), "%d tasks . %d patches today",
                 codex.tasks_completed, codex.patches_applied);
        tft.print(sumBuf);

        tft.setTextColor(COL_DIM, COL_BG);
        tft.setCursor(4, yBottom + 13);
        tft.print("< TAP TO CYCLE SCREENS >");

    } else {
        // === STANDBY CLOCK ===
        timeClient.update();
        String timeStr = timeClient.getFormattedTime();

        // Large clock
        tft.setTextColor(COL_GREEN, COL_BG);
        tft.setTextSize(4);
        // Center HH:MM
        String hhmm = timeStr.substring(0, 5);
        int cw = hhmm.length() * 6 * 4;
        tft.setCursor((TFT_WIDTH - cw) / 2, 60);
        tft.print(hhmm.c_str());

        // Seconds smaller
        tft.setTextSize(2);
        tft.setTextColor(COL_EMERALD, COL_BG);
        String ss = ":" + timeStr.substring(6, 8);
        tft.setCursor((TFT_WIDTH + cw) / 2 - 4, 76);
        tft.print(ss.c_str());

        // Blinking cursor
        tft.setTextSize(3);
        tft.setCursor((TFT_WIDTH - cw) / 2 + cw + 2, 56);
        if (cursorVisible) {
            tft.setTextColor(COL_GREEN, COL_BG);
            tft.print("_");
        } else {
            tft.setTextColor(COL_BG, COL_BG);
            tft.print("_");
        }

        // Status label
        tft.setTextSize(1);
        tft.setTextColor(COL_DIM, COL_BG);
        tft.setCursor(4, 130);
        tft.print("[IDLE — NO RECENT TASKS]");

        tft.setTextColor(COL_DIM, COL_BG);
        tft.setCursor(4, TFT_HEIGHT - 13);
        tft.print("< TAP TO CYCLE SCREENS >");
    }
}

// =====================================================================
// CONNECTING / ERROR SCREENS
// =====================================================================
void drawConnecting() {
    tft.fillScreen(COL_BG);
    tft.setTextColor(COL_GREEN, COL_BG);
    tft.setTextSize(2);
    tft.setCursor(4, 4);
    tft.print("[CODEX CLI]");
    tft.drawFastHLine(0, 24, TFT_WIDTH, COL_GREEN);

    tft.setTextSize(1);
    tft.setTextColor(COL_YELLOW, COL_BG);
    tft.setCursor(4, 40);
    tft.print("Connecting to WiFi...");
    tft.setCursor(4, 56);
    tft.print("SSID: ");
    tft.print(WIFI_SSID);

    // Matrix rain effect — just static dots
    tft.setTextColor(COL_DIM, COL_BG);
    tft.setCursor(4, 100);
    tft.print("01001000 01100101 01101100");
    tft.setCursor(4, 112);
    tft.print("01101100 01101111 00100001");
}

void drawError(String msg) {
    tft.fillScreen(COL_BG);
    tft.setTextColor(COL_RED, COL_BG);
    tft.setTextSize(2);
    tft.setCursor(4, 4);
    tft.print("[ERROR]");
    tft.drawFastHLine(0, 24, TFT_WIDTH, COL_RED);

    tft.setTextSize(1);
    tft.setTextColor(COL_WHITE, COL_BG);
    tft.setCursor(4, 40);
    tft.print(msg.c_str());

    tft.setTextColor(COL_DIM, COL_BG);
    tft.setCursor(4, 70);
    tft.print("Check WiFi and bridge/monitor.py");
    tft.setCursor(4, 84);
    char urlBuf[64];
    snprintf(urlBuf, sizeof(urlBuf), "http://%s:%d/api/codex", BRIDGE_HOST, BRIDGE_PORT);
    tft.print(urlBuf);
}

// =====================================================================
// TOUCH HANDLING
// =====================================================================
void handleTouch() {
    if (!touch.tirqTouched()) return;
    if (!touch.touched()) return;

    unsigned long now = millis();
    if (now - lastTouch < 500) return;  // 500ms cooldown
    lastTouch = now;

    TS_Point p = touch.getPoint();
    int tx = map(p.x, 240, 3800, 0, 320);
    int ty = map(p.y, 240, 3800, 0, 240);
    (void)tx; (void)ty;

    currentScreen = (currentScreen + 1) % 3;
    // Redraw immediately
    switch (currentScreen) {
        case 0: drawScreen0(); break;
        case 1: drawScreen1(); break;
        case 2: drawScreen2(); break;
    }
}

// =====================================================================
// SETUP
// =====================================================================
void setup() {
    Serial.begin(115200);
    Serial.println("[Codex Vibe Monitor] Booting...");

    // LED setup — ensure all OFF (HIGH = OFF for active low)
    pinMode(LED_R, OUTPUT);
    pinMode(LED_G, OUTPUT);
    pinMode(LED_B, OUTPUT);
    digitalWrite(LED_R, HIGH);
    digitalWrite(LED_G, HIGH);
    digitalWrite(LED_B, HIGH);

    // TFT
    tft.init();
    tft.setRotation(1);
    tft.fillScreen(COL_BG);

    // Touch on HSPI
    touchSPI.begin(TOUCH_CLK, TOUCH_MISO, TOUCH_MOSI, TOUCH_CS);
    touch.begin(touchSPI);
    touch.setRotation(1);

    // Show connecting screen
    drawConnecting();
    setLED(true, false, false);  // Red while connecting

    // WiFi
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    unsigned long wifiStart = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - wifiStart < 20000) {
        delay(500);
        Serial.print(".");
    }

    if (WiFi.status() != WL_CONNECTED) {
        drawError("WiFi Failed");
        setLED(true, false, false);
        return;
    }

    Serial.println("\n[WiFi] Connected: " + WiFi.localIP().toString());
    setLED(false, true, false);  // Green = connected

    // NTP
    timeClient.begin();
    timeClient.update();

    // Initial data fetch
    fetchData();

    // Draw initial screen
    drawScreen0();
}

// =====================================================================
// LOOP
// =====================================================================
void loop() {
    unsigned long now = millis();

    // Touch handling
    handleTouch();

    // Fetch interval
    if (now - lastFetch >= FETCH_INTERVAL) {
        lastFetch = now;
        bool ok = fetchData();
        // Redraw current screen
        switch (currentScreen) {
            case 0: drawScreen0(); break;
            case 1: drawScreen1(); break;
            case 2: drawScreen2(); break;
        }
    }

    // Blinking cursor update for screen2 clock (500ms)
    if (currentScreen == 2 && now - lastCursorToggle >= 500) {
        lastCursorToggle = now;
        cursorVisible = !cursorVisible;
        // Only partial redraw if no tasks
        if (codex.task_count == 0) {
            drawScreen2();
        }
    }

    // LED update
    updateLED();

    delay(20);
}
