#!/usr/bin/env python3
"""
Test descenderów (liter z częścią poniżej linii bazowej)
Litery: p, g, y, q, j powinny być wyrównane z literami normalnymi
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
    checksum = 0
    for byte in data:
        checksum ^= byte
    return checksum

def create_packet(screen_id, command, payload):
    header = struct.pack('BBBB', PROTOCOL_SOF, screen_id, command, len(payload))
    checksum = calculate_checksum(payload)
    return header + payload + struct.pack('BB', checksum, PROTOCOL_EOF)

def send_text(ser, screen_id, text, x, y, font_size, r, g, b, font_name=""):
    text_bytes = text.encode('ascii')[:32].ljust(32, b'\x00')
    font_bytes = font_name.encode('ascii')[:32].ljust(32, b'\x00')
    payload = struct.pack('BBHHBBBBB32s32s',
                         screen_id, CMD_DISPLAY_TEXT,
                         x, y, font_size, r, g, b,
                         len(text.encode('ascii')[:32]),
                         text_bytes, font_bytes)
    ser.write(create_packet(screen_id, CMD_DISPLAY_TEXT, payload))
    print(f"  '{text}' at ({x},{y}) RGB({r},{g},{b})")

def send_clear(ser, screen_id):
    payload = struct.pack('BB', screen_id, CMD_CLEAR_SCREEN)
    ser.write(create_packet(screen_id, CMD_CLEAR_SCREEN, payload))

def main():
    port = "/dev/ttyUSB0"
    screen_id = 1
    
    print("=" * 70)
    print("  Test Descenderów - ComicNeue-Bold-48.bdf")
    print("=" * 70)
    print("\nSprawdzamy czy litery z descenderami (p, g, y, q, j)")
    print("są prawidłowo wyrównane z literami normalnymi\n")
    
    try:
        ser = serial.Serial(port, 1000000, timeout=1)
        time.sleep(0.5)
        
        # Test 1: Oryginalne "Esp32"
        print("Test 1: Esp32 (oryginalny przykład)")
        send_clear(ser, screen_id)
        time.sleep(0.3)
        send_text(ser, screen_id, "Esp32", 23, 41, 2, 0, 255, 0, "fonts/ComicNeue-Bold-48.bdf")
        time.sleep(4)
        
        # Test 2: Tekst z descenderami
        print("\nTest 2: happy (descenders: p, y)")
        send_clear(ser, screen_id)
        time.sleep(0.3)
        send_text(ser, screen_id, "happy", 10, 41, 2, 255, 0, 0, "fonts/ComicNeue-Bold-48.bdf")
        time.sleep(4)
        
        # Test 3: Wszystkie descenders
        print("\nTest 3: gpqyj (wszystkie descenders)")
        send_clear(ser, screen_id)
        time.sleep(0.3)
        send_text(ser, screen_id, "gpqyj", 10, 41, 2, 0, 0, 255, "fonts/ComicNeue-Bold-48.bdf")
        time.sleep(4)
        
        # Test 4: Mieszane
        print("\nTest 4: Egypt (mieszane: E normalny, g,y descenders)")
        send_clear(ser, screen_id)
        time.sleep(0.3)
        send_text(ser, screen_id, "Egypt", 15, 41, 2, 255, 255, 0, "fonts/ComicNeue-Bold-48.bdf")
        time.sleep(4)
        
        # Test 5: Bez descenderów dla porównania
        print("\nTest 5: HELLO (bez descenderów, wielkie litery)")
        send_clear(ser, screen_id)
        time.sleep(0.3)
        send_text(ser, screen_id, "HELLO", 15, 41, 2, 0, 255, 255, "fonts/ComicNeue-Bold-48.bdf")
        time.sleep(4)
        
        # Powrót do Esp32
        print("\nFinal: Esp32")
        send_clear(ser, screen_id)
        time.sleep(0.3)
        send_text(ser, screen_id, "Esp32", 23, 41, 2, 0, 255, 0, "fonts/ComicNeue-Bold-48.bdf")
        
        print("\n" + "=" * 70)
        print("  Test zakończony!")
        print("  Litera 'p' w 'Esp32' powinna teraz być prawidłowo wyrównana")
        print("  z częścią poniżej linii bazowej (descender)")
        print("=" * 70)
        
    except serial.SerialException as e:
        print(f"✗ Błąd: {e}")
    except KeyboardInterrupt:
        print("\n✗ Przerwano")
    finally:
        if 'ser' in locals():
            ser.close()
            print("\n✓ Port zamknięty")

if __name__ == "__main__":
    main()

