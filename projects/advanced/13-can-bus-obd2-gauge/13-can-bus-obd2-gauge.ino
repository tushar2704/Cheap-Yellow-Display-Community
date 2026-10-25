/**
 * @file 13-can-bus-obd2-gauge.ino
 * @brief Automotive CAN Bus OBD-II Live Digital Gauge Cluster for Cheap Yellow Display 2.8" (ESP32-2432S028R)
 * @author Tushar Aggarwal (tushar2704)
 * @repository https://github.com/tushar2704/Cheap-Yellow-Display-Community
 * 
 * 📺 Follow on YouTube:
 * - Tech & Coding: https://youtube.com/@IamNotIntoTech
 * - 3D Printing & Enclosure: https://youtube.com/@Makerrdesigns
 * 
 * Hardware:
 * - ESP32-2432S028R (2.8" Resistive Touch CYD)
 * - Onboard RGB LED (GPIO 4, 16, 17)
 * - Onboard Speaker (GPIO 26)
 * - Onboard LDR Sensor (GPIO 34)
 */

#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>

// Hardware Pins (ESP32-2432S028R)
#define XPT2046_IRQ 36
#define XPT2046_MOSI 13
#define XPT2046_MISO 12
#define XPT2046_CLK 14
#define XPT2046_CS 33

#define LED_RED 4
#define LED_GREEN 16
#define LED_BLUE 17
#define SPEAKER_PIN 26
#define LDR_PIN 34
#define TFT_BL 21

#define TS_MINX 300
#define TS_MAXX 3800
#define TS_MINY 200
#define TS_MAXY 3700

SPIClass touchSPI(VSPI);
XPT2046_Touchscreen ts(XPT2046_CS, XPT2046_IRQ);
TFT_eSPI tft = TFT_eSPI();

// OBD-II Engine Metrics
struct EngineTelemetry {
  int rpm;
  int speedMph;
  int coolantTempC;
  float batteryVolts;
  int throttlePct;
  bool isMetric;
};

EngineTelemetry car = {
  3450,
  68,
  89,
  14.2,
  42,
  false
};

void setRgb(bool r, bool g, bool b) {
  digitalWrite(LED_RED, r ? LOW : HIGH);
  digitalWrite(LED_GREEN, g ? LOW : HIGH);
  digitalWrite(LED_BLUE, b ? LOW : HIGH);
}

void beep(int freq, int ms) {
  tone(SPEAKER_PIN, freq, ms);
}

void drawHeader() {
  tft.fillRect(0, 0, 320, 28, 0x1082);
  tft.setTextColor(0xFDC0, 0x1082);
  tft.setTextSize(1);
  tft.setCursor(10, 9);
  tft.print("🏎️ CAN BUS OBD-II GAUGE CLUSTER");

  tft.setTextColor(0x07E0, 0x1082);
  tft.setCursor(240, 9);
  tft.print("CAN: 500kbps");
}

void drawRpmArc(int cx, int cy, int r, int rpm) {
  // Clear gauge circle area
  tft.fillCircle(cx, cy, r + 4, 0x0000);

  // Outer bezel
  tft.drawCircle(cx, cy, r, 0x3186);
  tft.drawCircle(cx, cy, r - 1, 0x2124);

  // Redline zone indicator (6500 - 8000 RPM)
  for (int a = 20; a <= 70; a += 4) {
    float rad = (a * 3.14159) / 180.0;
    int rx = cx + cos(rad) * (r - 6);
    int ry = cy + sin(rad) * (r - 6);
    tft.drawPixel(rx, ry, 0xF800); // Redline tick
  }

  // Draw RPM Needle
  float needleAngle = map(rpm, 0, 8000, 140, 390) * (3.14159 / 180.0);
  int nx = cx + cos(needleAngle) * (r - 10);
  int ny = cy + sin(needleAngle) * (r - 10);
  tft.drawLine(cx, cy, nx, ny, (rpm > 6500) ? 0xF800 : 0x07FF);

  // Center hub
  tft.fillCircle(cx, cy, 6, TFT_WHITE);
  tft.drawCircle(cx, cy, 6, TFT_BLACK);

  // Digital RPM readout
  tft.setTextColor((rpm > 6500) ? 0xF800 : TFT_WHITE, 0x0000);
  tft.setTextSize(2);
  tft.setCursor(cx - 24, cy + r - 26);
  tft.printf("%04d", rpm);
  tft.setTextSize(1);
  tft.setTextColor(0xFDC0, 0x0000);
  tft.setCursor(cx - 10, cy + r - 10);
  tft.print("RPM");
}

void drawSpeedometer(int x, int y) {
  tft.fillRect(x, y, 160, 80, 0x0821);
  tft.drawRect(x, y, 160, 80, 0x2945);

  tft.setTextColor(TFT_WHITE, 0x0821);
  tft.setTextSize(4);
  tft.setCursor(x + 20, y + 16);
  int dispSpeed = car.isMetric ? (int)(car.speedMph * 1.609) : car.speedMph;
  tft.printf("%02d", dispSpeed);

  tft.setTextSize(2);
  tft.setTextColor(0x07E0, 0x0821);
  tft.setCursor(x + 90, y + 26);
  tft.print(car.isMetric ? "KM/H" : "MPH");

  tft.setTextSize(1);
  tft.setTextColor(TFT_LIGHTGREY, 0x0821);
  tft.setCursor(x + 20, y + 60);
  tft.printf("THROTTLE: %d%%", car.throttlePct);
}

void drawAuxGauges(int x, int y) {
  // Auxiliary Diagnostics Grid
  tft.fillRect(x, y, 160, 86, 0x1082);
  tft.drawRect(x, y, 160, 86, 0x31A6);

  // Coolant Temp
  tft.setTextColor(0xFDC0, 0x1082);
  tft.setTextSize(1);
  tft.setCursor(x + 12, y + 12);
  tft.print("COOLANT TEMP:");
  tft.setTextColor(TFT_WHITE, 0x1082);
  tft.setCursor(x + 105, y + 12);
  tft.printf("%d C", car.coolantTempC);

  // Battery Volts
  tft.setTextColor(0x07FF, 0x1082);
  tft.setCursor(x + 12, y + 36);
  tft.print("ALT VOLTAGE:");
  tft.setTextColor(TFT_WHITE, 0x1082);
  tft.setCursor(x + 105, y + 36);
  tft.printf("%.1fV", car.batteryVolts);

  // DTC Status
  tft.setTextColor(0x07E0, 0x1082);
  tft.setCursor(x + 12, y + 60);
  tft.print("DTC CODES: NO FAULTS");
}

void drawFooter() {
  tft.fillRect(0, 222, 320, 18, 0x0000);
  tft.setTextColor(0x07E0, 0x0000);
  tft.setTextSize(1);
  tft.setCursor(6, 226);
  tft.print("CYD Community | @IamNotIntoTech | @Makerrdesigns");
}

void updateShiftLight() {
  if (car.rpm > 6500) {
    setRgb(true, false, false); // Flashing Red at Redline
    beep(2600, 30);
  } else if (car.rpm > 5000) {
    setRgb(true, true, false); // Yellow warning
  } else if (car.rpm > 3000) {
    setRgb(false, true, true); // Cyan cruise
  } else {
    setRgb(false, true, false); // Green idle
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  pinMode(SPEAKER_PIN, OUTPUT);
  setRgb(false, false, false);

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(0x0000);

  touchSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  ts.begin(touchSPI);
  ts.setRotation(1);

  drawHeader();
  drawRpmArc(72, 125, 62, car.rpm);
  drawSpeedometer(150, 36);
  drawAuxGauges(150, 124);
  drawFooter();

  beep(2000, 60);
  updateShiftLight();
}

void loop() {
  // Touch to toggle Speedometer Units (MPH vs KM/H)
  if (ts.touched()) {
    TS_Point p = ts.getPoint();
    int x = map(p.x, TS_MINX, TS_MAXX, 0, 320);
    int y = map(p.y, TS_MINY, TS_MAXY, 0, 240);

    // Speedometer Box Tap (150, 36, 160, 80)
    if (x >= 150 && x <= 310 && y >= 36 && y <= 116) {
      car.isMetric = !car.isMetric;
      beep(2200, 30);
      drawSpeedometer(150, 36);
      delay(200);
    }
    // Tachometer Dial Tap -> Rev Throttle
    else if (x >= 10 && x <= 140 && y >= 50 && y <= 200) {
      car.rpm = min(7800, car.rpm + 1200);
      car.throttlePct = min(100, car.throttlePct + 25);
      car.speedMph = min(145, car.speedMph + 12);
      drawRpmArc(72, 125, 62, car.rpm);
      drawSpeedometer(150, 36);
      updateShiftLight();
      delay(150);
    }
  }

  // Dynamic simulation jitter
  static unsigned long lastTick = 0;
  if (millis() - lastTick > 150) {
    lastTick = millis();
    if (car.rpm > 3000) {
      car.rpm -= random(20, 80);
    } else {
      car.rpm += random(-30, 40);
    }
    car.rpm = constrain(car.rpm, 850, 7800);

    drawRpmArc(72, 125, 62, car.rpm);
    updateShiftLight();
  }

  delay(20);
}
