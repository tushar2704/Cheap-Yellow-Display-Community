/*
 * Project 48: Modular Sensor Workbench & I2C Lab Monitor
 * Board: Elecrow ESP32-2432S028R (2.8" ILI9341 TFT + XPT2046 Resistive Touch)
 * Channel: @IamNotIntoTech (https://youtube.com/@IamNotIntoTech)
 * 
 * Hardware Pinouts:
 *  - ILI9341 TFT SPI: MOSI=13, MISO=12, SCK=14, CS=15, DC=2, BL=21
 *  - XPT2046 Touch VSPI: MOSI=32, MISO=39, CLK=25, CS=33
 *  - HY2.0 Port (I2C): IO27 (SDA), IO22 (SCL)
 *  - RGB LED: IO4 (Red), IO16 (Green), IO17 (Blue) - Active LOW
 *  - Piezo Speaker: IO26 (Buzzer tone)
 *  - LDR Light Sensor: IO34 (ADC1)
 */

#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>

#define TFT_BL_PIN    21
#define TOUCH_MOSI    32
#define TOUCH_MISO    39
#define TOUCH_CLK     25
#define TOUCH_CS      33
#define TOUCH_IRQ     36

#define HY20_SDA_PIN  27
#define HY20_SCL_PIN  22

#define RGB_RED_PIN   4
#define RGB_GREEN_PIN 16
#define RGB_BLUE_PIN  17
#define SPEAKER_PIN   26
#define LDR_PIN       34

SPIClass touchSPI(VSPI);
XPT2046_Touchscreen ts(TOUCH_CS);
TFT_eSPI tft = TFT_eSPI();

// UI Tabs
enum WorkbenchTab {
    TAB_STRIP_CHART = 0,
    TAB_I2C_BUS = 1,
    TAB_ALARMS = 2
};
WorkbenchTab activeTab = TAB_STRIP_CHART;

// Data Channel Structure
#define CHART_HISTORY_LEN 64
struct SensorChannel {
    const char* label;
    const char* unit;
    float currentVal;
    float minVal;
    float maxVal;
    float avgVal;
    float alarmThreshold;
    uint16_t traceColor;
    float history[CHART_HISTORY_LEN];
};

SensorChannel chLDR = { "LDR Photocell", "lux", 0, 9999, -9999, 0, 3200, TFT_YELLOW, {0} };
SensorChannel chAux = { "HY2.0 Aux Signal", "mV", 0, 9999, -9999, 0, 2800, TFT_CYAN, {0} };

uint8_t selectedChannelIdx = 0; // 0 = LDR, 1 = Aux
bool isPaused = false;
uint32_t lastSampleTime = 0;
unsigned long lastTouchTime = 0;

// I2C Scanner State
bool i2cDevicePresent[128];
uint8_t detectedCount = 0;
String identifiedDeviceName = "Standalone / Simulated";

void setRGB(bool r, bool g, bool b) {
    digitalWrite(RGB_RED_PIN, r ? LOW : HIGH);
    digitalWrite(RGB_GREEN_PIN, g ? LOW : HIGH);
    digitalWrite(RGB_BLUE_PIN, b ? LOW : HIGH);
}

void playBeep(uint16_t freq, uint16_t durationMs) {
    tone(SPEAKER_PIN, freq, durationMs);
    delay(durationMs);
    noTone(SPEAKER_PIN);
}

const char* identifyI2C(uint8_t addr) {
    switch (addr) {
        case 0x23: return "BH1750 (Ambient Light)";
        case 0x38: return "AHT10/AHT20 (Temp/Hum)";
        case 0x40: return "INA219 (Power Monitor)";
        case 0x44: return "SHT30/SHT31 (Temp/Hum)";
        case 0x45: return "SHT35 (Precision Temp)";
        case 0x62: return "SCD40/SCD41 (NDIR CO2)";
        case 0x68: return "MPU6050 (6-DOF IMU)";
        case 0x76: return "BMP280/BME280 (Press/Temp)";
        case 0x77: return "BME680 (Gas Sensor)";
        default:   return "Generic I2C Peripheral";
    }
}

void scanI2CBus() {
    detectedCount = 0;
    identifiedDeviceName = "No Ext I2C (Internal Mode)";

    for (uint8_t addr = 1; addr < 127; addr++) {
        Wire.beginTransmission(addr);
        uint8_t error = Wire.endTransmission();
        if (error == 0) {
            i2cDevicePresent[addr] = true;
            detectedCount++;
            identifiedDeviceName = String("Found: ") + identifyI2C(addr);
        } else {
            i2cDevicePresent[addr] = false;
        }
    }
}

void drawTopTabs() {
    tft.fillRect(0, 0, 320, 26, 0x18C3);
    tft.drawFastHLine(0, 26, 320, 0x4208);

    // Tab 1: CHARTS
    uint16_t t1Bg = (activeTab == TAB_STRIP_CHART) ? 0x032E : 0x2124;
    tft.fillRect(6, 3, 94, 20, t1Bg);
    tft.drawRect(6, 3, 94, 20, (activeTab == TAB_STRIP_CHART) ? TFT_CYAN : 0x52AA);
    tft.setTextColor((activeTab == TAB_STRIP_CHART) ? TFT_CYAN : TFT_LIGHTGREY, t1Bg);
    tft.drawCentreString("CHARTS", 53, 5, 2);

    // Tab 2: I2C BUS
    uint16_t t2Bg = (activeTab == TAB_I2C_BUS) ? 0x032E : 0x2124;
    tft.fillRect(106, 3, 94, 20, t2Bg);
    tft.drawRect(106, 3, 94, 20, (activeTab == TAB_I2C_BUS) ? TFT_CYAN : 0x52AA);
    tft.setTextColor((activeTab == TAB_I2C_BUS) ? TFT_CYAN : TFT_LIGHTGREY, t2Bg);
    tft.drawCentreString("I2C BUS", 153, 5, 2);

    // Tab 3: ALARMS
    uint16_t t3Bg = (activeTab == TAB_ALARMS) ? 0x032E : 0x2124;
    tft.fillRect(206, 3, 108, 20, t3Bg);
    tft.drawRect(206, 3, 108, 20, (activeTab == TAB_ALARMS) ? TFT_CYAN : 0x52AA);
    tft.setTextColor((activeTab == TAB_ALARMS) ? TFT_CYAN : TFT_LIGHTGREY, t3Bg);
    tft.drawCentreString("ALARMS", 260, 5, 2);
}

void renderChartTab() {
    SensorChannel* ch = (selectedChannelIdx == 0) ? &chLDR : &chAux;

    // Channel header card (Y: 30..62)
    tft.fillRect(6, 30, 308, 32, 0x10A2);
    tft.drawRect(6, 30, 308, 32, 0x3186);

    tft.setTextColor(ch->traceColor, 0x10A2);
    tft.drawString(ch->label, 12, 36, 2);

    // Digital readout
    char valBuf[32];
    snprintf(valBuf, sizeof(valBuf), "%.1f %s", ch->currentVal, ch->unit);
    tft.setTextColor(TFT_WHITE, 0x10A2);
    tft.drawRightString(valBuf, 304, 36, 2);

    // Oscilloscope Grid Area (Y: 66..186, X: 6..314)
    const int ox = 6, oy = 66, ow = 308, oh = 120;
    tft.fillRect(ox, oy, ow, oh, TFT_BLACK);
    tft.drawRect(ox, oy, ow, oh, 0x2965);

    // Horizontal grid division lines
    for (int y = oy + 30; y < oy + oh; y += 30) {
        tft.drawFastHLine(ox, y, ow, 0x18E3);
    }
    // Vertical grid division lines
    for (int x = ox + 50; x < ox + ow; x += 50) {
        tft.drawFastVLine(x, oy, oh, 0x18E3);
    }

    // Min and Max scale markers
    float chartMin = ch->minVal - 10.0f;
    float chartMax = ch->maxVal + 10.0f;
    if (chartMax <= chartMin) chartMax = chartMin + 100.0f;

    char scaleBuf[16];
    tft.setTextColor(0x7BEF, TFT_BLACK);
    snprintf(scaleBuf, sizeof(scaleBuf), "%.0f", chartMax);
    tft.drawString(scaleBuf, ox + 4, oy + 4, 1);
    snprintf(scaleBuf, sizeof(scaleBuf), "%.0f", chartMin);
    tft.drawString(scaleBuf, ox + 4, oy + oh - 12, 1);

    // Plot waveform
    float dx = (float)(ow - 20) / (float)(CHART_HISTORY_LEN - 1);
    int prevX = -1, prevY = -1;

    for (int i = 0; i < CHART_HISTORY_LEN; i++) {
        int px = ox + 10 + (int)(i * dx);
        float norm = (ch->history[i] - chartMin) / (chartMax - chartMin);
        norm = constrain(norm, 0.0f, 1.0f);
        int py = (oy + oh - 6) - (int)(norm * (oh - 12));

        if (prevX >= 0) {
            tft.drawLine(prevX, prevY, px, py, ch->traceColor);
        }
        prevX = px;
        prevY = py;
    }

    // Bottom Action Bar
    tft.fillRect(0, 214, 320, 26, 0x18C3);
    tft.drawFastHLine(0, 213, 320, 0x4208);

    // Toggle Channel Button (6..96)
    tft.fillRect(6, 216, 90, 22, 0x2124);
    tft.drawRect(6, 216, 90, 22, 0x52AA);
    tft.setTextColor(TFT_WHITE, 0x2124);
    tft.drawCentreString(selectedChannelIdx == 0 ? "CH: LDR" : "CH: AUX", 51, 219, 2);

    // Pause/Resume Button (102..182)
    uint16_t pBg = isPaused ? 0x8000 : 0x032E;
    tft.fillRect(102, 216, 80, 22, pBg);
    tft.drawRect(102, 216, 80, 22, isPaused ? TFT_RED : 0x05BF);
    tft.setTextColor(TFT_WHITE, pBg);
    tft.drawCentreString(isPaused ? "RUN" : "PAUSE", 142, 219, 2);

    // Clear / Reset Button (188..248)
    tft.fillRect(188, 216, 60, 22, 0x2124);
    tft.drawRect(188, 216, 60, 22, 0x52AA);
    tft.setTextColor(TFT_WHITE, 0x2124);
    tft.drawCentreString("RESET", 218, 219, 2);

    // Mini stats in corner
    char statBuf[24];
    snprintf(statBuf, sizeof(statBuf), "@IamNotIntoTech");
    tft.setTextColor(0x9E7F, 0x18C3);
    tft.drawRightString(statBuf, 314, 219, 2);
}

void renderI2CTab() {
    tft.fillRect(0, 27, 320, 186, TFT_BLACK);

    // Header info
    tft.fillRect(6, 30, 308, 24, 0x10A2);
    tft.drawRect(6, 30, 308, 24, 0x3186);
    tft.setTextColor(TFT_WHITE, 0x10A2);
    tft.drawString("I2C Address Map (IO27:SDA, IO22:SCL)", 12, 34, 2);

    // Matrix: 16 columns x 8 rows (Addresses 0x00 to 0x7F)
    const int startX = 14;
    const int startY = 58;
    const int boxW = 17;
    const int boxH = 14;

    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 16; col++) {
            uint8_t addr = (row << 4) | col;
            int x = startX + col * 18;
            int y = startY + row * 15;

            uint16_t boxColor = 0x18E3;
            if (addr >= 1 && addr < 127) {
                if (i2cDevicePresent[addr]) {
                    boxColor = 0x07E0; // Bright Green for found
                }
            } else {
                boxColor = 0x0841; // Reserved range
            }

            tft.fillRect(x, y, boxW, boxH, boxColor);
            tft.drawRect(x, y, boxW, boxH, 0x2965);
        }
    }

    // Detected Peripheral Summary
    tft.fillRect(6, 182, 308, 26, 0x10A2);
    tft.drawRect(6, 182, 308, 26, 0x3186);
    tft.setTextColor(TFT_CYAN, 0x10A2);
    tft.drawString(identifiedDeviceName.c_str(), 12, 186, 2);

    // Bottom Bar
    tft.fillRect(0, 214, 320, 26, 0x18C3);
    tft.drawFastHLine(0, 213, 320, 0x4208);

    // Re-Scan Button
    tft.fillRect(6, 216, 110, 22, 0x032E);
    tft.drawRect(6, 216, 110, 22, 0x05BF);
    tft.setTextColor(TFT_WHITE, 0x032E);
    tft.drawCentreString("RE-SCAN BUS", 61, 219, 2);

    char devCountBuf[32];
    snprintf(devCountBuf, sizeof(devCountBuf), "Devices Found: %d", detectedCount);
    tft.setTextColor(TFT_YELLOW, 0x18C3);
    tft.drawRightString(devCountBuf, 314, 219, 2);
}

void renderAlarmsTab() {
    tft.fillRect(0, 27, 320, 186, TFT_BLACK);

    SensorChannel* ch = (selectedChannelIdx == 0) ? &chLDR : &chAux;

    // Card 1: Alarm Threshold Setting
    tft.fillRect(6, 32, 308, 70, 0x10A2);
    tft.drawRect(6, 32, 308, 70, 0x3186);

    tft.setTextColor(TFT_WHITE, 0x10A2);
    tft.drawString("Alarm Trigger Threshold", 14, 38, 2);

    char tBuf[32];
    snprintf(tBuf, sizeof(tBuf), "Limit: %.0f %s", ch->alarmThreshold, ch->unit);
    tft.setTextColor(TFT_YELLOW, 0x10A2);
    tft.drawString(tBuf, 14, 62, 2);

    // Threshold adjust buttons (-100 / +100)
    tft.fillRect(200, 42, 46, 24, 0x2124);
    tft.drawRect(200, 42, 46, 24, 0x52AA);
    tft.setTextColor(TFT_WHITE, 0x2124);
    tft.drawCentreString("-100", 223, 46, 2);

    tft.fillRect(254, 42, 46, 24, 0x2124);
    tft.drawRect(254, 42, 46, 24, 0x52AA);
    tft.setTextColor(TFT_WHITE, 0x2124);
    tft.drawCentreString("+100", 277, 46, 2);

    // Card 2: Live Alert Status
    bool inAlarm = (ch->currentVal >= ch->alarmThreshold);
    uint16_t statusBg = inAlarm ? 0x8000 : 0x0320;
    tft.fillRect(6, 110, 308, 60, statusBg);
    tft.drawRect(6, 110, 308, 60, inAlarm ? TFT_RED : 0x07E0);

    tft.setTextColor(TFT_WHITE, statusBg);
    tft.drawCentreString(inAlarm ? "WARNING: THRESHOLD EXCEEDED!" : "STATUS: NORMAL / SAFE", 160, 130, 2);

    // Bottom Bar
    tft.fillRect(0, 214, 320, 26, 0x18C3);
    tft.drawFastHLine(0, 213, 320, 0x4208);

    // Toggle Channel Button
    tft.fillRect(6, 216, 90, 22, 0x2124);
    tft.drawRect(6, 216, 90, 22, 0x52AA);
    tft.setTextColor(TFT_WHITE, 0x2124);
    tft.drawCentreString(selectedChannelIdx == 0 ? "CH: LDR" : "CH: AUX", 51, 219, 2);

    tft.setTextColor(0x9E7F, 0x18C3);
    tft.drawRightString("Buzzer & RGB Armed", 314, 219, 2);
}

void sampleSensors() {
    if (isPaused) return;

    // 1. Read Onboard LDR Photocell (IO34)
    int rawLDR = analogRead(LDR_PIN);
    float lux = (float)rawLDR * 0.95f;

    chLDR.currentVal = lux;
    if (lux < chLDR.minVal) chLDR.minVal = lux;
    if (lux > chLDR.maxVal) chLDR.maxVal = lux;

    // 2. Aux Signal (Simulated or ADC / I2C derived)
    static float auxAngle = 0.0f;
    auxAngle += 0.12f;
    float simulatedAux = 1800.0f + 600.0f * sinf(auxAngle);
    chAux.currentVal = simulatedAux;
    if (simulatedAux < chAux.minVal) chAux.minVal = simulatedAux;
    if (simulatedAux > chAux.maxVal) chAux.maxVal = simulatedAux;

    // Shift History FIFO
    float sumLDR = 0;
    float sumAux = 0;
    for (int i = 0; i < CHART_HISTORY_LEN - 1; i++) {
        chLDR.history[i] = chLDR.history[i + 1];
        chAux.history[i] = chAux.history[i + 1];
        sumLDR += chLDR.history[i];
        sumAux += chAux.history[i];
    }
    chLDR.history[CHART_HISTORY_LEN - 1] = chLDR.currentVal;
    chAux.history[CHART_HISTORY_LEN - 1] = chAux.currentVal;

    sumLDR += chLDR.currentVal;
    sumAux += chAux.currentVal;

    chLDR.avgVal = sumLDR / CHART_HISTORY_LEN;
    chAux.avgVal = sumAux / CHART_HISTORY_LEN;

    // Alarm monitoring
    SensorChannel* activeCh = (selectedChannelIdx == 0) ? &chLDR : &chAux;
    if (activeCh->currentVal >= activeCh->alarmThreshold) {
        setRGB(true, false, false); // RED
        playBeep(2600, 30);
    } else if (activeCh->currentVal >= activeCh->alarmThreshold * 0.85f) {
        setRGB(true, true, false); // Yellow / Amber
    } else {
        setRGB(false, true, false); // Green Normal
    }
}

void handleTouch() {
    if (!ts.touched()) return;

    TS_Point p = ts.getPoint();
    if (p.z > 50 || ts.touched()) {
        int touchX = map(p.x, 240, 3800, 0, 320);
        int touchY = map(p.y, 240, 3800, 0, 240);
        touchX = constrain(touchX, 0, 319);
        touchY = constrain(touchY, 0, 239);

        // Top Tabs (Y: 0..26)
        if (touchY <= 26) {
            if (millis() - lastTouchTime > 200) {
                lastTouchTime = millis();
                if (touchX >= 6 && touchX <= 100) {
                    activeTab = TAB_STRIP_CHART;
                    playBeep(1800, 25);
                    drawTopTabs();
                    tft.fillRect(0, 27, 320, 213, TFT_BLACK);
                    return;
                }
                if (touchX >= 106 && touchX <= 200) {
                    activeTab = TAB_I2C_BUS;
                    playBeep(1800, 25);
                    drawTopTabs();
                    scanI2CBus();
                    renderI2CTab();
                    return;
                }
                if (touchX >= 206 && touchX <= 314) {
                    activeTab = TAB_ALARMS;
                    playBeep(1800, 25);
                    drawTopTabs();
                    renderAlarmsTab();
                    return;
                }
            }
            return;
        }

        // Chart Tab Actions
        if (activeTab == TAB_STRIP_CHART && touchY >= 214) {
            if (millis() - lastTouchTime > 200) {
                lastTouchTime = millis();
                // Toggle Channel (6..96)
                if (touchX >= 6 && touchX <= 96) {
                    selectedChannelIdx = 1 - selectedChannelIdx;
                    playBeep(1600, 30);
                    return;
                }
                // Pause/Run (102..182)
                if (touchX >= 102 && touchX <= 182) {
                    isPaused = !isPaused;
                    playBeep(isPaused ? 1000 : 2000, 30);
                    return;
                }
                // Reset min/max (188..248)
                if (touchX >= 188 && touchX <= 248) {
                    SensorChannel* ch = (selectedChannelIdx == 0) ? &chLDR : &chAux;
                    ch->minVal = ch->currentVal;
                    ch->maxVal = ch->currentVal;
                    playBeep(1400, 30);
                    return;
                }
            }
            return;
        }

        // I2C Tab Actions
        if (activeTab == TAB_I2C_BUS && touchY >= 214) {
            if (millis() - lastTouchTime > 250) {
                lastTouchTime = millis();
                if (touchX >= 6 && touchX <= 116) {
                    playBeep(2100, 35);
                    scanI2CBus();
                    renderI2CTab();
                    return;
                }
            }
            return;
        }

        // Alarms Tab Actions
        if (activeTab == TAB_ALARMS) {
            if (millis() - lastTouchTime > 150) {
                lastTouchTime = millis();
                SensorChannel* ch = (selectedChannelIdx == 0) ? &chLDR : &chAux;
                // Threshold -100 (200..246, Y: 40..70)
                if (touchX >= 200 && touchX <= 246 && touchY >= 40 && touchY <= 70) {
                    ch->alarmThreshold = max(100.0f, ch->alarmThreshold - 100.0f);
                    playBeep(1200, 25);
                    renderAlarmsTab();
                    return;
                }
                // Threshold +100 (254..300, Y: 40..70)
                if (touchX >= 254 && touchX <= 300 && touchY >= 40 && touchY <= 70) {
                    ch->alarmThreshold += 100.0f;
                    playBeep(1600, 25);
                    renderAlarmsTab();
                    return;
                }
                // Bottom toggle channel
                if (touchY >= 214 && touchX >= 6 && touchX <= 96) {
                    selectedChannelIdx = 1 - selectedChannelIdx;
                    playBeep(1500, 25);
                    renderAlarmsTab();
                    return;
                }
            }
        }
    }
}

void setup() {
    Serial.begin(115200);

    pinMode(RGB_RED_PIN, OUTPUT);
    pinMode(RGB_GREEN_PIN, OUTPUT);
    pinMode(RGB_BLUE_PIN, OUTPUT);
    pinMode(LDR_PIN, INPUT);
    setRGB(false, false, false);

    pinMode(SPEAKER_PIN, OUTPUT);
    digitalWrite(SPEAKER_PIN, LOW);

    pinMode(TFT_BL_PIN, OUTPUT);
    digitalWrite(TFT_BL_PIN, HIGH);

    Wire.begin(HY20_SDA_PIN, HY20_SCL_PIN, 100000);

    // Dedicated VSPI for Touch Controller (Pins 25, 39, 32, 33)
    touchSPI.begin(TOUCH_CLK, TOUCH_MISO, TOUCH_MOSI, TOUCH_CS);
    ts.begin(touchSPI);
    ts.setRotation(1);

    tft.init();
    tft.setRotation(1);
    tft.fillScreen(TFT_BLACK);

    // Initial sensor sample
    for (int i = 0; i < CHART_HISTORY_LEN; i++) {
        chLDR.history[i] = 1200;
        chAux.history[i] = 1500;
    }

    scanI2CBus();
    drawTopTabs();
    playBeep(1600, 60);
    lastSampleTime = millis();
    Serial.println(F("[SYSTEM] Modular Sensor Workbench & I2C Lab Monitor Initialized."));
}

void loop() {
    handleTouch();

    if (millis() - lastSampleTime >= 120) {
        lastSampleTime = millis();
        sampleSensors();

        if (activeTab == TAB_STRIP_CHART) {
            renderChartTab();
        }
    }

    delay(10);
}
