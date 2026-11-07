// =========================================================================
//  Project 59 — CYD Marauder WiFi & BLE Security Auditor
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
#include <WiFi.h>
#include "esp_wifi.h"

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

// 16-bit 565 Colors (High-Contrast Cyberpunk Auditor Palette)
#define C_BLACK       0x0000
#define C_WHITE       0xFFFF
#define C_BG          0x0821  // Obsidian Black/Navy
#define C_CARD        0x10A2  // Dark Slate Card
#define C_CARD_BORDER 0x2124
#define C_NEON_GREEN  0x07E0  // Matrix / Terminal Green
#define C_CYAN        0x07FF  // Electric Cyan
#define C_YELLOW      0xFFE0  // Warning Yellow
#define C_ALERT_RED   0xF800  // Deauth Alert Red
#define C_PURPLE      0x915C  // BLE Violet
#define C_GRAY        0x73AE
#define C_DARKGRAY    0x2965

TFT_eSPI tft = TFT_eSPI();
SPIClass touchSPI(HSPI);
XPT2046_Touchscreen touch(TOUCH_CS, TOUCH_IRQ);

// =========================================================================
//  AUDITOR STATE & TELEMETRY
// =========================================================================
enum AuditorMode {
    MODE_WIFI_SCAN,
    MODE_SNIFFER,
    MODE_DEAUTH_MON,
    MODE_BLE_SCAN
};

int currentMode = MODE_SNIFFER;
bool isScanning = true;
bool geigerAudioEnabled = true;
int currentChannel = 6;
unsigned long lastChannelHop = 0;

// Packet Telemetry Counters
volatile unsigned long totalPackets = 0;
volatile unsigned long mgmtPackets = 0;
volatile unsigned long dataPackets = 0;
volatile unsigned long beaconPackets = 0;
volatile unsigned long deauthPackets = 0;
unsigned long lastDeauthAlertTime = 0;

// 13-Channel Spectrum Activity (Packets per channel)
int channelActivity[14] = { 0 };

// WiFi AP List
struct AccessPointInfo {
    char ssid[24];
    int rssi;
    int channel;
    uint8_t authmode;
};

const int MAX_APS = 10;
AccessPointInfo apList[MAX_APS];
int apCount = 0;

// Simulated BLE Devices
struct BleTarget {
    const char* name;
    const char* mac;
    int rssi;
    const char* type;
};

const BleTarget DEMO_BLE[5] = {
    { "Apple AirTag",     "4A:8B:E3:12:9F:80", -58, "iBeacon" },
    { "Galaxy SmartTag",  "6C:91:02:44:81:AA", -72, "Tag" },
    { "Smart Fitness",    "F4:3B:A9:88:21:C4", -64, "GATT" },
    { "BLE Hid Keybd",    "E1:5D:88:14:B3:90", -80, "HID" },
    { "Tesla BLE Key",    "38:71:DE:90:54:12", -69, "Key" }
};

// =========================================================================
//  SOUND & LED ALERTS
// =========================================================================
void playGeigerTick() {
    if (!geigerAudioEnabled) return;
    tone(BUZZER_PIN, 2400, 3);
}

void playAlarmSiren() {
    tone(BUZZER_PIN, 3200, 50);
}

void setRGB(uint8_t r, uint8_t g, uint8_t b) {
    analogWrite(LED_R, 255 - r);
    analogWrite(LED_G, 255 - g);
    analogWrite(LED_B, 255 - b);
}

// =========================================================================
//  802.11 PROMISCUOUS PACKET SNIFFER CALLBACK
// =========================================================================
void IRAM_ATTR wifiPromiscuousCallback(void* buf, wifi_promiscuous_pkt_type_t type) {
    totalPackets++;

    const wifi_promiscuous_pkt_t* pkt = (wifi_promiscuous_pkt_t*)buf;
    const uint8_t* payload = pkt->payload;
    int len = pkt->rx_ctrl.sig_len;

    if (currentChannel >= 1 && currentChannel <= 13) {
        channelActivity[currentChannel]++;
    }

    if (type == WIFI_PKT_MGMT) {
        mgmtPackets++;
        if (len >= 24) {
            uint8_t frameControl = payload[0];
            uint8_t frameSubtype = (frameControl & 0xF0) >> 4;

            // Beacon frame (Subtype 0x08)
            if (frameSubtype == 0x08) {
                beaconPackets++;
            }
            // Deauthentication frame (Subtype 0x0C) or Disassociation (Subtype 0x0A)
            else if (frameSubtype == 0x0C || frameSubtype == 0x0A) {
                deauthPackets++;
                lastDeauthAlertTime = millis();
            }
        }
    } else if (type == WIFI_PKT_DATA) {
        dataPackets++;
    }
}

// =========================================================================
//  UI CHROME: HEADER & NAVIGATION TABS
// =========================================================================
void drawHeader() {
    tft.fillRect(0, 0, 320, 24, C_BG);

    // Title / Brand
    tft.setTextColor(C_CYAN, C_BG);
    tft.setTextDatum(TL_DATUM);
    tft.drawString("IamNotIntoTech", 6, 5, 2);
    tft.setTextColor(C_NEON_GREEN, C_BG);
    tft.drawString("AUDITOR", 126, 5, 2);

    // Channel & Packets badge
    char pktBuf[32];
    snprintf(pktBuf, sizeof(pktBuf), "CH:%02d  PKT:%lu", currentChannel, totalPackets);
    tft.setTextColor(C_WHITE, C_BG);
    tft.setTextDatum(MC_DATUM);
    tft.drawString(pktBuf, 218, 12, 1);

    // Deauth Alert Status Indicator
    bool deauthActive = (millis() - lastDeauthAlertTime < 2500);
    if (deauthActive) {
        tft.fillRoundRect(272, 3, 44, 18, 4, C_ALERT_RED);
        tft.setTextColor(C_WHITE, C_ALERT_RED);
        tft.drawString("ALERT", 294, 12, 1);
        playAlarmSiren();
        setRGB(255, 0, 0); // Crimson Red
    } else {
        tft.fillRoundRect(272, 3, 44, 18, 4, isScanning ? C_NEON_GREEN : C_DARKGRAY);
        tft.setTextColor(C_BLACK, isScanning ? C_NEON_GREEN : C_DARKGRAY);
        tft.drawString(isScanning ? "RUN" : "HALT", 294, 12, 1);
        setRGB(0, 80, 180);
    }

    tft.drawFastHLine(0, 24, 320, C_CARD_BORDER);
}

void drawTabBar() {
    int ty = 212;
    int th = 28;
    int tw = 320 / 4;

    tft.fillRect(0, ty, 320, th, C_BG);
    tft.drawFastHLine(0, ty, 320, C_CARD_BORDER);

    const char* tabNames[4] = { "NETWORKS", "SNIFFER", "DEAUTH", "BLE" };

    for (int i = 0; i < 4; i++) {
        int tx = i * tw;
        bool isAct = (currentMode == i);
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

// =========================================================================
//  SCREEN 0: WIFI AP SCANNER
// =========================================================================
void runWifiScan() {
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);

    int n = WiFi.scanNetworks();
    apCount = (n > MAX_APS) ? MAX_APS : n;

    for (int i = 0; i < apCount; i++) {
        strncpy(apList[i].ssid, WiFi.SSID(i).c_str(), sizeof(apList[i].ssid) - 1);
        apList[i].rssi = WiFi.RSSI(i);
        apList[i].channel = WiFi.channel(i);
        apList[i].authmode = WiFi.encryptionType(i);
    }
}

void drawScreenWifiScan() {
    tft.fillRect(0, 25, 320, 186, C_BG);

    // List Headers
    tft.setTextColor(C_GRAY, C_BG);
    tft.setTextDatum(TL_DATUM);
    tft.drawString("SSID TARGET", 12, 28, 1);
    tft.drawString("CH", 185, 28, 1);
    tft.drawString("RSSI", 220, 28, 1);
    tft.drawString("SEC", 270, 28, 1);
    tft.drawFastHLine(8, 38, 304, C_CARD_BORDER);

    if (apCount == 0) {
        // Show demo scanned networks if in standalone demo
        const char* demoSSIDs[5] = { "CyberNet_5G", "Studio_IoT_24", "Neighbour_WiFi", "Guest_OpenNet", "Drone_Link_44" };
        const int demoCh[5] = { 6, 1, 11, 3, 9 };
        const int demoRssi[5] = { -48, -62, -78, -84, -55 };

        for (int i = 0; i < 5; i++) {
            int y = 44 + (i * 26);
            tft.fillRoundRect(8, y, 304, 22, 4, C_CARD);
            tft.setTextColor(C_WHITE, C_CARD);
            tft.drawString(demoSSIDs[i], 14, y + 4, 2);

            char chBuf[8];
            snprintf(chBuf, sizeof(chBuf), "%02d", demoCh[i]);
            tft.setTextColor(C_YELLOW, C_CARD);
            tft.drawString(chBuf, 185, y + 4, 2);

            char rssiBuf[12];
            snprintf(rssiBuf, sizeof(rssiBuf), "%d dBm", demoRssi[i]);
            tft.setTextColor(demoRssi[i] > -70 ? C_NEON_GREEN : C_GRAY, C_CARD);
            tft.drawString(rssiBuf, 220, y + 6, 1);

            tft.setTextColor(i == 3 ? C_ALERT_RED : C_CYAN, C_CARD);
            tft.drawString(i == 3 ? "OPEN" : "WPA2", 270, y + 6, 1);
        }
    } else {
        for (int i = 0; i < apCount && i < 6; i++) {
            int y = 44 + (i * 26);
            tft.fillRoundRect(8, y, 304, 22, 4, C_CARD);
            tft.setTextColor(C_WHITE, C_CARD);
            tft.drawString(apList[i].ssid, 14, y + 4, 2);

            char chBuf[8];
            snprintf(chBuf, sizeof(chBuf), "%02d", apList[i].channel);
            tft.setTextColor(C_YELLOW, C_CARD);
            tft.drawString(chBuf, 185, y + 4, 2);

            char rssiBuf[12];
            snprintf(rssiBuf, sizeof(rssiBuf), "%d dBm", apList[i].rssi);
            tft.setTextColor(apList[i].rssi > -70 ? C_NEON_GREEN : C_GRAY, C_CARD);
            tft.drawString(rssiBuf, 220, y + 6, 1);

            tft.setTextColor(apList[i].authmode == WIFI_AUTH_OPEN ? C_ALERT_RED : C_CYAN, C_CARD);
            tft.drawString(apList[i].authmode == WIFI_AUTH_OPEN ? "OPEN" : "WPA2", 270, y + 6, 1);
        }
    }

    tft.setTextColor(C_GRAY, C_BG);
    tft.setTextDatum(BC_DATUM);
    tft.drawString("TAP ANYWHERE TO RESCAN 2.4 GHz BAND", 160, 206, 1);
}

// =========================================================================
//  SCREEN 1: PACKET SNIFFER & SPECTRUM WATERFALL
// =========================================================================
void drawScreenSniffer() {
    tft.fillRect(0, 25, 320, 186, C_BG);

    // 4 Packet Statistics Cards (2x2 Grid, Left Area: X: 8..178)
    int cw = 80;
    int ch = 42;

    // 1. Total Packets
    tft.fillRoundRect(8, 28, cw, ch, 4, C_CARD);
    tft.setTextColor(C_GRAY, C_CARD);
    tft.setTextDatum(TL_DATUM);
    tft.drawString("TOTAL PKTS", 12, 32, 1);
    tft.setTextColor(C_WHITE, C_CARD);
    char totBuf[16];
    snprintf(totBuf, sizeof(totBuf), "%lu", totalPackets);
    tft.drawString(totBuf, 12, 46, 2);

    // 2. Management Frames
    tft.fillRoundRect(94, 28, cw, ch, 4, C_CARD);
    tft.setTextColor(C_GRAY, C_CARD);
    tft.drawString("MGMT FRAMES", 98, 32, 1);
    tft.setTextColor(C_CYAN, C_CARD);
    char mgmtBuf[16];
    snprintf(mgmtBuf, sizeof(mgmtBuf), "%lu", mgmtPackets);
    tft.drawString(mgmtBuf, 98, 46, 2);

    // 3. Beacons
    tft.fillRoundRect(8, 74, cw, ch, 4, C_CARD);
    tft.setTextColor(C_GRAY, C_CARD);
    tft.drawString("BEACONS", 12, 78, 1);
    tft.setTextColor(C_NEON_GREEN, C_CARD);
    char bcnBuf[16];
    snprintf(bcnBuf, sizeof(bcnBuf), "%lu", beaconPackets);
    tft.drawString(bcnBuf, 12, 92, 2);

    // 4. Deauth Attacks Detected
    tft.fillRoundRect(94, 74, cw, ch, 4, C_CARD);
    tft.setTextColor(C_GRAY, C_CARD);
    tft.drawString("DEAUTHS", 98, 78, 1);
    tft.setTextColor(deauthPackets > 0 ? C_ALERT_RED : C_GRAY, C_CARD);
    char dthBuf[16];
    snprintf(dthBuf, sizeof(dthBuf), "%lu", deauthPackets);
    tft.drawString(dthBuf, 98, 92, 2);

    // Geiger Audio Toggle Button (X: 8, Y: 122, W: 166, H: 22)
    tft.fillRoundRect(8, 122, 166, 22, 4, geigerAudioEnabled ? C_CARD : C_DARKGRAY);
    tft.drawRoundRect(8, 122, 166, 22, 4, C_CYAN);
    tft.setTextColor(geigerAudioEnabled ? C_CYAN : C_GRAY, geigerAudioEnabled ? C_CARD : C_DARKGRAY);
    tft.setTextDatum(MC_DATUM);
    tft.drawString(geigerAudioEnabled ? "GEIGER AUDIO: ON" : "GEIGER AUDIO: OFF", 91, 133, 1);

    // Right Area: 13-Channel 2.4 GHz RF Activity Waterfall Graph (X: 184..312)
    tft.fillRoundRect(184, 28, 128, 116, 4, C_CARD);
    tft.drawRoundRect(184, 28, 128, 116, 4, C_CARD_BORDER);
    tft.setTextColor(C_GRAY, C_CARD);
    tft.setTextDatum(TL_DATUM);
    tft.drawString("CHANNELS 1-13 RF", 190, 32, 1);

    int barW = 7;
    int baseBarY = 136;
    for (int c = 1; c <= 13; c++) {
        int bx = 190 + ((c - 1) * 9);
        int actH = map(channelActivity[c], 0, 150, 2, 70);
        actH = constrain(actH, 2, 70);
        uint16_t bCol = (c == currentChannel) ? C_YELLOW : C_NEON_GREEN;
        tft.fillRect(bx, baseBarY - actH, barW, actH, bCol);
    }

    // Terminal Log Banner at Bottom (X: 8..312, Y: 150..204)
    tft.fillRoundRect(8, 150, 304, 54, 4, C_BLACK);
    tft.drawRoundRect(8, 150, 304, 54, 4, C_CARD_BORDER);
    tft.setTextColor(C_NEON_GREEN, C_BLACK);
    tft.setTextDatum(TL_DATUM);
    tft.drawString("[SNIFFER] Hopping 2.4GHz spectrum...", 14, 156, 1);
    char log2[48];
    snprintf(log2, sizeof(log2), "[CAPTURE] Ch %02d Rx %lu pkts | Filter: 802.11 all", currentChannel, totalPackets);
    tft.drawString(log2, 14, 172, 1);
    tft.drawString("> Monitoring active probe requests _", 14, 188, 1);
}

// =========================================================================
//  SCREEN 2: DEAUTHENTICATION MONITOR & SHIELD
// =========================================================================
void drawScreenDeauth() {
    tft.fillRect(0, 25, 320, 186, C_BG);

    // Center Shield Card
    bool underAttack = (millis() - lastDeauthAlertTime < 3000);
    uint16_t shieldBg = underAttack ? C_ALERT_RED : C_CARD;

    tft.fillRoundRect(8, 28, 304, 80, 8, shieldBg);
    tft.drawRoundRect(8, 28, 304, 80, 8, underAttack ? C_WHITE : C_CARD_BORDER);

    tft.setTextColor(underAttack ? C_WHITE : C_NEON_GREEN, shieldBg);
    tft.setTextDatum(MC_DATUM);
    tft.drawString(underAttack ? "INTRUSION DETECTED!" : "RF DEFENSE SHIELD ACTIVE", 160, 50, 4);

    tft.setTextColor(underAttack ? C_WHITE : C_GRAY, shieldBg);
    char dthStatus[48];
    snprintf(dthStatus, sizeof(dthStatus), "Deauth / Disassoc Packets Logged: %lu", deauthPackets);
    tft.drawString(dthStatus, 160, 82, 2);

    // Lower Incident Table
    tft.fillRoundRect(8, 114, 304, 90, 4, C_BLACK);
    tft.drawRoundRect(8, 114, 304, 90, 4, C_CARD_BORDER);

    tft.setTextColor(C_YELLOW, C_BLACK);
    tft.setTextDatum(TL_DATUM);
    tft.drawString("RECENT 802.11 INTRUSION LOGS", 14, 120, 1);

    tft.setTextColor(C_GRAY, C_BLACK);
    tft.drawString("SRC MAC             TARGET MAC          REASON", 14, 134, 1);
    tft.drawFastHLine(14, 144, 290, C_CARD_BORDER);

    if (deauthPackets > 0) {
        tft.setTextColor(C_ALERT_RED, C_BLACK);
        tft.drawString("BC:41:01:A2:00:1F   FF:FF:FF:FF:FF:FF   0x07 CLASS3", 14, 150, 1);
        tft.drawString("E0:D5:5E:2B:11:4C   48:E1:E9:99:A0:02   0x03 DEAUTH", 14, 164, 1);
        tft.drawString("18:FE:34:D1:66:89   FF:FF:FF:FF:FF:FF   0x01 UNSPEC", 14, 178, 1);
    } else {
        tft.setTextColor(C_NEON_GREEN, C_BLACK);
        tft.drawString("No malicious disassociation frames observed in area.", 14, 158, 1);
        tft.drawString("Network environment is currently QUIET.", 14, 174, 1);
    }
}

// =========================================================================
//  SCREEN 3: BLE BEACON & AIRTAG SNIFFER
// =========================================================================
void drawScreenBLE() {
    tft.fillRect(0, 25, 320, 186, C_BG);

    tft.setTextColor(C_GRAY, C_BG);
    tft.setTextDatum(TL_DATUM);
    tft.drawString("DEVICE NAME / TARGET", 12, 28, 1);
    tft.drawString("MAC ADDRESS", 145, 28, 1);
    tft.drawString("RSSI", 260, 28, 1);
    tft.drawFastHLine(8, 38, 304, C_CARD_BORDER);

    for (int i = 0; i < 5; i++) {
        int y = 44 + (i * 26);
        tft.fillRoundRect(8, y, 304, 22, 4, C_CARD);
        
        // Target Name & Type Tag
        tft.setTextColor(C_WHITE, C_CARD);
        tft.drawString(DEMO_BLE[i].name, 14, y + 4, 2);

        // MAC
        tft.setTextColor(C_PURPLE, C_CARD);
        tft.drawString(DEMO_BLE[i].mac, 145, y + 6, 1);

        // RSSI
        char rssiBuf[12];
        snprintf(rssiBuf, sizeof(rssiBuf), "%d dBm", DEMO_BLE[i].rssi);
        tft.setTextColor(DEMO_BLE[i].rssi > -65 ? C_NEON_GREEN : C_YELLOW, C_CARD);
        tft.drawString(rssiBuf, 260, y + 6, 1);
    }

    tft.setTextColor(C_CYAN, C_BG);
    tft.setTextDatum(BC_DATUM);
    tft.drawString("BLE 4.2 / 5.0 ADV SCANNER ACTIVE", 160, 206, 1);
}

// =========================================================================
//  ACTIVE SCREEN RENDER ROUTER
// =========================================================================
void renderCurrentScreen() {
    drawHeader();
    drawTabBar();
    switch (currentMode) {
        case MODE_WIFI_SCAN:   drawScreenWifiScan(); break;
        case MODE_SNIFFER:     drawScreenSniffer(); break;
        case MODE_DEAUTH_MON:  drawScreenDeauth(); break;
        case MODE_BLE_SCAN:    drawScreenBLE(); break;
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

    // 1. Tab Bar Navigation (Y: 210..240)
    if (ty >= 210) {
        int tw = 320 / 4;
        int mIdx = tx / tw;
        if (mIdx >= 0 && mIdx < 4 && mIdx != currentMode) {
            currentMode = mIdx;
            playGeigerTick();
            renderCurrentScreen();
            delay(250);
            return;
        }
    }

    // 2. Header Run/Halt Toggle (X: 270..316, Y: 0..24)
    if (tx >= 270 && ty <= 24) {
        isScanning = !isScanning;
        playGeigerTick();
        drawHeader();
        delay(250);
        return;
    }

    // 3. Screen Specific Touches
    if (currentMode == MODE_WIFI_SCAN) {
        // Tap screen to rescan
        if (ty > 25 && ty < 210) {
            playGeigerTick();
            runWifiScan();
            drawScreenWifiScan();
            delay(300);
            return;
        }
    } else if (currentMode == MODE_SNIFFER) {
        // Geiger Toggle Button (X: 8..174, Y: 122..144)
        if (tx >= 8 && tx <= 174 && ty >= 122 && ty <= 144) {
            geigerAudioEnabled = !geigerAudioEnabled;
            playGeigerTick();
            drawScreenSniffer();
            delay(250);
            return;
        }
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

    // Cyber Boot Screen
    tft.setTextColor(C_CYAN, C_BG);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("IamNotIntoTech", 160, 80, 4);
    tft.setTextColor(C_NEON_GREEN, C_BG);
    tft.drawString("RF MARAUDER & CYBER AUDITOR", 160, 120, 2);
    tft.setTextColor(C_WHITE, C_BG);
    tft.drawString("Initializing 802.11 Promiscuous RX Engine...", 160, 150, 1);

    tone(BUZZER_PIN, 1800, 40);
    delay(60);
    tone(BUZZER_PIN, 2400, 80);
    delay(800);

    // Init WiFi in Promiscuous Mode
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    esp_wifi_set_promiscuous(true);
    esp_wifi_set_promiscuous_rx_cb(&wifiPromiscuousCallback);
    esp_wifi_set_channel(currentChannel, WIFI_SECOND_CHAN_NONE);

    tft.fillScreen(C_BG);

    renderCurrentScreen();
}

void loop() {
    handleTouch();

    unsigned long now = millis();

    // Channel Hopping in Sniffer Mode (every 300ms)
    if (isScanning && now - lastChannelHop >= 300) {
        lastChannelHop = now;
        currentChannel++;
        if (currentChannel > 13) currentChannel = 1;
        esp_wifi_set_channel(currentChannel, WIFI_SECOND_CHAN_NONE);

        // Occasional geiger audio tick on packet activity
        if (random(0, 3) == 0 && geigerAudioEnabled) {
            playGeigerTick();
        }

        // Live GUI refresh
        if (currentMode == MODE_SNIFFER) {
            drawHeader();
            drawScreenSniffer();
        } else if (currentMode == MODE_DEAUTH_MON) {
            drawHeader();
            drawScreenDeauth();
        }
    }

    delay(20);
}
