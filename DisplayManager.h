#pragma once

#include "led-matrix.h"
#include "SerialProtocol.h"
#include "BdfFont.h"
#include <Magick++.h>
#include <vector>
#include <string>
#include <memory>
#include <map>

// 8-bit color palette for performance optimization
struct Color8 {
    uint8_t r, g, b;
    Color8(uint8_t red = 0, uint8_t green = 0, uint8_t blue = 0) : r(red), g(green), b(blue) {}
};

// 8-bit color palette with 256 colors
class ColorPalette {
public:
    static const int PALETTE_SIZE = 256;
    static Color8 palette[PALETTE_SIZE];
    static bool initialized;
    
    // Lookup table for fast RGB to 8-bit conversion
    static uint8_t rgb_lookup[256][256][256];
    static bool lookup_initialized;
    
    static void initialize();
    static void initializeLookupTable();
    static uint8_t rgbTo8bit(uint8_t r, uint8_t g, uint8_t b);
    static uint8_t rgbTo8bitFast(uint8_t r, uint8_t g, uint8_t b);
    static Color8 getColor(uint8_t index);
};

struct DisplayElement {
    enum Type { GIF, TEXT };
    Type type;
    uint8_t element_id;    // Unique element ID (0-255)
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
    std::string font_name; // BDF font file name
    uint8_t font_size;
    uint8_t color_index; // 8-bit color index instead of RGB
    uint64_t scroll_offset;
    uint64_t scroll_delay_us;
    uint64_t last_scroll_time;
    
    DisplayElement() : type(GIF), element_id(0), x(0), y(0), width(0), height(0), active(false),
                      current_frame(0), last_frame_time(0), frame_delay_us(100000),
                      font_size(1), color_index(255), // White color index
                      scroll_offset(0), scroll_delay_us(1000000), last_scroll_time(0) {}
};

// Simple command cache for deduplication
struct CommandCache {
    uint32_t gif_checksums[256];  // One checksum per element_id
    uint32_t text_checksums[256];
    
    CommandCache() {
        for (int i = 0; i < 256; i++) {
            gif_checksums[i] = 0;
            text_checksums[i] = 0;
        }
    }
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
    
    // Clear only text elements (keep GIFs)
    void clearText();
    
    // Set brightness
    void setBrightness(uint8_t brightness);
    
    // Add GIF element
    bool addGifElement(const std::string& filename, uint16_t x, uint16_t y, 
                      uint16_t width, uint16_t height, uint8_t element_id);
    
    // Add text element
    bool addTextElement(const std::string& text, uint16_t x, uint16_t y,
                       uint8_t font_size, uint8_t color_index, const std::string& font_name, uint8_t element_id);
    
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
    
    // Font cache to avoid reloading fonts from disk
    std::map<std::string, BdfFont> font_cache;
    
    // Command cache for deduplication
    CommandCache command_cache;
    
    // Diagnostic display flag
    bool diagnostic_drawn;
    
    // Display dirty flag - true when redraw is needed
    bool display_dirty;
    
    // Screen bounds
    static const int SCREEN_WIDTH = 192;
    static const int SCREEN_HEIGHT = 192;
    
    // Helper functions
    void drawGifElement(const DisplayElement& element);
    void drawTextElement(const DisplayElement& element);
    void updateGifElement(DisplayElement& element);
    void updateTextElement(DisplayElement& element);
    
    // Checksum calculation for command deduplication
    uint32_t calculateGifChecksum(const GifCommand* cmd);
    uint32_t calculateTextChecksum(const TextCommand* cmd);
    
    // Bounds checking
    bool isWithinBounds(uint16_t x, uint16_t y, uint16_t width, uint16_t height);
    void clipToBounds(uint16_t& x, uint16_t& y, uint16_t& width, uint16_t& height);
    
    // Text rendering helpers
    void drawChar(char c, uint16_t x, uint16_t y, uint8_t font_size, uint8_t color_index);
    void drawString(const std::string& str, uint16_t x, uint16_t y, 
                   uint8_t font_size, uint8_t color_index);
    
    // Time utilities
    uint64_t getCurrentTimeUs();
    
    // Command processing
    void processGifCommand(GifCommand* cmd);
    void processTextCommand(TextCommand* cmd);
    void processClearCommand(ClearCommand* cmd);
    void processClearTextCommand(ClearCommand* cmd);
    void processBrightnessCommand(BrightnessCommand* cmd);
    void processStatusCommand(StatusCommand* cmd);
};
