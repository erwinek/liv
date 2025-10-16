### Python Test Script

import serial
import struct

# Open serial port
ser = serial.Serial('/dev/ttyUSB0', 1000000)

# Load GIF
def send_gif(filename, x, y, w, h):
    payload = struct.pack('BBHHH', 1, 0x01, x, y, w, h)
    payload += filename.encode('ascii').ljust(64, b'\x00')
    packet = create_packet(1, 0x01, payload)
    ser.write(packet)

# Display text
def send_text(text, x, y, font_size, r, g, b):
    payload = struct.pack('BBHBBBBBB', 1, 0x02, x, y, font_size, r, g, b, len(text))
    payload += text.encode('ascii').ljust(32, b'\x00')
    packet = create_packet(1, 0x02, payload)
    ser.write(packet)

# Clear screen
def clear_screen():
    payload = struct.pack('BB', 1, 0x03)
    packet = create_packet(1, 0x03, payload)
    ser.write(packet)
