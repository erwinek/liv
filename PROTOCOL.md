# LED Display Controller - Serial Protocol

## Overview
The LED Display Controller supports RS232 communication at 1000000 bps for remote control of the 192x192 LED matrix display.

## Protocol Format
All packets follow this structure:
```
[SOF][ScreenID][Command][PayloadLength][Payload][Checksum][EOF]
```

- **SOF**: Start of Frame (0xAA)
- **ScreenID**: Screen identifier (0-255)
- **Command**: Command type (see below)
- **PayloadLength**: Length of payload data (0-256)
- **Payload**: Command-specific data
- **Checksum**: XOR checksum of payload
- **EOF**: End of Frame (0x55)

## Commands

### 1. Load GIF (0x01)
Load and display a GIF file at specified position and size.

**Payload Structure:**
```
[ScreenID][Command][X_Pos][Y_Pos][Width][Height][Filename(64 bytes)]
```

- **X_Pos**: Left position (0-191)
- **Y_Pos**: Top position (0-191)
- **Width**: Display width (1-192)
- **Height**: Display height (1-192)
- **Filename**: ASCII filename, null-terminated, padded to 64 bytes

**Example:**
```python
# Load anim/1.gif at position (0,0) with size 96x96
payload = struct.pack('BBHHH', 1, 0x01, 0, 0, 96, 96)
payload += b'anim/1.gif\x00' + b'\x00' * 54  # Pad to 64 bytes
```

### 2. Display Text (0x02)
Display text at specified position with font size and color.

**Payload Structure:**
```
[ScreenID][Command][X_Pos][Y_Pos][FontSize][R][G][B][TextLength][Text(32 bytes)]
```

- **X_Pos**: Left position (0-191)
- **Y_Pos**: Top position (0-191)
- **FontSize**: Font size (1-8)
- **R, G, B**: Color components (0-255)
- **TextLength**: Length of text (0-32)
- **Text**: ASCII text, padded to 32 bytes

**Example:**
```python
# Display "Hello" at (10,10) with font size 2, yellow color
text = "Hello"
payload = struct.pack('BBHBBBBBB', 1, 0x02, 10, 10, 2, 255, 255, 0, len(text))
payload += text.encode('ascii').ljust(32, b'\x00')
```

### 3. Clear Screen (0x03)
Clear the entire screen.

**Payload Structure:**
```
[ScreenID][Command]
```

### 4. Set Brightness (0x04)
Set display brightness.

**Payload Structure:**
```
[ScreenID][Command][Brightness]
```

- **Brightness**: Brightness level (0-100)

### 5. Get Status (0x05)
Request status information.

**Payload Structure:**
```
[ScreenID][Command]
```

## Responses

All commands receive a response with this structure:
```
[ScreenID][Command(0x80)][ResponseCode][DataLength][Data]
```

**Response Codes:**
- **0x00**: OK
- **0x01**: General Error
- **0x02**: File Not Found
- **0x03**: Invalid Parameters
- **0x04**: Protocol Error

## Usage Examples

### Python Test Script
```python
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
```

### C/C++ Example
```c
// Create GIF command
GifCommand cmd;
cmd.screen_id = 1;
cmd.command = CMD_LOAD_GIF;
cmd.x_pos = 0;
cmd.y_pos = 0;
cmd.width = 96;
cmd.height = 96;
strncpy(cmd.filename, "anim/1.gif", sizeof(cmd.filename)-1);

// Create packet
ProtocolPacket packet;
packet.sof = PROTOCOL_SOF;
packet.screen_id = 1;
packet.command = CMD_LOAD_GIF;
packet.payload_length = sizeof(GifCommand);
packet.eof = PROTOCOL_EOF;
memcpy(packet.payload, &cmd, sizeof(GifCommand));
packet.checksum = calculate_checksum(packet.payload, packet.payload_length);

// Send packet
write(serial_fd, &packet, sizeof(ProtocolPacket));
```

## Error Handling

- **Bounds Checking**: Content that extends beyond 192x192 screen is clipped
- **File Not Found**: Returns error response if GIF file doesn't exist
- **Invalid Parameters**: Returns error for out-of-range values
- **Protocol Errors**: Invalid packets are rejected with error response

## Performance Notes

- GIF animations are synchronized across all elements
- Text scrolling is automatic for long text
- Multiple elements can be displayed simultaneously
- Brightness changes apply to entire display
- Commands are processed in real-time during display updates
