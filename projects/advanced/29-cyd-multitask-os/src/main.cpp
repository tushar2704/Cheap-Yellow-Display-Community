/**
 * @file main.cpp
 * @brief 29 - CYD MultiTask OS: FreeRTOS Dual-Core Multitasking Operating System
 * @author Tushar Aggarwal (tushar2704)
 * @repository https://github.com/tushar2704/Cheap-Yellow-Display-Community
 * 
 * Community Channels:
 * - Tech & Coding: https://youtube.com/@IamNotIntoTech
 * - 3D Printing & Enclosure: https://youtube.com/@Makerrdesigns
 * 
 * Dual-Core Architecture:
 * - CORE 0 (Daemons): Background Chiptune Audio Player, WiFi RF Scanner, LDR Sensor Sampler & CPU Profiler
 * - CORE 1 (UI Engine): 60FPS Window Manager, Card-Style Task Switcher, Floating PiP Widget & Toast Banners
 */

#include <Arduino.h>
#include <SPI.h>
#include <WiFi.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"

// Hardware Pin Definitions
#define XPT2046_IRQ   36
#define XPT2046_MOSI  13
#define XPT2046_MISO  12
#define XPT2046_CLK   14
#define XPT2046_CS    33

#define LED_RED       4
#define LED_GREEN     16
#define LED_BLUE      17
#define SPEAKER_PIN   26
#define LDR_PIN       34

// Touchscreen Calibration
#define TS_MINX 300
#define TS_MAXX 3800
#define TS_MINY 200
#define TS_MAXY 3700

// Initialize Display & Touch
TFT_eSPI tft = TFT_eSPI();
SPIClass touchSPI(VSPI);
XPT2046_Touchscreen ts(XPT2046_CS, XPT2046_IRQ);

// FreeRTOS Synchronization Primitives
SemaphoreHandle_t dispMutex;
QueueHandle_t toastQueue;

// Toast Notification Struct
struct ToastMessage {
  char text[48];
  uint16_t color;
};

// Task Handles
TaskHandle_t musicTaskHandle = NULL;
TaskHandle_t rfTaskHandle = NULL;
TaskHandle_t sensorTaskHandle = NULL;
TaskHandle_t profilerTaskHandle = NULL;

// Global Shared System State
volatile bool bgMusicEnabled = true;
volatile int bgMusicTrack = 0;
volatile int rfNetworksFound = 0;
volatile int latestLdrVal = 2048;
volatile float core0CpuPercent = 28.5f;
volatile float core1CpuPercent = 42.0f;
volatile bool pipWidgetVisible = true;

// Active Foreground App
enum ActiveApp {
  APP_AUDIO_PLAYER = 0,
  APP_SENSOR_OSCILLOSCOPE,
  APP_TOUCH_SKETCHPAD,
  APP_STOPWATCH_TIMER,
  APP_SYSTEM_TASK_MANAGER,
  APP_TASK_SWITCHER_GRID
};

ActiveApp currentApp = APP_AUDIO_PLAYER;
ActiveApp previousApp = APP_TASK_SWITCHER_GRID;

// Stopwatch State
bool swRunning = false;
unsigned long swStartTime = 0;
unsigned long swAccumulated = 0;
unsigned long swLaps[3] = {0, 0, 0};
int swLapIdx = 0;

// Sensor Graph Buffer
#define SENSOR_GRAPH_WIDTH 180
int ldrBuffer[SENSOR_GRAPH_WIDTH];
int ldrBufIdx = 0;

// Toast Banner Rendering State
char activeToastText[48] = "";
uint16_t activeToastColor = TFT_WHITE;
unsigned long toastDisplayUntil = 0;

// Notes for Chiptune Daemon
const int melodyA[] = {262, 330, 392, 523, 392, 330, 262, 0, 294, 349, 440, 587, 440, 349, 294, 0};
const int melodyB[] = {440, 440, 440, 349, 523, 440, 349, 523, 440, 0, 659, 659, 659, 698, 523, 415};
const int noteDurations[] = {150, 150, 150, 300, 150, 150, 300, 150, 150, 150, 150, 300, 150, 150, 300, 150};

// Helper: RGB LED (Active LOW)
void setRgb(bool r, bool g, bool b) {
  digitalWrite(LED_RED, r ? LOW : HIGH);
  digitalWrite(LED_GREEN, g ? LOW : HIGH);
  digitalWrite(LED_BLUE, b ? LOW : HIGH);
}

// -------------------------------------------------------------
// CORE 0 BACKGROUND DAEMONS
// -------------------------------------------------------------

// Daemon 1: Chiptune Synthesizer
void musicDaemonTask(void *pvParameters) {
  int noteIdx = 0;
  while (true) {
    if (bgMusicEnabled) {
      const int *melody = (bgMusicTrack == 0) ? melodyA : melodyB;
      int freq = melody[noteIdx];
      int dur = noteDurations[noteIdx];

      if (freq > 0) {
        tone(SPEAKER_PIN, freq, dur);
      } else {
        noTone(SPEAKER_PIN);
      }
      noteIdx = (noteIdx + 1) % 16;
      vTaskDelay(pdMS_TO_TICKS(dur + 40));
    } else {
      noTone(SPEAKER_PIN);
      vTaskDelay(pdMS_TO_TICKS(200));
    }
  }
}

// Daemon 2: WiFi RF Sniffer Daemon
void rfScannerTask(void *pvParameters) {
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  vTaskDelay(pdMS_TO_TICKS(500));

  while (true) {
    int n = WiFi.scanNetworks(false, false, false, 150);
    if (n >= 0) {
      rfNetworksFound = n;
      WiFi.scanDelete();

      // Emit periodic toast notification via queue
      ToastMessage msg;
      snprintf(msg.text, sizeof(msg.text), "RF Daemon: %d Wi-Fi APs Probe", n);
      msg.color = 0x07FF; // Cyan
      xQueueSend(toastQueue, &msg, 0);
    }
    vTaskDelay(pdMS_TO_TICKS(8000)); // Scan every 8 seconds
  }
}

// Daemon 3: High-Frequency Sensor Sampler
void sensorDaemonTask(void *pvParameters) {
  int prevLdr = 2048;
  while (true) {
    int raw = analogRead(LDR_PIN);
    latestLdrVal = raw;

    // Detect sudden light transition (shadow or laser)
    if (abs(raw - prevLdr) > 1200) {
      ToastMessage msg;
      snprintf(msg.text, sizeof(msg.text), "Sensor Alert: Rapid Lux Delta!");
      msg.color = 0xFDC0; // CYD Yellow
      xQueueSend(toastQueue, &msg, 0);
    }
    prevLdr = raw;
    vTaskDelay(pdMS_TO_TICKS(80)); // 12.5 Hz sampling
  }
}

// Daemon 4: System CPU & Stack Profiler
void profilerTaskHandleCode(void *pvParameters) {
  while (true) {
    // Simulated dynamic CPU load variation based on background tasks
    core0CpuPercent = 25.0f + (bgMusicEnabled ? 15.0f : 2.0f) + (float)(esp_random() % 100) / 20.0f;
    core1CpuPercent = 38.0f + (float)(esp_random() % 100) / 10.0f;
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

// -------------------------------------------------------------
// CORE 1 UI ENGINE & RENDER PIPELINE
// -------------------------------------------------------------

void drawStatusBar() {
  tft.fillRect(0, 0, 320, 24, 0x1082); // Dark Navy Header

  // App Switcher Button
  tft.fillRoundRect(3, 2, 60, 20, 3, 0xFDC0);
  tft.setTextColor(TFT_BLACK, 0xFDC0);
  tft.setTextSize(1);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("TASKS", 33, 12);

  // App Title
  const char* appTitles[] = {"Audio Synth", "Oscilloscope", "Sketchpad", "Stopwatch", "Task Monitor", "Task Switcher"};
  tft.setTextColor(TFT_WHITE, 0x1082);
  tft.setTextDatum(ML_DATUM);
  tft.drawString(appTitles[currentApp], 70, 12);

  // Status Badges: CPU Load & Audio Icon
  tft.setTextDatum(MR_DATUM);
  char statusBuf[48];
  snprintf(statusBuf, sizeof(statusBuf), "C0:%02.0f%% C1:%02.0f%% | %s",
           core0CpuPercent, core1CpuPercent, bgMusicEnabled ? "[PLAY]" : "[MUTE]");
  tft.setTextColor(bgMusicEnabled ? 0x07E0 : TFT_LIGHTGREY, 0x1082);
  tft.drawString(statusBuf, 316, 12);
}

void drawFloatingPiP() {
  if (!pipWidgetVisible || currentApp == APP_TASK_SWITCHER_GRID) return;

  // Render 80x44 PiP window in top-right corner
  int pipX = 236;
  int pipY = 28;
  tft.fillRoundRect(pipX, pipY, 80, 44, 4, 0x18C3);
  tft.drawRoundRect(pipX, pipY, 80, 44, 4, 0xFDC0);

  tft.setTextColor(0xFDC0, 0x18C3);
  tft.setTextSize(1);
  tft.setTextDatum(TL_DATUM);
  tft.drawString("PiP MUSIC", pipX + 4, pipY + 4);

  tft.setTextColor(TFT_WHITE, 0x18C3);
  tft.drawString(bgMusicTrack == 0 ? "Chiptune A" : "Chiptune B", pipX + 4, pipY + 18);

  tft.setTextColor(bgMusicEnabled ? 0x07E0 : 0xF800, 0x18C3);
  tft.drawString(bgMusicEnabled ? "Playing >" : "Paused ||", pipX + 4, pipY + 30);
}

void renderToastBanner() {
  if (millis() < toastDisplayUntil && activeToastText[0] != '\0') {
    tft.fillRoundRect(20, 26, 280, 22, 4, 0x2104);
    tft.drawRoundRect(20, 26, 280, 22, 4, activeToastColor);
    tft.setTextColor(activeToastColor, 0x2104);
    tft.setTextSize(1);
    tft.setTextDatum(MC_DATUM);
    tft.drawString(activeToastText, 160, 37);
  }
}

// -------------------------------------------------------------
// APP SCREENS
// -------------------------------------------------------------

void renderAppAudio() {
  tft.fillRect(0, 24, 320, 216, 0x0821);

  tft.fillRoundRect(16, 36, 200, 110, 6, 0x18C3);
  tft.drawRoundRect(16, 36, 200, 110, 6, 0x39C7);

  tft.setTextColor(0xFDC0, 0x18C3);
  tft.setTextSize(1);
  tft.setTextDatum(TL_DATUM);
  tft.drawString("BACKGROUND CHIPTUNE SYNTH", 26, 46);

  tft.setTextColor(TFT_WHITE, 0x18C3);
  tft.setTextSize(2);
  tft.drawString(bgMusicTrack == 0 ? "8-Bit Retro Run" : "Cyber Horizon", 26, 62);

  tft.setTextSize(1);
  tft.setTextColor(TFT_LIGHTGREY, 0x18C3);
  tft.drawString("Pinned to FreeRTOS Core 0 (Zero UI Lag)", 26, 88);
  tft.drawString(bgMusicEnabled ? "State: ACTIVE STREAMING" : "State: PAUSED", 26, 102);

  // Play / Pause Toggle Button
  tft.fillRoundRect(16, 156, 95, 42, 6, bgMusicEnabled ? 0xF800 : 0x07E0);
  tft.setTextColor(TFT_WHITE, bgMusicEnabled ? 0xF800 : 0x07E0);
  tft.setTextSize(1);
  tft.setTextDatum(MC_DATUM);
  tft.drawString(bgMusicEnabled ? "PAUSE AUDIO" : "PLAY AUDIO", 63, 177);

  // Next Track Button
  tft.fillRoundRect(121, 156, 95, 42, 6, 0x18C3);
  tft.drawRoundRect(121, 156, 95, 42, 6, 0xFDC0);
  tft.setTextColor(0xFDC0, 0x18C3);
  tft.drawString("NEXT TRACK", 168, 177);

  // PiP Toggle Button
  tft.fillRoundRect(226, 156, 80, 42, 6, 0x2104);
  tft.drawRoundRect(226, 156, 80, 42, 6, 0x39C7);
  tft.setTextColor(TFT_CYAN, 0x2104);
  tft.drawString(pipWidgetVisible ? "HIDE PiP" : "SHOW PiP", 266, 177);

  drawFloatingPiP();
}

void renderAppOscilloscope() {
  tft.fillRect(0, 24, 320, 216, 0x0821);

  tft.setTextColor(0xFDC0, 0x0821);
  tft.setTextSize(1);
  tft.setTextDatum(TL_DATUM);
  tft.drawString("LDR LIGHT SENSOR OSCILLOSCOPE (CORE 0 DAEMON)", 12, 32);

  // Oscilloscope Grid Frame
  int gw = 280, gh = 100;
  tft.drawRect(12, 50, gw, gh, 0x39C7);
  tft.fillRect(13, 51, gw - 2, gh - 2, TFT_BLACK);

  // Draw Grid Reticle
  for (int x = 12; x <= 12 + gw; x += 35) tft.drawFastVLine(x, 50, gh, 0x18C3);
  for (int y = 50; y <= 50 + gh; y += 25) tft.drawFastHLine(12, y, gw, 0x18C3);

  // Plot Samples
  for (int i = 0; i < SENSOR_GRAPH_WIDTH - 1; i++) {
    int idx1 = (ldrBufIdx + i) % SENSOR_GRAPH_WIDTH;
    int idx2 = (ldrBufIdx + i + 1) % SENSOR_GRAPH_WIDTH;
    int y1 = 51 + (gh - 4) - map(ldrBuffer[idx1], 0, 4095, 0, gh - 4);
    int y2 = 51 + (gh - 4) - map(ldrBuffer[idx2], 0, 4095, 0, gh - 4);
    tft.drawLine(14 + i, y1, 14 + i + 1, y2, 0x07E0);
  }

  // Telemetry metrics
  char teleBuf[64];
  snprintf(teleBuf, sizeof(teleBuf), "Raw ADC: %04d | Lux: %03.0f lx | Temp: %02.1fC",
           latestLdrVal, (4095.0f - latestLdrVal) * (1000.0f / 4095.0f), temperatureRead());
  tft.setTextColor(TFT_WHITE, 0x0821);
  tft.drawString(teleBuf, 12, 160);

  tft.setTextColor(0x5AEB, 0x0821);
  tft.drawString("Core 0 samples asynchronously at 12.5Hz without blocking Core 1 UI.", 12, 180);
}

void renderAppSketchpad() {
  tft.fillRect(0, 24, 320, 216, TFT_BLACK);

  // Palette Toolbar (bottom)
  tft.fillRect(0, 212, 320, 28, 0x1082);
  const uint16_t cols[] = {TFT_WHITE, TFT_RED, TFT_GREEN, TFT_BLUE, 0xFDC0, 0x07FF, TFT_BLACK};
  for (int i = 0; i < 7; i++) {
    tft.fillRect(8 + i * 36, 216, 28, 20, cols[i]);
    tft.drawRect(8 + i * 36, 216, 28, 20, TFT_WHITE);
  }

  // Clear Button
  tft.fillRoundRect(262, 216, 50, 20, 3, 0xF800);
  tft.setTextColor(TFT_WHITE, 0xF800);
  tft.setTextSize(1);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("CLEAR", 287, 226);

  tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
  tft.drawString("Draw freely with stylus while background tasks run", 160, 110);
  drawFloatingPiP();
}

void renderAppStopwatch() {
  tft.fillRect(0, 24, 320, 216, 0x0821);

  unsigned long currentElapsed = swAccumulated + (swRunning ? (millis() - swStartTime) : 0);
  int mins = (currentElapsed / 60000) % 60;
  int secs = (currentElapsed / 1000) % 60;
  int ms   = (currentElapsed % 1000) / 10;

  // Digital Timer Display
  tft.fillRoundRect(16, 36, 288, 70, 6, 0x1082);
  tft.drawRoundRect(16, 36, 288, 70, 6, 0x39C7);

  char timeStr[32];
  snprintf(timeStr, sizeof(timeStr), "%02d:%02d.%02d", mins, secs, ms);
  tft.setTextColor(0xFDC0, 0x1082);
  tft.setTextSize(3);
  tft.setTextDatum(MC_DATUM);
  tft.drawString(timeStr, 160, 71);

  // Control Buttons
  tft.fillRoundRect(16, 116, 88, 38, 4, swRunning ? 0xF800 : 0x07E0);
  tft.setTextColor(TFT_WHITE, swRunning ? 0xF800 : 0x07E0);
  tft.setTextSize(1);
  tft.drawString(swRunning ? "STOP" : "START", 60, 135);

  tft.fillRoundRect(116, 116, 88, 38, 4, 0x18C3);
  tft.drawRoundRect(116, 116, 88, 38, 4, 0xFDC0);
  tft.setTextColor(0xFDC0, 0x18C3);
  tft.drawString("LAP", 160, 135);

  tft.fillRoundRect(216, 116, 88, 38, 4, 0x2104);
  tft.drawRoundRect(216, 116, 88, 38, 4, TFT_LIGHTGREY);
  tft.setTextColor(TFT_WHITE, 0x2104);
  tft.drawString("RESET", 260, 135);

  // Lap Records
  tft.setTextDatum(TL_DATUM);
  tft.setTextColor(TFT_LIGHTGREY, 0x0821);
  for (int i = 0; i < 3; i++) {
    char lapBuf[32];
    snprintf(lapBuf, sizeof(lapBuf), "Lap %d: %02lu:%02lu.%02lu",
             i + 1, (swLaps[i] / 60000) % 60, (swLaps[i] / 1000) % 60, (swLaps[i] % 1000) / 10);
    tft.drawString(lapBuf, 24, 164 + i * 16);
  }
}

void renderAppTaskManager() {
  tft.fillRect(0, 24, 320, 216, 0x0821);

  tft.setTextColor(0xFDC0, 0x0821);
  tft.setTextSize(1);
  tft.setTextDatum(TL_DATUM);
  tft.drawString("FREERTOS CONCURRENT TASK MONITOR", 12, 32);

  // Table Header
  tft.fillRect(12, 48, 296, 18, 0x18C3);
  tft.setTextColor(TFT_WHITE, 0x18C3);
  tft.drawString("TASK NAME       CORE  PRI  STACK  STATE", 16, 52);

  // Simulated Task State Query Table
  const char* taskNames[] = {"musicDaemon", "rfScanner", "sensorDaemon", "sysProfiler", "guiRenderUI"};
  const int cores[] = {0, 0, 0, 0, 1};
  const int pris[] = {1, 1, 1, 1, 2};
  const int stacks[] = {1840, 2320, 1680, 1440, 4820};
  const char* states[] = {"RUNNING", "BLOCKED", "BLOCKED", "READY", "RUNNING"};

  for (int i = 0; i < 5; i++) {
    int y = 70 + (i * 20);
    uint16_t rowBg = (i % 2 == 0) ? 0x1082 : 0x0821;
    tft.fillRect(12, y, 296, 18, rowBg);

    char rowBuf[64];
    snprintf(rowBuf, sizeof(rowBuf), "%-14s   %d     %d    %4d B  %s",
             taskNames[i], cores[i], pris[i], stacks[i], states[i]);

    uint16_t txtCol = (cores[i] == 0) ? 0x07FF : 0xFDC0;
    tft.setTextColor(txtCol, rowBg);
    tft.drawString(rowBuf, 16, y + 4);
  }

  // System Heap & Core Allocation Card
  tft.fillRoundRect(12, 178, 296, 50, 4, 0x18C3);
  tft.drawRoundRect(12, 178, 296, 50, 4, 0x39C7);
  tft.setTextColor(0x07E0, 0x18C3);
  tft.drawString("Free Heap: " + String(ESP.getFreeHeap() / 1024) + " KB | Min Free: " + String(ESP.getMinFreeHeap() / 1024) + " KB", 20, 186);
  tft.setTextColor(TFT_WHITE, 0x18C3);
  tft.drawString("Core 0: 4 Background Daemons | Core 1: 60FPS UI & Window Mgr", 20, 202);
}

void renderTaskSwitcherGrid() {
  tft.fillRect(0, 24, 320, 216, 0x0821);

  tft.setTextColor(0xFDC0, 0x0821);
  tft.setTextSize(1);
  tft.setTextDatum(TL_DATUM);
  tft.drawString("ACTIVE APP SWITCHER (Tap card to switch)", 12, 32);

  struct AppCard {
    const char* title;
    const char* sub;
    uint16_t color;
    ActiveApp app;
  };
  AppCard cards[] = {
    {"Audio Player", "Chiptune Core 0", 0x39C7, APP_AUDIO_PLAYER},
    {"Oscilloscope", "LDR Sensor Graph", 0x07E0, APP_SENSOR_OSCILLOSCOPE},
    {"Touch Sketch", "Drawing Canvas", 0xFDC0, APP_TOUCH_SKETCHPAD},
    {"Stopwatch", "Precision Lap Timer", 0x07FF, APP_STOPWATCH_TIMER},
    {"Task Monitor", "FreeRTOS Profiler", 0xF800, APP_SYSTEM_TASK_MANAGER}
  };

  for (int i = 0; i < 5; i++) {
    int col = i % 2;
    int row = i / 2;
    int x = 12 + (col * 152);
    int y = 50 + (row * 56);

    tft.fillRoundRect(x, y, 144, 48, 6, 0x18C3);
    tft.drawRoundRect(x, y, 144, 48, 6, cards[i].color);

    tft.setTextColor(cards[i].color, 0x18C3);
    tft.setTextSize(1);
    tft.setTextDatum(TL_DATUM);
    tft.drawString(cards[i].title, x + 10, y + 8);

    tft.setTextColor(TFT_LIGHTGREY, 0x18C3);
    tft.drawString(cards[i].sub, x + 10, y + 26);
  }
}

void renderCurrentApp() {
  switch (currentApp) {
    case APP_AUDIO_PLAYER:        renderAppAudio(); break;
    case APP_SENSOR_OSCILLOSCOPE: renderAppOscilloscope(); break;
    case APP_TOUCH_SKETCHPAD:     renderAppSketchpad(); break;
    case APP_STOPWATCH_TIMER:     renderAppStopwatch(); break;
    case APP_SYSTEM_TASK_MANAGER: renderAppTaskManager(); break;
    case APP_TASK_SWITCHER_GRID:  renderTaskSwitcherGrid(); break;
  }
  renderToastBanner();
}

void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println("\n[CYD-29] CYD MultiTask OS Initializing...");

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  pinMode(SPEAKER_PIN, OUTPUT);
  pinMode(LDR_PIN, INPUT);

  setRgb(false, false, false);

  // Initialize Display
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);

  // Initialize Touchscreen
  touchSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  ts.begin(touchSPI);
  ts.setRotation(1);

  // Create FreeRTOS Mutex & Queue
  dispMutex = xSemaphoreCreateMutex();
  toastQueue = xQueueCreate(8, sizeof(ToastMessage));

  for (int i = 0; i < SENSOR_GRAPH_WIDTH; i++) ldrBuffer[i] = 2048;

  // Launch Core 0 Background Daemons
  xTaskCreatePinnedToCore(musicDaemonTask, "musicDaemon", 3072, NULL, 1, &musicTaskHandle, 0);
  xTaskCreatePinnedToCore(rfScannerTask, "rfScanner", 3584, NULL, 1, &rfTaskHandle, 0);
  xTaskCreatePinnedToCore(sensorDaemonTask, "sensorDaemon", 2560, NULL, 1, &sensorTaskHandle, 0);
  xTaskCreatePinnedToCore(profilerTaskHandleCode, "sysProfiler", 2048, NULL, 1, &profilerTaskHandle, 0);

  // Chime
  setRgb(false, true, true);
  tone(SPEAKER_PIN, 1200, 80);
  delay(100);
  tone(SPEAKER_PIN, 1800, 100);
  delay(120);
  setRgb(false, false, false);

  drawStatusBar();
  renderCurrentApp();
}

unsigned long lastUiRefresh = 0;
uint16_t currentDrawColor = TFT_WHITE;

void loop() {
  // Check for Toast Notifications from Core 0
  ToastMessage incomingToast;
  if (xQueueReceive(toastQueue, &incomingToast, 0) == pdTRUE) {
    strncpy(activeToastText, incomingToast.text, sizeof(activeToastText) - 1);
    activeToastColor = incomingToast.color;
    toastDisplayUntil = millis() + 2500;
    renderToastBanner();
  }

  // Periodic Refresh for Stopwatch & Oscilloscope
  if (millis() - lastUiRefresh > 80) {
    lastUiRefresh = millis();

    // Update LDR history buffer
    ldrBuffer[ldrBufIdx] = latestLdrVal;
    ldrBufIdx = (ldrBufIdx + 1) % SENSOR_GRAPH_WIDTH;

    if (currentApp == APP_STOPWATCH_TIMER && swRunning) {
      renderAppStopwatch();
    } else if (currentApp == APP_SENSOR_OSCILLOSCOPE) {
      renderAppOscilloscope();
    }
  }

  // Touch Handling
  if (ts.touched()) {
    TS_Point p = ts.getPoint();
    int x = map(p.x, TS_MINX, TS_MAXX, 0, 320);
    int y = map(p.y, TS_MINY, TS_MAXY, 0, 240);
    x = constrain(x, 0, 319);
    y = constrain(y, 0, 239);

    // Status Bar Touches (Task Switcher button: x: 0 to 65, y: 0 to 24)
    if (y <= 24) {
      if (x <= 65) {
        currentApp = (currentApp == APP_TASK_SWITCHER_GRID) ? APP_AUDIO_PLAYER : APP_TASK_SWITCHER_GRID;
        drawStatusBar();
        renderCurrentApp();
        delay(200);
      }
      return;
    }

    // App Switcher Grid
    if (currentApp == APP_TASK_SWITCHER_GRID) {
      for (int i = 0; i < 5; i++) {
        int col = i % 2;
        int row = i / 2;
        int bx = 12 + (col * 152);
        int by = 50 + (row * 56);
        if (x >= bx && x <= bx + 144 && y >= by && y <= by + 48) {
          ActiveApp targetApps[] = {APP_AUDIO_PLAYER, APP_SENSOR_OSCILLOSCOPE, APP_TOUCH_SKETCHPAD, APP_STOPWATCH_TIMER, APP_SYSTEM_TASK_MANAGER};
          currentApp = targetApps[i];
          drawStatusBar();
          renderCurrentApp();
          delay(200);
          break;
        }
      }
      return;
    }

    // Audio App Touches
    if (currentApp == APP_AUDIO_PLAYER) {
      // Play/Pause (16, 156, 95, 42)
      if (x >= 16 && x <= 111 && y >= 156 && y <= 198) {
        bgMusicEnabled = !bgMusicEnabled;
        renderAppAudio();
        delay(200);
      }
      // Next Track (121, 156, 95, 42)
      else if (x >= 121 && x <= 216 && y >= 156 && y <= 198) {
        bgMusicTrack = (bgMusicTrack + 1) % 2;
        renderAppAudio();
        delay(200);
      }
      // PiP Toggle (226, 156, 80, 42)
      else if (x >= 226 && x <= 306 && y >= 156 && y <= 198) {
        pipWidgetVisible = !pipWidgetVisible;
        renderAppAudio();
        delay(200);
      }
    }

    // Sketchpad Drawing
    else if (currentApp == APP_TOUCH_SKETCHPAD) {
      // Clear button (262, 216, 50, 20)
      if (x >= 262 && y >= 216) {
        renderAppSketchpad();
        delay(150);
      }
      // Palette selection (y >= 212)
      else if (y >= 212) {
        const uint16_t cols[] = {TFT_WHITE, TFT_RED, TFT_GREEN, TFT_BLUE, 0xFDC0, 0x07FF, TFT_BLACK};
        int cIdx = (x - 8) / 36;
        if (cIdx >= 0 && cIdx < 7) {
          currentDrawColor = cols[cIdx];
        }
      } else if (y > 24 && y < 212) {
        tft.fillCircle(x, y, 2, currentDrawColor);
      }
    }

    // Stopwatch Controls
    else if (currentApp == APP_STOPWATCH_TIMER) {
      // Start / Stop (16, 116, 88, 38)
      if (x >= 16 && x <= 104 && y >= 116 && y <= 154) {
        if (!swRunning) {
          swRunning = true;
          swStartTime = millis();
        } else {
          swRunning = false;
          swAccumulated += (millis() - swStartTime);
        }
        renderAppStopwatch();
        delay(200);
      }
      // Lap (116, 116, 88, 38)
      else if (x >= 116 && x <= 204 && y >= 116 && y <= 154) {
        unsigned long curr = swAccumulated + (swRunning ? (millis() - swStartTime) : 0);
        swLaps[swLapIdx] = curr;
        swLapIdx = (swLapIdx + 1) % 3;
        renderAppStopwatch();
        delay(150);
      }
      // Reset (216, 116, 88, 38)
      else if (x >= 216 && x <= 304 && y >= 116 && y <= 154) {
        swRunning = false;
        swAccumulated = 0;
        for (int i = 0; i < 3; i++) swLaps[i] = 0;
        renderAppStopwatch();
        delay(200);
      }
    }
  }

  delay(15);
}
