"""
CYD (Cheap Yellow Display) MicroPython Helper Module
Board: ESP32-2432S028R / ESP32-2432S028C
Author: tushar2704 <tushar.inseec@gmail.com>
License: AGPL-3.0
"""

from machine import Pin, PWM, ADC
import time

# Pin Definitions
PIN_LED_R = 4
PIN_LED_G = 16
PIN_LED_B = 17
PIN_AUDIO = 26
PIN_LDR = 34
PIN_BL = 21

class CYD:
    def __init__(self):
        # RGB LEDs (Active-LOW PWM)
        self.led_r = PWM(Pin(PIN_LED_R), freq=1000)
        self.led_g = PWM(Pin(PIN_LED_G), freq=1000)
        self.led_b = PWM(Pin(PIN_LED_B), freq=1000)
        self.set_rgb(0, 0, 0) # Off

        # Backlight PWM
        self.bl = PWM(Pin(PIN_BL), freq=5000)
        self.set_backlight(100)

        # Audio Buzzer / Amp
        self.audio = PWM(Pin(PIN_AUDIO), freq=2000, duty=0)

        # Ambient LDR
        self.ldr = ADC(Pin(PIN_LDR))
        self.ldr.atten(ADC.ATTN_11DB)

    def set_backlight(self, percent):
        """Set screen backlight brightness (0 to 100%)."""
        val = max(0, min(100, percent))
        duty = int((val / 100.0) * 1023)
        self.bl.duty(duty)

    def set_rgb(self, r, g, b):
        """Set Onboard RGB LED (0-255 each, Active-LOW)."""
        # Invert for active-low
        dr = int(((255 - max(0, min(255, r))) / 255.0) * 1023)
        dg = int(((255 - max(0, min(255, g))) / 255.0) * 1023)
        db = int(((255 - max(0, min(255, b))) / 255.0) * 1023)
        self.led_r.duty(dr)
        self.led_g.duty(dg)
        self.led_b.duty(db)

    def beep(self, freq=2000, duration_ms=40):
        """Generate audio tone on GPIO 26."""
        self.audio.freq(freq)
        self.audio.duty(512)
        time.sleep_ms(duration_ms)
        self.audio.duty(0)

    def read_ldr(self):
        """Read raw ADC value (0-4095)."""
        return self.ldr.read()

    def read_lux(self):
        """Estimate ambient light in Lux."""
        raw = self.read_ldr()
        return int((raw / 4095.0) * 1200)

# Default instance
cyd = CYD()
