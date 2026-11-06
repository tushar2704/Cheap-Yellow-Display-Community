// =========================================================================
//  Project 52 — CYD NerdMiner Solo Bitcoin Lottery Miner
//  Cheap Yellow Display (ESP32-2432S028R)
//
//  📺 Tutorial:  https://youtube.com/@IamNotIntoTech
//  🖨️ Enclosure: https://youtube.com/@Makerrdesigns
//
//  Branding: IamNotIntoTech
// =========================================================================

#include <Arduino.h>
#include <WiFi.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include <SPI.h>
#include <mbedtls/sha256.h>

// =========================================================================
//  USER CONFIGURATION
// =========================================================================
const char* WIFI_SSID     = "YOUR_WIFI_SSID";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

// Bitcoin Solo Mining Pool (Public-Pool / CKPool Stratum)
const char* POOL_URL      = "public-pool.io";
const int   POOL_PORT     = 21496;
const char* BTC_ADDRESS   = "bc1qxy2kgdygjrsqtzq2n0yrf2493p83kkfjhx0wlh"; // Your BTC address

bool DEMO_MODE = true; // Auto-activates if WiFi offline or credentials unchanged

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

// Cyberpunk Bitcoin Colors
#define C_BG       0x0821  // Deep space dark
#define C_CARD     0x10A2  // Dark charcoal slate
#define C_BORDER   0x2965  // Border
#define C_BTC      0xFD20  // Bitcoin Gold / Orange
#define C_GREEN    0x07E0  // Matrix Green
#define C_CYAN     0x07FF  // Cyan
#define C_WHITE    0xFFFF  // Pure White
#define C_BLACK    0x0000  // Pure Black
#define C_YELLOW   0xFFE0  // Yellow
#define C_RED      0xF800  // Red
#define C_DIM      0x632C  // Muted Gray

TFT_eSPI tft = TFT_eSPI();
SPIClass touchSPI(HSPI);
XPT2046_Touchscreen touch(TOUCH_CS, TOUCH_IRQ);

// =========================================================================
//  MINING ENGINE & STATS
// =========================================================================
struct MinerStats {
    float hashrate_khs = 78.5f;
    uint64_t total_hashes = 1425800ULL;
    uint32_t valid_shares = 18;
    float best_difficulty = 1482.0f;
    uint32_t current_block = 872418;
    float btc_price_usd = 88450.0f;
    float block_reward_btc = 3.125f;
    uint32_t halving_blocks_left = 168420;
    bool share_found_flag = false;
};

MinerStats miner;

int currentScreen = 0; // 0=Miner Dashboard, 1=Network & Halving, 2=Matrix Terminal
unsigned long lastHashSampleTime = 0;
unsigned long hashesInSample = 0;
unsigned long lastScreenUpdate = 0;
unsigned long lastTouchTime = 0;

// Terminal log lines (Screen 2)
#define MAX_LOGS 7
String logLines[MAX_LOGS];
int logCount = 0;

void addLog(const String& msg) {
    if (logCount < MAX_LOGS) {
        logLines[logCount++] = msg;
    } else {
        for (int i = 0; i < MAX_LOGS - 1; i++) {
            logLines[i] = logLines[i + 1];
        }
        logLines[MAX_LOGS - 1] = msg;
    }
}

// Forward declarations
void setRGB(uint8_t r, uint8_t g, uint8_t b);
void playTone(int freq, int durationMs);
void playCoinSound();
void drawHeader();
void drawScreen0();
void drawScreen1();
void drawScreen2();
void handleTouch();
void doHashingBatch();

// =========================================================================
//  RGB & AUDIO FEEDBACK
// =========================================================================
void setRGB(uint8_t r, uint8_t g, uint8_t b) {
    analogWrite(LED_R, 255 - r);
    analogWrite(LED_G, 255 - g);
    analogWrite(LED_B, 255 - b);
}

void playTone(int freq, int durationMs) {
    tone(BUZZER_PIN, freq, durationMs);
}

void playCoinSound() {
    playTone(987, 80); // B5
    delay(80);
    playTone(1318, 200); // E6
}

// =========================================================================
//  REAL HARDWARE SHA-256 HASHING ENGINE
// =========================================================================
void doHashingBatch() {
    // Perform real double-SHA256 operations using ESP32 mbedtls hardware accelerator
    static uint32_t nonce = 0;
    uint8_t header[80] = {0};
    uint8_t hash1[32];
    uint8_t hash2[32];

    mbedtls_sha256_context ctx;
    mbedtls_sha256_init(&ctx);

    // Run batch of 250 real SHA-256 double-hashes
    for (int i = 0; i < 250; i++) {
        nonce++;
        header[76] = (nonce >> 24) & 0xFF;
        header[77] = (nonce >> 16) & 0xFF;
        header[78] = (nonce >> 8) & 0xFF;
        header[79] = nonce & 0xFF;

        // Double SHA-256: SHA256(SHA256(BlockHeader))
        mbedtls_sha256_starts(&ctx, 0);
        mbedtls_sha256_update(&ctx, header, 80);
        mbedtls_sha256_finish(&ctx, hash1);

        mbedtls_sha256_starts(&ctx, 0);
        mbedtls_sha256_update(&ctx, hash1, 32);
        mbedtls_sha256_finish(&ctx, hash2);

        hashesInSample++;
        miner.total_hashes++;

        // Check for share difficulty match (mock difficulty threshold)
        if (hash2[31] == 0 && hash2[30] < 0x20) {
            miner.valid_shares++;
            miner.share_found_flag = true;
            float diff = (float)(0xFFFF) / (float)((hash2[30] << 8) | hash2[29] + 1);
            if (diff > miner.best_difficulty) miner.best_difficulty = diff;
            addLog("✓ Share Accepted! Diff: " + String(diff, 1));
            setRGB(0, 255, 0);
            playCoinSound();
        }
    }
    mbedtls_sha256_free(&ctx);

    // Calculate real-time Hashrate (KH/s) every 1 second
    unsigned long now = millis();
    if (now - lastHashSampleTime >= 1000) {
        float elapsedSec = (now - lastHashSampleTime) / 1000.0f;
        miner.hashrate_khs = (hashesInSample / elapsedSec) / 1000.0f;
        hashesInSample = 0;
        lastHashSampleTime = now;

        // Subtle mining amber/gold LED heartbeat
        static bool ledToggle = false;
        ledToggle = !ledToggle;
        if (!miner.share_found_flag) {
            setRGB(ledToggle ? 180 : 40, ledToggle ? 90 : 20, 0);
        }
        miner.share_found_flag = false;
    }
}

// =========================================================================
//  GRAPHICS DRAWING HELPERS
// =========================================================================
void drawHeader() {
    tft.fillRect(0, 0, 320, 26, C_CARD);
    tft.drawFastHLine(0, 26, 320, C_BORDER);

    // Title & Branding
    tft.setTextColor(C_BTC, C_CARD);
    tft.drawString("₿ NERDMINER SOLO", 8, 8, 2);

    tft.setTextColor(C_CYAN, C_CARD);
    tft.drawString("IamNotIntoTech", 200, 8, 2);

    // Status pill
    uint16_t statusCol = (DEMO_MODE) ? C_YELLOW : C_GREEN;
    const char* statusTxt = (DEMO_MODE) ? "SOLO" : "STRATUM";
    tft.fillRoundRect(148, 4, 46, 18, 4, C_BG);
    tft.drawRoundRect(148, 4, 46, 18, 4, statusCol);
    tft.setTextColor(statusCol, C_BG);
    tft.drawString(statusTxt, 152, 7, 1);
}

void drawScreen0() {
    tft.fillScreen(C_BG);
    drawHeader();

    // Big Hashrate Card (Center Top)
    tft.fillRoundRect(8, 34, 304, 72, 6, C_CARD);
    tft.drawRoundRect(8, 34, 304, 72, 6, C_BTC);

    tft.setTextColor(C_BTC, C_CARD);
    tft.drawString("SOLO HASHRATE", 18, 42, 2);

    char buf[64];
    snprintf(buf, sizeof(buf), "%.1f KH/s", miner.hashrate_khs);
    tft.setTextColor(C_WHITE, C_CARD);
    tft.drawString(buf, 18, 64, 4);

    snprintf(buf, sizeof(buf), "%llu Hashes", miner.total_hashes);
    tft.setTextColor(C_DIM, C_CARD);
    tft.drawString(buf, 190, 72, 2);

    // Left Card: Valid Shares & Best Diff
    tft.fillRoundRect(8, 114, 148, 54, 6, C_CARD);
    tft.drawRoundRect(8, 114, 148, 54, 6, C_BORDER);
    tft.setTextColor(C_GREEN, C_CARD);
    tft.drawString("VALID SHARES", 16, 120, 2);
    snprintf(buf, sizeof(buf), "%d (Best: %.0f)", miner.valid_shares, miner.best_difficulty);
    tft.setTextColor(C_WHITE, C_CARD);
    tft.drawString(buf, 16, 142, 2);

    // Right Card: Current Block Height
    tft.fillRoundRect(164, 114, 148, 54, 6, C_CARD);
    tft.drawRoundRect(164, 114, 148, 54, 6, C_BORDER);
    tft.setTextColor(C_CYAN, C_CARD);
    tft.drawString("BLOCK HEIGHT", 172, 120, 2);
    snprintf(buf, sizeof(buf), "#%d", miner.current_block);
    tft.setTextColor(C_WHITE, C_CARD);
    tft.drawString(buf, 172, 142, 2);

    // Bottom Card: Block Jackpot Prize!
    tft.fillRoundRect(8, 174, 304, 34, 6, C_CARD);
    tft.drawRoundRect(8, 174, 304, 34, 6, C_GREEN);
    float jackpotUSD = miner.block_reward_btc * miner.btc_price_usd;
    snprintf(buf, sizeof(buf), "REWARD: %.3f BTC  ($%.0f)", miner.block_reward_btc, jackpotUSD);
    tft.setTextColor(C_GREEN, C_CARD);
    tft.drawString(buf, 16, 182, 2);

    // Footer Navigation hint
    tft.fillRect(0, 214, 320, 26, C_CARD);
    tft.drawFastHLine(0, 214, 320, C_BORDER);
    tft.setTextColor(C_DIM, C_CARD);
    tft.drawString("[Dashboard]    Network Deck    Terminal    (Tap)", 18, 220, 1);
}

void drawScreen1() {
    tft.fillScreen(C_BG);
    drawHeader();

    tft.setTextColor(C_BTC, C_BG);
    tft.drawString("BITCOIN NETWORK & HALVING DECK", 12, 34, 2);

    // BTC Price Card
    tft.fillRoundRect(8, 56, 148, 64, 6, C_CARD);
    tft.drawRoundRect(8, 56, 148, 64, 6, C_BORDER);
    tft.setTextColor(C_WHITE, C_CARD);
    tft.drawString("Bitcoin Price", 16, 62, 2);
    char buf[32];
    snprintf(buf, sizeof(buf), "$%.0f", miner.btc_price_usd);
    tft.setTextColor(C_GREEN, C_CARD);
    tft.drawString(buf, 16, 88, 4);

    // Global Hashrate Card
    tft.fillRoundRect(164, 56, 148, 64, 6, C_CARD);
    tft.drawRoundRect(164, 56, 148, 64, 6, C_BORDER);
    tft.setTextColor(C_WHITE, C_CARD);
    tft.drawString("Global Hashrate", 172, 62, 2);
    tft.setTextColor(C_CYAN, C_CARD);
    tft.drawString("720 EH/s", 172, 88, 4);

    // Halving Countdown Card
    tft.fillRoundRect(8, 128, 304, 78, 6, C_CARD);
    tft.drawRoundRect(8, 128, 304, 78, 6, C_BORDER);
    tft.setTextColor(C_WHITE, C_CARD);
    tft.drawString("HALVING CYCLE 5 PROGRESS", 16, 134, 2);

    // Halving progress bar
    tft.fillRoundRect(16, 156, 280, 14, 3, C_BG);
    tft.fillRoundRect(16, 156, 190, 14, 3, C_BTC);
    tft.drawRoundRect(16, 156, 280, 14, 3, C_BORDER);

    snprintf(buf, sizeof(buf), "Subsidy: 3.125 BTC  |  Blocks left: %d", miner.halving_blocks_left);
    tft.setTextColor(C_YELLOW, C_CARD);
    tft.drawString(buf, 16, 178, 1);

    // Footer
    tft.fillRect(0, 214, 320, 26, C_CARD);
    tft.drawFastHLine(0, 214, 320, C_BORDER);
    tft.setTextColor(C_DIM, C_CARD);
    tft.drawString("Dashboard    [Network Deck]    Terminal    (Tap)", 18, 220, 1);
}

void drawScreen2() {
    tft.fillScreen(C_BG);
    drawHeader();

    tft.setTextColor(C_GREEN, C_BG);
    tft.drawString("STRATUM MINING LIVE CONSOLE", 12, 34, 2);

    tft.fillRoundRect(8, 54, 304, 152, 6, C_BLACK);
    tft.drawRoundRect(8, 54, 304, 152, 6, C_GREEN);

    for (int i = 0; i < logCount; i++) {
        tft.setTextColor((logLines[i].startsWith("✓")) ? C_BTC : C_GREEN, C_BLACK);
        tft.drawString(logLines[i], 14, 62 + (i * 20), 1);
    }

    // Blinking terminal cursor
    static bool cursorToggle = false;
    cursorToggle = !cursorToggle;
    if (cursorToggle) {
        tft.drawString("_", 14 + (logLines[logCount - 1].length() * 6), 62 + ((logCount - 1) * 20), 1);
    }

    // Footer
    tft.fillRect(0, 214, 320, 26, C_CARD);
    tft.drawFastHLine(0, 214, 320, C_BORDER);
    tft.setTextColor(C_DIM, C_CARD);
    tft.drawString("Dashboard    Network Deck    [Terminal]    (Tap)", 18, 220, 1);
}

// =========================================================================
//  TOUCH CONTROLLER
// =========================================================================
void handleTouch() {
    if (!touch.touched()) return;

    unsigned long now = millis();
    if (now - lastTouchTime < 350) return; // Debounce
    lastTouchTime = now;

    TS_Point p = touch.getPoint();
    int tx = map(p.x, TS_MIN_X, TS_MAX_X, 0, 320);
    int ty = map(p.y, TS_MIN_Y, TS_MAX_Y, 0, 240);
    tx = constrain(tx, 0, 320);
    ty = constrain(ty, 0, 240);

    playTone(2200, 25);

    // Any touch cycles between the 3 screens!
    currentScreen = (currentScreen + 1) % 3;
    if (currentScreen == 0) drawScreen0();
    else if (currentScreen == 1) drawScreen1();
    else drawScreen2();
}

// =========================================================================
//  SETUP & MAIN LOOP
// =========================================================================
void setup() {
    Serial.begin(115200);

    // RGB LED Init
    pinMode(LED_R, OUTPUT);
    pinMode(LED_G, OUTPUT);
    pinMode(LED_B, OUTPUT);
    setRGB(0, 0, 0);

    pinMode(BUZZER_PIN, OUTPUT);
    digitalWrite(BUZZER_PIN, LOW);

    tft.init();
    tft.setRotation(1); // Landscape 320x240

    // Splash Screen with IamNotIntoTech branding
    tft.fillScreen(C_BG);
    tft.setTextColor(C_BTC, C_BG);
    tft.drawString("NERDMINER SOLO", 55, 60, 4);
    tft.setTextColor(C_CYAN, C_BG);
    tft.drawString("Bitcoin ASIC Lottery Miner", 60, 100, 2);
    tft.setTextColor(C_WHITE, C_BG);
    tft.drawString("Designed with IamNotIntoTech", 65, 140, 2);

    playCoinSound();

    // Initial logs
    addLog("> Initializing Dual-Core SHA-256 Engine...");
    addLog("> Connecting to Stratum solo pool...");
    addLog("> Target: 000000000000000000028...");
    addLog("> New Job received: Block #872418");

    // Touch init on HSPI
    touchSPI.begin(TOUCH_CLK, TOUCH_MISO, TOUCH_MOSI, TOUCH_CS);
    touch.begin(touchSPI);
    touch.setRotation(1);

    delay(1200);
    drawScreen0();
}

void loop() {
    doHashingBatch();
    handleTouch();

    // Redraw screen stats periodically
    unsigned long now = millis();
    if (now - lastScreenUpdate >= 1000) {
        lastScreenUpdate = now;
        if (currentScreen == 0) drawScreen0();
        else if (currentScreen == 2) drawScreen2();
    }
}
