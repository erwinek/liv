#!/usr/bin/env python3
"""
Test script to simulate the Esp32Demo example
Tests the ComicNeue-Bold-48.bdf font display
"""

import serial
import struct
import time
import sys

# Protocol constants
PROTOCOL_SOF = 0xAA
PROTOCOL_EOF = 0x55
CMD_DISPLAY_TEXT = 0x02
CMD_CLEAR_SCREEN = 0x03

def calculate_checksum(data):
    """Calculate XOR checksum for data"""
    checksum = 0
    for byte in data:
        checksum ^= byte
    return checksum

def create_packet(screen_id, command, payload):
    """Create a protocol packet"""
    header = struct.pack('BBBB', PROTOCOL_SOF, screen_id, command, len(payload))
    checksum = calculate_checksum(payload)
    packet = header + payload + struct.pack('BB', checksum, PROTOCOL_EOF)
    
    print(f"Packet structure:")
    print(f"  Total packet: {packet.hex()}")
    print(f"  Total length: {len(packet)} bytes")
    
    return packet

def send_text_command(ser, screen_id, text, x, y, font_size, r, g, b, font_name=""):
    """Send text display command matching Esp32Demo"""
    text_bytes = text.encode('ascii')[:32]
    text_length = len(text_bytes)
    text_bytes = text_bytes.ljust(32, b'\x00')
    
    font_bytes = font_name.encode('ascii')[:32]
    font_bytes = font_bytes.ljust(32, b'\x00')
    
    payload = struct.pack('BBHHBBBBB32s32s',
                         screen_id,
                         CMD_DISPLAY_TEXT,
                         x,
                         y,
                         font_size,
                         r,
                         g,
                         b,
                         text_length,
                         text_bytes,
                         font_bytes)
    
    packet = create_packet(screen_id, CMD_DISPLAY_TEXT, payload)
    ser.write(packet)
    print(f"Sent TEXT: '{text}' at ({x},{y}) color RGB({r},{g},{b}) font={font_name}")

def send_clear_command(ser, screen_id):
    """Send clear screen command"""
    payload = struct.pack('BB', screen_id, CMD_CLEAR_SCREEN)
    packet = create_packet(screen_id, CMD_CLEAR_SCREEN, payload)
    ser.write(packet)
    print(f"Sent CLEAR command")

def main():
    # Use ttyUSB0 (same as led-image-viewer)
    port = "/dev/ttyUSB0"
    
    if len(sys.argv) >= 2:
        port = sys.argv[1]
    
    print(f"=== Testing Esp32Demo with ComicNeue-Bold-48.bdf ===")
    print(f"Using port: {port}")
    screen_id = 1
    
    try:
        # Open serial port
        ser = serial.Serial(port, 1000000, timeout=1)
        print(f"Connected to {port} at 1000000 bps")
        time.sleep(0.5)
        
        # Simulate Esp32Demo behavior
        print("\n--- Clear Screen ---")
        send_clear_command(ser, screen_id)
        time.sleep(0.5)
        
        print("\n--- Display 'Esp32' with ComicNeue-Bold-48.bdf ---")
        # Parameters from Esp32Demo.ino line 31:
        # matrix.displayText("Esp32", 23, 41, 2, 0, 255, 0, "fonts/ComicNeue-Bold-48.bdf");
        send_text_command(ser, screen_id, "Esp32", 23, 41, 2, 0, 255, 0, "fonts/ComicNeue-Bold-48.bdf")
        
        print("\nTest completed! Text should be displayed in green using ComicNeue-Bold-48.bdf font")
        print("The text 'Esp32' should be visible at position (23, 41)")
        
    except serial.SerialException as e:
        print(f"Serial error: {e}")
        print("Note: Make sure led-image-viewer is running and not locking the serial port")
    except KeyboardInterrupt:
        print("\nTest interrupted")
    finally:
        if 'ser' in locals():
            ser.close()
            print("Serial port closed")

if __name__ == "__main__":
    main()

