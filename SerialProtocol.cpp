#include "SerialProtocol.h"
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <cstring>
#include <cstdlib>
#include <iostream>

SerialProtocol::SerialProtocol() : serial_fd(-1) {
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
    tty.c_cflag |= CREAD | CLOCAL; // Enable reading and ignore modem control lines
    
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
    
    std::cout << "Serial protocol initialized on " << device << " at " << baudrate << " bps" << std::endl;
    return true;
}

void SerialProtocol::processData() {
    uint8_t byte;
    ssize_t bytes_read = read(serial_fd, &byte, 1);
    
    while (bytes_read > 0) {
        addToBuffer(byte);
        bytes_read = read(serial_fd, &byte, 1);
    }
    
    processBuffer();
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
    packet.payload_length = sizeof(Response) - sizeof(ProtocolPacket) + data_len;
    packet.eof = PROTOCOL_EOF;
    
    memcpy(packet.payload, &response, sizeof(Response));
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

uint8_t SerialProtocol::calculateChecksum(const uint8_t* data, uint8_t length) {
    uint8_t checksum = 0;
    for (uint8_t i = 0; i < length; i++) {
        checksum ^= data[i];
    }
    return checksum;
}

bool SerialProtocol::validatePacket(const ProtocolPacket* packet) {
    if (!packet) return false;
    
    // Check SOF and EOF
    if (packet->sof != PROTOCOL_SOF || packet->eof != PROTOCOL_EOF) {
        return false;
    }
    
    // Check payload length
    if (packet->payload_length > PROTOCOL_MAX_PAYLOAD) {
        return false;
    }
    
    // Verify checksum
    uint8_t calculated_checksum = calculateChecksum(packet->payload, packet->payload_length);
    if (packet->checksum != calculated_checksum) {
        return false;
    }
    
    return true;
}

void SerialProtocol::parsePacket(const ProtocolPacket* packet) {
    if (!validatePacket(packet)) {
        sendResponse(packet->screen_id, RESP_PROTOCOL_ERROR);
        return;
    }
    
    void* command = nullptr;
    
    switch (packet->command) {
        case CMD_LOAD_GIF:
            command = parseGifCommand(packet->payload, packet->payload_length);
            break;
        case CMD_DISPLAY_TEXT:
            command = parseTextCommand(packet->payload, packet->payload_length);
            break;
        case CMD_CLEAR_SCREEN:
            command = parseClearCommand(packet->payload, packet->payload_length);
            break;
        case CMD_SET_BRIGHTNESS:
            command = parseBrightnessCommand(packet->payload, packet->payload_length);
            break;
        case CMD_GET_STATUS:
            command = parseStatusCommand(packet->payload, packet->payload_length);
            break;
        default:
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
    
    // Keep buffer size reasonable
    if (rx_buffer.size() > 1024) {
        rx_buffer.erase(rx_buffer.begin(), rx_buffer.begin() + 512);
    }
}

void SerialProtocol::processBuffer() {
    // Look for complete packets
    for (size_t i = 0; i < rx_buffer.size(); i++) {
        if (rx_buffer[i] == PROTOCOL_SOF) {
            // Check if we have enough data for a complete packet
            if (i + sizeof(ProtocolPacket) <= rx_buffer.size()) {
                ProtocolPacket* packet = (ProtocolPacket*)(&rx_buffer[i]);
                
                // Verify EOF is at the expected position
                if (packet->eof == PROTOCOL_EOF) {
                    parsePacket(packet);
                    
                    // Remove processed packet from buffer
                    rx_buffer.erase(rx_buffer.begin(), rx_buffer.begin() + sizeof(ProtocolPacket));
                    i = 0; // Restart search
                }
            }
        }
    }
}

void* SerialProtocol::parseGifCommand(const uint8_t* payload, uint8_t length) {
    if (length < sizeof(GifCommand)) {
        return nullptr;
    }
    
    GifCommand* cmd = (GifCommand*)malloc(sizeof(GifCommand));
    if (!cmd) return nullptr;
    
    memcpy(cmd, payload, sizeof(GifCommand));
    
    // Validate parameters
    if (cmd->x_pos >= 192 || cmd->y_pos >= 192) {
        free(cmd);
        return nullptr;
    }
    
    return cmd;
}

void* SerialProtocol::parseTextCommand(const uint8_t* payload, uint8_t length) {
    if (length < sizeof(TextCommand)) {
        return nullptr;
    }
    
    TextCommand* cmd = (TextCommand*)malloc(sizeof(TextCommand));
    if (!cmd) return nullptr;
    
    memcpy(cmd, payload, sizeof(TextCommand));
    
    // Validate parameters
    if (cmd->x_pos >= 192 || cmd->y_pos >= 192 || 
        cmd->font_size == 0 || cmd->font_size > 8 ||
        cmd->text_length > PROTOCOL_MAX_TEXT_LENGTH) {
        free(cmd);
        return nullptr;
    }
    
    return cmd;
}

void* SerialProtocol::parseClearCommand(const uint8_t* payload, uint8_t length) {
    if (length < sizeof(ClearCommand)) {
        return nullptr;
    }
    
    ClearCommand* cmd = (ClearCommand*)malloc(sizeof(ClearCommand));
    if (!cmd) return nullptr;
    
    memcpy(cmd, payload, sizeof(ClearCommand));
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
