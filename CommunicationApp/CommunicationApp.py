# simple uart send/receive app without sophisticated GUI,just to be able to enhance it with some gui one day
# currently based on:
# https://forbot.pl/forum/topic/17581-python-i-komunikacja-uart/

import serial
import select
import sys

uartDebug = serial.Serial("/dev/ttyACM1", 115200, timeout=0.1)
uartBT = serial.Serial("/dev/rfcomm0", 9600, timeout=0.1)


def process_read():
    data = uartDebug.readline()
    if data:
        data = data.decode()
        print(data)


def process_keyboard():
    if select.select([sys.stdin], [], [], 0)[0]:
        line = sys.stdin.readline()
        uartBT.write(line.encode())


while True:
    process_read()
    process_keyboard()
