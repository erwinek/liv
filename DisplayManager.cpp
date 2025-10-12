#include "DisplayManager.h"
#include "LedImgViewer.h"
#include <sys/time.h>
#include <algorithm>
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <cstdlib>

DisplayManager::DisplayManager(rgb_matrix::RGBMatrix* matrix) 
    : matrix(matrix), canvas(nullptr), current_brightness(100), last_update_time(0) {
}

DisplayManager::~DisplayManager() {
    serial_protocol.close();
}

bool DisplayManager::init() {
    canvas = matrix->CreateFrameCanvas();
    if (!canvas) {
        std::cerr << "Failed to create canvas" << std::endl;
        return false;
    }
    
    // Check if serial device exists
    if (access("/dev/ttyUSB0", F_OK) != 0) {
        std::cerr << "Serial device /dev/ttyUSB0 not found. Available devices:" << std::endl;
        system("ls -la /dev/tty* 2>/dev/null | grep -E '(USB|ACM)'");
        std::cerr << "Trying alternative devices..." << std::endl;
        
        // Try common alternatives
        const char* devices[] = {"/dev/ttyUSB1", "/dev/ttyACM0", "/dev/ttyACM1", "/dev/serial0", "/dev/ttyS0"};
        bool found = false;
        
        for (int i = 0; i < 5; i++) {
            if (access(devices[i], F_OK) == 0) {
                std::cout << "Found device: " << devices[i] << std::endl;
                if (serial_protocol.init(devices[i], 1000000)) {
                    std::cout << "Successfully initialized serial protocol on " << devices[i] << std::endl;
                    serial_protocol.sendTestData();
                    found = true;
                    break;
                }
            }
        }
        
        if (!found) {
            std::cerr << "Warning: No suitable serial device found" << std::endl;
        }
    } else {
        // Initialize serial protocol (default to /dev/ttyUSB0)
        if (!serial_protocol.init("/dev/ttyUSB0", 1000000)) {
            std::cerr << "Warning: Failed to initialize serial protocol" << std::endl;
            // Continue without serial - not critical for basic functionality
        } else {
            // Send test data to verify communication
            std::cout << "Sending test data to verify UART communication..." << std::endl;
            serial_protocol.sendTestData();
        }
    }
    
    // Add diagnostic display elements
    addDiagnosticElements();
    
    last_update_time = getCurrentTimeUs();
    return true;
}

void DisplayManager::processSerialCommands() {
    serial_protocol.processData();
    
    while (serial_protocol.hasPendingCommand()) {
        void* command = serial_protocol.getNextCommand();
        if (!command) continue;
        
        CommandType cmd_type = serial_protocol.getCommandType(command);
        
        switch (cmd_type) {
            case CMD_LOAD_GIF:
                processGifCommand((GifCommand*)command);
                break;
            case CMD_DISPLAY_TEXT:
                processTextCommand((TextCommand*)command);
                break;
            case CMD_CLEAR_SCREEN:
                processClearCommand((ClearCommand*)command);
                break;
            case CMD_SET_BRIGHTNESS:
                processBrightnessCommand((BrightnessCommand*)command);
                break;
            case CMD_GET_STATUS:
                processStatusCommand((StatusCommand*)command);
                break;
            default:
                break;
        }
        
        serial_protocol.freeCommand(command);
    }
}

void DisplayManager::updateDisplay() {
    uint64_t current_time = getCurrentTimeUs();
    
    // Only clear canvas if we have elements to draw
    bool has_active_elements = false;
    for (const auto& element : elements) {
        if (element.active) {
            has_active_elements = true;
            break;
        }
    }
    
    if (has_active_elements) {
        // Clear canvas only when we have elements to draw
        canvas->Clear();
    }
    
    // Update and draw all active elements
    for (auto& element : elements) {
        if (!element.active) continue;
        
        switch (element.type) {
            case DisplayElement::GIF:
                updateGifElement(element);
                drawGifElement(element);
                break;
            case DisplayElement::TEXT:
                updateTextElement(element);
                drawTextElement(element);
                break;
        }
    }
    
    // Swap canvas
    canvas = matrix->SwapOnVSync(canvas, 1);
    last_update_time = current_time;
}

void DisplayManager::clearScreen() {
    canvas->Clear();
    elements.clear();
}

void DisplayManager::setBrightness(uint8_t brightness) {
    current_brightness = std::min(brightness, (uint8_t)100);
    matrix->SetBrightness(current_brightness);
}

bool DisplayManager::addGifElement(const std::string& filename, uint16_t x, uint16_t y, 
                                  uint16_t width, uint16_t height) {
    // Check bounds
    if (!isWithinBounds(x, y, width, height)) {
        return false;
    }
    
    // Load GIF
    std::vector<Magick::Image> frames;
    std::string err_msg;
    
    if (!LoadImageAndScale(filename.c_str(), width, height, false, false, &frames, &err_msg)) {
        std::cerr << "Failed to load GIF: " << filename << " - " << err_msg << std::endl;
        return false;
    }
    
    // Create new element
    DisplayElement element;
    element.type = DisplayElement::GIF;
    element.x = x;
    element.y = y;
    element.width = width;
    element.height = height;
    element.filename = filename;
    element.gif_frames = frames;
    element.current_frame = 0;
    element.last_frame_time = getCurrentTimeUs();
    element.active = true;
    
    if (!frames.empty()) {
        element.frame_delay_us = frames[0].animationDelay() * 10000;
        if (element.frame_delay_us <= 0) element.frame_delay_us = 100000;
    }
    
    elements.push_back(element);
    return true;
}

bool DisplayManager::addTextElement(const std::string& text, uint16_t x, uint16_t y,
                                   uint8_t font_size, uint8_t r, uint8_t g, uint8_t b) {
    // Check bounds
    if (!isWithinBounds(x, y, font_size * text.length(), font_size * 8)) {
        return false;
    }
    
    // Create new element
    DisplayElement element;
    element.type = DisplayElement::TEXT;
    element.x = x;
    element.y = y;
    element.width = font_size * text.length();
    element.height = font_size * 8;
    element.text = text;
    element.font_size = font_size;
    element.color_r = r;
    element.color_g = g;
    element.color_b = b;
    element.scroll_offset = 0;
    element.last_scroll_time = getCurrentTimeUs();
    element.active = true;
    
    elements.push_back(element);
    return true;
}

void DisplayManager::removeElement(uint16_t x, uint16_t y) {
    for (auto it = elements.begin(); it != elements.end(); ++it) {
        if (it->x == x && it->y == y) {
            it->active = false;
            elements.erase(it);
            break;
        }
    }
}

std::string DisplayManager::getStatus() {
    std::string status = "Screen: 192x192, Elements: " + std::to_string(elements.size()) + 
                        ", Brightness: " + std::to_string(current_brightness);
    return status;
}

void DisplayManager::drawGifElement(const DisplayElement& element) {
    if (element.gif_frames.empty() || element.current_frame >= element.gif_frames.size()) {
        return;
    }
    
    const Magick::Image& img = element.gif_frames[element.current_frame];
    
    // Clip to screen bounds
    uint16_t draw_x = element.x;
    uint16_t draw_y = element.y;
    uint16_t draw_width = element.width;
    uint16_t draw_height = element.height;
    clipToBounds(draw_x, draw_y, draw_width, draw_height);
    
    // Draw image
    for (size_t y = 0; y < img.rows() && y < draw_height; ++y) {
        for (size_t x = 0; x < img.columns() && x < draw_width; ++x) {
            const Magick::Color& c = img.pixelColor(x, y);
            if (c.alphaQuantum() < 255) {
                canvas->SetPixel(draw_x + x, draw_y + y,
                               ScaleQuantumToChar(c.redQuantum()),
                               ScaleQuantumToChar(c.greenQuantum()),
                               ScaleQuantumToChar(c.blueQuantum()));
            }
        }
    }
}

void DisplayManager::drawTextElement(const DisplayElement& element) {
    if (element.text.empty()) return;
    
    // Simple character-based font rendering
    uint16_t x = element.x;
    uint16_t y = element.y;
    
    // Handle scrolling
    std::string display_text = element.text;
    if (element.scroll_offset > 0) {
        size_t offset = element.scroll_offset / element.font_size;
        if (offset < display_text.length()) {
            display_text = display_text.substr(offset);
        } else {
            display_text = "";
        }
    }
    
    drawString(display_text, x, y, element.font_size, 
              element.color_r, element.color_g, element.color_b);
}

void DisplayManager::updateGifElement(DisplayElement& element) {
    uint64_t current_time = getCurrentTimeUs();
    
    if (current_time - element.last_frame_time >= element.frame_delay_us) {
        element.current_frame = (element.current_frame + 1) % element.gif_frames.size();
        element.last_frame_time = current_time;
    }
}

void DisplayManager::updateTextElement(DisplayElement& element) {
    uint64_t current_time = getCurrentTimeUs();
    
    // Simple scrolling for long text
    if (element.text.length() * element.font_size > SCREEN_WIDTH - element.x) {
        if (current_time - element.last_scroll_time >= element.scroll_delay_us) {
            element.scroll_offset = (element.scroll_offset + 1) % 
                                  (element.text.length() * element.font_size);
            element.last_scroll_time = current_time;
        }
    }
}

bool DisplayManager::isWithinBounds(uint16_t x, uint16_t y, uint16_t width, uint16_t height) {
    return (x < SCREEN_WIDTH && y < SCREEN_HEIGHT && 
            x + width <= SCREEN_WIDTH && y + height <= SCREEN_HEIGHT);
}

void DisplayManager::clipToBounds(uint16_t& x, uint16_t& y, uint16_t& width, uint16_t& height) {
    if (x >= SCREEN_WIDTH || y >= SCREEN_HEIGHT) {
        width = 0;
        height = 0;
        return;
    }
    
    if (x + width > SCREEN_WIDTH) {
        width = SCREEN_WIDTH - x;
    }
    
    if (y + height > SCREEN_HEIGHT) {
        height = SCREEN_HEIGHT - y;
    }
}

void DisplayManager::drawChar(char c, uint16_t x, uint16_t y, uint8_t font_size, 
                             uint8_t r, uint8_t g, uint8_t b) {
    // Simple 5x7 character font
    static const uint8_t font[95][7] = {
        {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // space
        {0x04, 0x04, 0x04, 0x04, 0x04, 0x00, 0x04}, // !
        {0x0A, 0x0A, 0x0A, 0x00, 0x00, 0x00, 0x00}, // "
        // ... (simplified - would need full font data)
    };
    
    if (c < 32 || c > 126) c = 32; // Use space for invalid chars
    
    const uint8_t* char_data = font[c - 32];
    
    for (int row = 0; row < 7; row++) {
        for (int col = 0; col < 5; col++) {
            if (char_data[row] & (0x10 >> col)) {
                // Draw pixel scaled by font_size
                for (int sy = 0; sy < font_size; sy++) {
                    for (int sx = 0; sx < font_size; sx++) {
                        canvas->SetPixel(x + col * font_size + sx, 
                                       y + row * font_size + sy, r, g, b);
                    }
                }
            }
        }
    }
}

void DisplayManager::drawString(const std::string& str, uint16_t x, uint16_t y, 
                               uint8_t font_size, uint8_t r, uint8_t g, uint8_t b) {
    uint16_t current_x = x;
    
    for (char c : str) {
        if (current_x >= SCREEN_WIDTH) break;
        
        drawChar(c, current_x, y, font_size, r, g, b);
        current_x += font_size * 6; // 5 pixels + 1 space
    }
}

uint64_t DisplayManager::getCurrentTimeUs() {
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    return tv.tv_sec * 1000000ULL + tv.tv_usec;
}

void DisplayManager::processGifCommand(GifCommand* cmd) {
    if (!cmd) return;
    
    std::string filename(cmd->filename);
    bool success = addGifElement(filename, cmd->x_pos, cmd->y_pos, cmd->width, cmd->height);
    
    if (success) {
        serial_protocol.sendResponse(cmd->screen_id, RESP_OK);
    } else {
        serial_protocol.sendResponse(cmd->screen_id, RESP_FILE_NOT_FOUND);
    }
}

void DisplayManager::processTextCommand(TextCommand* cmd) {
    if (!cmd) return;
    
    std::string text(cmd->text, cmd->text_length);
    bool success = addTextElement(text, cmd->x_pos, cmd->y_pos, cmd->font_size, 
                                 cmd->color_r, cmd->color_g, cmd->color_b);
    
    if (success) {
        serial_protocol.sendResponse(cmd->screen_id, RESP_OK);
    } else {
        serial_protocol.sendResponse(cmd->screen_id, RESP_INVALID_PARAMS);
    }
}

void DisplayManager::processClearCommand(ClearCommand* cmd) {
    if (!cmd) return;
    
    clearScreen();
    serial_protocol.sendResponse(cmd->screen_id, RESP_OK);
}

void DisplayManager::processBrightnessCommand(BrightnessCommand* cmd) {
    if (!cmd) return;
    
    setBrightness(cmd->brightness);
    serial_protocol.sendResponse(cmd->screen_id, RESP_OK);
}

void DisplayManager::processStatusCommand(StatusCommand* cmd) {
    if (!cmd) return;
    
    std::string status = getStatus();
    serial_protocol.sendResponse(cmd->screen_id, RESP_OK, 
                               (const uint8_t*)status.c_str(), status.length());
}

void DisplayManager::addDiagnosticElements() {
    // Draw simple test pattern directly on canvas
    std::cout << "Drawing diagnostic pattern directly on canvas..." << std::endl;
    
    // Clear canvas first
    canvas->Clear();
    
    // Draw corner squares
    for (int y = 0; y < 20; y++) {
        for (int x = 0; x < 20; x++) {
            canvas->SetPixel(x, y, 255, 0, 0);        // Red top-left
            canvas->SetPixel(SCREEN_WIDTH-1-x, y, 0, 255, 0);      // Green top-right
            canvas->SetPixel(x, SCREEN_HEIGHT-1-y, 0, 0, 255);     // Blue bottom-left
            canvas->SetPixel(SCREEN_WIDTH-1-x, SCREEN_HEIGHT-1-y, 255, 255, 0); // Yellow bottom-right
        }
    }
    
    // Draw center cross
    for (int i = 0; i < 20; i++) {
        canvas->SetPixel(SCREEN_WIDTH/2 + i, SCREEN_HEIGHT/2, 255, 255, 255);  // Horizontal
        canvas->SetPixel(SCREEN_WIDTH/2, SCREEN_HEIGHT/2 + i, 255, 255, 255);  // Vertical
    }
    
    // Draw border
    for (int i = 0; i < SCREEN_WIDTH; i++) {
        canvas->SetPixel(i, 0, 255, 255, 255);           // Top border
        canvas->SetPixel(i, SCREEN_HEIGHT-1, 255, 255, 255); // Bottom border
    }
    for (int i = 0; i < SCREEN_HEIGHT; i++) {
        canvas->SetPixel(0, i, 255, 255, 255);           // Left border
        canvas->SetPixel(SCREEN_WIDTH-1, i, 255, 255, 255); // Right border
    }
    
    // Force immediate display
    canvas = matrix->SwapOnVSync(canvas, 1);
    
    std::cout << "Diagnostic pattern drawn - you should see colored squares in corners and white border" << std::endl;
}
