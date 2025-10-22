#!/usr/bin/env python3
"""
Simple test for Screen #2 - Just display XVnx.gif at (0,0)
"""

import serial
import struct
import time
import sys

# Protocol constants - MUST match SerialProtocol.h
PROTOCOL_PREAMBLE_1 = 0xAA
PROTOCOL_PREAMBLE_2 = 0x55
PROTOCOL_PREAMBLE_3 = 0xAA
PROTOCOL_SOF = 0x55  # Start of Frame
PROTOCOL_EOF = 0xAA  # End of Frame
CMD_LOAD_GIF = 0x01

def calculate_checksum(data):
    checksum = 0
    for byte in data:
        checksum ^= byte
    return checksum

def send_gif(ser, screen_id, filename, x, y, width, height):
    """Send GIF load command for Screen #2 with proper preamble"""
    # Pad filename to 64 bytes
    filename_bytes = filename.encode('ascii')[:64].ljust(64, b'\x00')
    
    # Pack the GifCommand structure (payload)
    # GifCommand: screen_id(1) + command(1) + element_id(1) + x(2) + y(2) + width(2) + height(2) + filename(64) = 75 bytes
    # IMPORTANT: Use '<' prefix for little-endian, no padding (same as __attribute__((packed)))
    element_id = 1  # Use element ID 1
    payload = struct.pack('<BBBHHHH64s', 
                         screen_id, CMD_LOAD_GIF, element_id,
                         x, y, width, height,
                         filename_bytes)
    
    print(f"  Payload size: {len(payload)} bytes (expected 75)")
    
    # Create packet with PREAMBLE
    preamble = struct.pack('<BBB', PROTOCOL_PREAMBLE_1, PROTOCOL_PREAMBLE_2, PROTOCOL_PREAMBLE_3)
    header = struct.pack('<BBBB', PROTOCOL_SOF, screen_id, CMD_LOAD_GIF, len(payload))
    checksum = calculate_checksum(payload)
    packet = preamble + header + payload + struct.pack('<BB', checksum, PROTOCOL_EOF)
    
    ser.write(packet)
    print(f"✓ Sent: {filename} at ({x},{y}) size {width}x{height} to Screen #{screen_id}")
    print(f"  Packet size: {len(packet)} bytes (preamble + header + payload + checksum + EOF)")

def main():
    # IMPORTANT: Cross-connected serial ports!
    # led-image-viewer uses /dev/ttyUSB0 (RX)
    # This test script uses /dev/ttyUSB1 (TX)
    # Ports are cross-connected: ttyUSB0 <-> ttyUSB1 (TX/RX swapped)
    port = sys.argv[1] if len(sys.argv) >= 2 else "/dev/ttyUSB1"
    
    print("=" * 60)
    print("Screen #2: Display XVnx.gif")
    print("led-image-viewer RX: /dev/ttyUSB0")
    print("test script TX:      /dev/ttyUSB1")
    print("Ports are cross-connected (TX/RX swapped)")
    print("=" * 60)
    
    #while(True):
    try:
        ser = serial.Serial(port, 1000000, timeout=1)
        print(f"✓ Connected to {port}")
        time.sleep(0.5)
        
        # Display XVnx.gif at (0,0) on Screen #2
        # Screen #2 is 64px wide x 512px tall
        send_gif(ser, 
            screen_id=2, 
            filename="anim/XVnx.gif",
            x=0,
            y=500-80,
            width=64,   # Full width
            height=80) # Full height
        
    except Exception as e:
        print(f"✗ Error: {e}")
        
    time.sleep(0.5)




if __name__ == '__main__':
    main()
