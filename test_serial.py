#!/usr/bin/env python3
"""
Test script for LED Display Controller serial protocol
"""

import serial
import struct
import time
import sys

# Protocol constants
PROTOCOL_SOF = 0xAA
PROTOCOL_EOF = 0x55
CMD_LOAD_GIF = 0x01
CMD_DISPLAY_TEXT = 0x02
CMD_CLEAR_SCREEN = 0x03
CMD_SET_BRIGHTNESS = 0x04
CMD_GET_STATUS = 0x05
CMD_RESPONSE = 0x80

def calculate_checksum(data):
    """Calculate XOR checksum for data"""
    checksum = 0
    for byte in data:
        checksum ^= byte
    return checksum

def rgb_to_8bit(r, g, b):
    """Convert RGB to 8-bit color index (simplified palette)"""
    # Basic color mapping for common colors
    if r == 255 and g == 255 and b == 255: return 1   # White
    if r == 0 and g == 0 and b == 0: return 0         # Black
    if r == 255 and g == 0 and b == 0: return 2       # Red
    if r == 0 and g == 255 and b == 0: return 3       # Green
    if r == 0 and g == 0 and b == 255: return 4       # Blue
    if r == 255 and g == 255 and b == 0: return 5     # Yellow
    if r == 255 and g == 0 and b == 255: return 6     # Magenta
    if r == 0 and g == 255 and b == 255: return 7     # Cyan
    if r == 128 and g == 128 and b == 128: return 8   # Gray
    if r == 192 and g == 192 and b == 192: return 9   # Light Gray
    if r == 64 and g == 64 and b == 64: return 10     # Dark Gray
    if r == 255 and g == 128 and b == 0: return 11    # Orange
    if r == 128 and g == 0 and b == 128: return 12    # Purple
    if r == 0 and g == 128 and b == 0: return 13      # Dark Green
    if r == 0 and g == 0 and b == 128: return 14      # Dark Blue
    if r == 128 and g == 128 and b == 0: return 15    # Olive
    
    # For other colors, use a simple quantization
    r_q = (r // 51) * 51  # Quantize to 0, 51, 102, 153, 204, 255
    g_q = (g // 51) * 51
    b_q = (b // 51) * 51
    
    # Map to palette index (16 + RGB cube position)
    return 16 + (r_q // 51) * 36 + (g_q // 51) * 6 + (b_q // 51)

def create_packet(screen_id, command, payload):
    """Create a protocol packet"""
    # Create packet header: SOF + screen_id + command + payload_length
    header = struct.pack('BBBB', PROTOCOL_SOF, screen_id, command, len(payload))
    
    # Calculate checksum of payload only
    checksum = calculate_checksum(payload)
    
    # Create complete packet: header + payload + checksum + EOF
    packet = header + payload + struct.pack('BB', checksum, PROTOCOL_EOF)
    
    # Debug: print packet structure
    print(f"Packet structure:")
    print(f"  Header: {header.hex()}")
    print(f"  Payload ({len(payload)} bytes): {payload.hex()}")
    print(f"  Checksum: 0x{checksum:02x}")
    print(f"  EOF: 0x{PROTOCOL_EOF:02x}")
    print(f"  Total packet: {packet.hex()}")
    print(f"  Total length: {len(packet)} bytes")
    
    return packet

def send_gif_command(ser, screen_id, filename, x, y, width, height):
    """Send GIF load command"""
    # Pad filename to 64 bytes
    filename_bytes = filename.encode('ascii')[:63] + b'\x00'
    filename_bytes = filename_bytes.ljust(64, b'\x00')
    
    payload = struct.pack('BBHHHH64s', screen_id, CMD_LOAD_GIF, x, y, width, height, filename_bytes)
    
    packet = create_packet(screen_id, CMD_LOAD_GIF, payload)
    ser.write(packet)
    print(f"Sent GIF command: {filename} at ({x},{y}) size {width}x{height}")

def send_text_command(ser, screen_id, text, x, y, font_size, r, g, b):
    """Send text display command"""
    text_bytes = text.encode('ascii')[:31]  # Max 32 chars
    text_bytes = text_bytes.ljust(32, b'\x00')
    
    # Convert RGB to 8-bit color index
    color_index = rgb_to_8bit(r, g, b)
    
    payload = struct.pack('BBHHBBBBB', screen_id, CMD_DISPLAY_TEXT, x, y, 
                         font_size, r, g, b, len(text)) + text_bytes
    
    packet = create_packet(screen_id, CMD_DISPLAY_TEXT, payload)
    ser.write(packet)
    print(f"Sent TEXT command: '{text}' at ({x},{y}) font {font_size} color ({r},{g},{b}) -> index {color_index}")

def send_clear_command(ser, screen_id):
    """Send clear screen command"""
    payload = struct.pack('BB', screen_id, CMD_CLEAR_SCREEN)
    packet = create_packet(screen_id, CMD_CLEAR_SCREEN, payload)
    ser.write(packet)
    print(f"Sent CLEAR command for screen {screen_id}")

def send_brightness_command(ser, screen_id, brightness):
    """Send brightness command"""
    payload = struct.pack('BBB', screen_id, CMD_SET_BRIGHTNESS, brightness)
    packet = create_packet(screen_id, CMD_SET_BRIGHTNESS, payload)
    ser.write(packet)
    print(f"Sent BRIGHTNESS command: {brightness}% for screen {screen_id}")

def send_status_command(ser, screen_id):
    """Send status request command"""
    payload = struct.pack('BB', screen_id, CMD_GET_STATUS)
    packet = create_packet(screen_id, CMD_GET_STATUS, payload)
    ser.write(packet)
    print(f"Sent STATUS command for screen {screen_id}")

def read_response(ser, timeout=1.0):
    """Read response from serial port"""
    ser.timeout = timeout
    response = ser.read(1024)
    if response:
        print(f"Received response: {response.hex()}")
        return response
    return None

def main():
    if len(sys.argv) < 2:
        print("Usage: python3 test_serial.py <serial_port>")
        print("Example: python3 test_serial.py /dev/ttyUSB0")
        sys.exit(1)
    
    port = sys.argv[1]
    screen_id = 1
    
    try:
        # Open serial port
        ser = serial.Serial(port, 1000000, timeout=1)
        print(f"Connected to {port} at 1000000 bps")
        
        # Test sequence
        print("\n=== LED Display Controller Test ===")
        
        # 1. Clear screen
        send_clear_command(ser, screen_id)
        time.sleep(0.5)
        
        # 2. Set brightness
        send_brightness_command(ser, screen_id, 80)
        time.sleep(0.5)
        
        # 3. Display text
        send_text_command(ser, screen_id, "HELLO World!", 10, 10, 2, 255, 255, 0)
        time.sleep(2)
        
        # 4. Load GIF (if available) - moved to not cover text
        send_gif_command(ser, screen_id, "anim/1.gif", 96, 96, 96, 96)
        time.sleep(2)
        
        # 5. More text
        send_text_command(ser, screen_id, "Test Complete", 10, 100, 3, 0, 255, 0)
        time.sleep(2)
        
        # 6. Get status
        send_status_command(ser, screen_id)
        time.sleep(0.5)
        
        # Read any responses
        print("\nReading responses...")
        for i in range(10):
            response = read_response(ser, 0.5)
            if not response:
                break
        
        print("\nTest completed!")
        
    except serial.SerialException as e:
        print(f"Serial error: {e}")
    except KeyboardInterrupt:
        print("\nTest interrupted by user")
    finally:
        if 'ser' in locals():
            ser.close()
            print("Serial port closed")

if __name__ == "__main__":
    main()
