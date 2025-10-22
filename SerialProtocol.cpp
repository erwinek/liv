#include "SerialProtocol.h"
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <time.h>
#include <sys/ioctl.h>

SerialProtocol::SerialProtocol() : serial_fd(-1), 
    last_garbage_time_us(0),
    esp32_restart_detected_time_us(0),
    esp32_restart_grace_period(false) {
    memset(&old_tio, 0, sizeof(old_tio));
}

SerialProtocol::~SerialProtocol() {
    close();
    // Free any pending commands
    for (auto cmd : pending_commands) {
        free(cmd);
    }
}

bool SerialProtocol::init(const char* device_path) {
    // Open serial port
    serial_fd = open(device_path, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (serial_fd < 0) {
        std::cerr << "Failed to open serial port: " << device_path << std::endl;
        perror("open");
        return false;
    }
    
    // Save current terminal settings
    if (tcgetattr(serial_fd, &old_tio) < 0) {
        std::cerr << "Failed to get serial port attributes" << std::endl;
        close();
        return false;
    }
    
    // Configure serial port
    struct termios tio;
    memset(&tio, 0, sizeof(tio));
    
    // Set baud rate to 1000000 (1Mbps)
    cfsetispeed(&tio, B1000000);
    cfsetospeed(&tio, B1000000);
    
    // 8N1 mode
    tio.c_cflag = B1000000 | CS8 | CLOCAL | CREAD;
    tio.c_iflag = IGNPAR;
    tio.c_oflag = 0;
    tio.c_lflag = 0;
    
    // Non-blocking read
    tio.c_cc[VTIME] = 0;
    tio.c_cc[VMIN] = 0;
    
    // Flush port and apply settings
    tcflush(serial_fd, TCIFLUSH);
    if (tcsetattr(serial_fd, TCSANOW, &tio) < 0) {
        std::cerr << "Failed to set serial port attributes" << std::endl;
        close();
        return false;
    }
    
    std::cout << "Serial protocol initialized on " << device_path << " at 1000000 baud" << std::endl;
    return true;
}

void SerialProtocol::processData() {
    // Check if we're in ESP32 restart grace period
    if (esp32_restart_grace_period) {
        uint64_t current_time = getCurrentTimeUs();
        uint64_t elapsed = current_time - esp32_restart_detected_time_us;
        
        if (elapsed < RESTART_GRACE_PERIOD_US) {
            // Still in grace period - just flush and ignore data
            uint8_t dummy[1024];
            ssize_t bytes_read = read(serial_fd, dummy, sizeof(dummy));
            if (bytes_read > 0) {
                std::cout << "ESP32 restart grace period: ignoring " << bytes_read 
                          << " bytes (remaining: " << (RESTART_GRACE_PERIOD_US - elapsed) / 1000 << " ms)" << std::endl;
            }
            rx_buffer.clear();
            return;
        } else {
            // Grace period ended
            std::cout << "ESP32 restart grace period ended - resuming normal operation" << std::endl;
            esp32_restart_grace_period = false;
            rx_buffer.clear();
        }
    }
    
    // Read data from serial port
    uint8_t buffer[1024];
    ssize_t bytes_read = read(serial_fd, buffer, sizeof(buffer));
    
    if (bytes_read > 0) {
        std::cout << "=== Received " << bytes_read << " bytes from serial port ===" << std::endl;
        
        // Add all received bytes to buffer
        for (ssize_t i = 0; i < bytes_read; i++) {
            if (buffer[i] == PROTOCOL_SOF) {
                std::cout << "*** SOF received (0xAA) at buffer position " << rx_buffer.size() << std::endl;
            }
            if (buffer[i] != 0 || rx_buffer.empty()) {
                std::cout << "RX[" << rx_buffer.size() << "]: 0x" << std::hex << (int)buffer[i] << std::dec << std::endl;
            }
            addToBuffer(buffer[i]);
        }
    }
    
    if (rx_buffer.size() > 0) {
        std::cout << "Buffer size: " << rx_buffer.size() << ", calling processBuffer()" << std::endl;
        processBuffer();
    }
}

void SerialProtocol::sendResponse(uint8_t screen_id, ResponseCode code, const uint8_t* data, uint8_t data_len) {
    Response response;
    response.screen_id = screen_id;
    response.command = CMD_RESPONSE;
    response.response_code = code;
    response.data_length = data_len;
    
    if (data && data_len > 0) {
        memcpy(response.data, data, data_len);
    }
    
    // Prepare buffer with preamble + packet
    uint8_t send_buffer[sizeof(ProtocolPacket) + 3];
    
    // Add preamble first (3 bytes)
    send_buffer[0] = PROTOCOL_PREAMBLE_1;
    send_buffer[1] = PROTOCOL_PREAMBLE_2;
    send_buffer[2] = PROTOCOL_PREAMBLE_3;
    
    ProtocolPacket packet;
    packet.sof = PROTOCOL_SOF;
    packet.screen_id = screen_id;
    packet.command = CMD_RESPONSE;
    // Response header is 4 bytes (screen_id, command, response_code, data_length) + actual data
    packet.payload_length = 4 + data_len;
    packet.eof = PROTOCOL_EOF;
    
    // Copy only the actual response size to avoid buffer overflow
    memcpy(packet.payload, &response, 4 + data_len);
    packet.checksum = calculateChecksum(packet.payload, packet.payload_length);
    
    // Copy packet after preamble
    memcpy(send_buffer + 3, &packet, sizeof(ProtocolPacket));
    
    // Send via serial port
    ssize_t sent = write(serial_fd, send_buffer, 3 + sizeof(ProtocolPacket));
    
    std::cout << ">>> RESPONSE SENT via serial port: screen_id=" << (int)screen_id 
              << " code=" << (int)code 
              << " bytes_sent=" << sent << " (including preamble)" << std::endl;
}

bool SerialProtocol::hasPendingCommand() {
    return !pending_commands.empty();
}

void* SerialProtocol::getNextCommand() {
    if (pending_commands.empty()) {
        return nullptr;
    }
    
    void* cmd = pending_commands.front();
    pending_commands.erase(pending_commands.begin());
    return cmd;
}

CommandType SerialProtocol::getCommandType(void* command) {
    if (!command) return (CommandType)0;
    
    uint8_t* cmd_bytes = (uint8_t*)command;
    return (CommandType)cmd_bytes[1]; // command field is at offset 1
}

void SerialProtocol::freeCommand(void* command) {
    if (command) {
        free(command);
    }
}

void SerialProtocol::close() {
    if (serial_fd != -1) {
        // Restore old terminal settings
        tcsetattr(serial_fd, TCSANOW, &old_tio);
        ::close(serial_fd);
        serial_fd = -1;
    }
}

void SerialProtocol::sendTestData() {
    if (serial_fd == -1) {
        std::cout << "Serial port not open, cannot send test data" << std::endl;
        return;
    }
    
    // Send a simple test message
    const char* test_msg = "LED_TEST\r\n";
    ssize_t bytes_sent = write(serial_fd, test_msg, strlen(test_msg));
    
    if (bytes_sent > 0) {
        std::cout << "Sent test data via serial port: " << test_msg << " (" << bytes_sent << " bytes)" << std::endl;
    } else {
        std::cout << "Failed to send test data via serial port" << std::endl;
    }
}

uint8_t SerialProtocol::calculateChecksum(const uint8_t* data, uint8_t length) {
    uint8_t checksum = 0;
    for (uint8_t i = 0; i < length; i++) {
        checksum ^= data[i];
    }
    return checksum;
}

bool SerialProtocol::validatePacket(const ProtocolPacket* packet) {
    if (!packet) return false;
    
    std::cout << "validatePacket: SOF=" << (int)packet->sof << " EOF=" << (int)packet->eof 
              << " payload_length=" << (int)packet->payload_length << std::endl;
    
    // Check SOF and EOF
    if (packet->sof != PROTOCOL_SOF || packet->eof != PROTOCOL_EOF) {
        std::cout << "SOF/EOF validation failed: SOF=" << (int)packet->sof << " EOF=" << (int)packet->eof << std::endl;
        return false;
    }
    
    // Note: payload_length is uint8_t so it's always <= 255, which is less than PROTOCOL_MAX_PAYLOAD (256)
    // No need to check upper bound
    
    // Verify checksum
    uint8_t calculated_checksum = calculateChecksum(packet->payload, packet->payload_length);
    std::cout << "Checksum: received=" << (int)packet->checksum << " calculated=" << (int)calculated_checksum << std::endl;
    
    if (packet->checksum != calculated_checksum) {
        std::cout << "Checksum validation failed" << std::endl;
        return false;
    }
    
    std::cout << "All validations passed" << std::endl;
    return true;
}

void SerialProtocol::parsePacket(const ProtocolPacket* packet) {
    std::cout << "parsePacket: command=" << (int)packet->command << " payload_length=" << (int)packet->payload_length << std::endl;
    
    if (!validatePacket(packet)) {
        std::cout << "Packet validation failed" << std::endl;
        sendResponse(packet->screen_id, RESP_PROTOCOL_ERROR);
        return;
    }
    
    std::cout << "Packet validation passed" << std::endl;
    
    void* command = nullptr;
    
    switch (packet->command) {
        case CMD_LOAD_GIF:
            std::cout << "Parsing GIF command" << std::endl;
            command = parseGifCommand(packet->payload, packet->payload_length);
            break;
        case CMD_DISPLAY_TEXT:
            std::cout << "Parsing TEXT command" << std::endl;
            command = parseTextCommand(packet->payload, packet->payload_length);
            break;
        case CMD_CLEAR_SCREEN:
            std::cout << "Parsing CLEAR command" << std::endl;
            command = parseClearCommand(packet->payload, packet->payload_length, packet->screen_id, packet->command);
            break;
        case CMD_CLEAR_TEXT:
            std::cout << "Parsing CLEAR_TEXT command" << std::endl;
            command = parseClearCommand(packet->payload, packet->payload_length, packet->screen_id, packet->command);
            break;
        case CMD_DELETE_ELEMENT:
            std::cout << "Parsing DELETE_ELEMENT command" << std::endl;
            command = parseDeleteElementCommand(packet->payload, packet->payload_length, packet->screen_id, packet->command);
            break;
        case CMD_SET_BRIGHTNESS:
            std::cout << "Parsing BRIGHTNESS command" << std::endl;
            command = parseBrightnessCommand(packet->payload, packet->payload_length);
            break;
        case CMD_GET_STATUS:
            std::cout << "Parsing STATUS command" << std::endl;
            command = parseStatusCommand(packet->payload, packet->payload_length);
            break;
        default:
            std::cout << "Unknown command: " << (int)packet->command << std::endl;
            sendResponse(packet->screen_id, RESP_ERROR);
            return;
    }
    
    if (command) {
        pending_commands.push_back(command);
        sendResponse(packet->screen_id, RESP_OK);
    } else {
        sendResponse(packet->screen_id, RESP_INVALID_PARAMS);
    }
}

void SerialProtocol::addToBuffer(uint8_t byte) {
    rx_buffer.push_back(byte);
    
    // Increased buffer size to handle high traffic and noise on the line
    // With preamble, we need more space for synchronization
    if (rx_buffer.size() > 2048) {
        std::cout << "Buffer overflow! Removing 512 bytes" << std::endl;
        rx_buffer.erase(rx_buffer.begin(), rx_buffer.begin() + 512);
    }
}


void SerialProtocol::processBuffer() {
    // Aggressive garbage removal for better synchronization
    // Look for preamble pattern: [0xAA][0x55][0xAA][0x55] (SOF)
    while (!rx_buffer.empty()) {
        // Find preamble + SOF sequence
        size_t preamble_position = rx_buffer.size();
        for (size_t i = 0; i + 3 < rx_buffer.size(); i++) {
            // Look for: PREAMBLE_1(0xAA), PREAMBLE_2(0x55), PREAMBLE_3(0xAA), SOF(0x55)
            if (rx_buffer[i]   == PROTOCOL_PREAMBLE_1 &&
                rx_buffer[i+1] == PROTOCOL_PREAMBLE_2 &&
                rx_buffer[i+2] == PROTOCOL_PREAMBLE_3 &&
                rx_buffer[i+3] == PROTOCOL_SOF) {
                preamble_position = i;
                std::cout << "*** PREAMBLE+SOF found at position " << i << " [0xAA 0x55 0xAA 0x55]" << std::endl;
                break;
            }
        }
        
        // If no valid preamble+SOF found, buffer contains only garbage
        if (preamble_position == rx_buffer.size()) {
            size_t garbage_size = rx_buffer.size();
            
            // Keep last 3 bytes in case preamble is being received
            if (garbage_size > 3) {
                std::cout << "No valid preamble+SOF in buffer, clearing " << (garbage_size - 3) << " bytes of garbage (keeping last 3)" << std::endl;
                rx_buffer.erase(rx_buffer.begin(), rx_buffer.begin() + (garbage_size - 3));
                
                // Detect potential ESP32 restart
                if (garbage_size > 100) {
                    detectESP32Restart(garbage_size);
                }
            }
            return;
        }
        
        // Remove any garbage before preamble
        if (preamble_position > 0) {
            std::cout << "Removing " << preamble_position << " bytes of garbage before preamble" << std::endl;
            rx_buffer.erase(rx_buffer.begin(), rx_buffer.begin() + preamble_position);
        }
        
        // Now preamble+SOF is at position 0-3
        // Skip preamble (3 bytes) to get to actual packet start (SOF at position 3)
        std::cout << "Preamble verified, SOF at position 3" << std::endl;
        
        // Check if we have enough data for packet header
        // Preamble (3) + SOF (1) + screen_id (1) + command (1) + payload_length (1) = 7 bytes minimum
        if (rx_buffer.size() < 7) {
            std::cout << "Not enough data for header yet (have " << rx_buffer.size() << " bytes, need 7)" << std::endl;
            return; // Wait for more data
        }
        
        // Read header (skip preamble, start from SOF at position 3)
        uint8_t screen_id = rx_buffer[4];       // Position 4 (after preamble+SOF)
        uint8_t command = rx_buffer[5];         // Position 5
        uint8_t payload_length = rx_buffer[6];  // Position 6
        
        std::cout << "Packet header: screen_id=" << (int)screen_id 
                  << " command=" << (int)command 
                  << " payload_length=" << (int)payload_length << std::endl;
        
        // Calculate total packet size including preamble
        // Preamble (3) + SOF (1) + screen_id (1) + command (1) + payload_length (1) + payload + checksum (1) + EOF (1)
        size_t total_packet_size = 3 + 4 + payload_length + 2; // preamble + header + payload + checksum + EOF
        
        // Check if we have complete packet
        if (rx_buffer.size() < total_packet_size) {
            std::cout << "Not enough data for complete packet (need " << total_packet_size << " bytes, have " << rx_buffer.size() << ")" << std::endl;
            return; // Wait for more data
        }
        
        // Check EOF at the calculated position
        size_t eof_position = total_packet_size - 1;
        if (rx_buffer[eof_position] != PROTOCOL_EOF) {
            std::cout << "EOF mismatch: expected 0xAA, got 0x" << std::hex << (int)rx_buffer[eof_position] << std::dec 
                      << " - this preamble was false positive, removing first byte" << std::endl;
            // This preamble was false positive, remove first byte and continue searching
            rx_buffer.erase(rx_buffer.begin());
            continue;
        }
        
        std::cout << "Complete valid packet found, parsing..." << std::endl;
        
        // Create a temporary ProtocolPacket for parsing
        // Skip preamble (3 bytes), start from SOF (position 3)
        ProtocolPacket packet;
        packet.sof = rx_buffer[3];              // SOF at position 3
        packet.screen_id = rx_buffer[4];        // screen_id at position 4
        packet.command = rx_buffer[5];          // command at position 5
        packet.payload_length = rx_buffer[6];   // payload_length at position 6
        
        // Copy payload (starts at position 7)
        memcpy(packet.payload, &rx_buffer[7], payload_length);
        
        // Set checksum and EOF
        packet.checksum = rx_buffer[eof_position - 1];
        packet.eof = rx_buffer[eof_position];
        
        parsePacket(&packet);
        
        // Remove processed packet from buffer (including preamble)
        rx_buffer.erase(rx_buffer.begin(), rx_buffer.begin() + total_packet_size);
        std::cout << "Packet processed and removed, buffer now has " << rx_buffer.size() << " bytes" << std::endl;
        
        // Continue processing if there's more data
    }
}

void* SerialProtocol::parseGifCommand(const uint8_t* payload, uint8_t length) {
    std::cout << "parseGifCommand: length=" << (int)length << " sizeof(GifCommand)=" << sizeof(GifCommand) << std::endl;

    // Accept two payload variants:
    // 1) Full GifCommand (74 bytes): [screen_id][command][x:2][y:2][w:2][h:2][filename:64]
    // 2) Legacy (70 bytes):          [screen_id][command][x:2][y:2][w:2][h:2][filename:60]

    if (!(length == sizeof(GifCommand) || length == 70)) {
        std::cout << "parseGifCommand: unsupported payload length " << (int)length << std::endl;
        return nullptr;
    }

    GifCommand* cmd = (GifCommand*)malloc(sizeof(GifCommand));
    if (!cmd) {
        std::cout << "parseGifCommand: malloc failed" << std::endl;
        return nullptr;
    }

    if (length == sizeof(GifCommand)) {
        // Direct copy for current format
        memcpy(cmd, payload, sizeof(GifCommand));
        
        // Debug: show raw bytes
        std::cout << "parseGifCommand: Raw payload bytes: ";
        for (int i = 0; i < 15; i++) {
            printf("%02x ", payload[i]);
        }
        std::cout << std::endl;
    } else {
        // Legacy 70-byte format parser (filename 60 bytes)
        // Offsets (little-endian):
        // 0: screen_id (u8)
        // 1: command   (u8)
        // 2: x (u16), 4: y (u16), 6: w (u16), 8: h (u16)
        // 10: filename (60 bytes)
        const uint8_t* p = payload;
        cmd->screen_id = p[0];
        cmd->command = p[1];

        uint16_t x, y, w, h;
        memcpy(&x, p + 2, 2);
        memcpy(&y, p + 4, 2);
        memcpy(&w, p + 6, 2);
        memcpy(&h, p + 8, 2);
        cmd->x_pos = x;
        cmd->y_pos = y;
        cmd->width = w;
        cmd->height = h;

        // Copy 60-byte filename and pad with zeros to 64
        memset(cmd->filename, 0, sizeof(cmd->filename));
        memcpy(cmd->filename, p + 10, 60);
        cmd->filename[63] = '\0';
    }

    std::cout << "parseGifCommand: screen_id=" << (int)cmd->screen_id
              << " command=" << (int)cmd->command
              << " x=" << cmd->x_pos << " y=" << cmd->y_pos
              << " w=" << cmd->width << " h=" << cmd->height
              << " filename=" << cmd->filename << std::endl;

    // NOTE: Bounds check is done in DisplayManager, not here
    // Parser doesn't know screen dimensions (could be 192x192, 64x512, etc.)

    std::cout << "parseGifCommand: success, returning command" << std::endl;
    return cmd;
}

void* SerialProtocol::parseTextCommand(const uint8_t* payload, uint8_t length) {
    std::cout << "parseTextCommand: length=" << (int)length << " sizeof(TextCommand)=" << sizeof(TextCommand) << std::endl;
    
    if (length < sizeof(TextCommand)) {
        std::cout << "parseTextCommand: payload too short" << std::endl;
        return nullptr;
    }
    
    TextCommand* cmd = (TextCommand*)malloc(sizeof(TextCommand));
    if (!cmd) {
        std::cout << "parseTextCommand: malloc failed" << std::endl;
        return nullptr;
    }
    
    memcpy(cmd, payload, sizeof(TextCommand));
    
    std::cout << "parseTextCommand: x=" << cmd->x_pos << " y=" << cmd->y_pos 
              << " text_length=" << (int)cmd->text_length << std::endl;
    
    // Validate parameters
    if (cmd->x_pos >= 192 || cmd->y_pos >= 192 || 
        cmd->text_length > PROTOCOL_MAX_TEXT_LENGTH) {
        std::cout << "parseTextCommand: validation failed" << std::endl;
        free(cmd);
        return nullptr;
    }
    
    std::cout << "parseTextCommand: success, returning command" << std::endl;
    return cmd;
}

void* SerialProtocol::parseClearCommand(const uint8_t* payload, uint8_t length, uint8_t packet_screen_id, uint8_t packet_command) {
    ClearCommand* cmd = (ClearCommand*)malloc(sizeof(ClearCommand));
    if (!cmd) return nullptr;
    
    if (length >= sizeof(ClearCommand)) {
        // Payload contains full command structure
        memcpy(cmd, payload, sizeof(ClearCommand));
    } else {
        // Payload is empty - use values from packet header
        cmd->screen_id = packet_screen_id;
        cmd->command = packet_command;
    }
    
    std::cout << "parseClearCommand: created command with screen_id=" << (int)cmd->screen_id 
              << " command=" << (int)cmd->command << std::endl;
    
    return cmd;
}

void* SerialProtocol::parseDeleteElementCommand(const uint8_t* payload, uint8_t length, uint8_t packet_screen_id, uint8_t packet_command) {
    DeleteElementCommand* cmd = (DeleteElementCommand*)malloc(sizeof(DeleteElementCommand));
    if (!cmd) return nullptr;
    
    if (length >= sizeof(DeleteElementCommand)) {
        // Payload contains full command structure
        memcpy(cmd, payload, sizeof(DeleteElementCommand));
    } else if (length >= 1) {
        // Payload contains only element_id
        cmd->screen_id = packet_screen_id;
        cmd->command = packet_command;
        cmd->element_id = payload[0];
    } else {
        // Invalid - need at least element_id
        free(cmd);
        return nullptr;
    }
    
    std::cout << "parseDeleteElementCommand: screen_id=" << (int)cmd->screen_id 
              << " command=" << (int)cmd->command 
              << " element_id=" << (int)cmd->element_id << std::endl;
    
    return cmd;
}

void* SerialProtocol::parseBrightnessCommand(const uint8_t* payload, uint8_t length) {
    if (length < sizeof(BrightnessCommand)) {
        return nullptr;
    }
    
    BrightnessCommand* cmd = (BrightnessCommand*)malloc(sizeof(BrightnessCommand));
    if (!cmd) return nullptr;
    
    memcpy(cmd, payload, sizeof(BrightnessCommand));
    
    // Validate brightness (0-100)
    if (cmd->brightness > 100) {
        free(cmd);
        return nullptr;
    }
    
    return cmd;
}

void* SerialProtocol::parseStatusCommand(const uint8_t* payload, uint8_t length) {
    if (length < sizeof(StatusCommand)) {
        return nullptr;
    }
    
    StatusCommand* cmd = (StatusCommand*)malloc(sizeof(StatusCommand));
    if (!cmd) return nullptr;
    
    memcpy(cmd, payload, sizeof(StatusCommand));
    return cmd;
}

uint64_t SerialProtocol::getCurrentTimeUs() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000ULL + (uint64_t)ts.tv_nsec / 1000ULL;
}

void SerialProtocol::detectESP32Restart(size_t garbage_bytes) {
    // If we receive >200 bytes of garbage, it's likely ESP32 restarted
    // ESP32 boot messages can be 500-1000+ bytes
    if (garbage_bytes > 200) {
        uint64_t current_time = getCurrentTimeUs();
        
        // If we haven't seen garbage in a while, this is likely a restart
        if (last_garbage_time_us == 0 || (current_time - last_garbage_time_us) > 5000000) { // 5 seconds
            std::cout << "=== ESP32 RESTART DETECTED (" << garbage_bytes << " bytes of garbage) ===" << std::endl;
            std::cout << "=== Entering " << (RESTART_GRACE_PERIOD_US / 1000000) << " second grace period ===" << std::endl;
            
            esp32_restart_detected_time_us = current_time;
            esp32_restart_grace_period = true;
            
            // Clear buffer
            rx_buffer.clear();
        }
        
        last_garbage_time_us = current_time;
    }
}

