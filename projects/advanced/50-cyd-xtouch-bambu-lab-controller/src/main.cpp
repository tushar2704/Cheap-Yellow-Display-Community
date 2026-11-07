// =========================================================================
//  Project 50 — CYD X-Touch Bambu Lab 3D Printer Controller
//  Cheap Yellow Display (ESP32-2432S028R)
//
//  📺 Tutorial:  https://youtube.com/@IamNotIntoTech
//  🖨️ Enclosure: https://youtube.com/@Makerrdesigns
//
//  Branding: IamNotIntoTech
// =========================================================================

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include <SPI.h>

// =========================================================================
//  USER CONFIGURATION
// =========================================================================
const char* WIFI_SSID     = "YOUR_WIFI_SSID";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

// Bambu Lab Printer Config (leave empty or unchanged for Demo Simulation Mode)
const char* PRINTER_IP          = "192.168.1.150";
const int   PRINTER_PORT        = 8883;                 // Bambu Local MQTT TLS port
const char* PRINTER_SERIAL      = "01P00A123456789";   // 15-digit device serial
const char* PRINTER_ACCESS_CODE = "12345678";          // 8-character LAN access code

bool DEMO_MODE = true; // Auto-activates if MQTT unreachable or credentials default

// =========================================================================
//  HARDWARE PIN DEFINITIONS (CYD 2.8" ESP32-2432S028R)
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

// Touch calibration
#define TS_MIN_X 240
#define TS_MAX_X 3800
#define TS_MIN_Y 240
#define TS_MAX_Y 3800

// Colors
#define C_BG        0x0821  // Dark charcoal slate
#define C_CARD      0x10A2  // Darker gray card
#define C_BORDER    0x2965  // Subtle border
#define C_CYAN      0x07FF  // Electric Cyan
#define C_GREEN     0x07E0  // Bright Matrix Green
#define C_ORANGE    0xFD20  // Bambu Orange
#define C_RED       0xF800  // Warning Red
#define C_WHITE     0xFFFF  // Crisp White
#define C_YELLOW    0xFFE0  // Yellow
#define C_DIM       0x632C  // Muted Gray
#define C_HOTEND    0xF940  // Hotend flame orange
#define C_BED       0x05BF  // Bed blue

// =========================================================================
//  OBJECTS & STATE
// =========================================================================
TFT_eSPI tft = TFT_eSPI();
SPIClass touchSPI(HSPI);
XPT2046_Touchscreen touch(TOUCH_CS, TOUCH_IRQ);

WiFiClientSecure wifiSecure;
PubSubClient mqttClient(wifiSecure);

// Printer State Data
struct BambuTelemetry {
    // Temperatures
    float nozzle_temp = 220.0f;
    float nozzle_target = 220.0f;
    float bed_temp = 60.0f;
    float bed_target = 60.0f;
    float chamber_temp = 38.0f;

    // Progress
    int print_pct = 42;
    int current_layer = 142;
    int total_layers = 380;
    int remaining_minutes = 104; // 1h 44m
    String print_status = "RUNNING"; // RUNNING, PAUSED, IDLE, FINISH
    String gcode_state = "PREPARE";
    String current_file = "Benchy_Speed_0.16.gcode";

    // Speed profile
    int speed_lvl = 1; // 1=Standard (100%), 2=Sport (124%), 3=Ludicrous (166%), 0=Silent (50%)
    const char* speed_names[4] = {"SILENT (50%)", "STANDARD (100%)", "SPORT (124%)", "LUDICROUS (166%)"};

    // Fans & Light
    int part_fan_pct = 80;
    int aux_fan_pct = 60;
    bool chamber_light = true;

    // AMS Slots
    int active_ams_slot = 0; // 0..3
    uint16_t ams_colors[4] = { 0x0000, 0xFFFF, 0x05FF, 0xF800 }; // Black, White, Cyan, Red
    const char* ams_materials[4] = { "PLA Basic", "PETG HF", "PLA Silk", "TPU 95A" };
    int ams_remain_pct[4] = { 72, 45, 90, 30 };
};

BambuTelemetry printer;

int currentScreen = 0; // 0=Print HUD, 1=AMS Bay, 2=Hardware & Controls
unsigned long lastTouchTime = 0;
unsigned long lastMqttRetry = 0;
unsigned long lastSimUpdate = 0;
unsigned long lastLedBreathe = 0;
int ledBreatheVal = 0;
bool ledBreatheDir = true;

// Forward declarations
void setRGB(uint8_t r, uint8_t g, uint8_t b);
void playTone(int freq, int durationMs);
void drawHeader();
void drawScreen0();
void drawScreen1();
void drawScreen2();
void handleTouch();
void simulateTelemetry();

// =========================================================================
//  RGB & AUDIO HELPERS
// =========================================================================
void setRGB(uint8_t r, uint8_t g, uint8_t b) {
    // CYD uses active-low RGB LEDs (0=ON, 255=OFF)
    analogWrite(LED_R, 255 - r);
    analogWrite(LED_G, 255 - g);
    analogWrite(LED_B, 255 - b);
}

void playTone(int freq, int durationMs) {
    tone(BUZZER_PIN, freq, durationMs);
}

// =========================================================================
//  MQTT CALLBACK & PARSING
// =========================================================================
void onMqttMessage(char* topic, byte* payload, unsigned int length) {
    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, payload, length);
    if (err) return;

    JsonObject printObj = doc["print"];
    if (printObj.isNull()) return;

    if (printObj.containsKey("nozzle_temper")) printer.nozzle_temp = printObj["nozzle_temper"];
    if (printObj.containsKey("nozzle_target_temper")) printer.nozzle_target = printObj["nozzle_target_temper"];
    if (printObj.containsKey("bed_temper")) printer.bed_temp = printObj["bed_temper"];
    if (printObj.containsKey("bed_target_temper")) printer.bed_target = printObj["bed_target_temper"];
    if (printObj.containsKey("mc_remaining_time")) printer.remaining_minutes = printObj["mc_remaining_time"];
    if (printObj.containsKey("mc_percent")) printer.print_pct = printObj["mc_percent"];
    if (printObj.containsKey("layer_num")) printer.current_layer = printObj["layer_num"];
    if (printObj.containsKey("total_layer_num")) printer.total_layers = printObj["total_layer_num"];
    if (printObj.containsKey("gcode_state")) {
        const char* state = printObj["gcode_state"];
        printer.gcode_state = String(state);
        if (printer.gcode_state == "RUNNING") printer.print_status = "PRINTING";
        else if (printer.gcode_state == "PAUSE") printer.print_status = "PAUSED";
        else if (printer.gcode_state == "FINISH") printer.print_status = "FINISHED";
        else printer.print_status = printer.gcode_state;
    }

    if (currentScreen == 0) drawScreen0();
}

void connectMQTT() {
    if (DEMO_MODE) return;
    if (mqttClient.connected()) return;

    unsigned long now = millis();
    if (now - lastMqttRetry < 10000) return;
    lastMqttRetry = now;

    wifiSecure.setInsecure(); // Bambu uses local self-signed TLS cert
    mqttClient.setServer(PRINTER_IP, PRINTER_PORT);
    mqttClient.setCallback(onMqttMessage);

    char clientId[64];
    snprintf(clientId, sizeof(clientId), "CYD-Bambu-%06X", (uint32_t)ESP.getEfuseMac());

    // Bambu local MQTT credentials: username="bblp", password=PRINTER_ACCESS_CODE
    if (mqttClient.connect(clientId, "bblp", PRINTER_ACCESS_CODE)) {
        char reportTopic[128];
        snprintf(reportTopic, sizeof(reportTopic), "device/%s/report", PRINTER_SERIAL);
        mqttClient.subscribe(reportTopic);
        DEMO_MODE = false;
    } else {
        DEMO_MODE = true; // Fallback to demo mode if connection fails
    }
}

// =========================================================================
//  DEMO SIMULATION ENGINE
// =========================================================================
void simulateTelemetry() {
    unsigned long now = millis();
    if (now - lastSimUpdate < 1000) return;
    lastSimUpdate = now;

    // Simulate print progression
    static int secondCount = 0;
    secondCount++;

    if (printer.print_status == "PRINTING" || printer.print_status == "RUNNING") {
        if (secondCount % 5 == 0) {
            printer.print_pct++;
            if (printer.print_pct > 100) printer.print_pct = 0;
            if (printer.remaining_minutes > 0) printer.remaining_minutes--;
            printer.current_layer = (printer.print_pct * printer.total_layers) / 100;
        }

        // Slight temperature jitter for realism
        printer.nozzle_temp = 219.5f + (float)(random(0, 15)) / 10.0f;
        printer.bed_temp = 59.8f + (float)(random(0, 8)) / 10.0f;
    }

    // LED status update
    if (printer.print_status == "PAUSED") {
        setRGB(255, 100, 0); // Amber on pause
    } else if (printer.print_pct >= 100) {
        setRGB(0, 255, 0); // Green on complete
    } else {
        // Cyan breathe while printing
        if (ledBreatheDir) {
            ledBreatheVal += 5;
            if (ledBreatheVal >= 250) ledBreatheDir = false;
        } else {
            ledBreatheVal -= 5;
            if (ledBreatheVal <= 20) ledBreatheDir = true;
        }
        setRGB(0, ledBreatheVal, ledBreatheVal);
    }
}

// =========================================================================
//  GRAPHICS DRAWING HELPERS
// =========================================================================
void drawHeader() {
    tft.fillRect(0, 0, 320, 26, C_CARD);
    tft.drawFastHLine(0, 26, 320, C_BORDER);

    // Title & Branding
    tft.setTextColor(C_ORANGE, C_CARD);
    tft.setTextSize(1);
    tft.drawString("⚡ BAMBU LAB CONTROLLER", 8, 8, 2);

    tft.setTextColor(C_CYAN, C_CARD);
    tft.drawString("IamNotIntoTech", 200, 8, 2);

    // Status pill
    uint16_t statusCol = (DEMO_MODE) ? C_YELLOW : C_GREEN;
    const char* statusTxt = (DEMO_MODE) ? "DEMO" : "ONLINE";
    tft.fillRoundRect(152, 4, 42, 18, 4, C_BG);
    tft.drawRoundRect(152, 4, 42, 18, 4, statusCol);
    tft.setTextColor(statusCol, C_BG);
    tft.drawString(statusTxt, 156, 7, 1);
}

void drawScreen0() {
    tft.fillScreen(C_BG);
    drawHeader();

    // Hotend card (Left)
    tft.fillRoundRect(8, 34, 148, 54, 6, C_CARD);
    tft.drawRoundRect(8, 34, 148, 54, 6, C_BORDER);
    tft.setTextColor(C_HOTEND, C_CARD);
    tft.drawString("NOZZLE", 16, 40, 2);
    char buf[32];
    snprintf(buf, sizeof(buf), "%.0f/%.0f C", printer.nozzle_temp, printer.nozzle_target);
    tft.setTextColor(C_WHITE, C_CARD);
    tft.drawString(buf, 16, 60, 2);

    // Bed card (Right)
    tft.fillRoundRect(164, 34, 148, 54, 6, C_CARD);
    tft.drawRoundRect(164, 34, 148, 54, 6, C_BORDER);
    tft.setTextColor(C_BED, C_CARD);
    tft.drawString("HEATBED", 172, 40, 2);
    snprintf(buf, sizeof(buf), "%.0f/%.0f C", printer.bed_temp, printer.bed_target);
    tft.setTextColor(C_WHITE, C_CARD);
    tft.drawString(buf, 172, 60, 2);

    // Print Progress Card (Center Big)
    tft.fillRoundRect(8, 94, 304, 68, 6, C_CARD);
    tft.drawRoundRect(8, 94, 304, 68, 6, C_BORDER);

    // Progress Bar
    tft.fillRoundRect(20, 104, 280, 16, 4, C_BG);
    int fillW = (printer.print_pct * 280) / 100;
    if (fillW > 0) {
        tft.fillRoundRect(20, 104, fillW, 16, 4, C_GREEN);
    }
    tft.drawRoundRect(20, 104, 280, 16, 4, C_BORDER);

    // Progress percentage & time left
    snprintf(buf, sizeof(buf), "%d%%", printer.print_pct);
    tft.setTextColor(C_WHITE, C_CARD);
    tft.drawString(buf, 20, 130, 4);

    int hours = printer.remaining_minutes / 60;
    int mins = printer.remaining_minutes % 60;
    snprintf(buf, sizeof(buf), "Rem: %02dh %02dm", hours, mins);
    tft.setTextColor(C_YELLOW, C_CARD);
    tft.drawString(buf, 95, 134, 2);

    snprintf(buf, sizeof(buf), "Layer %d/%d", printer.current_layer, printer.total_layers);
    tft.setTextColor(C_CYAN, C_CARD);
    tft.drawString(buf, 210, 134, 2);

    // Speed profile button (Bottom Left)
    tft.fillRoundRect(8, 168, 148, 34, 6, C_CARD);
    tft.drawRoundRect(8, 168, 148, 34, 6, C_ORANGE);
    tft.setTextColor(C_ORANGE, C_CARD);
    tft.drawString(printer.speed_names[printer.speed_lvl], 16, 178, 2);

    // Pause / Resume button (Bottom Right)
    tft.fillRoundRect(164, 168, 148, 34, 6, C_CARD);
    bool isPaused = (printer.print_status == "PAUSED");
    uint16_t btnCol = isPaused ? C_GREEN : C_RED;
    const char* btnTxt = isPaused ? "▶ RESUME" : "⏸ PAUSE";
    tft.drawRoundRect(164, 168, 148, 34, 6, btnCol);
    tft.setTextColor(btnCol, C_CARD);
    tft.drawString(btnTxt, 195, 178, 2);

    // Footer Navigation hint
    tft.fillRect(0, 212, 320, 28, C_CARD);
    tft.drawFastHLine(0, 212, 320, C_BORDER);
    tft.setTextColor(C_DIM, C_CARD);
    tft.drawString("[HUD]    AMS Bay    Hardware    (Tap to switch)", 18, 220, 1);
}

void drawScreen1() {
    tft.fillScreen(C_BG);
    drawHeader();

    tft.setTextColor(C_ORANGE, C_BG);
    tft.drawString("AMS FILAMENT BAY (4 SLOTS)", 12, 34, 2);

    for (int i = 0; i < 4; i++) {
        int x = 10 + (i * 76);
        int y = 56;
        int w = 70;
        int h = 145;

        // Slot card
        tft.fillRoundRect(x, y, w, h, 6, C_CARD);
        bool isActive = (printer.active_ams_slot == i);
        tft.drawRoundRect(x, y, w, h, 6, isActive ? C_GREEN : C_BORDER);

        // Slot header
        char slotBuf[16];
        snprintf(slotBuf, sizeof(slotBuf), "SLOT %d", i + 1);
        tft.setTextColor(isActive ? C_GREEN : C_WHITE, C_CARD);
        tft.drawString(slotBuf, x + 12, y + 8, 2);

        // Spool Color Circle
        tft.fillCircle(x + 35, y + 50, 20, printer.ams_colors[i]);
        tft.drawCircle(x + 35, y + 50, 20, C_WHITE);

        // Material label
        tft.setTextColor(C_CYAN, C_CARD);
        tft.drawString(printer.ams_materials[i], x + 6, y + 80, 1);

        // Gauge bar for remaining filament
        tft.fillRoundRect(x + 10, y + 102, 50, 10, 2, C_BG);
        int fillH = (printer.ams_remain_pct[i] * 50) / 100;
        tft.fillRoundRect(x + 10, y + 102, fillH, 10, 2, C_YELLOW);
        tft.drawRoundRect(x + 10, y + 102, 50, 10, 2, C_BORDER);

        snprintf(slotBuf, sizeof(slotBuf), "%d%%", printer.ams_remain_pct[i]);
        tft.setTextColor(C_WHITE, C_CARD);
        tft.drawString(slotBuf, x + 20, y + 120, 1);
    }

    // Footer
    tft.fillRect(0, 212, 320, 28, C_CARD);
    tft.drawFastHLine(0, 212, 320, C_BORDER);
    tft.setTextColor(C_DIM, C_CARD);
    tft.drawString("HUD    [AMS Bay]    Hardware    (Tap to switch)", 18, 220, 1);
}

void drawScreen2() {
    tft.fillScreen(C_BG);
    drawHeader();

    tft.setTextColor(C_CYAN, C_BG);
    tft.drawString("CHAMBER & HARDWARE CONTROLS", 12, 34, 2);

    // Chamber Temp Card
    tft.fillRoundRect(8, 56, 148, 68, 6, C_CARD);
    tft.drawRoundRect(8, 56, 148, 68, 6, C_BORDER);
    tft.setTextColor(C_WHITE, C_CARD);
    tft.drawString("Chamber Temp", 16, 64, 2);
    char buf[32];
    snprintf(buf, sizeof(buf), "%.1f C", printer.chamber_temp);
    tft.setTextColor(C_YELLOW, C_CARD);
    tft.drawString(buf, 16, 90, 4);

    // Chamber Light Toggle Card
    tft.fillRoundRect(164, 56, 148, 68, 6, C_CARD);
    tft.drawRoundRect(164, 56, 148, 68, 6, printer.chamber_light ? C_YELLOW : C_BORDER);
    tft.setTextColor(C_WHITE, C_CARD);
    tft.drawString("Chamber Light", 172, 64, 2);
    tft.setTextColor(printer.chamber_light ? C_GREEN : C_DIM, C_CARD);
    tft.drawString(printer.chamber_light ? "💡 ON" : "OFF", 172, 90, 4);

    // Part Fan Card
    tft.fillRoundRect(8, 132, 148, 68, 6, C_CARD);
    tft.drawRoundRect(8, 132, 148, 68, 6, C_BORDER);
    tft.setTextColor(C_WHITE, C_CARD);
    tft.drawString("Part Fan", 16, 140, 2);
    snprintf(buf, sizeof(buf), "%d%%", printer.part_fan_pct);
    tft.setTextColor(C_CYAN, C_CARD);
    tft.drawString(buf, 16, 166, 4);

    // Aux Fan Card
    tft.fillRoundRect(164, 132, 148, 68, 6, C_CARD);
    tft.drawRoundRect(164, 132, 148, 68, 6, C_BORDER);
    tft.setTextColor(C_WHITE, C_CARD);
    tft.drawString("Auxiliary Fan", 172, 140, 2);
    snprintf(buf, sizeof(buf), "%d%%", printer.aux_fan_pct);
    tft.setTextColor(C_CYAN, C_CARD);
    tft.drawString(buf, 172, 166, 4);

    // Footer
    tft.fillRect(0, 212, 320, 28, C_CARD);
    tft.drawFastHLine(0, 212, 320, C_BORDER);
    tft.setTextColor(C_DIM, C_CARD);
    tft.drawString("HUD    AMS Bay    [Hardware]    (Tap to switch)", 18, 220, 1);
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

    playTone(1800, 30); // Touch feedback click

    // If bottom bar touched: switch screens
    if (ty >= 200) {
        currentScreen = (currentScreen + 1) % 3;
        if (currentScreen == 0) drawScreen0();
        else if (currentScreen == 1) drawScreen1();
        else drawScreen2();
        return;
    }

    // Screen-specific controls
    if (currentScreen == 0) {
        // Speed button (Bottom Left)
        if (tx >= 8 && tx <= 156 && ty >= 168 && ty <= 202) {
            printer.speed_lvl = (printer.speed_lvl + 1) % 4;
            drawScreen0();
            return;
        }

        // Pause/Resume button (Bottom Right)
        if (tx >= 164 && tx <= 312 && ty >= 168 && ty <= 202) {
            if (printer.print_status == "PAUSED") {
                printer.print_status = "PRINTING";
            } else {
                printer.print_status = "PAUSED";
            }
            drawScreen0();
            return;
        }
    } else if (currentScreen == 1) {
        // Touch on AMS slot to switch active extruder slot (Demo mode)
        for (int i = 0; i < 4; i++) {
            int sx = 10 + (i * 76);
            if (tx >= sx && tx <= sx + 70) {
                printer.active_ams_slot = i;
                drawScreen1();
                return;
            }
        }
    } else if (currentScreen == 2) {
        // Chamber light toggle (Top right card)
        if (tx >= 164 && tx <= 312 && ty >= 56 && ty <= 124) {
            printer.chamber_light = !printer.chamber_light;
            drawScreen2();
            return;
        }
    }
}

// =========================================================================
//  SETUP & MAIN LOOP
// =========================================================================
void setup() {
    Serial.begin(115200);

    // Initialize RGB LED pins (Active LOW)
    pinMode(LED_R, OUTPUT);
    pinMode(LED_G, OUTPUT);
    pinMode(LED_B, OUTPUT);
    setRGB(0, 0, 0); // Turn all OFF

    pinMode(BUZZER_PIN, OUTPUT);
    digitalWrite(BUZZER_PIN, LOW);

    // Display init
    tft.init();
    tft.setRotation(1); // Landscape 320x240
    tft.fillScreen(TFT_BLACK);

    // Splash Screen with IamNotIntoTech branding
    tft.fillScreen(C_BG);
    tft.setTextColor(C_ORANGE, C_BG);
    tft.drawString("X-TOUCH CONTROLLER", 40, 60, 4);
    tft.setTextColor(C_CYAN, C_BG);
    tft.drawString("Bambu Lab 3D Printer Hub", 60, 100, 2);
    tft.setTextColor(C_WHITE, C_BG);
    tft.drawString("Designed with IamNotIntoTech", 65, 140, 2);

    playTone(1200, 80);
    delay(100);
    playTone(1800, 120);

    // Touch init on HSPI
    touchSPI.begin(TOUCH_CLK, TOUCH_MISO, TOUCH_MOSI, TOUCH_CS);
    touch.begin(touchSPI);
    touch.setRotation(1);

    // WiFi Connection
    tft.setTextColor(C_DIM, C_BG);
    tft.drawString("Connecting to WiFi...", 85, 180, 2);

    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    int timeout = 0;
    while (WiFi.status() != WL_CONNECTED && timeout < 10) {
        delay(500);
        timeout++;
    }

    if (WiFi.status() == WL_CONNECTED) {
        connectMQTT();
    } else {
        DEMO_MODE = true;
    }

    delay(1000);
    drawScreen0();
}

void loop() {
    if (!DEMO_MODE && WiFi.status() == WL_CONNECTED) {
        if (!mqttClient.connected()) connectMQTT();
        mqttClient.loop();
    }

    simulateTelemetry();
    handleTouch();
    delay(20);
}

