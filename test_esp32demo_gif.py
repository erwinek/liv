#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Test Esp32Demo z animowanym GIF
Symuluje działanie przykładu Esp32Demo.ino z wyświetlaniem GIF i tekstu
"""

import serial
import struct
import time
import sys

# Protokół
SOF = 0xAA
EOF = 0x55
SCREEN_ID = 1

# Komendy
CMD_LOAD_GIF = 0x01
CMD_DISPLAY_TEXT = 0x02
CMD_CLEAR_SCREEN = 0x03

def calculate_checksum(data):
    """Oblicza sumę kontrolną XOR"""
    checksum = 0
    for byte in data:
        checksum ^= byte
    return checksum

def send_clear_screen(ser):
    """Wysyła komendę wyczyszczenia ekranu"""
    packet = bytearray()
    packet.append(SOF)
    packet.append(SCREEN_ID)
    packet.append(CMD_CLEAR_SCREEN)
    packet.append(0)  # payload_length = 0
    packet.append(calculate_checksum([SCREEN_ID, CMD_CLEAR_SCREEN, 0]))
    packet.append(EOF)
    
    ser.write(packet)
    print(f"Sent CLEAR_SCREEN command")

def send_load_gif(ser, filename, x=0, y=0, width=96, height=96):
    """Wysyła komendę załadowania GIF"""
    # Przygotuj payload (GifCommand bez SOF/EOF, ale z screen_id i command)
    payload = bytearray()
    
    # uint8_t screen_id
    payload.append(SCREEN_ID)
    
    # uint8_t command
    payload.append(CMD_LOAD_GIF)
    
    # uint16_t x, y, width, height (little endian)
    payload.extend(struct.pack('<H', x))
    payload.extend(struct.pack('<H', y))
    payload.extend(struct.pack('<H', width))
    payload.extend(struct.pack('<H', height))
    
    # char filename[64] (PROTOCOL_MAX_FILENAME)
    filename_bytes = filename.encode('utf-8')[:64]
    filename_bytes += b'\x00' * (64 - len(filename_bytes))
    payload.extend(filename_bytes)
    
    payload_length = len(payload)
    
    print(f"DEBUG: payload_length={payload_length}, expected={1+1+2+2+2+2+64}")
    print(f"DEBUG: filename_bytes length={len(filename_bytes)}")
    print(f"DEBUG: payload breakdown: screen_id=1, command=1, x=2, y=2, w=2, h=2, filename={len(filename_bytes)}")
    
    # Zbuduj pakiet
    packet = bytearray()
    packet.append(SOF)
    packet.append(SCREEN_ID)
    packet.append(CMD_LOAD_GIF)
    packet.append(payload_length)
    packet.extend(payload)
    
    checksum = calculate_checksum([SCREEN_ID, CMD_LOAD_GIF, payload_length] + list(payload))
    packet.append(checksum)
    packet.append(EOF)
    
    print(f"DEBUG: packet size={len(packet)}, packet[3] (payload_length byte)={packet[3]}")
    print(f"DEBUG: First 10 bytes of packet: {' '.join(f'{b:02x}' for b in packet[:10])}")
    
    ser.write(packet)
    print(f"Sent LOAD_GIF: {filename} at ({x}, {y}) size {width}x{height}")

def send_display_text(ser, text, x=0, y=0, font_size=2, r=255, g=255, b=255, font_name=""):
    """Wysyła komendę wyświetlenia tekstu"""
    # Przygotuj payload (TextCommand z screen_id i command)
    payload = bytearray()
    
    # uint8_t screen_id
    payload.append(SCREEN_ID)
    
    # uint8_t command
    payload.append(CMD_DISPLAY_TEXT)
    
    # uint16_t x, y (little endian)
    payload.extend(struct.pack('<H', x))
    payload.extend(struct.pack('<H', y))
    
    # uint8_t font_size
    payload.append(font_size)
    
    # uint8_t r, g, b
    payload.append(r)
    payload.append(g)
    payload.append(b)
    
    # uint8_t text_length
    text_length = len(text)
    payload.append(text_length)
    
    # char text[32]
    text_bytes = text.encode('utf-8')[:32]
    text_bytes += b'\x00' * (32 - len(text_bytes))
    payload.extend(text_bytes)
    
    # char font_name[32]
    font_name_bytes = font_name.encode('utf-8')[:32]
    font_name_bytes += b'\x00' * (32 - len(font_name_bytes))
    payload.extend(font_name_bytes)
    
    payload_length = len(payload)
    
    # Zbuduj pakiet
    packet = bytearray()
    packet.append(SOF)
    packet.append(SCREEN_ID)
    packet.append(CMD_DISPLAY_TEXT)
    packet.append(payload_length)
    packet.extend(payload)
    
    checksum = calculate_checksum([SCREEN_ID, CMD_DISPLAY_TEXT, payload_length] + list(payload))
    packet.append(checksum)
    packet.append(EOF)
    
    ser.write(packet)
    print(f"Sent DISPLAY_TEXT: '{text}' at ({x}, {y}) RGB({r},{g},{b}) font_size={font_size} font={font_name}")

def main():
    port = '/dev/ttyUSB0'
    baudrate = 1000000
    
    print(f"Connecting to {port} at {baudrate} baud...")
    
    try:
        with serial.Serial(port, baudrate, timeout=1) as ser:
            print("Connected!")
            time.sleep(0.5)
            
            # Wyczyść ekran
            send_clear_screen(ser)
            time.sleep(0.5)
            
            # Załaduj animowany GIF w lewym górnym rogu
            send_load_gif(ser, "anim/2.gif", x=0, y=0, width=64, height=64)
            time.sleep(0.1)
            
            # Wyświetl tekst "Esp32" w kolorze zielonym
            send_display_text(ser, "Esp32", x=23, y=41, font_size=2, 
                            r=0, g=255, b=0,
                            font_name="fonts/ComicNeue-Bold-48.bdf")
            
            print("\n✅ Wszystkie komendy wysłane pomyślnie!")
            print("\nNa matrycy LED powinny być widoczne:")
            print("  • Animowany GIF (2.gif) w lewym górnym rogu (0, 0) o rozmiarze 64x64")
            print("  • Tekst 'Esp32' w kolorze zielonym na pozycji (23, 41)")
            
    except serial.SerialException as e:
        print(f"❌ Błąd połączenia z {port}: {e}")
        sys.exit(1)
    except Exception as e:
        print(f"❌ Błąd: {e}")
        sys.exit(1)

if __name__ == "__main__":
    main()

