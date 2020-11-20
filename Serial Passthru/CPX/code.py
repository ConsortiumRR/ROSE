import board
import busio
import neopixel
import time
import digitalio
from struct import unpack

pixels = neopixel.NeoPixel(board.NEOPIXEL, 10, brightness=0.2, auto_write=False)
led = digitalio.DigitalInOut(board.D13)
led.direction = digitalio.Direction.OUTPUT
uart = busio.UART(board.TX, board.RX, baudrate=9600)

BLACK = (0,0,0)
WHITE = (255,255,255)
RED = (255, 0, 0)
ROSE = (153, 51, 51)
MAGENTA = (255, 0, 255)
VIOLET = (51, 0, 153)
BLUE = (0, 0, 153)
AZURE = (0, 102, 204)
CYAN = (51, 204, 255)
AQUA = (20, 102, 20)
GREEN = (0, 255, 0)
LIME = (128, 255, 0)
YELLOW = (255, 180, 0)
ORANGE = (255, 102, 0)
SIENNA = (102, 24, 0)

color_lib = [BLACK, WHITE, RED, ROSE, MAGENTA, VIOLET, BLUE, AZURE, CYAN, AQUA, GREEN, LIME, YELLOW, ORANGE, SIENNA]


while True:
    data = uart.read(1)
    print("howdy")
    colorindex = data[0]

    if data is not None:
        print(colorindex)
        print(color_lib[colorindex])
        pixels.fill(color_lib[colorindex])
        pixels.show()