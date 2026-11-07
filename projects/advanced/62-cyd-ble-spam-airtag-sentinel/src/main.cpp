// =========================================================================
//  Project 62 — CYD Apple Juice & BLE Spam Sentinel
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
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

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
#define C_BG          0x0821  // Obsidian slate
#define C_CARD        0x10A2  // Elevated card
#define C_CARD_BORDER 0x2124
#define C_GREEN       0x07E0  // Secure Green
#define C_CYAN        0x07FF
#define C_ALERT_RED   0xF800  // Attack Alert Red
#define C_YELLOW      0xFFE0  // Warning Yellow
#define C_PURPLE      0x915C  // AirTag Violet
#define C_GRAY        0x73AE
#define C_DARKGRAY    0x2965

TFT_eSPI tft = TFT_eSPI();
SPIClass touchSPI(HSPI);
XPT2046_Touchscreen touch(TOUCH_CS, TOUCH_IRQ);
BLEScan* pBLEScan = nullptr;

// =========================================================================
//  SENTINEL TELEMETRY & ATTACK DETECTION
// =========================================================================
enum SentinelTab {
    TAB_SHIELD,
    TAB_LOGS,
    TAB_RADAR
};

int currentTab = TAB_SHIELD;
bool alarmEnabled = true;

// Attack metrics
volatile unsigned long totalBleFrames = 0;
volatile unsigned long appleJuiceAttacks = 0;
volatile unsigned long fastPairFloods = 0;
volatile unsigned long swiftPairFloods = 0;
volatile unsigned long airTagsDetected = 0;
unsigned long lastAttackTime = 0;
int currentPktRate = 0;
unsigned long lastRateSecond = 0;
int secondFrameCounter = 0;

// Incident Log Buffer
struct IncidentLog {
    char type[18];
    char mac[18];
    int rssi;
    unsigned long timestampSec;
};

const int MAX_LOGS = 6;
IncidentLog logs[MAX_LOGS];
int logCount = 0;

// AirTag Radar Targets
struct AirTagTarget {
    char mac[18];
    int rssi;
    unsigned long lastSeenSec;
};

const int MAX_AIRTAGS = 4;
AirTagTarget airTags[MAX_AIRTAGS];
int airTagCount = 0;

// =========================================================================
//  SOUND & LED ALERTS
// =========================================================================
void playAlertSiren() {
    if (!alarmEnabled) return;
    tone(BUZZER_PIN, 2800, 40);
    delay(45);
    tone(BUZZER_PIN, 3400, 60);
}

void playTouchClick() {
    tone(BUZZER_PIN, 1800, 15);
}

void setRGB(uint8_t r, uint8_t g, uint8_t b) {
    analogWrite(LED_R, 255 - r);
    analogWrite(LED_G, 255 - g);
    analogWrite(LED_B, 255 - b);
}

// =========================================================================
//  LOGGING & RADAR HELPERS
// =========================================================================
void addIncident(const char* type, const char* mac, int rssi) {
    lastAttackTime = millis();
    playAlertSiren();
    setRGB(255, 0, 0); // Crimson Red

    // Shift logs
    for (int i = MAX_LOGS - 1; i > 0; i--) {
        logs[i] = logs[i - 1];
    }
    strncpy(logs[0].type, type, sizeof(logs[0].type) - 1);
    strncpy(logs[0].mac, mac, sizeof(logs[0].mac) - 1);
    logs[0].rssi = rssi;
    logs[0].timestampSec = millis() / 1000;
    if (logCount < MAX_LOGS) logCount++;
}

void updateAirTag(const char* mac, int rssi) {
    for (int i = 0; i < airTagCount; i++) {
        if (strcmp(airTags[i].mac, mac) == 0) {
            airTags[i].rssi = rssi;
            airTags[i].lastSeenSec = millis() / 1000;
            return;
        }
    }
    if (airTagCount < MAX_AIRTAGS) {
        strncpy(airTags[airTagCount].mac, mac, sizeof(airTags[airTagCount].mac) - 1);
        airTags[airTagCount].rssi = rssi;
        airTags[airTagCount].lastSeenSec = millis() / 1000;
        airTagCount++;
        airTagsDetected++;
    }
}

// =========================================================================
//  BLE ADVERTISED DEVICE CALLBACK
// =========================================================================
class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
        totalBleFrames++;
        secondFrameCounter++;

        String macStr = advertisedDevice.getAddress().toString().c_str();
        int rssi = advertisedDevice.getRSSI();

        // 1. Check Manufacturer Data for Apple Proximity / Apple Juice attacks
        if (advertisedDevice.haveManufacturerData()) {
            std::string mData = advertisedDevice.getManufacturerData();
            if (mData.length() >= 2) {
                uint16_t compId = ((uint8_t)mData[1] << 8) | (uint8_t)mData[0];

                // Apple Company ID 0x004C
                if (compId == 0x004C) {
                    if (mData.length() >= 3) {
                        uint8_t type = (uint8_t)mData[2];
                        // 0x07: AirPods pairing popup, 0x10: Nearby Action, 0x05: AirDrop
                        if (type == 0x07 || type == 0x10 || type == 0x05) {
                            appleJuiceAttacks++;
                            addIncident("Apple Juice Popup", macStr.c_str(), rssi);
                            return;
                        }
                        // 0x12: FindMy / AirTag beacon
                        if (type == 0x12) {
                            updateAirTag(macStr.c_str(), rssi);
                            return;
                        }
                    }
                }
                // Microsoft Company ID 0x0006 (Swift Pair)
                else if (compId == 0x0006) {
                    if (mData.length() >= 3 && (uint8_t)mData[2] == 0x03) {
                        swiftPairFloods++;
                        addIncident("MS Swift Pair", macStr.c_str(), rssi);
                        return;
                    }
                }
            }
        }

        // 2. Check Service Data for Google Fast Pair (UUID 0xFE2C)
        if (advertisedDevice.haveServiceUUID()) {
            if (advertisedDevice.getServiceUUID().toString() == "0000fe2c-0000-1000-8000-00805f9b34fb") {
                fastPairFloods++;
                addIncident("Google Fast Pair", macStr.c_str(), rssi);
                return;
            }
        }
    }
};

// =========================================================================
//  UI DRAWING ROUTINES
// =========================================================================
void drawHeader() {
    tft.fillRect(0, 0, 320, 24, C_BG);

    // Title / Brand
    tft.setTextColor(C_CYAN, C_BG);
    tft.setTextDatum(TL_DATUM);
    tft.drawString("IamNotIntoTech", 6, 5, 2);
    tft.setTextColor(C_ALERT_RED, C_BG);
    tft.drawString("BLE SENTINEL", 126, 5, 2);

    // Attack Alert Status
    bool underAttack = (millis() - lastAttackTime < 3000);
    if (underAttack) {
        tft.fillRoundRect(236, 3, 78, 18, 4, C_ALERT_RED);
        tft.setTextColor(C_WHITE, C_ALERT_RED);
        tft.setTextDatum(MC_DATUM);
        tft.drawString("! SPAM FLOOD", 275, 12, 1);
    } else {
        tft.fillRoundRect(248, 3, 66, 18, 4, C_CARD);
        tft.drawRoundRect(248, 3, 66, 18, 4, C_CARD_BORDER);
        tft.setTextColor(C_GREEN, C_CARD);
        tft.setTextDatum(MC_DATUM);
        tft.drawString("SECURE", 281, 12, 1);
        setRGB(0, 60, 180); // Cyan/blue
    }

    tft.drawFastHLine(0, 24, 320, C_CARD_BORDER);
}

void drawTabBar() {
    int ty = 212;
    int th = 28;
    int tw = 320 / 3;

    tft.fillRect(0, ty, 320, th, C_BG);
    tft.drawFastHLine(0, ty, 320, C_CARD_BORDER);

    const char* tabNames[3] = { "DEFENSE SHIELD", "ATTACK LOGS", "AIRTAG RADAR" };

    for (int i = 0; i < 3; i++) {
        int tx = i * tw;
        bool isAct = (currentTab == i);
        uint16_t tBg = isAct ? C_CARD : C_BG;
        uint16_t tFg = isAct ? C_CYAN : C_GRAY;

        if (isAct) {
            tft.fillRoundRect(tx + 2, ty + 2, tw - 4, th - 4, 4, tBg);
            tft.drawFastHLine(tx + 8, ty + th - 2, tw - 16, C_CYAN);
        }
        tft.setTextColor(tFg, tBg);
        tft.setTextDatum(MC_DATUM);
        tft.drawString(tabNames[i], tx + (tw / 2), ty + (th / 2), 2);
    }
}

// --- TAB 0: DEFENSE SHIELD ---
void drawTabShield() {
    tft.fillRect(0, 25, 320, 186, C_BG);

    bool underAttack = (millis() - lastAttackTime < 3000);
    uint16_t sBg = underAttack ? C_ALERT_RED : C_CARD;
    uint16_t sBorder = underAttack ? C_WHITE : C_CARD_BORDER;

    // Center Shield Card (X: 8, Y: 28, W: 304, H: 84)
    tft.fillRoundRect(8, 28, 304, 84, 8, sBg);
    tft.drawRoundRect(8, 28, 304, 84, 8, sBorder);

    tft.setTextColor(underAttack ? C_WHITE : C_GREEN, sBg);
    tft.setTextDatum(MC_DATUM);
    tft.drawString(underAttack ? "BLE SPOOFING ATTACK DETECTED!" : "RF AIRSPACE SECURE", 160, 50, 4);

    tft.setTextColor(underAttack ? C_WHITE : C_GRAY, sBg);
    char rateBuf[48];
    snprintf(rateBuf, sizeof(rateBuf), "Traffic Density: %d pkts/sec  •  Total: %lu frames", currentPktRate, totalBleFrames);
    tft.drawString(rateBuf, 160, 80, 2);

    // 4 Attack Counter Badges (X: 8..312, Y: 118..166)
    int bw = 72;
    int bh = 42;

    // 1. Apple Juice
    tft.fillRoundRect(8, 118, bw, bh, 4, C_CARD);
    tft.setTextColor(C_GRAY, C_CARD);
    tft.setTextDatum(TL_DATUM);
    tft.drawString("APPLE JUICE", 12, 122, 1);
    tft.setTextColor(appleJuiceAttacks > 0 ? C_ALERT_RED : C_GRAY, C_CARD);
    char ajBuf[16];
    snprintf(ajBuf, sizeof(ajBuf), "%lu", appleJuiceAttacks);
    tft.drawString(ajBuf, 12, 136, 2);

    // 2. Android Fast Pair
    tft.fillRoundRect(86, 118, bw, bh, 4, C_CARD);
    tft.setTextColor(C_GRAY, C_CARD);
    tft.drawString("FAST PAIR", 90, 122, 1);
    tft.setTextColor(fastPairFloods > 0 ? C_YELLOW : C_GRAY, C_CARD);
    char fpBuf[16];
    snprintf(fpBuf, sizeof(fpBuf), "%lu", fastPairFloods);
    tft.drawString(fpBuf, 90, 136, 2);

    // 3. MS Swift Pair
    tft.fillRoundRect(164, 118, bw, bh, 4, C_CARD);
    tft.setTextColor(C_GRAY, C_CARD);
    tft.drawString("SWIFT PAIR", 168, 122, 1);
    tft.setTextColor(swiftPairFloods > 0 ? C_CYAN : C_GRAY, C_CARD);
    char spBuf[16];
    snprintf(spBuf, sizeof(spBuf), "%lu", swiftPairFloods);
    tft.drawString(spBuf, 168, 136, 2);

    // 4. AirTags
    tft.fillRoundRect(242, 118, bw, bh, 4, C_CARD);
    tft.setTextColor(C_GRAY, C_CARD);
    tft.drawString("AIRTAGS", 246, 122, 1);
    tft.setTextColor(airTagsDetected > 0 ? C_PURPLE : C_GRAY, C_CARD);
    char atBuf[16];
    snprintf(atBuf, sizeof(atBuf), "%lu", airTagsDetected);
    tft.drawString(atBuf, 246, 136, 2);

    // Audio alarm toggle pill (X: 8, Y: 168, W: 304, H: 24)
    tft.fillRoundRect(8, 168, 304, 24, 4, alarmEnabled ? C_CARD : C_DARKGRAY);
    tft.drawRoundRect(8, 168, 304, 24, 4, alarmEnabled ? C_GREEN : C_CARD_BORDER);
    tft.setTextColor(alarmEnabled ? C_GREEN : C_GRAY, alarmEnabled ? C_CARD : C_DARKGRAY);
    tft.setTextDatum(MC_DATUM);
    tft.drawString(alarmEnabled ? "ACOUSTIC INTRUSION ALARM: ACTIVE" : "ACOUSTIC INTRUSION ALARM: MUTED", 160, 180, 2);
}

// --- TAB 1: ATTACK LOGS ---
void drawTabLogs() {
    tft.fillRect(0, 25, 320, 186, C_BG);

    tft.setTextColor(C_GRAY, C_BG);
    tft.setTextDatum(TL_DATUM);
    tft.drawString("ATTACK VECTOR", 12, 28, 1);
    tft.drawString("SPOOFED MAC", 145, 28, 1);
    tft.drawString("RSSI", 270, 28, 1);
    tft.drawFastHLine(8, 38, 304, C_CARD_BORDER);

    if (logCount == 0) {
        tft.setTextColor(C_GREEN, C_BG);
        tft.setTextDatum(MC_DATUM);
        tft.drawString("No malicious spoofing floods logged.", 160, 100, 2);
        tft.setTextColor(C_GRAY, C_BG);
        tft.drawString("Airspace is clear of Flipper Zero / AppleJuice packets.", 160, 120, 1);
    } else {
        for (int i = 0; i < logCount && i < 5; i++) {
            int y = 44 + (i * 26);
            tft.fillRoundRect(8, y, 304, 22, 4, C_CARD);

            tft.setTextColor(C_ALERT_RED, C_CARD);
            tft.drawString(logs[i].type, 14, y + 4, 1);

            tft.setTextColor(C_WHITE, C_CARD);
            tft.drawString(logs[i].mac, 145, y + 4, 1);

            char rssiBuf[12];
            snprintf(rssiBuf, sizeof(rssiBuf), "%d dBm", logs[i].rssi);
            tft.setTextColor(logs[i].rssi > -65 ? C_YELLOW : C_GRAY, C_CARD);
            tft.drawString(rssiBuf, 270, y + 4, 1);
        }
    }
}

// --- TAB 2: AIRTAG RADAR ---
void drawTabRadar() {
    tft.fillRect(0, 25, 320, 186, C_BG);

    tft.setTextColor(C_GRAY, C_BG);
    tft.setTextDatum(TL_DATUM);
    tft.drawString("TRACKING BEACON MAC", 12, 28, 1);
    tft.drawString("SIGNAL LEVEL", 185, 28, 1);
    tft.drawString("DISTANCE", 255, 28, 1);
    tft.drawFastHLine(8, 38, 304, C_CARD_BORDER);

    if (airTagCount == 0) {
        // Show demo airtag if clean
        tft.fillRoundRect(8, 44, 304, 26, 4, C_CARD);
        tft.setTextColor(C_PURPLE, C_CARD);
        tft.drawString("4A:8B:E3:12:9F:80", 14, 50, 2);

        // Signal gauge
        tft.fillRoundRect(185, 50, 55, 10, 3, C_GREEN);
        tft.setTextColor(C_WHITE, C_CARD);
        tft.drawString("-58 dBm", 255, 50, 1);

        tft.setTextColor(C_GRAY, C_BG);
        tft.setTextDatum(BC_DATUM);
        tft.drawString("APPLE FINDMY & TILE AIRTAG RADAR ACTIVE", 160, 195, 1);
    } else {
        for (int i = 0; i < airTagCount && i < 4; i++) {
            int y = 44 + (i * 32);
            tft.fillRoundRect(8, y, 304, 28, 4, C_CARD);

            tft.setTextColor(C_PURPLE, C_CARD);
            tft.drawString(airTags[i].mac, 14, y + 6, 2);

            int barW = map(airTags[i].rssi, -100, -40, 5, 55);
            barW = constrain(barW, 5, 55);
            tft.fillRoundRect(185, y + 8, barW, 10, 3, C_GREEN);

            char distBuf[16];
            snprintf(distBuf, sizeof(distBuf), "%d dBm", airTags[i].rssi);
            tft.setTextColor(C_WHITE, C_CARD);
            tft.drawString(distBuf, 255, y + 6, 1);
        }
    }
}

void renderCurrentTab() {
    drawHeader();
    drawTabBar();
    switch (currentTab) {
        case TAB_SHIELD: drawTabShield(); break;
        case TAB_LOGS:   drawTabLogs(); break;
        case TAB_RADAR:  drawTabRadar(); break;
    }
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

    // 1. Tab Bar Navigation (Y >= 210)
    if (ty >= 210) {
        int tw = 320 / 3;
        int tIdx = tx / tw;
        if (tIdx >= 0 && tIdx < 3 && tIdx != currentTab) {
            currentTab = tIdx;
            playTouchClick();
            renderCurrentTab();
            delay(250);
            return;
        }
    }

    // 2. Alarm Mute Toggle (Tab 0, Y: 168..192)
    if (currentTab == TAB_SHIELD && ty >= 168 && ty <= 192) {
        alarmEnabled = !alarmEnabled;
        playTouchClick();
        drawTabShield();
        delay(250);
        return;
    }

    // 3. Simulated Demo Attack Trigger (Tap Center Shield)
    if (currentTab == TAB_SHIELD && ty >= 28 && ty <= 112) {
        appleJuiceAttacks++;
        addIncident("AppleJuice Demo", "DE:AD:BE:EF:44:21", -52);
        drawTabShield();
        delay(300);
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

    // Sentinel Boot Screen
    tft.setTextColor(C_CYAN, C_BG);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("IamNotIntoTech", 160, 80, 4);
    tft.setTextColor(C_ALERT_RED, C_BG);
    tft.drawString("BLE SPAM & AIRTAG SENTINEL", 160, 120, 2);
    tft.setTextColor(C_WHITE, C_BG);
    tft.drawString("Initializing BLE 5.0 Core Promiscuous Scan...", 160, 150, 1);

    playTouchClick();
    delay(100);
    tone(BUZZER_PIN, 2400, 60);
    delay(700);

    // Init BLE
    BLEDevice::init("");
    pBLEScan = BLEDevice::getScan();
    pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
    pBLEScan->setActiveScan(true);
    pBLEScan->setInterval(100);
    pBLEScan->setWindow(99);

    tft.fillScreen(C_BG);

    renderCurrentTab();
}

void loop() {
    handleTouch();

    // Run non-blocking 1-second BLE scan slice
    pBLEScan->start(1, false);
    pBLEScan->clearResults();

    unsigned long now = millis();

    // 1-second packet rate calculation
    if (now - lastRateSecond >= 1000) {
        lastRateSecond = now;
        currentPktRate = secondFrameCounter;
        secondFrameCounter = 0;

        if (currentTab == TAB_SHIELD) {
            drawHeader();
            drawTabShield();
        }
    }

    delay(20);
}
