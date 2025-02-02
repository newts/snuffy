# SPDX-FileCopyrightText: 2020 by Bryan Siepert, written for Adafruit Industries
#
# SPDX-License-Identifier: Unlicense
# SPDX-FileCopyrightText: 2021 ladyada for Adafruit Industries
# SPDX-License-Identifier: MIT

import time
import board
import busio
import adafruit_scd30
import displayio
from displayio import I2CDisplay as I2CDisplayBus
import terminalio
from adafruit_display_text import label
import adafruit_displayio_ssd1306

displayio.release_displays()

# SCD-30 has tempremental I2C with clock stretching, datasheet recommends
# starting at 50KHz
i2c = busio.I2C(board.GP1, board.GP0, frequency=50000)  # board.SCL, board.SDA,
scd = adafruit_scd30.SCD30(i2c)
# scd.temperature_offset = 10

display_bus = I2CDisplayBus(i2c, device_address=0x3C)
WIDTH = 128
HEIGHT = 64
BORDER = 2
display = adafruit_displayio_ssd1306.SSD1306(display_bus, width=WIDTH, height=HEIGHT)

# Make the display context
splash = displayio.Group()
display.root_group = splash

color_bitmap = displayio.Bitmap(WIDTH, HEIGHT, 1)
color_palette = displayio.Palette(1)
color_palette[0] = 0xFFFFFF  # White



print("Temperature offset:", scd.temperature_offset)

# scd.measurement_interval = 4
print("Measurement interval:", scd.measurement_interval)

scd.self_calibration_enabled = True
print("Self-calibration enabled:", scd.self_calibration_enabled)

# scd.ambient_pressure = 1100
print("Ambient Pressure:", scd.ambient_pressure)

# scd.altitude = 100
print("Altitude:", scd.altitude, "meters above sea level")

# scd.forced_recalibration_reference = 409
print("Forced recalibration reference:", scd.forced_recalibration_reference)
print("")


font_height = 10
def clear():
    # Draw a smaller inner rectangle
    inner_bitmap = displayio.Bitmap(WIDTH - BORDER * 2, HEIGHT - BORDER * 2, 1)
    inner_palette = displayio.Palette(1)
    inner_palette[0] = 0x000000  # Black
    inner_sprite = displayio.TileGrid(
        inner_bitmap, pixel_shader=inner_palette, x=BORDER, y=BORDER
    )
    splash.append(inner_sprite)


font_height = 16
def display_co2(line, val, unit):
    text = str(int(val)) + unit
    text_area = label.Label(terminalio.FONT, text=text, color=0xFFFFFF, x=20, y=font_height * line)
    splash.append(text_area)

while True:
    data = scd.data_available
    if data:
        clear()
        print("CO2:", scd.CO2, "PPM")
        display_co2(1, scd.CO2, " PPM")
        print("Temperature:", scd.temperature, "degrees C")
        display_co2(2, scd.temperature, " C")
        print("Humidity::", scd.relative_humidity, "%%rH")
        display_co2(3, scd.relative_humidity, " %")
        print("")


    time.sleep(5.5)
