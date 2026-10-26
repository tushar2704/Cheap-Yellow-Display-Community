"""
CYD MicroPython WiFi Scanner
Board: ESP32-2432S028R / ESP32-2432S028C
Author: tushar2704 <tushar.inseec@gmail.com>
"""

import network
import time
from cyd import cyd

print(">>> CYD MicroPython WiFi Scanner Starting <<<")

cyd.set_rgb(0, 255, 255) # Cyan
cyd.beep(1800, 30)
time.sleep_ms(40)
cyd.beep(2400, 50)

wlan = network.WLAN(network.STA_IF)
wlan.active(True)

print("Scanning 2.4 GHz airwaves...")
cyd.set_rgb(255, 180, 0) # Amber while scanning
networks = wlan.scan()

print(f"\\nFound {len(networks)} networks:")
print("--------------------------------------------------")
print(f"{'SSID':<25} {'RSSI (dBm)':<12} {'Channel':<8}")
print("--------------------------------------------------")

for net in networks:
    ssid = net[0].decode('utf-8', 'ignore')
    rssi = net[3]
    ch = net[2]
    print(f"{ssid:<25} {rssi:<12} {ch:<8}")

cyd.set_rgb(0, 255, 0) # Green done
cyd.beep(2200, 40)
print("--------------------------------------------------")
print(f"LDR Ambient Reading: {cyd.read_ldr()} ({cyd.read_lux()} Lux)")
