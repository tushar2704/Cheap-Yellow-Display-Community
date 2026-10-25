/*
 * Cheap Yellow Display (CYD) - Local Voice AI & Smart Assistant Terminal
 * Hardware: ESP32-2432S028R (2.8" TFT 320x240 ILI9341, XPT2046 Resistive Touch)
 * 
 * Target: 2.8" CYD ESP32-2432S028R
 * Author: tushar2704 <tushar.inseec@gmail.com>
 * Channels: @IamNotIntoTech | @Makerrdesigns
 * Dual Licensed: AGPL-3.0 for firmware code | CC BY-NC-ND 4.0 for media/assets
 * Year: 2026
 */

#include <Arduino.h>
#include <SPI.h>
#include <WiFi.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>

// Touch Pins
#define XPT2046_IRQ 36
#define XPT2046_MOSI 32
#define XPT2046_MISO 39
#define XPT2046_CLK 25
#define XPT2046_CS 33

// Onboard Peripherals
#define CYD_LED_RED 4
#define CYD_LED_GREEN 16
#define CYD_LED_BLUE 17
#define CYD_BUZZER 26
#define CYD_LDR 34

SPIClass touchSPI = SPIClass(VSPI);
XPT2046_Touchscreen ts(XPT2046_CS, XPT2046_IRQ);
TFT_eSPI tft = TFT_eSPI();

enum AssistantState { STATE_IDLE, STATE_LISTENING, STATE_THINKING, STATE_SPEAKING };
AssistantState aiState = STATE_IDLE;

String lastTranscription = "Turn on the living room neon lights";
String aiResponse = "Living room lights set to Warm Glow (80%).";
const char* selectedModel = "Llama-3-8B / Whisper-v3";

void beep(int freq, int durationMs) {
  tone(CYD_BUZZER, freq, durationMs);
}

void setRGB(bool r, bool g, bool b) {
  digitalWrite(CYD_LED_RED, r ? LOW : HIGH);
  digitalWrite(CYD_LED_GREEN, g ? LOW : HIGH);
  digitalWrite(CYD_LED_BLUE, b ? LOW : HIGH);
}

void drawHeader() {
  tft.fillRect(0, 0, 320, 26, 0x0821);
  tft.drawFastHLine(0, 26, 320, TFT_CYAN);
  
  tft.setTextColor(TFT_CYAN, 0x0821);
  tft.setTextDatum(ML_DATUM);
  tft.drawString("VOICE AI / LLM SMART ASSISTANT", 8, 13, 2);
  
  tft.setTextDatum(MR_DATUM);
  tft.setTextColor(TFT_WHITE, 0x0821);
  tft.drawString("LOCAL LLM", 312, 13, 2);
}

void drawAnimatedVoiceOrb(int cx, int cy, float phase) {
  // Clear Orb Area
  tft.fillRect(cx - 50, cy - 50, 100, 100, TFT_BLACK);
  
  if (aiState == STATE_IDLE) {
    tft.drawCircle(cx, cy, 32, TFT_NAVY);
    tft.fillCircle(cx, cy, 26, 0x01E8);
    tft.fillCircle(cx, cy, 14, TFT_CYAN);
  } else if (aiState == STATE_LISTENING) {
    // Pulsing Neon Green & Cyan Waves
    int r1 = 24 + (int)(12.0f * sinf(phase * 3.0f));
    int r2 = 36 + (int)(8.0f * cosf(phase * 2.5f));
    tft.drawCircle(cx, cy, r2, TFT_CYAN);
    tft.drawCircle(cx, cy, r1, TFT_GREEN);
    tft.fillCircle(cx, cy, 18, TFT_WHITE);
  } else if (aiState == STATE_THINKING) {
    // Rotating Orange/Yellow Thinking Rings
    for (int i = 0; i < 6; i++) {
      float a = phase * 4.0f + (i * 1.047f);
      int px = cx + (int)(cosf(a) * 32.0f);
      int py = cy + (int)(sinf(a) * 32.0f);
      tft.fillCircle(px, py, 5, TFT_YELLOW);
    }
    tft.fillCircle(cx, cy, 16, 0xFD20);
  } else { // SPEAKING
    // Expanding Magenta/Violet Soundwaves
    int r = 18 + (int)(16.0f * fabsf(sinf(phase * 5.0f)));
    tft.drawCircle(cx, cy, r + 14, TFT_MAGENTA);
    tft.drawCircle(cx, cy, r + 6, TFT_CYAN);
    tft.fillCircle(cx, cy, r, TFT_WHITE);
  }
}

void renderConversationView() {
  // Transcribed Query Card (Top)
  tft.fillRect(6, 126, 308, 40, 0x10A2);
  tft.drawRect(6, 126, 308, 40, 0x2965);
  tft.setTextColor(TFT_SILVER, 0x10A2);
  tft.setTextDatum(TL_DATUM);
  tft.drawString("USER QUERY:", 12, 130, 1);
  tft.setTextColor(TFT_CYAN, 0x10A2);
  tft.drawString(lastTranscription, 12, 144, 2);
  
  // AI LLM Response Card (Bottom)
  tft.fillRect(6, 170, 308, 40, 0x0841);
  tft.drawRect(6, 170, 308, 40, 0x2965);
  tft.setTextColor(TFT_SILVER, 0x0841);
  tft.drawString("ASSISTANT INTENT:", 12, 174, 1);
  tft.setTextColor(TFT_GREEN, 0x0841);
  tft.drawString(aiResponse, 12, 188, 2);
  
  // Touch Action / Push-To-Talk Bar
  tft.fillRect(6, 214, 308, 24, (aiState == STATE_LISTENING) ? TFT_RED : 0x18C3);
  tft.drawRect(6, 214, 308, 24, TFT_DARKGREY);
  tft.setTextColor(TFT_WHITE, (aiState == STATE_LISTENING) ? TFT_RED : 0x18C3);
  tft.setTextDatum(MC_DATUM);
  
  const char* btnLabel = (aiState == STATE_IDLE) ? "TAP TO SPEAK (PUSH-TO-TALK)" :
                         (aiState == STATE_LISTENING) ? "LISTENING (TAP TO STOP)..." :
                         (aiState == STATE_THINKING) ? "LLM PROCESSING INTENT..." : "PLAYING AUDIO RESPONSE...";
  tft.drawString(btnLabel, 160, 226, 2);
}

void updateAssistantAnimation() {
  static float phase = 0.0f;
  phase += 0.15f;
  drawAnimatedVoiceOrb(160, 76, phase);
}

void cycleAssistantWorkflow() {
  static uint32_t stateTimer = 0;
  
  if (aiState == STATE_LISTENING) {
    if (millis() - stateTimer > 3500) {
      aiState = STATE_THINKING;
      stateTimer = millis();
      setRGB(true, true, false); // Yellow thinking
      beep(1800, 25);
      renderConversationView();
    }
  } else if (aiState == STATE_THINKING) {
    if (millis() - stateTimer > 2500) {
      aiState = STATE_SPEAKING;
      stateTimer = millis();
      setRGB(false, false, true); // Blue speaking
      beep(2400, 40);
      renderConversationView();
    }
  } else if (aiState == STATE_SPEAKING) {
    if (millis() - stateTimer > 3000) {
      aiState = STATE_IDLE;
      setRGB(false, true, false); // Green standby
      beep(1400, 30);
      renderConversationView();
    }
  }
}

void setup() {
  Serial.begin(115200);
  
  pinMode(CYD_LED_RED, OUTPUT);
  pinMode(CYD_LED_GREEN, OUTPUT);
  pinMode(CYD_LED_BLUE, OUTPUT);
  setRGB(false, true, false);
  
  pinMode(CYD_BUZZER, OUTPUT);
  beep(1800, 40);
  
  touchSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  ts.begin(touchSPI);
  ts.setRotation(1);
  
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  
  drawHeader();
  renderConversationView();
}

void loop() {
  updateAssistantAnimation();
  cycleAssistantWorkflow();
  
  if (ts.touched()) {
    TS_Point p = ts.getPoint();
    int touchY = map(p.y, 240, 3800, 0, 240);
    
    if (touchY > 210) {
      if (aiState == STATE_IDLE) {
        aiState = STATE_LISTENING;
        setRGB(true, false, false); // Red listening
        beep(2200, 30);
      } else {
        aiState = STATE_IDLE;
        setRGB(false, true, false);
        beep(1200, 25);
      }
      renderConversationView();
      delay(250);
    }
  }
  
  delay(40);
}
