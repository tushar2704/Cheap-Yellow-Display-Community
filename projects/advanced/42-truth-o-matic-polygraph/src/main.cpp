/**
 * @file main.cpp
 * @brief CYD Truth-O-Matic 9000 - Cyberpunk Polygraph & Biometric Interrogator
 * @author Tushar Aggarwal (@tushar2704)
 * @repository https://github.com/tushar2704/Cheap-Yellow-Display-Community
 * 
 * Hardware:
 * - Cheap Yellow Display (ESP32-2432S028R) 2.8" Resistive Touch
 * - Display: ILI9341 320x240 Landscape
 * - Touch: XPT2046 SPI (CLK=25, MISO=39, MOSI=32, CS=33, IRQ=36)
 * - Audio: GPIO 26 Speaker / Piezo buzzer (Biometric beeps, laser sweeps, siren)
 * - RGB LED: GPIO 4 (Red), GPIO 16 (Green), GPIO 17 (Blue) [Active LOW]
 * - LDR Ambient Sensor: GPIO 34 (Finger shadow detection)
 */

#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>

// ---------------- Pinout ----------------
#define XPT2046_MOSI  32
#define XPT2046_MISO  39
#define XPT2046_CLK   25
#define XPT2046_CS    33
#define XPT2046_IRQ   36

#define LED_RED       4
#define LED_GREEN     16
#define LED_BLUE      17
#define SPEAKER_PIN   26
#define LDR_PIN       34

// ---------------- Display & Touch ----------------
TFT_eSPI tft = TFT_eSPI();
SPIClass touchSPI(VSPI);
XPT2046_Touchscreen ts(XPT2046_CS, XPT2046_IRQ);

// ---------------- Color Palette ----------------
#define COL_BG          0x0821  // Deep Cyber Black
#define COL_PANEL       0x10A4  // Surface Slate
#define COL_BORDER      0x2147  // Grid lines
#define COL_CYAN        0x07FF  // Scanning Cyan
#define COL_NEON_GREEN  0x07E0  // Truth
#define COL_NEON_RED    0xF800  // Lie / Cap
#define COL_AMBER       0xFDC0  // Suspicious
#define COL_WHITE       0xFFFF  // Text
#define COL_GRAY        0x7BEF  // Dim Text
#define COL_LASER       0xF81F  // Magenta scanner beam

// ---------------- Interrogation States ----------------
enum InterrogateState {
  STATE_IDLE,          // Waiting for subject thumb
  STATE_CALIBRATING,   // Initial contact, sampling baseline GSR/LDR
  STATE_SCANNING,      // Active polygraph test in progress
  STATE_VERDICT        // Final judgment displayed
};

InterrogateState currentState = STATE_IDLE;

// ---------------- Secret Creator Override Modes ----------------
enum OverrideMode {
  OVERRIDE_AUTO,       // Fair 50/50 randomized
  OVERRIDE_FORCE_TRUTH,// Top-left corner secretly tapped
  OVERRIDE_FORCE_LIE   // Top-right corner secretly tapped
};

OverrideMode creatorOverride = OVERRIDE_AUTO;

// ---------------- Biometric Parameters ----------------
int bpm = 72;
int stressLevel = 15;        // 0 to 100%
float ecgPhase = 0.0f;
int scanLaserY = 70;
int scanLaserDir = 2;
unsigned long scanStartTime = 0;
unsigned long lastHeartbeat = 0;
unsigned long stateChangeTime = 0;
bool touchDetected = false;
int touchPressX = 0;
int touchPressY = 0;

// Polygraph Waveform Buffer (170 pixels wide chart)
#define CHART_W 170
#define CHART_H 80
#define CHART_X 10
#define CHART_Y 60
uint8_t chartHistory[CHART_W];
int chartIndex = 0;

// ---------------- Verdicts ----------------
struct Verdict {
  const char* title;
  const char* subtitle;
  bool isTruth;
  bool isSuspicious;
};

const Verdict TRUTH_VERDICTS[] = {
  {"100% NO CAP", "BIOMETRICS CERTIFIED HONEST", true, false},
  {"ANGELIC CITIZEN", "CONSCIENCE CLEAN AS A WHISTLE", true, false},
  {"TRUTH VERIFIED", "ZERO DECEPTION REGISTERED", true, false},
  {"SPOTLESS RECORD", "NO PHYSIOLOGICAL TWITCHES", true, false},
  {"PURE FACTS", "VOICE AND PULSE IN HARMONY", true, false}
};

const Verdict LIE_VERDICTS[] = {
  {"100% PURE CAP!", "CRIMINAL LEVEL FIB DETECTED", false, false},
  {"DECEPTION ALERT!", "SWEATY PALMS NEVER LIE", false, false},
  {"FBI DISPATCHED!", "RUN WHILE YOU STILL CAN", false, false},
  {"PATHOLOGICAL FIB", "PULSE RATE THROUGH THE ROOF", false, false},
  {"TOTAL FICTION!", "PUPILS MASSIVELY DILATED", false, false},
  {"DISQUALIFIED!", "CONSCIENCE COMPLETELY OFFLINE", false, false}
};

const Verdict SUSPICIOUS_VERDICTS[] = {
  {"HEAVY SUSPICION", "50% TRUTH / 50% EXCUSES", false, true},
  {"NERVOUS TWITCH", "GUILTY SWEAT DETECTED", false, true},
  {"HALF-TRUTH", "HIDING CRITICAL DETAILS", false, true}
};

Verdict activeVerdict;

// ---------------- Audio Engine ----------------
void playTone(int freq, int durationMs) {
  tone(SPEAKER_PIN, freq, durationMs);
  delay(durationMs);
  noTone(SPEAKER_PIN);
}

void playHeartbeat(bool highStress) {
  int freq1 = highStress ? 160 : 120;
  int freq2 = highStress ? 220 : 160;
  
  tone(SPEAKER_PIN, freq1, 25);
  delay(35);
  noTone(SPEAKER_PIN);
  delay(40);
  tone(SPEAKER_PIN, freq2, 35);
  delay(45);
  noTone(SPEAKER_PIN);
}

void playChirp() {
  for (int f = 800; f < 1800; f += 200) {
    tone(SPEAKER_PIN, f, 6);
    delay(6);
  }
  noTone(SPEAKER_PIN);
}

void playTruthFanfare() {
  int notes[] = { 523, 659, 784, 1046 };
  for (int n : notes) {
    tone(SPEAKER_PIN, n, 90);
    delay(100);
  }
  noTone(SPEAKER_PIN);
}

void playLieSiren() {
  for (int i = 0; i < 3; i++) {
    for (int f = 400; f < 1200; f += 50) {
      tone(SPEAKER_PIN, f, 10);
      delay(8);
    }
    for (int f = 1200; f > 400; f -= 50) {
      tone(SPEAKER_PIN, f, 10);
      delay(8);
    }
  }
  noTone(SPEAKER_PIN);
}

// ---------------- RGB LED Control ----------------
void setRGB(bool r, bool g, bool b) {
  digitalWrite(LED_RED, r ? LOW : HIGH);
  digitalWrite(LED_GREEN, g ? LOW : HIGH);
  digitalWrite(LED_BLUE, b ? LOW : HIGH);
}

// ---------------- Drawing Helpers ----------------
void drawInterfaceFrames() {
  tft.fillScreen(COL_BG);
  
  // Header Panel
  tft.fillRect(0, 0, 320, 36, COL_PANEL);
  tft.drawFastHLine(0, 36, 320, COL_CYAN);
  
  tft.setTextFont(2);
  tft.setTextColor(COL_CYAN, COL_PANEL);
  tft.setCursor(8, 4);
  tft.print("TRUTH-O-MATIC 9000");
  
  tft.setTextFont(1);
  tft.setTextColor(COL_GRAY, COL_PANEL);
  tft.setCursor(8, 22);
  tft.print("QUANTUM BIOMETRIC DECEPTION ANALYZER");

  // Chart Container
  tft.drawRect(CHART_X - 1, CHART_Y - 1, CHART_W + 2, CHART_H + 2, COL_BORDER);
  tft.fillRect(CHART_X, CHART_Y, CHART_W, CHART_H, 0x0000);
  
  // Scanner Pad Container
  tft.drawRoundRect(195, 50, 115, 110, 8, COL_CYAN);
  tft.fillRoundRect(197, 52, 111, 106, 6, 0x0041);
  
  // Biometric Pad Grid
  for (int y = 60; y < 150; y += 15) {
    tft.drawFastHLine(205, y, 95, 0x0185);
  }
  
  // Stress Bar Container
  tft.drawRect(CHART_X - 1, 150, CHART_W + 2, 14, COL_BORDER);
  tft.fillRect(CHART_X, 151, CHART_W, 12, 0x0000);
  
  tft.setTextFont(1);
  tft.setTextColor(COL_GRAY, COL_BG);
  tft.setCursor(CHART_X, 168);
  tft.print("STRESS INDEX");
  
  // Bottom Status Banner
  tft.fillRoundRect(8, 185, 304, 48, 6, COL_PANEL);
  tft.drawRoundRect(8, 185, 304, 48, 6, COL_BORDER);
}

void drawThumbprintIcon(int cx, int cy, uint16_t color) {
  // Biometric fingerprint ridges
  for (int r = 8; r <= 36; r += 7) {
    tft.drawCircle(cx, cy, r, color);
    tft.drawCircle(cx, cy + 8, r, color);
  }
}

void updateStressBar(int pct) {
  pct = constrain(pct, 0, 100);
  int filledW = map(pct, 0, 100, 0, CHART_W);
  
  uint16_t barCol = COL_NEON_GREEN;
  if (pct > 45) barCol = COL_AMBER;
  if (pct > 75) barCol = COL_NEON_RED;
  
  tft.fillRect(CHART_X, 151, filledW, 12, barCol);
  tft.fillRect(CHART_X + filledW, 151, CHART_W - filledW, 12, 0x0000);
  
  // Print Percentage
  tft.setTextFont(1);
  tft.setTextColor(COL_WHITE, COL_BG);
  tft.setCursor(CHART_X + 130, 168);
  tft.printf("%3d%%", pct);
}

void updatePolygraphChart(int sample) {
  // Shift history
  chartHistory[chartIndex] = constrain(sample, 0, CHART_H - 1);
  
  // Redraw chart segment
  tft.fillRect(CHART_X, CHART_Y, CHART_W, CHART_H, 0x0000);
  
  // Grid lines
  for (int y = CHART_Y + 16; y < CHART_Y + CHART_H; y += 16) {
    tft.drawFastHLine(CHART_X, y, CHART_W, 0x08A2);
  }
  
  // Plot wave
  uint16_t waveCol = COL_CYAN;
  if (stressLevel > 70) waveCol = COL_NEON_RED;
  else if (stressLevel > 40) waveCol = COL_AMBER;
  
  for (int i = 0; i < CHART_W - 1; i++) {
    int idx1 = (chartIndex + 1 + i) % CHART_W;
    int idx2 = (chartIndex + 2 + i) % CHART_W;
    int y1 = CHART_Y + CHART_H - 1 - chartHistory[idx1];
    int y2 = CHART_Y + CHART_H - 1 - chartHistory[idx2];
    tft.drawLine(CHART_X + i, y1, CHART_X + i + 1, y2, waveCol);
  }
  
  chartIndex = (chartIndex + 1) % CHART_W;
}

void drawStatusMessage(const char* line1, const char* line2, uint16_t col1, uint16_t col2) {
  tft.fillRoundRect(10, 187, 300, 44, 4, COL_PANEL);
  
  tft.setTextFont(2);
  tft.setTextColor(col1, COL_PANEL);
  tft.setCursor(18, 192);
  tft.print(line1);
  
  tft.setTextFont(1);
  tft.setTextColor(col2, COL_PANEL);
  tft.setCursor(18, 214);
  tft.print(line2);
}

int scanMessageStage = 0;

void resetChart() {
  for (int i = 0; i < CHART_W; i++) {
    chartHistory[i] = CHART_H / 2;
  }
  chartIndex = 0;
}

// ---------------- Secret Creator Override Check ----------------
void checkSecretOverride(int tx, int ty) {
  // Top-Left corner: Force Truth
  if (tx < 50 && ty < 40) {
    if (creatorOverride != OVERRIDE_FORCE_TRUTH) {
      creatorOverride = OVERRIDE_FORCE_TRUTH;
      tft.fillCircle(4, 4, 3, COL_NEON_GREEN); // Subtle indicator
      playTone(1500, 40);
    }
  }
  // Top-Right corner: Force Lie
  else if (tx > 270 && ty < 40) {
    if (creatorOverride != OVERRIDE_FORCE_LIE) {
      creatorOverride = OVERRIDE_FORCE_LIE;
      tft.fillCircle(315, 4, 3, COL_NEON_RED); // Subtle indicator
      playTone(400, 40);
    }
  }
  // Bottom-Left corner: Reset to Auto
  else if (tx < 50 && ty > 200) {
    if (creatorOverride != OVERRIDE_AUTO) {
      creatorOverride = OVERRIDE_AUTO;
      tft.fillCircle(4, 4, 3, COL_PANEL);
      tft.fillCircle(315, 4, 3, COL_PANEL);
      playTone(900, 40);
    }
  }
}

// ---------------- Main Setup ----------------
void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println(F("[TRUTH-O-MATIC 9000] Initializing Biometric Engine..."));

  // Pins
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  pinMode(SPEAKER_PIN, OUTPUT);
  pinMode(LDR_PIN, INPUT);
  
  setRGB(false, false, false);
  noTone(SPEAKER_PIN);

  // Initialize Display
  tft.init();
  tft.setRotation(1);
  
  // Initialize Touch
  touchSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  ts.begin(touchSPI);
  ts.setRotation(1);

  // Boot UI
  drawInterfaceFrames();
  drawThumbprintIcon(252, 105, COL_CYAN);
  resetChart();
  drawStatusMessage("SYSTEM READY // AWAITING SUBJECT", "PLACE THUMB FIRMLY ON BIOMETRIC SCANNER", COL_CYAN, COL_WHITE);
  
  // Boot Chime
  playTone(880, 70);
  playTone(1320, 100);
}

// ---------------- Main Loop ----------------
void loop() {
  unsigned long now = millis();
  
  // 1. Read Touch
  touchDetected = false;
  if (ts.touched()) {
    TS_Point p = ts.getPoint();
    touchPressX = map(p.x, 240, 3800, 0, 320);
    touchPressY = map(p.y, 240, 3800, 0, 240);
    touchPressX = constrain(touchPressX, 0, 319);
    touchPressY = constrain(touchPressY, 0, 239);
    touchDetected = true;
    
    // Check hidden creator corners
    checkSecretOverride(touchPressX, touchPressY);
  }

  // 2. Read Ambient Light & Finger Shadow
  int ldrVal = analogRead(LDR_PIN);

  // 3. State Machine
  switch (currentState) {
    
    // ==========================================
    case STATE_IDLE: {
      setRGB(false, false, true); // Idle blue pulse
      
      // Idle Heartbeat (every 1000ms / 60 BPM)
      if (now - lastHeartbeat >= 1000) {
        lastHeartbeat = now;
        playHeartbeat(false);
      }
      
      // Baseline resting ECG signal
      ecgPhase += 0.15f;
      int wave = (CHART_H / 2) + sin(ecgPhase) * 6;
      updatePolygraphChart(wave);
      updateStressBar(12);

      // Check if user touches the thumbprint scanner area (X: 195-310, Y: 50-160)
      if (touchDetected && touchPressX >= 185 && touchPressX <= 315 && touchPressY >= 45 && touchPressY <= 165) {
        currentState = STATE_CALIBRATING;
        stateChangeTime = now;
        scanStartTime = now;
        setRGB(true, true, false); // Amber
        drawStatusMessage("CALIBRATING BIOMETRICS...", "MEASURING ELECTRODERMAL RESPONSE (GSR)", COL_AMBER, COL_WHITE);
        playChirp();
      }
      break;
    }

    // ==========================================
    case STATE_CALIBRATING: {
      // 1.5 Second Calibration ramp
      int progress = map(now - stateChangeTime, 0, 1500, 0, 100);
      updateStressBar(progress / 3);
      
      // Moving laser scan line across fingerprint pad
      tft.drawFastHLine(200, scanLaserY, 105, 0x0041); // clear old line
      scanLaserY += scanLaserDir * 3;
      if (scanLaserY > 145 || scanLaserY < 60) scanLaserDir = -scanLaserDir;
      tft.drawFastHLine(200, scanLaserY, 105, COL_LASER);
      
      // Fast pitch blip
      if ((now / 80) % 2 == 0) {
        tone(SPEAKER_PIN, 1200 + progress * 5, 15);
      }

      // Check if finger was lifted early
      if (!touchDetected && (now - stateChangeTime > 200)) {
        currentState = STATE_IDLE;
        drawThumbprintIcon(252, 105, COL_CYAN);
        drawStatusMessage("CONTACT LOST // TEST ABORTED", "MAINTAIN FIRM CONTACT ON SCANNER", COL_NEON_RED, COL_GRAY);
        setRGB(true, false, false);
        playTone(300, 150);
        delay(600);
        break;
      }

      if (now - stateChangeTime >= 1500) {
        currentState = STATE_SCANNING;
        stateChangeTime = now;
        scanStartTime = now;
        bpm = 75;
        scanMessageStage = 0;
        drawStatusMessage("INTERROGATION IN PROGRESS", "SPEAK YOUR ANSWER CLEARLY NOW!", COL_CYAN, COL_WHITE);
      }
      break;
    }

    // ==========================================
    case STATE_SCANNING: {
      unsigned long elapsed = now - stateChangeTime;
      
      // If subject lifted finger during scan
      if (!touchDetected && elapsed > 300) {
        currentState = STATE_IDLE;
        drawThumbprintIcon(252, 105, COL_CYAN);
        drawStatusMessage("SUBJECT WITHDREW // TEST ABORTED", "DO NOT MOVE FINGER UNTIL VERDICT", COL_NEON_RED, COL_WHITE);
        setRGB(true, false, false);
        playTone(250, 200);
        delay(600);
        break;
      }

      // Simulated stress climbing based on duration & override
      int targetStress = 25 + map(elapsed, 0, 4500, 0, 60);
      if (creatorOverride == OVERRIDE_FORCE_LIE) targetStress += 25;
      else if (creatorOverride == OVERRIDE_FORCE_TRUTH) targetStress -= 20;
      stressLevel = constrain(targetStress, 5, 98);
      
      bpm = map(stressLevel, 10, 95, 72, 160);
      
      // Heartbeat speed tracks BPM
      int beatInterval = 60000 / bpm;
      if (now - lastHeartbeat >= beatInterval) {
        lastHeartbeat = now;
        playHeartbeat(stressLevel > 60);
      }
      
      // Visual Laser sweep
      tft.drawFastHLine(200, scanLaserY, 105, 0x0041);
      scanLaserY += scanLaserDir * 4;
      if (scanLaserY > 145 || scanLaserY < 60) scanLaserDir = -scanLaserDir;
      tft.drawFastHLine(200, scanLaserY, 105, (stressLevel > 70) ? COL_NEON_RED : COL_CYAN);
      
      // Polygraph spikes (spikes get erratic with stress)
      ecgPhase += 0.35f;
      int jMax = max(1, stressLevel / 3);
      int jitter = random(-jMax, jMax + 1);
      int wave = (CHART_H / 2) + sin(ecgPhase) * (10 + (stressLevel / 4)) + jitter;
      if (random(0, 10) == 0 && stressLevel > 50) {
        wave += (random(0, 2) ? 25 : -25); // Polygraph deception spike
      }
      updatePolygraphChart(wave);
      updateStressBar(stressLevel);
      
      // Dynamic status message (updated only on state transition to prevent screen flicker)
      if (elapsed > 3000 && scanMessageStage < 2) {
        scanMessageStage = 2;
        drawStatusMessage("CRITICAL THRESHOLD REACHED", "ANALYZING MICRO-TREMORS & VOICE STRESS", COL_NEON_RED, COL_WHITE);
        setRGB(true, false, false);
      } else if (elapsed > 1500 && scanMessageStage < 1) {
        scanMessageStage = 1;
        drawStatusMessage("DETECTING VASOMOTOR FLUX", "RECORDING TRANSIENT GALVANIC SURGES", COL_AMBER, COL_WHITE);
        setRGB(true, true, false);
      }

      // After 4.5 seconds of interrogation -> Reveal Verdict!
      if (elapsed >= 4500) {
        currentState = STATE_VERDICT;
        stateChangeTime = now;
        
        // Pick Verdict based on override or stress
        int verdictType = 0; // 0=Truth, 1=Suspicious, 2=Lie
        if (creatorOverride == OVERRIDE_FORCE_TRUTH) {
          verdictType = 0;
        } else if (creatorOverride == OVERRIDE_FORCE_LIE) {
          verdictType = 2;
        } else {
          // Fair auto calculation
          if (stressLevel > 65) verdictType = 2;
          else if (stressLevel > 40) verdictType = 1;
          else verdictType = 0;
        }
        
        if (verdictType == 0) {
          activeVerdict = TRUTH_VERDICTS[random(0, sizeof(TRUTH_VERDICTS) / sizeof(Verdict))];
        } else if (verdictType == 1) {
          activeVerdict = SUSPICIOUS_VERDICTS[random(0, sizeof(SUSPICIOUS_VERDICTS) / sizeof(Verdict))];
        } else {
          activeVerdict = LIE_VERDICTS[random(0, sizeof(LIE_VERDICTS) / sizeof(Verdict))];
        }
        
        // Render Verdict Stamp
        tft.fillRoundRect(8, 50, 304, 120, 8, COL_PANEL);
        tft.drawRoundRect(8, 50, 304, 120, 8, activeVerdict.isTruth ? COL_NEON_GREEN : (activeVerdict.isSuspicious ? COL_AMBER : COL_NEON_RED));
        
        tft.setTextFont(4);
        if (activeVerdict.isTruth) {
          tft.setTextColor(COL_NEON_GREEN, COL_PANEL);
          setRGB(false, true, false);
        } else if (activeVerdict.isSuspicious) {
          tft.setTextColor(COL_AMBER, COL_PANEL);
          setRGB(true, true, false);
        } else {
          tft.setTextColor(COL_NEON_RED, COL_PANEL);
          setRGB(true, false, false);
        }
        
        tft.drawCentreString(activeVerdict.title, 160, 72, 4);
        
        tft.setTextFont(2);
        tft.setTextColor(COL_WHITE, COL_PANEL);
        tft.drawCentreString(activeVerdict.subtitle, 160, 115, 2);
        
        drawStatusMessage("TEST COMPLETE // TOUCH SCREEN TO RESET", "POLYGRAPH TRANSCRIPT LOGGED TO MEMORY", COL_CYAN, COL_WHITE);
        
        // Play Audio Result
        if (activeVerdict.isTruth) {
          playTruthFanfare();
        } else {
          playLieSiren();
        }
      }
      break;
    }

    // ==========================================
    case STATE_VERDICT: {
      // Flashing verdict effect
      if ((now / 300) % 2 == 0) {
        if (activeVerdict.isTruth) setRGB(false, true, false);
        else if (activeVerdict.isSuspicious) setRGB(true, true, false);
        else setRGB(true, false, false);
      } else {
        setRGB(false, false, false);
      }
      
      // Tap anywhere after 2 seconds to reset
      if (touchDetected && (now - stateChangeTime > 2000)) {
        currentState = STATE_IDLE;
        setRGB(false, false, false);
        drawInterfaceFrames();
        drawThumbprintIcon(252, 105, COL_CYAN);
        resetChart();
        drawStatusMessage("SYSTEM READY // AWAITING SUBJECT", "PLACE THUMB FIRMLY ON BIOMETRIC SCANNER", COL_CYAN, COL_WHITE);
        playTone(900, 80);
        delay(300);
      }
      break;
    }
  }
  
  delay(15);
}
