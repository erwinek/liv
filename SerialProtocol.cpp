#include "SerialProtocol.h"
#include <fcntl.h>
#include <termios.h>
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
}

SerialProtocol::~SerialProtocol() {
    close();
    // Free any pending commands
    for (auto cmd : pending_commands) {
        free(cmd);
    }
}

bool SerialProtocol::init(const char* device, int baudrate) {
    // Open serial device
    serial_fd = open(device, O_RDWR | O_NOCTTY | O_NDELAY);
    if (serial_fd == -1) {
        std::cerr << "Failed to open serial device: " << device << std::endl;
        return false;
    }
    
    // Configure serial port
    struct termios tty;
    if (tcgetattr(serial_fd, &tty) != 0) {
        std::cerr << "Failed to get serial attributes" << std::endl;
        close();
        return false;
    }
    
    // Set baud rate
    speed_t speed;
    switch (baudrate) {
        case 9600: speed = B9600; break;
        case 19200: speed = B19200; break;
        case 38400: speed = B38400; break;
        case 57600: speed = B57600; break;
        case 115200: speed = B115200; break;
        case 230400: speed = B230400; break;
        case 460800: speed = B460800; break;
        case 1000000: speed = B1000000; break;
        default:
            std::cerr << "Unsupported baud rate: " << baudrate << std::endl;
            close();
            return false;
    }
    
    cfsetospeed(&tty, speed);
    cfsetispeed(&tty, speed);
    
    // Configure 8N1
    tty.c_cflag &= ~PARENB;        // No parity
    tty.c_cflag &= ~CSTOPB;        // One stop bit
    tty.c_cflag &= ~CSIZE;         // Clear size bits
    tty.c_cflag |= CS8;            // 8 data bits
    tty.c_cflag &= ~CRTSCTS;       // No hardware flow control
    tty.c_cflag |= CREAD;          // Enable reading
    tty.c_cflag &= ~CLOCAL;        // Don't ignore modem control lines (needed for DTR/RTS control!)
    
    std::cout << "Serial config: Hardware flow control OFF, Modem control lines ENABLED" << std::endl;
    
    // Configure input modes
    tty.c_iflag &= ~(IXON | IXOFF | IXANY); // No software flow control
    tty.c_iflag &= ~(ICANON | ECHO | ECHOE | ISIG); // Raw input
    
    // Configure output modes
    tty.c_oflag &= ~OPOST; // Raw output
    
    // Configure local modes
    tty.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); // Raw input
    
    // Set timeouts
    tty.c_cc[VTIME] = 0; // No timeout
    tty.c_cc[VMIN] = 0;  // Non-blocking read
    
    // Apply settings
    if (tcsetattr(serial_fd, TCSANOW, &tty) != 0) {
        std::cerr << "Failed to set serial attributes" << std::endl;
        close();
        return false;
    }
    
    // Initialize ESP32 with proper DTR/RTS sequence
    initESP32ResetSequence();
    
    // Flush any existing data in the buffers (important after ESP32 restart)
    tcflush(serial_fd, TCIOFLUSH);
    std::cout << "UART buffers flushed" << std::endl;
    
    std::cout << "Serial protocol initialized on " << device << " at " << baudrate << " bps" << std::endl;
    return true;
}

void SerialProtocol::processData() {
    // Check if we're in ESP32 restart grace period
    if (esp32_restart_grace_period) {
        uint64_t current_time = getCurrentTimeUs();
        uint64_t elapsed = current_time - esp32_restart_detected_time_us;
        
        if (elapsed < RESTART_GRACE_PERIOD_US) {
            // Still in grace period - just flush and ignore data
            uint8_t dummy[256];
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
            flushUartBuffers();
            rx_buffer.clear();
        }
    }
    
    uint8_t byte;
    ssize_t bytes_read = read(serial_fd, &byte, 1);
    
    // Reduce debug output during normal operation (only show non-zero bytes or when buffer is building)
    while (bytes_read > 0) {
        if (byte != 0 || rx_buffer.empty()) {
            //std::cout << "Received byte: 0x" << std::hex << (int)byte << std::dec << std::endl;
        }
        addToBuffer(byte);
        bytes_read = read(serial_fd, &byte, 1);
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
    
    write(serial_fd, &packet, sizeof(ProtocolPacket));
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
    ssize_t bytes_written = write(serial_fd, test_msg, strlen(test_msg));
    
    if (bytes_written > 0) {
        std::cout << "Sent test data: " << test_msg << " (" << bytes_written << " bytes)" << std::endl;
    } else {
        std::cout << "Failed to send test data" << std::endl;
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
    
    // Keep buffer size reasonable - limit to 2x max packet size (262 bytes)
    // This helps with finding SOF more reliably
    if (rx_buffer.size() > 512) {
        std::cout << "Buffer overflow! Removing 256 bytes and flushing UART" << std::endl;
        rx_buffer.erase(rx_buffer.begin(), rx_buffer.begin() + 256);
        // Buffer overflow suggests ESP32 restart or heavy corruption
        flushUartBuffers();
    }
}

void SerialProtocol::flushUartBuffers() {
    if (serial_fd >= 0) {
        tcflush(serial_fd, TCIOFLUSH);
        std::cout << "UART buffers flushed (ESP32 restart recovery)" << std::endl;
    }
}

void SerialProtocol::processBuffer() {
    // Aggressive garbage removal for better synchronization
    while (!rx_buffer.empty()) {
        // Find first SOF marker
        size_t sof_position = rx_buffer.size();
        for (size_t i = 0; i < rx_buffer.size(); i++) {
            if (rx_buffer[i] == PROTOCOL_SOF) {
                sof_position = i;
                break;
            }
        }
        
        // If no SOF found, buffer contains only garbage - clear it all
        // Also flush UART buffers to remove any remaining garbage from ESP32 restart
        if (sof_position == rx_buffer.size()) {
            size_t garbage_size = rx_buffer.size();
            std::cout << "No SOF found in buffer, clearing " << garbage_size << " bytes of garbage" << std::endl;
            
            // Detect potential ESP32 restart
            detectESP32Restart(garbage_size);
            
            rx_buffer.clear();
            flushUartBuffers();  // Clear system UART buffers too!
            return;
        }
        
        // Remove any garbage before SOF
        if (sof_position > 0) {
            std::cout << "Removing " << sof_position << " bytes of garbage before SOF" << std::endl;
            rx_buffer.erase(rx_buffer.begin(), rx_buffer.begin() + sof_position);
            // If we removed a lot of garbage (>100 bytes), flush UART buffers too
            // This helps recovery after ESP32 restart
            if (sof_position > 100) {
                flushUartBuffers();
            }
        }
        
        // Now SOF is at position 0
        std::cout << "Found SOF at position 0" << std::endl;
        
        // Check if we have enough data for packet header (SOF + screen_id + command + payload_length)
        if (rx_buffer.size() < 4) {
            std::cout << "Not enough data for header yet (have " << rx_buffer.size() << " bytes)" << std::endl;
            return; // Wait for more data
        }
        
        uint8_t screen_id = rx_buffer[1];
        uint8_t command = rx_buffer[2];
        uint8_t payload_length = rx_buffer[3];
        
        std::cout << "Packet header: screen_id=" << (int)screen_id 
                  << " command=" << (int)command 
                  << " payload_length=" << (int)payload_length << std::endl;
        
        // Note: payload_length is uint8_t (0-255), so max packet size is 261 bytes
        // Calculate total packet size: SOF + screen_id + command + payload_length + payload + checksum + EOF
        size_t total_packet_size = 4 + payload_length + 2; // header + payload + checksum + EOF
        
        // Check if we have complete packet
        if (rx_buffer.size() < total_packet_size) {
            std::cout << "Not enough data for complete packet (need " << total_packet_size << " bytes, have " << rx_buffer.size() << ")" << std::endl;
            return; // Wait for more data
        }
        
        // Check EOF at the calculated position
        size_t eof_position = total_packet_size - 1;
        if (rx_buffer[eof_position] != PROTOCOL_EOF) {
            std::cout << "EOF mismatch: expected 0x55, got 0x" << std::hex << (int)rx_buffer[eof_position] << std::dec 
                      << " - this SOF is false, removing it" << std::endl;
            // This SOF was false positive, remove it and continue searching
            rx_buffer.erase(rx_buffer.begin());
            continue;
        }
        
        std::cout << "Complete valid packet found, parsing..." << std::endl;
        
        // Create a temporary ProtocolPacket for parsing
        ProtocolPacket packet;
        packet.sof = rx_buffer[0];
        packet.screen_id = rx_buffer[1];
        packet.command = rx_buffer[2];
        packet.payload_length = rx_buffer[3];
        
        // Copy payload
        memcpy(packet.payload, &rx_buffer[4], payload_length);
        
        // Set checksum and EOF
        packet.checksum = rx_buffer[eof_position - 1];
        packet.eof = rx_buffer[eof_position];
        
        parsePacket(&packet);
        
        // Remove processed packet from buffer
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

    // Validate parameters
    if (cmd->x_pos >= 192 || cmd->y_pos >= 192) {
        std::cout << "parseGifCommand: position out of bounds" << std::endl;
        free(cmd);
        return nullptr;
    }

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
            
            // Aggressively flush everything
            flushUartBuffers();
            rx_buffer.clear();
        }
        
        last_garbage_time_us = current_time;
    }
}

void SerialProtocol::setDTR(bool state) {
    if (serial_fd < 0) return;
    
    int status;
    if (ioctl(serial_fd, TIOCMGET, &status) == -1) {
        std::cerr << "Failed to get serial line status" << std::endl;
        return;
    }
    
    if (state) {
        status |= TIOCM_DTR;  // Set DTR HIGH
    } else {
        status &= ~TIOCM_DTR; // Set DTR LOW
    }
    
    if (ioctl(serial_fd, TIOCMSET, &status) == -1) {
        std::cerr << "Failed to set DTR" << std::endl;
        return;
    }
    
    std::cout << "DTR set to " << (state ? "HIGH" : "LOW") << std::endl;
}

void SerialProtocol::setRTS(bool state) {
    if (serial_fd < 0) {
        std::cerr << "ERROR: serial_fd < 0, cannot set RTS" << std::endl;
        return;
    }
    
    int status;
    if (ioctl(serial_fd, TIOCMGET, &status) == -1) {
        std::cerr << "Failed to get serial line status for RTS" << std::endl;
        return;
    }
    
    std::cout << "Current modem status before RTS change: 0x" << std::hex << status << std::dec << std::endl;
    
    if (state) {
        status |= TIOCM_RTS;  // Set RTS HIGH
    } else {
        status &= ~TIOCM_RTS; // Set RTS LOW
    }
    
    if (ioctl(serial_fd, TIOCMSET, &status) == -1) {
        std::cerr << "Failed to set RTS" << std::endl;
        perror("ioctl TIOCMSET");
        return;
    }
    
    // Verify the change
    int verify_status;
    if (ioctl(serial_fd, TIOCMGET, &verify_status) == 0) {
        bool rts_actual = (verify_status & TIOCM_RTS) != 0;
        std::cout << "RTS requested: " << (state ? "HIGH" : "LOW") 
                  << ", actual: " << (rts_actual ? "HIGH" : "LOW");
        if (rts_actual != state) {
            std::cout << " ⚠️ MISMATCH!";
        }
        std::cout << " (status=0x" << std::hex << verify_status << std::dec << ")" << std::endl;
    } else {
        std::cout << "RTS set to " << (state ? "HIGH" : "LOW") << " (verification failed)" << std::endl;
    }
}

void SerialProtocol::initESP32ResetSequence() {
    std::cout << "Initializing ESP32 reset sequence (2-transistor auto-reset circuit)..." << std::endl;
    
    // 2-TRANSISTOR CIRCUIT (both DTR and RTS inverted!):
    // Confirmed by test: DTR=LOW + RTS=LOW → EN=HIGH + GPIO0=HIGH ✓
    // 
    // DTR=LOW,  RTS=LOW  -> EN=HIGH, GPIO0=HIGH (Normal Mode - running) ✓
    // DTR=LOW,  RTS=HIGH -> EN=HIGH, GPIO0=LOW  (Bootloader Mode)
    // DTR=HIGH, RTS=LOW  -> EN=LOW,  GPIO0=HIGH (Reset active)
    // DTR=HIGH, RTS=HIGH -> EN=LOW,  GPIO0=LOW  (Reset in bootloader)
    
    // Step 1: Prepare GPIO0 for normal mode BEFORE releasing reset
    std::cout << "Setting RTS=LOW (GPIO0=HIGH for normal mode)" << std::endl;
    setRTS(false);  // RTS LOW = GPIO0 HIGH
    usleep(50000);  // 50ms - ensure GPIO0 is stable
    
    // Step 2: Assert reset with GPIO0 HIGH
    std::cout << "Asserting reset: DTR=HIGH, RTS=LOW (EN=LOW, GPIO0=HIGH)" << std::endl;
    setDTR(true);   // DTR HIGH = EN LOW (ESP32 in reset)
    usleep(100000); // 100ms - hold in reset
    
    // Step 3: Release reset - ESP32 samples GPIO0=HIGH and boots into normal mode
    std::cout << "Releasing reset: DTR=LOW, RTS=LOW (EN=HIGH, GPIO0=HIGH)" << std::endl;
    setDTR(false);  // DTR LOW = EN HIGH (ESP32 boots)
    setRTS(false);  // Keep RTS LOW = GPIO0 stays HIGH
    
    // Step 4: Final state verified by test
    std::cout << "Final state: DTR=LOW, RTS=LOW → EN=HIGH, GPIO0=HIGH ✓" << std::endl;
    std::cout << "Waiting for ESP32 to boot..." << std::endl;
    usleep(1000000); // 1 second - wait for ESP32 to fully boot
    
    std::cout << "ESP32 should be running in normal mode" << std::endl;
}
