#pragma once

#include <stdint.h>
#include <string>
#include <vector>

// Protocol constants
#define PROTOCOL_SOF 0xAA  // Start of Frame
#define PROTOCOL_EOF 0x55  // End of Frame
#define PROTOCOL_MAX_PAYLOAD 256
#define PROTOCOL_MAX_FILENAME 64
#define PROTOCOL_MAX_TEXT_LINES 10
#define PROTOCOL_MAX_TEXT_LENGTH 32

// Command types
typedef enum {
    CMD_LOAD_GIF = 0x01,
    CMD_DISPLAY_TEXT = 0x02,
    CMD_CLEAR_SCREEN = 0x03,
    CMD_SET_BRIGHTNESS = 0x04,
    CMD_GET_STATUS = 0x05,
    CMD_RESPONSE = 0x80
} CommandType;

// Response codes
typedef enum {
    RESP_OK = 0x00,
    RESP_ERROR = 0x01,
    RESP_FILE_NOT_FOUND = 0x02,
    RESP_INVALID_PARAMS = 0x03,
    RESP_PROTOCOL_ERROR = 0x04
} ResponseCode;

// GIF display command structure
typedef struct {
    uint8_t screen_id;
    uint8_t command;
    uint16_t x_pos;        // Left position
    uint16_t y_pos;        // Top position  
    uint16_t width;        // Display width
    uint16_t height;       // Display height
    char filename[PROTOCOL_MAX_FILENAME];
} __attribute__((packed)) GifCommand;

// Text display command structure
typedef struct {
    uint8_t screen_id;
    uint8_t command;
    uint16_t x_pos;        // Left position
    uint16_t y_pos;        // Top position
    uint8_t font_size;     // Font size (1-8)
    uint8_t color_r;       // Red component
    uint8_t color_g;       // Green component
    uint8_t color_b;       // Blue component
    uint8_t text_length;   // Length of text
    char text[PROTOCOL_MAX_TEXT_LENGTH];
} __attribute__((packed)) TextCommand;

// Clear screen command structure
typedef struct {
    uint8_t screen_id;
    uint8_t command;
} __attribute__((packed)) ClearCommand;

// Brightness command structure
typedef struct {
    uint8_t screen_id;
    uint8_t command;
    uint8_t brightness;    // 0-100
} __attribute__((packed)) BrightnessCommand;

// Status request structure
typedef struct {
    uint8_t screen_id;
    uint8_t command;
} __attribute__((packed)) StatusCommand;

// Response structure
typedef struct {
    uint8_t screen_id;
    uint8_t command;
    uint8_t response_code;
    uint8_t data_length;
    uint8_t data[PROTOCOL_MAX_PAYLOAD];
} __attribute__((packed)) Response;

// Protocol packet structure
typedef struct {
    uint8_t sof;
    uint8_t screen_id;
    uint8_t command;
    uint8_t payload_length;
    uint8_t payload[PROTOCOL_MAX_PAYLOAD];
    uint8_t checksum;
    uint8_t eof;
} __attribute__((packed)) ProtocolPacket;

class SerialProtocol {
public:
    SerialProtocol();
    ~SerialProtocol();
    
    // Initialize serial communication
    bool init(const char* device, int baudrate = 1000000);
    
    // Process incoming data
    void processData();
    
    // Send response
    void sendResponse(uint8_t screen_id, ResponseCode code, const uint8_t* data = nullptr, uint8_t data_len = 0);
    
    // Check if there are pending commands
    bool hasPendingCommand();
    
    // Get next command (caller must free the memory)
    void* getNextCommand();
    
    // Get command type
    CommandType getCommandType(void* command);
    
    // Cleanup command memory
    void freeCommand(void* command);
    
    // Close serial connection
    void close();

private:
    int serial_fd;
    std::vector<uint8_t> rx_buffer;
    std::vector<void*> pending_commands;
    
    // Protocol functions
    uint8_t calculateChecksum(const uint8_t* data, uint8_t length);
    bool validatePacket(const ProtocolPacket* packet);
    void parsePacket(const ProtocolPacket* packet);
    void addToBuffer(uint8_t byte);
    void processBuffer();
    
    // Command parsing
    void* parseGifCommand(const uint8_t* payload, uint8_t length);
    void* parseTextCommand(const uint8_t* payload, uint8_t length);
    void* parseClearCommand(const uint8_t* payload, uint8_t length);
    void* parseBrightnessCommand(const uint8_t* payload, uint8_t length);
    void* parseStatusCommand(const uint8_t* payload, uint8_t length);
};
