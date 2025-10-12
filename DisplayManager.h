#pragma once

#include "led-matrix.h"
#include "SerialProtocol.h"
#include "BdfFont.h"
#include <Magick++.h>
#include <vector>
#include <string>
#include <memory>

struct DisplayElement {
    enum Type { GIF, TEXT };
    Type type;
    uint16_t x, y, width, height;
    bool active;
    
    // For GIF elements
    std::vector<Magick::Image> gif_frames;
    std::string filename;
    size_t current_frame;
    uint64_t last_frame_time;
    uint64_t frame_delay_us;
    
    // For text elements
    std::string text;
    uint8_t font_size;
    uint8_t color_r, color_g, color_b;
    uint64_t scroll_offset;
    uint64_t scroll_delay_us;
    uint64_t last_scroll_time;
    
    DisplayElement() : type(GIF), x(0), y(0), width(0), height(0), active(false),
                      current_frame(0), last_frame_time(0), frame_delay_us(100000),
                      font_size(1), color_r(255), color_g(255), color_b(255),
                      scroll_offset(0), scroll_delay_us(1000000), last_scroll_time(0) {}
};

class DisplayManager {
public:
    DisplayManager(rgb_matrix::RGBMatrix* matrix);
    ~DisplayManager();
    
    // Initialize display manager
    bool init();
    
    // Process serial commands
    void processSerialCommands();
    
    // Update display (call this in main loop)
    void updateDisplay();
    
    // Clear entire screen
    void clearScreen();
    
    // Set brightness
    void setBrightness(uint8_t brightness);
    
    // Add GIF element
    bool addGifElement(const std::string& filename, uint16_t x, uint16_t y, 
                      uint16_t width, uint16_t height);
    
    // Add text element
    bool addTextElement(const std::string& text, uint16_t x, uint16_t y,
                       uint8_t font_size, uint8_t r, uint8_t g, uint8_t b);
    
    // Remove element at position
    void removeElement(uint16_t x, uint16_t y);
    
    // Get status information
    std::string getStatus();
    
    // Add diagnostic elements for testing
    void addDiagnosticElements();

private:
    rgb_matrix::RGBMatrix* matrix;
    rgb_matrix::FrameCanvas* canvas;
    SerialProtocol serial_protocol;
    std::vector<DisplayElement> elements;
    uint8_t current_brightness;
    uint64_t last_update_time;
    
    // BDF Font
    BdfFont bdf_font;
    
    // Screen bounds
    static const int SCREEN_WIDTH = 192;
    static const int SCREEN_HEIGHT = 192;
    
    // Helper functions
    void drawGifElement(const DisplayElement& element);
    void drawTextElement(const DisplayElement& element);
    void updateGifElement(DisplayElement& element);
    void updateTextElement(DisplayElement& element);
    
    // Bounds checking
    bool isWithinBounds(uint16_t x, uint16_t y, uint16_t width, uint16_t height);
    void clipToBounds(uint16_t& x, uint16_t& y, uint16_t& width, uint16_t& height);
    
    // Text rendering helpers
    void drawChar(char c, uint16_t x, uint16_t y, uint8_t font_size, 
                 uint8_t r, uint8_t g, uint8_t b);
    void drawString(const std::string& str, uint16_t x, uint16_t y, 
                   uint8_t font_size, uint8_t r, uint8_t g, uint8_t b);
    
    // Time utilities
    uint64_t getCurrentTimeUs();
    
    // Command processing
    void processGifCommand(GifCommand* cmd);
    void processTextCommand(TextCommand* cmd);
    void processClearCommand(ClearCommand* cmd);
    void processBrightnessCommand(BrightnessCommand* cmd);
    void processStatusCommand(StatusCommand* cmd);
};
