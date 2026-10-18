"""
hello_cyd.py — Pure MicroPython Hardware Demo for Cheap Yellow Display 2.8"
Author: Tushar Aggarwal (tushar2704)
Repository: https://github.com/tushar2704/Cheap-Yellow-Display-Community
YouTube: @IamNotIntoTech | @Makerrdesigns
"""

import time
from machine import Pin, SPI, PWM, ADC

# Pin Definitions for ESP32-2432S028R
TFT_MISO = 12
TFT_MOSI = 13
TFT_SCK  = 14
TFT_CS   = 15
TFT_DC   = 2
TFT_BL   = 21

LED_RED   = 4
LED_GREEN = 16
LED_BLUE  = 17
LDR_PIN   = 34

def main():
    print("============================================")
    print("🟡 Cheap Yellow Display — MicroPython Quickstart")
    print("Author: Tushar Aggarwal (@tushar2704)")
    print("============================================")

    # 1. Backlight On
    bl = Pin(TFT_BL, Pin.OUT)
    bl.value(1)

    # 2. RGB LEDs Test (Active LOW)
    led_r = Pin(LED_RED, Pin.OUT)
    led_g = Pin(LED_GREEN, Pin.OUT)
    led_b = Pin(LED_BLUE, Pin.OUT)

    # Cycle RGB LEDs
    print("Testing Onboard RGB LEDs...")
    led_r.value(0); led_g.value(1); led_b.value(1) # Red
    time.sleep_ms(300)
    led_r.value(1); led_g.value(0); led_b.value(1) # Green
    time.sleep_ms(300)
    led_r.value(1); led_g.value(1); led_b.value(0) # Blue
    time.sleep_ms(300)
    led_r.value(1); led_g.value(1); led_b.value(1) # Off

    # 3. Read LDR Ambient Light Sensor
    ldr = ADC(Pin(LDR_PIN))
    ldr.atten(ADC.ATTN_11DB) # 0 - 3.3V range
    print("Reading Ambient Light Sensor (LDR GPIO 34)...")
    for _ in range(5):
        val = ldr.read()
        print(f"LDR Raw Value: {val} (0=Dark, 4095=Bright)")
        time.sleep_ms(250)

    # 4. Display SPI Bus Setup
    spi = SPI(2, baudrate=40000000, polarity=0, phase=0,
              sck=Pin(TFT_SCK), mosi=Pin(TFT_MOSI), miso=Pin(TFT_MISO))
    print("SPI Display Bus Initialized successfully at 40MHz.")
    print("CYD Hardware is ready for MicroPython graphics!")

if __name__ == "__main__":
    main()
