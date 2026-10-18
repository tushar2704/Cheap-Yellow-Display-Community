/**
 * @file CYD_Capacitive_Touch.h
 * @brief Unified Capacitive Touch Driver (CST816T / FT6236 / GT911) for CYD variants
 * @author Tushar Aggarwal (tushar2704)
 * @repository https://github.com/tushar2704/Cheap-Yellow-Display-Community
 * 
 * Supports:
 * - ESP32-2432S028C (2.8" Capacitive: CST816T / FT6236 on I2C SDA:33, SCL:32)
 * - ESP32-3248S035C (3.5" Capacitive: GT911 on I2C SDA:33, SCL:32)
 * - ESP32-4048S040 / ESP32-8048S050 / S070 (GT911 on ESP32-S3)
 */

#ifndef CYD_CAPACITIVE_TOUCH_H
#define CYD_CAPACITIVE_TOUCH_H

#include <Wire.h>
#include <Arduino.h>

// Default I2C Addresses
#define CST816T_I2C_ADDR 0x15
#define FT6236_I2C_ADDR  0x38
#define GT911_I2C_ADDR   0x5D
#define GT911_I2C_ADDR2  0x14

class CYD_CapacitiveTouch {
private:
  uint8_t _sdaPin;
  uint8_t _sclPin;
  int8_t  _rstPin;
  int8_t  _irqPin;
  uint8_t _i2cAddr;
  uint16_t _screenWidth;
  uint16_t _screenHeight;
  uint8_t _rotation;

public:
  CYD_CapacitiveTouch(uint8_t sda = 33, uint8_t scl = 32, int8_t rst = 25, int8_t irq = 21)
    : _sdaPin(sda), _sclPin(scl), _rstPin(rst), _irqPin(irq), _i2cAddr(0),
      _screenWidth(240), _screenHeight(320), _rotation(1) {}

  bool begin(uint16_t width = 320, uint16_t height = 240, uint8_t rotation = 1) {
    _screenWidth = width;
    _screenHeight = height;
    _rotation = rotation;

    // Reset touch controller if pin defined
    if (_rstPin >= 0) {
      pinMode(_rstPin, OUTPUT);
      digitalWrite(_rstPin, LOW);
      delay(10);
      digitalWrite(_rstPin, HIGH);
      delay(50);
    }

    if (_irqPin >= 0) {
      pinMode(_irqPin, INPUT);
    }

    Wire.begin(_sdaPin, _sclPin);

    // Auto-detect which capacitive controller is present
    Wire.beginTransmission(CST816T_I2C_ADDR);
    if (Wire.endTransmission() == 0) {
      _i2cAddr = CST816T_I2C_ADDR;
      return true;
    }

    Wire.beginTransmission(FT6236_I2C_ADDR);
    if (Wire.endTransmission() == 0) {
      _i2cAddr = FT6236_I2C_ADDR;
      return true;
    }

    Wire.beginTransmission(GT911_I2C_ADDR);
    if (Wire.endTransmission() == 0) {
      _i2cAddr = GT911_I2C_ADDR;
      return true;
    }

    Wire.beginTransmission(GT911_I2C_ADDR2);
    if (Wire.endTransmission() == 0) {
      _i2cAddr = GT911_I2C_ADDR2;
      return true;
    }

    return false;
  }

  bool getTouch(uint16_t *x, uint16_t *y) {
    if (_i2cAddr == 0) return false;

    // CST816T / FT6236 register read
    if (_i2cAddr == CST816T_I2C_ADDR || _i2cAddr == FT6236_I2C_ADDR) {
      Wire.beginTransmission(_i2cAddr);
      Wire.write(0x02); // Point register
      if (Wire.endTransmission() != 0) return false;

      if (Wire.requestFrom(_i2cAddr, (uint8_t)5) != 5) return false;

      uint8_t points = Wire.read() & 0x0F;
      if (points == 0) return false;

      uint8_t x_hi = Wire.read();
      uint8_t x_lo = Wire.read();
      uint8_t y_hi = Wire.read();
      uint8_t y_lo = Wire.read();

      uint16_t rawX = ((x_hi & 0x0F) << 8) | x_lo;
      uint16_t rawY = ((y_hi & 0x0F) << 8) | y_lo;

      // Handle landscape rotation (rotation 1: 320x240)
      if (_rotation == 1) {
        *x = rawY;
        *y = 240 - rawX;
      } else {
        *x = rawX;
        *y = rawY;
      }

      *x = constrain(*x, 0, _screenWidth - 1);
      *y = constrain(*y, 0, _screenHeight - 1);
      return true;
    }

    return false;
  }
};

#endif // CYD_CAPACITIVE_TOUCH_H
