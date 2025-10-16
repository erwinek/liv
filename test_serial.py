#!/usr/bin/env python3
"""
Fixed test script for LED Display Controller serial protocol
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

def create_packet(screen_id, command, payload):
    """Create a protocol packet
    
    Packet structure:
    - SOF (1 byte): 0xAA
    - screen_id (1 byte)
    - command (1 byte)
    - payload_length (1 byte)
    - payload (variable)
    - checksum (1 byte) - XOR of payload
    - EOF (1 byte): 0x55
    """
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
    """Send GIF load command
    
    GifCommand structure:
    - screen_id (1 byte)
    - command (1 byte)
    - x_pos (2 bytes, uint16)
    - y_pos (2 bytes, uint16)
    - width (2 bytes, uint16)
    - height (2 bytes, uint16)
    - filename (64 bytes, null-terminated string)
    """
    # Pad filename to 64 bytes
    filename_bytes = filename.encode('ascii')[:64]
    filename_bytes = filename_bytes.ljust(64, b'\x00')
    
    # Pack the GifCommand structure - this goes in the payload
    payload = struct.pack('BBHHHH64s', 
                         screen_id,      # screen_id
                         CMD_LOAD_GIF,   # command
                         x,              # x_pos
                         y,              # y_pos
                         width,          # width
                         height,         # height
                         filename_bytes) # filename
    
    packet = create_packet(screen_id, CMD_LOAD_GIF, payload)
    ser.write(packet)
    print(f"Sent GIF command: {filename} at ({x},{y}) size {width}x{height}")

def send_text_command(ser, screen_id, text, x, y, font_size, r, g, b):
    """Send text display command
    
    TextCommand structure:
    - screen_id (1 byte)
    - command (1 byte)
    - x_pos (2 bytes, uint16)
    - y_pos (2 bytes, uint16)
    - font_size (1 byte)
    - color_r (1 byte)
    - color_g (1 byte)
    - color_b (1 byte)
    - text_length (1 byte)
    - text (32 bytes, null-terminated string)
    """
    text_bytes = text.encode('ascii')[:32]
    text_length = len(text_bytes)
    text_bytes = text_bytes.ljust(32, b'\x00')
    
    # Pack the TextCommand structure - this goes in the payload
    payload = struct.pack('BBHHBBBBB32s',
                         screen_id,          # screen_id
                         CMD_DISPLAY_TEXT,   # command
                         x,                  # x_pos
                         y,                  # y_pos
                         font_size,          # font_size
                         r,                  # color_r
                         g,                  # color_g
                         b,                  # color_b
                         text_length,        # text_length
                         text_bytes)         # text
    
    packet = create_packet(screen_id, CMD_DISPLAY_TEXT, payload)
    ser.write(packet)
    print(f"Sent TEXT command: '{text}' at ({x},{y}) font {font_size} color ({r},{g},{b})")

def send_clear_command(ser, screen_id):
    """Send clear screen command
    
    ClearCommand structure:
    - screen_id (1 byte)
    - command (1 byte)
    """
    payload = struct.pack('BB', screen_id, CMD_CLEAR_SCREEN)
    packet = create_packet(screen_id, CMD_CLEAR_SCREEN, payload)
    ser.write(packet)
    print(f"Sent CLEAR command for screen {screen_id}")

def send_brightness_command(ser, screen_id, brightness):
    """Send brightness command
    
    BrightnessCommand structure:
    - screen_id (1 byte)
    - command (1 byte)
    - brightness (1 byte) - 0-100
    """
    payload = struct.pack('BBB', screen_id, CMD_SET_BRIGHTNESS, brightness)
    packet = create_packet(screen_id, CMD_SET_BRIGHTNESS, payload)
    ser.write(packet)
    print(f"Sent BRIGHTNESS command: {brightness}% for screen {screen_id}")

def send_status_command(ser, screen_id):
    """Send status request command
    
    StatusCommand structure:
    - screen_id (1 byte)
    - command (1 byte)
    """
    payload = struct.pack('BB', screen_id, CMD_GET_STATUS)
    packet = create_packet(screen_id, CMD_GET_STATUS, payload)
    ser.write(packet)
    print(f"Sent STATUS command for screen {screen_id}")

def read_response(ser, timeout=1.0):
    """Read response from serial port"""
    ser.timeout = timeout
    response = ser.read(1024)
    if response:
        print(f"Received response ({len(response)} bytes): {response.hex()}")
        return response
    return None

def main():
    # Default to /dev/ttyUSB1 (cross-connected with /dev/ttyUSB0 used by led-image-viewer)
    port = "/dev/ttyUSB1"
    
    if len(sys.argv) >= 2:
        port = sys.argv[1]
    
    print(f"Note: LED viewer uses /dev/ttyUSB0, test script uses /dev/ttyUSB1 (cross-connected)")
    print(f"Using port: {port}")
    screen_id = 1
    
    try:
        # Open serial port
        ser = serial.Serial(port, 1000000, timeout=1)
        print(f"Connected to {port} at 1000000 bps")
        time.sleep(0.5)  # Give time for connection to establish
        
        # Test sequence
        print("\n=== LED Display Controller Test ===")
        
        # 1. Clear screen
        print("\n--- Test 1: Clear Screen ---")
        send_clear_command(ser, screen_id)
        read_response(ser, 0.5)
        time.sleep(0.5)
        
        # 2. Set brightness
        print("\n--- Test 2: Set Brightness ---")
        send_brightness_command(ser, screen_id, 80)
        read_response(ser, 0.5)
        time.sleep(0.5)
        
        # 3. Display text
        print("\n--- Test 3: Display Text ---")
        send_text_command(ser, screen_id, "HELLO!", 10, 10, 2, 255, 255, 0)
        read_response(ser, 0.5)
        time.sleep(2)
        
        # 4. More text
        print("\n--- Test 4: More Text ---")
        send_text_command(ser, screen_id, "Test OK", 10, 40, 3, 0, 255, 0)
        read_response(ser, 0.5)
        time.sleep(2)
        
        # 5. Load GIF (if available)
        print("\n--- Test 5: Load GIF ---")
        send_gif_command(ser, screen_id, "anim/1.gif", 96, 96, 96, 96)
        read_response(ser, 0.5)
        time.sleep(2)
        
        # 6. Get status
        print("\n--- Test 6: Get Status ---")
        send_status_command(ser, screen_id)
        read_response(ser, 0.5)
        
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

