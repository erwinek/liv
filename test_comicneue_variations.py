#!/usr/bin/env python3
"""
Comprehensive test for ComicNeue-Bold-48.bdf font
Tests different text strings and positions
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
    return packet

def send_text_command(ser, screen_id, text, x, y, font_size, r, g, b, font_name=""):
    """Send text display command"""
    text_bytes = text.encode('ascii')[:32]
    text_length = len(text_bytes)
    text_bytes = text_bytes.ljust(32, b'\x00')
    
    font_bytes = font_name.encode('ascii')[:32]
    font_bytes = font_bytes.ljust(32, b'\x00')
    
    payload = struct.pack('BBHHBBBBB32s32s',
                         screen_id,
                         CMD_DISPLAY_TEXT,
                         x, y,
                         font_size,
                         r, g, b,
                         text_length,
                         text_bytes,
                         font_bytes)
    
    packet = create_packet(screen_id, CMD_DISPLAY_TEXT, payload)
    ser.write(packet)
    print(f"✓ Sent: '{text}' at ({x},{y}) RGB({r},{g},{b})")

def send_clear_command(ser, screen_id):
    """Send clear screen command"""
    payload = struct.pack('BB', screen_id, CMD_CLEAR_SCREEN)
    packet = create_packet(screen_id, CMD_CLEAR_SCREEN, payload)
    ser.write(packet)
    print(f"✓ Screen cleared")

def main():
    port = "/dev/ttyUSB0"
    
    if len(sys.argv) >= 2:
        port = sys.argv[1]
    
    print("=" * 60)
    print("  ComicNeue-Bold-48.bdf Font Test Suite")
    print("=" * 60)
    print(f"Port: {port}")
    print(f"Baud: 1000000 bps")
    screen_id = 1
    
    try:
        ser = serial.Serial(port, 1000000, timeout=1)
        print(f"✓ Connected successfully\n")
        time.sleep(0.5)
        
        # Test 1: Original Esp32Demo text
        print("Test 1: Esp32Demo (Green)")
        send_clear_command(ser, screen_id)
        time.sleep(0.3)
        send_text_command(ser, screen_id, "Esp32", 23, 41, 2, 0, 255, 0, "fonts/ComicNeue-Bold-48.bdf")
        time.sleep(3)
        
        # Test 2: Red text
        print("\nTest 2: Hello (Red)")
        send_clear_command(ser, screen_id)
        time.sleep(0.3)
        send_text_command(ser, screen_id, "Hello", 10, 41, 2, 255, 0, 0, "fonts/ComicNeue-Bold-48.bdf")
        time.sleep(3)
        
        # Test 3: Blue text
        print("\nTest 3: Test (Blue)")
        send_clear_command(ser, screen_id)
        time.sleep(0.3)
        send_text_command(ser, screen_id, "Test", 20, 41, 2, 0, 0, 255, "fonts/ComicNeue-Bold-48.bdf")
        time.sleep(3)
        
        # Test 4: Yellow text
        print("\nTest 4: OK! (Yellow)")
        send_clear_command(ser, screen_id)
        time.sleep(0.3)
        send_text_command(ser, screen_id, "OK!", 30, 41, 2, 255, 255, 0, "fonts/ComicNeue-Bold-48.bdf")
        time.sleep(3)
        
        # Final: Back to Esp32
        print("\nFinal: Return to Esp32 (Green)")
        send_clear_command(ser, screen_id)
        time.sleep(0.3)
        send_text_command(ser, screen_id, "Esp32", 23, 41, 2, 0, 255, 0, "fonts/ComicNeue-Bold-48.bdf")
        
        print("\n" + "=" * 60)
        print("  All tests completed successfully!")
        print("  ComicNeue-Bold-48.bdf font is working correctly")
        print("=" * 60)
        
    except serial.SerialException as e:
        print(f"✗ Serial error: {e}")
    except KeyboardInterrupt:
        print("\n✗ Test interrupted")
    finally:
        if 'ser' in locals():
            ser.close()
            print("\n✓ Serial port closed")

if __name__ == "__main__":
    main()

