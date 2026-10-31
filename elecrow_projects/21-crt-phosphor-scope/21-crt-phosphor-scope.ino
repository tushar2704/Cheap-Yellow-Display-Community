/**
 * 📟 Project 21: Retro CRT Phosphor Bench Oscilloscope
 * 
 * Hardware: Elecrow CrowPanel 2.8" (ESP32-2432S028R)
 * Channel Branding: IamNotIntoTech (https://youtube.com/@IamNotIntoTech)
 * Author: Tushar Aggarwal (tushar2704)
 * 
 * Features:
 * - High-speed ADC waveform sampling on IO34 (LDR / Analog input).
 * - Simulated CRT Green Phosphor display with decaying persistence trails.
 * - 8x10 division laboratory oscilloscope graticule with edge trigger lock.
 * - Auto-computing metrics: Vpp, Vavg, Vmin, Vmax, and Frequency counter.
 * - Onboard calibration PWM generator on IO27 (1kHz square wave output).
 * - Hardware active-low RGB LED indicating trigger lock and run/stop state.
 * - Calibrated non-inverted resistive touch controls.
 */

#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>

// Hardware Pin Definitions for Elecrow 2.8" CYD
#define TFT_BL_PIN    21
#define TOUCH_MOSI    32
#define TOUCH_MISO    39
#define TOUCH_CLK     25
#define TOUCH_CS      33
#define TOUCH_IRQ     36

#define LED_RED       4
#define LED_GREEN     16
#define LED_BLUE      17
#define BUZZER_PIN    26

#define ANALOG_IN_PIN 34
#define PWM_OUT_PIN   27
#define PWM_CHANNEL   0

TFT_eSPI tft = TFT_eSPI();
SPIClass touchSPI = SPIClass(VSPI);
XPT2046_Touchscreen ts(TOUCH_CS);

const int SCOPE_X = 10;
const int SCOPE_Y = 28;
const int SCOPE_W = 240;
const int SCOPE_H = 168;

const int BUFFER_SIZE = 240;
uint16_t sampleBuffer[BUFFER_SIZE];
uint16_t prevBuffer[BUFFER_SIZE];

bool isRunning = true;
int timebaseIndex = 1; // 0=0.5ms, 1=1ms, 2=2ms, 3=5ms
const char* timebaseLabels[4] = { "0.5ms", "1.0ms", "2.0ms", "5.0ms" };
int timebaseDelays[4] = { 2, 5, 10, 25 }; // sampling delays in microseconds

int voltsDivIndex = 1; // 0=200mV, 1=500mV, 2=1V
const char* voltsDivLabels[3] = { "200mV", "500mV", "1.0V" };

float calculatedVpp = 0.0;
float calculatedVavg = 0.0;
float calculatedFreq = 1000.0;
bool triggerLocked = false;
bool pwmOutputActive = true;
unsigned long lastTouchTime = 0;

void setLED(bool r, bool g, bool b) {
  digitalWrite(LED_RED, r ? LOW : HIGH);
  digitalWrite(LED_GREEN, g ? LOW : HIGH);
  digitalWrite(LED_BLUE, b ? LOW : HIGH);
}

void beep(int freq = 2000, int ms = 30) {
  tone(BUZZER_PIN, freq, ms);
}

void initPWM() {
  ledcSetup(PWM_CHANNEL, 1000, 8); // 1 kHz, 8-bit resolution
  ledcAttachPin(PWM_OUT_PIN, PWM_CHANNEL);
  ledcWrite(PWM_CHANNEL, 128);     // 50% duty cycle
}

void updateStatusLED() {
  if (!isRunning) {
    setLED(false, false, true); // Blue: Trace Frozen / Stopped
  } else if (triggerLocked) {
    setLED(false, true, false); // Green: Trigger Locked & Running
  } else {
    setLED(true, true, false);  // Yellow: Free-run / Searching trigger
  }
}

void drawGraticule() {
  // Clear oscilloscope viewport
  tft.fillRect(SCOPE_X, SCOPE_Y, SCOPE_W, SCOPE_H, TFT_BLACK);
  tft.drawRect(SCOPE_X - 1, SCOPE_Y - 1, SCOPE_W + 2, SCOPE_H + 2, tft.color565(30, 80, 40));

  // 8 Vertical divisions (every 30 px)
  for (int x = SCOPE_X + 30; x < SCOPE_X + SCOPE_W; x += 30) {
    for (int y = SCOPE_Y; y < SCOPE_Y + SCOPE_H; y += 6) {
      tft.drawPixel(x, y, tft.color565(15, 45, 20));
    }
  }

  // 6 Horizontal divisions (every 28 px)
  for (int y = SCOPE_Y + 28; y < SCOPE_Y + SCOPE_H; y += 28) {
    for (int x = SCOPE_X; x < SCOPE_X + SCOPE_W; x += 6) {
      tft.drawPixel(x, y, tft.color565(15, 45, 20));
    }
  }

  // Center crosshair axis
  int midX = SCOPE_X + SCOPE_W / 2;
  int midY = SCOPE_Y + SCOPE_H / 2;
  for (int x = SCOPE_X; x < SCOPE_X + SCOPE_W; x += 4) {
    tft.drawPixel(x, midY, tft.color565(35, 90, 40));
  }
  for (int y = SCOPE_Y; y < SCOPE_Y + SCOPE_H; y += 4) {
    tft.drawPixel(midX, y, tft.color565(35, 90, 40));
  }
}

void acquireWaveform() {
  if (!isRunning) return;

  int sampleDelay = timebaseDelays[timebaseIndex];
  int triggerLevel = 2048; // mid-scale ADC (0-4095)
  triggerLocked = false;

  // Simple rising edge trigger search
  uint32_t triggerStart = micros();
  while ((micros() - triggerStart) < 15000) {
    int val = analogRead(ANALOG_IN_PIN);
    if (val < (triggerLevel - 50)) {
      while ((micros() - triggerStart) < 15000) {
        val = analogRead(ANALOG_IN_PIN);
        if (val >= triggerLevel) {
          triggerLocked = true;
          break;
        }
      }
      break;
    }
  }

  // Sample array
  uint16_t minVal = 4095;
  uint16_t maxVal = 0;
  uint32_t sumVal = 0;

  for (int i = 0; i < BUFFER_SIZE; i++) {
    uint16_t sample = analogRead(ANALOG_IN_PIN);
    sampleBuffer[i] = sample;
    if (sample < minVal) minVal = sample;
    if (sample > maxVal) maxVal = sample;
    sumVal += sample;
    if (sampleDelay > 0) delayMicroseconds(sampleDelay);
  }

  calculatedVpp = ((maxVal - minVal) * 3.3) / 4095.0;
  calculatedVavg = ((sumVal / (float)BUFFER_SIZE) * 3.3) / 4095.0;

  // Estimate frequency from zero-crossings
  int crossings = 0;
  for (int i = 1; i < BUFFER_SIZE; i++) {
    if ((sampleBuffer[i-1] < triggerLevel) && (sampleBuffer[i] >= triggerLevel)) {
      crossings++;
    }
  }
  float totalPeriodUs = (BUFFER_SIZE * (sampleDelay + 10));
  if (crossings >= 2 && totalPeriodUs > 0) {
    calculatedFreq = (crossings * 1000000.0) / totalPeriodUs;
  }
}

void renderPhosphorTrace() {
  // Render CRT decay trail from previous buffer in dim green
  for (int i = 1; i < BUFFER_SIZE; i++) {
    int x1 = SCOPE_X + (i - 1);
    int y1 = SCOPE_Y + SCOPE_H - map(prevBuffer[i-1], 0, 4095, 2, SCOPE_H - 2);
    int x2 = SCOPE_X + i;
    int y2 = SCOPE_Y + SCOPE_H - map(prevBuffer[i], 0, 4095, 2, SCOPE_H - 2);

    y1 = constrain(y1, SCOPE_Y, SCOPE_Y + SCOPE_H - 1);
    y2 = constrain(y2, SCOPE_Y, SCOPE_Y + SCOPE_H - 1);

    tft.drawLine(x1, y1, x2, y2, tft.color565(10, 45, 15)); // Phosphor decay
  }

  // Render fresh trace in electric CRT green
  for (int i = 1; i < BUFFER_SIZE; i++) {
    int x1 = SCOPE_X + (i - 1);
    int y1 = SCOPE_Y + SCOPE_H - map(sampleBuffer[i-1], 0, 4095, 2, SCOPE_H - 2);
    int x2 = SCOPE_X + i;
    int y2 = SCOPE_Y + SCOPE_H - map(sampleBuffer[i], 0, 4095, 2, SCOPE_H - 2);

    y1 = constrain(y1, SCOPE_Y, SCOPE_Y + SCOPE_H - 1);
    y2 = constrain(y2, SCOPE_Y, SCOPE_Y + SCOPE_H - 1);

    // Double thickness for glowing CRT aesthetic
    tft.drawLine(x1, y1, x2, y2, TFT_GREEN);
    tft.drawLine(x1, y1 + 1, x2, y2 + 1, TFT_GREEN);
  }

  // Backup buffer for next persistence frame
  memcpy(prevBuffer, sampleBuffer, sizeof(sampleBuffer));
}

void drawTelemetryBar() {
  // Right sidebar Telemetry Matrix (X: 254 to 320)
  int sx = 254;
  tft.fillRect(sx, SCOPE_Y, 66, SCOPE_H, tft.color565(12, 18, 15));
  tft.drawRect(sx - 1, SCOPE_Y - 1, 68, SCOPE_H + 2, tft.color565(30, 80, 40));

  tft.setTextColor(TFT_LIGHTGREY, tft.color565(12, 18, 15));
  tft.drawString("Vpp", sx + 4, SCOPE_Y + 6, 1);
  tft.setTextColor(TFT_GREEN, tft.color565(12, 18, 15));
  char vppBuf[12];
  snprintf(vppBuf, sizeof(vppBuf), "%.2fV", calculatedVpp);
  tft.drawString(vppBuf, sx + 4, SCOPE_Y + 18, 1);

  tft.setTextColor(TFT_LIGHTGREY, tft.color565(12, 18, 15));
  tft.drawString("Vavg", sx + 4, SCOPE_Y + 36, 1);
  tft.setTextColor(TFT_WHITE, tft.color565(12, 18, 15));
  char vavgBuf[12];
  snprintf(vavgBuf, sizeof(vavgBuf), "%.2fV", calculatedVavg);
  tft.drawString(vavgBuf, sx + 4, SCOPE_Y + 48, 1);

  tft.setTextColor(TFT_LIGHTGREY, tft.color565(12, 18, 15));
  tft.drawString("FREQ", sx + 4, SCOPE_Y + 66, 1);
  tft.setTextColor(TFT_YELLOW, tft.color565(12, 18, 15));
  char freqBuf[12];
  if (calculatedFreq >= 1000.0) {
    snprintf(freqBuf, sizeof(freqBuf), "%.1fkHz", calculatedFreq / 1000.0);
  } else {
    snprintf(freqBuf, sizeof(freqBuf), "%dHz", (int)calculatedFreq);
  }
  tft.drawString(freqBuf, sx + 4, SCOPE_Y + 78, 1);

  tft.setTextColor(TFT_LIGHTGREY, tft.color565(12, 18, 15));
  tft.drawString("TIME/DIV", sx + 4, SCOPE_Y + 98, 1);
  tft.setTextColor(TFT_CYAN, tft.color565(12, 18, 15));
  tft.drawString(timebaseLabels[timebaseIndex], sx + 4, SCOPE_Y + 110, 1);

  tft.setTextColor(TFT_LIGHTGREY, tft.color565(12, 18, 15));
  tft.drawString("VOLT/DIV", sx + 4, SCOPE_Y + 128, 1);
  tft.setTextColor(TFT_CYAN, tft.color565(12, 18, 15));
  tft.drawString(voltsDivLabels[voltsDivIndex], sx + 4, SCOPE_Y + 140, 1);
}

void drawHeader() {
  tft.fillRect(0, 0, 320, 26, tft.color565(10, 25, 15));
  tft.setTextColor(TFT_GREEN, tft.color565(10, 25, 15));
  tft.setTextSize(1);
  tft.drawString("CRT BENCH OSCILLOSCOPE", 8, 6, 2);

  char statBuf[32];
  snprintf(statBuf, sizeof(statBuf), "%s  %s", isRunning ? "RUN" : "STOP", triggerLocked ? "TRIG'D" : "AUTO");
  tft.setTextColor(isRunning ? (triggerLocked ? TFT_GREEN : TFT_YELLOW) : TFT_RED, tft.color565(10, 25, 15));
  tft.drawRightString(statBuf, 312, 6, 2);
}

void drawBottomControls() {
  // 4 Bottom touch buttons (Y: 204 to 240)
  int bw = 74;
  int bh = 32;

  // 1. Timebase
  tft.fillRoundRect(8, 204, bw, bh, 4, tft.color565(25, 35, 30));
  tft.drawRoundRect(8, 204, bw, bh, 4, tft.color565(40, 80, 50));
  tft.setTextColor(TFT_CYAN, tft.color565(25, 35, 30));
  tft.setTextSize(1);
  tft.drawCentreString(timebaseLabels[timebaseIndex], 45, 212, 2);

  // 2. Volts/Div
  tft.fillRoundRect(86, 204, bw, bh, 4, tft.color565(25, 35, 30));
  tft.drawRoundRect(86, 204, bw, bh, 4, tft.color565(40, 80, 50));
  tft.setTextColor(TFT_CYAN, tft.color565(25, 35, 30));
  tft.drawCentreString(voltsDivLabels[voltsDivIndex], 123, 212, 2);

  // 3. Run/Stop
  uint16_t rsBg = isRunning ? tft.color565(20, 50, 30) : tft.color565(60, 20, 20);
  tft.fillRoundRect(164, 204, bw, bh, 4, rsBg);
  tft.drawRoundRect(164, 204, bw, bh, 4, isRunning ? TFT_GREEN : TFT_RED);
  tft.setTextColor(TFT_WHITE, rsBg);
  tft.drawCentreString(isRunning ? "RUN" : "HOLD", 201, 212, 2);

  // 4. PWM Gen Toggle
  uint16_t pwmBg = pwmOutputActive ? tft.color565(50, 45, 20) : tft.color565(25, 35, 30);
  tft.fillRoundRect(242, 204, bw - 2, bh, 4, pwmBg);
  tft.drawRoundRect(242, 204, bw - 2, bh, 4, pwmOutputActive ? TFT_YELLOW : TFT_DARKGREY);
  tft.setTextColor(pwmOutputActive ? TFT_YELLOW : TFT_LIGHTGREY, pwmBg);
  tft.drawCentreString("1kHz CAL", 278, 212, 2);
}

void checkTouch() {
  if (ts.touched()) {
    TS_Point p = ts.getPoint();
    if (p.z > 50 || ts.touched()) {
      if (millis() - lastTouchTime > 180) {
        lastTouchTime = millis();
        // Calibrated non-inverted coordinate mapping for 2.8" CYD
        int screenX = map(p.x, 240, 3800, 0, 320);
        int screenY = map(p.y, 240, 3800, 0, 240);
        screenX = constrain(screenX, 0, 319);
        screenY = constrain(screenY, 0, 239);

        if (screenY >= 195) {
          if (screenX < 80) {
            // Timebase step
            timebaseIndex = (timebaseIndex + 1) % 4;
            beep(2000, 30);
            drawBottomControls();
            drawTelemetryBar();
          } else if (screenX >= 80 && screenX < 160) {
            // Volts/Div step
            voltsDivIndex = (voltsDivIndex + 1) % 3;
            beep(2200, 30);
            drawBottomControls();
            drawTelemetryBar();
          } else if (screenX >= 160 && screenX < 240) {
            // RUN / STOP toggle
            isRunning = !isRunning;
            beep(isRunning ? 2600 : 1600, 50);
            updateStatusLED();
            drawHeader();
            drawBottomControls();
          } else if (screenX >= 240) {
            // PWM Generator Toggle
            pwmOutputActive = !pwmOutputActive;
            ledcWrite(PWM_CHANNEL, pwmOutputActive ? 128 : 0);
            beep(2400, 40);
            drawBottomControls();
          }
        }
      }
    }
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("CRT Bench Oscilloscope Initializing for @IamNotIntoTech...");

  pinMode(TFT_BL_PIN, OUTPUT);
  digitalWrite(TFT_BL_PIN, HIGH);

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  beep(2200, 60);

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);

  // Initialize VSPI Touch Controller
  touchSPI.begin(TOUCH_CLK, TOUCH_MISO, TOUCH_MOSI, TOUCH_CS);
  ts.begin(touchSPI);
  ts.setRotation(1);

  initPWM();

  drawHeader();
  drawGraticule();
  drawTelemetryBar();
  drawBottomControls();
  updateStatusLED();
}

unsigned long lastGraticuleRefresh = 0;

void loop() {
  checkTouch();

  if (isRunning) {
    acquireWaveform();
    drawGraticule();
    renderPhosphorTrace();

    // Refresh telemetry numbers every 300ms
    if (millis() - lastGraticuleRefresh >= 300) {
      lastGraticuleRefresh = millis();
      drawTelemetryBar();
      updateStatusLED();
    }
  }

  delay(15);
}
