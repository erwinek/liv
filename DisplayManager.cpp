#include "DisplayManager.h"
#include "LedImgViewer.h"
#include <sys/time.h>
#include <algorithm>
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <cstdlib>
#include <climits>

// Initialize color palette
Color8 ColorPalette::palette[PALETTE_SIZE];
bool ColorPalette::initialized = false;
uint8_t ColorPalette::rgb_lookup[256][256][256];
bool ColorPalette::lookup_initialized = false;

void ColorPalette::initialize() {
    if (initialized) return;
    
    // Create a 256-color palette with good coverage
    int index = 0;
    
    // Basic colors (16 colors)
    palette[index++] = Color8(0, 0, 0);       // Black
    palette[index++] = Color8(255, 255, 255); // White
    palette[index++] = Color8(255, 0, 0);     // Red
    palette[index++] = Color8(0, 255, 0);     // Green
    palette[index++] = Color8(0, 0, 255);     // Blue
    palette[index++] = Color8(255, 255, 0);   // Yellow
    palette[index++] = Color8(255, 0, 255);   // Magenta
    palette[index++] = Color8(0, 255, 255);   // Cyan
    palette[index++] = Color8(128, 128, 128); // Gray
    palette[index++] = Color8(192, 192, 192); // Light Gray
    palette[index++] = Color8(64, 64, 64);    // Dark Gray
    palette[index++] = Color8(255, 128, 0);   // Orange
    palette[index++] = Color8(128, 0, 128);   // Purple
    palette[index++] = Color8(0, 128, 0);     // Dark Green
    palette[index++] = Color8(0, 0, 128);     // Dark Blue
    palette[index++] = Color8(128, 128, 0);   // Olive
    
    // Generate RGB cube (6x6x6 = 216 colors)
    for (int r = 0; r < 6; r++) {
        for (int g = 0; g < 6; g++) {
            for (int b = 0; b < 6; b++) {
                if (index < PALETTE_SIZE) {
                    palette[index++] = Color8(
                        (r * 255) / 5,
                        (g * 255) / 5,
                        (b * 255) / 5
                    );
                }
            }
        }
    }
    
    // Fill remaining slots with grayscale
    while (index < PALETTE_SIZE) {
        int gray = (index * 255) / (PALETTE_SIZE - 1);
        palette[index++] = Color8(gray, gray, gray);
    }
    
    initialized = true;
    initializeLookupTable();
}

void ColorPalette::initializeLookupTable() {
    if (lookup_initialized) return;
    
    std::cout << "Initializing RGB lookup table for fast color conversion..." << std::endl;
    
    // Pre-calculate lookup table for all possible RGB combinations
    for (int r = 0; r < 256; r += 4) {  // Sample every 4th value for memory efficiency
        for (int g = 0; g < 256; g += 4) {
            for (int b = 0; b < 256; b += 4) {
                uint8_t color_index = rgbTo8bit(r, g, b);
                
                // Fill 4x4x4 cube around this point
                for (int dr = 0; dr < 4 && r + dr < 256; dr++) {
                    for (int dg = 0; dg < 4 && g + dg < 256; dg++) {
                        for (int db = 0; db < 4 && b + db < 256; db++) {
                            rgb_lookup[r + dr][g + dg][b + db] = color_index;
                        }
                    }
                }
            }
        }
    }
    
    lookup_initialized = true;
    std::cout << "RGB lookup table initialized" << std::endl;
}

uint8_t ColorPalette::rgbTo8bit(uint8_t r, uint8_t g, uint8_t b) {
    if (!initialized) initialize();
    
    // Find closest color in palette
    int best_index = 0;
    int best_distance = INT_MAX;
    
    for (int i = 0; i < PALETTE_SIZE; i++) {
        int dr = r - palette[i].r;
        int dg = g - palette[i].g;
        int db = b - palette[i].b;
        int distance = dr*dr + dg*dg + db*db;
        
        if (distance < best_distance) {
            best_distance = distance;
            best_index = i;
        }
    }
    
    return best_index;
}

uint8_t ColorPalette::rgbTo8bitFast(uint8_t r, uint8_t g, uint8_t b) {
    if (!lookup_initialized) initializeLookupTable();
    return rgb_lookup[r][g][b];
}

Color8 ColorPalette::getColor(uint8_t index) {
    if (!initialized) initialize();
    if (index >= PALETTE_SIZE) index = 0;
    return palette[index];
}

DisplayManager::DisplayManager(rgb_matrix::RGBMatrix* matrix, bool swap_dimensions, uint8_t screen_id) 
    : matrix(matrix), canvas(nullptr), current_brightness(100), my_screen_id(screen_id), last_update_time(0), diagnostic_drawn(false), display_dirty(true) {
    // Initialize color palette
    ColorPalette::initialize();
    
    // Set screen dimensions based on matrix size
    // V-mapper swaps dimensions in the library, so matrix reports swapped values already
    // We need to use them directly (width becomes height, height becomes width from our perspective)
    SCREEN_WIDTH = matrix->width();
    SCREEN_HEIGHT = matrix->height();
    
    if (swap_dimensions) {
        std::cout << "DisplayManager initialized for " << SCREEN_WIDTH << "x" << SCREEN_HEIGHT 
                  << " screen (V-mapper active - library reports: width=" << matrix->width() 
                  << ", height=" << matrix->height() << ")" << std::endl;
    } else {
        std::cout << "DisplayManager initialized for " << SCREEN_WIDTH << "x" << SCREEN_HEIGHT << " screen" << std::endl;
    }
    
    // Load BDF font
    if (!bdf_font.loadFromFile("fonts/5x7.bdf")) {
        std::cerr << "Failed to load BDF font, using fallback" << std::endl;
    }
}

DisplayManager::~DisplayManager() {
    serial_protocol.close();
}

bool DisplayManager::init(const std::string& serial_port) {
    canvas = matrix->CreateFrameCanvas();
    if (!canvas) {
        std::cerr << "Failed to create canvas" << std::endl;
        return false;
    }
    
    // Initialize serial protocol with specified port
    if (!serial_protocol.init(serial_port.c_str())) {
        std::cerr << "Warning: Failed to initialize serial protocol on " << serial_port << std::endl;
        std::cerr << "Make sure ESP32 is connected and you have permission to access the port" << std::endl;
        // Continue without protocol - not critical for basic functionality
    } else {
        // Send test data to verify communication
        std::cout << "Sending test data to verify serial communication..." << std::endl;
        serial_protocol.sendTestData();
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
            case CMD_CLEAR_TEXT:
                processClearTextCommand((ClearCommand*)command); // Same structure as ClearCommand
                break;
            case CMD_DELETE_ELEMENT:
                processDeleteElementCommand((DeleteElementCommand*)command);
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
    bool has_animated_content = false;
    
    for (const auto& element : elements) {
        if (element.active) {
            has_active_elements = true;
            // Check if this is animated content (GIF, scrolling text, or blinking text)
            if (element.type == DisplayElement::GIF || 
                (element.type == DisplayElement::TEXT && static_cast<int>(element.text.length() * element.font_size) > SCREEN_WIDTH - element.x) ||
                (element.type == DisplayElement::TEXT && element.blink_interval_ms > 0)) {
                has_animated_content = true;
            }
        }
    }
    
    // Only redraw if display is dirty or has animated content
    if (!display_dirty && !has_animated_content) {
        return; // Skip rendering for static content that hasn't changed
    }
    
    if (has_active_elements) {
        // Clear canvas only when we have elements to draw
        canvas->Clear();
        diagnostic_drawn = false; // Reset flag when we have elements
    } else {
        // If no elements, draw diagnostic pattern only once
        if (!diagnostic_drawn) {
            addDiagnosticElements();
            diagnostic_drawn = true;
        }
        return; // Don't process elements since there are none
    }
    
    // Update and draw all active elements
    // Draw in two passes: GIFs first, then TEXT on top
    
    // Pass 1: Draw GIF elements (background layer)
    for (auto& element : elements) {
        if (!element.active) continue;
        
        if (element.type == DisplayElement::GIF) {
            updateGifElement(element);
            drawGifElement(element);
        }
    }
    
    // Pass 2: Draw TEXT elements (foreground layer - always on top)
    for (auto& element : elements) {
        if (!element.active) continue;
        
        if (element.type == DisplayElement::TEXT) {
            updateTextElement(element);
            drawTextElement(element);
        }
    }
    
    // Clear dirty flag after rendering
    display_dirty = false;
    
    // Swap canvas only when we actually rendered something
    canvas = matrix->SwapOnVSync(canvas, 1);
    last_update_time = current_time;
}

void DisplayManager::clearScreen() {
    canvas->Clear();
    elements.clear();
    
    // Clear command cache since all elements are removed
    for (int i = 0; i < 256; i++) {
        command_cache.gif_checksums[i] = 0;
        command_cache.text_checksums[i] = 0;
    }
    
    diagnostic_drawn = false; // Reset flag when clearing screen
    display_dirty = true; // Mark display as needing update
    std::cout << "Screen cleared, cache reset" << std::endl;
}

void DisplayManager::clearText() {
    // Remove only TEXT elements, keep GIF elements
    size_t before_count = elements.size();
    
    auto it = elements.begin();
    while (it != elements.end()) {
        if (it->type == DisplayElement::TEXT) {
            // Clear cache for this text element
            command_cache.text_checksums[it->element_id] = 0;
            it = elements.erase(it);
        } else {
            ++it;
        }
    }
    
    size_t after_count = elements.size();
    std::cout << "clearText: removed " << (before_count - after_count) 
              << " text elements, " << after_count << " elements remaining, cache updated" << std::endl;
    
    display_dirty = true; // Mark display as needing update
}

void DisplayManager::setBrightness(uint8_t brightness) {
    current_brightness = std::min(brightness, (uint8_t)100);
    matrix->SetBrightness(current_brightness);
}

bool DisplayManager::addGifElement(const std::string& filename, uint16_t x, uint16_t y, 
                                  uint16_t width, uint16_t height, uint8_t element_id) {
    std::cout << "addGifElement called: ID=" << (int)element_id << " " << filename << " at (" << x << "," << y << ") size " << width << "x" << height << std::endl;
    
    // Check if element with same ID already exists - if so, remove it first
    auto it = elements.begin();
    while (it != elements.end()) {
        if (it->element_id == element_id) {
            std::cout << "Removing duplicate element with ID=" << (int)element_id << std::endl;
            // Note: we don't clear cache here because the new element will update it
            it = elements.erase(it);
        } else {
            ++it;
        }
    }
    
    // Check bounds
    if (!isWithinBounds(x, y, width, height)) {
        std::cout << "Bounds check failed" << std::endl;
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
    element.element_id = element_id;
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
    display_dirty = true; // Mark display as needing update
    std::cout << "GIF element added successfully. Total elements: " << elements.size() << std::endl;
    return true;
}

bool DisplayManager::addTextElement(const std::string& text, uint16_t x, uint16_t y,
                                   uint8_t font_size, uint8_t color_index, const std::string& font_name, 
                                   uint8_t element_id, uint16_t blink_interval_ms) {
    // Check if element with same ID already exists
    for (auto& element : elements) {
        if (element.element_id == element_id) {
            // Update existing element text without recreating it (prevents flicker)
            element.text = text;
            element.x = x;
            element.y = y;
            element.color_index = color_index;
            element.font_name = font_name;
            element.width = font_size * text.length();
            element.height = font_size * 8;
            element.blink_interval_ms = blink_interval_ms;
            element.blink_visible = true;
            element.last_blink_time = getCurrentTimeUs();
            display_dirty = true;
            std::cout << "Element ID=" << (int)element_id << " updated: '" << text << "'"
                      << " blink=" << blink_interval_ms << "ms" << std::endl;
            return true;
        }
    }
    
    // No existing element found, create new one
    // Check bounds
    if (!isWithinBounds(x, y, font_size * text.length(), font_size * 8)) {
        std::cout << "Text bounds check failed" << std::endl;
        return false;
    }
    
    // Create new element
    DisplayElement element;
    element.type = DisplayElement::TEXT;
    element.element_id = element_id;
    element.x = x;
    element.y = y;
    element.width = font_size * text.length();
    element.height = font_size * 8;
    element.text = text;
    element.font_name = font_name;
    element.font_size = font_size;
    element.color_index = color_index;
    element.scroll_offset = 0;
    element.last_scroll_time = getCurrentTimeUs();
    element.blink_interval_ms = blink_interval_ms;
    element.blink_visible = true;
    element.last_blink_time = getCurrentTimeUs();
    element.active = true;
    
    elements.push_back(element);
    std::cout << "Element ID=" << (int)element_id << " added. Total elements: " << elements.size()
              << " blink=" << blink_interval_ms << "ms" << std::endl;
    display_dirty = true; // Mark display as needing update
    return true;
}

void DisplayManager::removeElement(uint16_t x, uint16_t y) {
    for (auto it = elements.begin(); it != elements.end(); ++it) {
        if (it->x == x && it->y == y) {
            // Clear cache for this element
            if (it->type == DisplayElement::GIF) {
                command_cache.gif_checksums[it->element_id] = 0;
            } else if (it->type == DisplayElement::TEXT) {
                command_cache.text_checksums[it->element_id] = 0;
            }
            
            it->active = false;
            elements.erase(it);
            std::cout << "Element removed at (" << x << "," << y << "), cache cleared" << std::endl;
            break;
        }
    }
}

std::string DisplayManager::getStatus() {
    std::string status = "Screen: " + std::to_string(SCREEN_WIDTH) + "x" + std::to_string(SCREEN_HEIGHT) + 
                        ", Elements: " + std::to_string(elements.size()) + 
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
                // Convert 24-bit RGB to 8-bit color index using fast lookup
                uint8_t r = ScaleQuantumToChar(c.redQuantum());
                uint8_t g = ScaleQuantumToChar(c.greenQuantum());
                uint8_t b = ScaleQuantumToChar(c.blueQuantum());
                uint8_t color_index = ColorPalette::rgbTo8bitFast(r, g, b);
                Color8 color = ColorPalette::getColor(color_index);
                
                canvas->SetPixel(draw_x + x, draw_y + y, color.r, color.g, color.b);
            }
        }
    }
}

void DisplayManager::drawTextElement(const DisplayElement& element) {
    if (element.text.empty()) return;
    
    // Check blink visibility - if blinking is enabled and text is hidden, don't draw
    if (element.blink_interval_ms > 0 && !element.blink_visible) {
        return; // Text is currently hidden due to blinking
    }
    
    // Load font if specified and different from current
    if (!element.font_name.empty() && element.font_name != "fonts/5x7.bdf") {
        // Check if font is already in cache
        BdfFont* font_to_use = nullptr;
        
        auto it = font_cache.find(element.font_name);
        if (it != font_cache.end()) {
            // Font found in cache, use it
            font_to_use = &(it->second);
        } else {
            // Font not in cache, load it and cache it
            BdfFont new_font;
            if (new_font.loadFromFile(element.font_name)) {
                font_cache[element.font_name] = std::move(new_font);
                font_to_use = &font_cache[element.font_name];
                std::cout << "Font " << element.font_name << " loaded and cached" << std::endl;
            }
        }
        
        if (font_to_use) {
            // Use cached font for rendering
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
            
            // Draw with custom font - native size (no scaling)
            // Calculate baseline position for proper vertical alignment
            int baseline_y = y + font_to_use->getFontAscent();
            
            uint16_t current_x = x;
            for (char c : display_text) {
                if (current_x >= SCREEN_WIDTH) break;
                
                const BdfChar* bdf_char = font_to_use->getChar(static_cast<uint32_t>(c));
                if (bdf_char) {
                    // Draw character using cached font at native size
                    // In BDF: y_offset is distance from baseline to character's bottom edge
                    // Characters are drawn from top (row=0) to bottom (row=height-1)
                    int bytes_per_row = (bdf_char->width + 7) / 8;
                    for (int row = 0; row < bdf_char->height; row++) {
                        for (int col = 0; col < bdf_char->width; col++) {
                            int byte_index = row * bytes_per_row + (col / 8);
                            int bit_index = 7 - (col % 8);
                            
                            if (byte_index < (int)bdf_char->bitmap.size()) {
                                uint8_t byte_val = bdf_char->bitmap[byte_index];
                                if (byte_val & (1 << bit_index)) {
                                    Color8 color = ColorPalette::getColor(element.color_index);
                                    // Apply x_offset horizontally
                                    int px = current_x + col + bdf_char->x_offset;
                                    // Apply baseline-relative positioning:
                                    // In BDF: y_offset is offset from baseline to bottom-left corner
                                    // bottom = baseline_y - y_offset (screen coords, Y grows down)
                                    // top = bottom - height
                                    // For row r (0=top): py = baseline_y - y_offset - height + row
                                    int py = baseline_y - bdf_char->y_offset - bdf_char->height + row;
                                    
                                    if (px >= 0 && px < SCREEN_WIDTH && py >= 0 && py < SCREEN_HEIGHT) {
                                        canvas->SetPixel(px, py, color.r, color.g, color.b);
                                    }
                                }
                            }
                        }
                    }
                    // Move to next character position using DWIDTH (advancement width)
                    current_x += bdf_char->dwidth;
                }
            }
            return;
        }
    }
    
    // Fallback to default font
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
    
    drawString(display_text, x, y, element.font_size, element.color_index);
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
    
    // Handle text blinking
    if (element.blink_interval_ms > 0) {
        uint64_t blink_interval_us = element.blink_interval_ms * 1000;  // Convert ms to us
        if (current_time - element.last_blink_time >= blink_interval_us) {
            element.blink_visible = !element.blink_visible;  // Toggle visibility
            element.last_blink_time = current_time;
            // Mark display as dirty to force redraw
            display_dirty = true;
        }
    }
    
    // Simple scrolling for long text
    if (static_cast<int>(element.text.length() * element.font_size) > SCREEN_WIDTH - element.x) {
        if (current_time - element.last_scroll_time >= element.scroll_delay_us) {
            element.scroll_offset = (element.scroll_offset + 1) % 
                                  (element.text.length() * element.font_size);
            element.last_scroll_time = current_time;
        }
    }
}

bool DisplayManager::isWithinBounds(uint16_t x, uint16_t y, uint16_t width, uint16_t height) {
    bool result = (x < SCREEN_WIDTH && y < SCREEN_HEIGHT && 
                   x + width <= SCREEN_WIDTH && y + height <= SCREEN_HEIGHT);
    
    if (!result) {
        std::cout << "Bounds check FAILED: x=" << x << " y=" << y << " w=" << width << " h=" << height 
                  << " (Screen: " << SCREEN_WIDTH << "x" << SCREEN_HEIGHT << ")" << std::endl;
        std::cout << "  x < SCREEN_WIDTH: " << (x < SCREEN_WIDTH) << std::endl;
        std::cout << "  y < SCREEN_HEIGHT: " << (y < SCREEN_HEIGHT) << std::endl;
        std::cout << "  x+w <= SCREEN_WIDTH: " << (x + width) << " <= " << SCREEN_WIDTH << " = " << (x + width <= SCREEN_WIDTH) << std::endl;
        std::cout << "  y+h <= SCREEN_HEIGHT: " << (y + height) << " <= " << SCREEN_HEIGHT << " = " << (y + height <= SCREEN_HEIGHT) << std::endl;
    }
    
    return result;
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
                             uint8_t color_index) {
    if (!canvas) return;
    
    // Get character from BDF font
    const BdfChar* bdf_char = bdf_font.getChar(static_cast<uint32_t>(c));
    if (!bdf_char) {
        std::cout << "drawChar: No BDF char found for '" << c << "' (ASCII " << (int)c << ")" << std::endl;
        // Fallback: draw a simple rectangle for unknown characters
        Color8 color = ColorPalette::getColor(color_index);
        for (int sy = 0; sy < font_size * 7; sy++) {
            for (int sx = 0; sx < font_size * 5; sx++) {
                canvas->SetPixel(x + sx, y + sy, color.r, color.g, color.b);
            }
        }
        return;
    }

    
    
    // Debug prints removed for performance
    int bytes_per_row = (bdf_char->width + 7) / 8;
    
    int pixels_drawn = 0;
    
    // Draw character scaled by font_size
    for (int row = 0; row < bdf_char->height; row++) {
        for (int col = 0; col < bdf_char->width; col++) {
            // Get bit from bitmap
            int byte_index = row * bytes_per_row + col / 8;
            int bit_index = 7 - (col % 8); // BDF uses MSB first
            
            if (byte_index < static_cast<int>(bdf_char->bitmap.size())) {
                uint8_t byte_val = bdf_char->bitmap[byte_index];
                if (byte_val & (1 << bit_index)) {
                    // Draw pixel scaled by font_size
                    for (int sy = 0; sy < font_size; sy++) {
                        for (int sx = 0; sx < font_size; sx++) {
                            int pixel_x = x + (col + bdf_char->x_offset) * font_size + sx;
                            int pixel_y = y + (row + bdf_char->y_offset) * font_size + sy;
                            
                            // Check bounds
                            if (pixel_x >= 0 && pixel_x < SCREEN_WIDTH && 
                                pixel_y >= 0 && pixel_y < SCREEN_HEIGHT) {
                                Color8 color = ColorPalette::getColor(color_index);
                                canvas->SetPixel(pixel_x, pixel_y, color.r, color.g, color.b);
                                pixels_drawn++;
                            }
                        }
                    }
                }
            }
        }
    }
    
    // Debug print removed for performance
}

void DisplayManager::drawString(const std::string& str, uint16_t x, uint16_t y, 
                               uint8_t font_size, uint8_t color_index) {
    uint16_t current_x = x;
    
    // Debug print removed for performance
    
    for (char c : str) {
        if (current_x >= SCREEN_WIDTH) break;
        
        drawChar(c, current_x, y, font_size, color_index);
        
        // Get character width from BDF font
        const BdfChar* bdf_char = bdf_font.getChar(static_cast<uint32_t>(c));
        if (bdf_char) {
            current_x += bdf_char->dwidth * font_size; // use DWIDTH for proper spacing
        } else {
            current_x += font_size * 6; // fallback spacing
        }
    }
}

uint64_t DisplayManager::getCurrentTimeUs() {
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    return tv.tv_sec * 1000000ULL + tv.tv_usec;
}

uint32_t DisplayManager::calculateGifChecksum(const GifCommand* cmd) {
    if (!cmd) return 0;
    
    // Calculate simple checksum of relevant parameters (excluding screen_id and command)
    uint32_t checksum = 0;
    
    // Add element_id
    checksum += cmd->element_id;
    
    // Add position and size
    checksum += cmd->x_pos;
    checksum += cmd->y_pos;
    checksum += cmd->width;
    checksum += cmd->height;
    
    // Add filename bytes
    for (int i = 0; i < PROTOCOL_MAX_FILENAME && cmd->filename[i] != '\0'; i++) {
        checksum += (uint8_t)cmd->filename[i];
    }
    
    return checksum;
}

uint32_t DisplayManager::calculateTextChecksum(const TextCommand* cmd) {
    if (!cmd) return 0;
    
    // Calculate simple checksum of relevant parameters (excluding screen_id and command)
    uint32_t checksum = 0;
    
    // Add element_id
    checksum += cmd->element_id;
    
    // Add position
    checksum += cmd->x_pos;
    checksum += cmd->y_pos;
    
    // Add color
    checksum += cmd->color_r;
    checksum += cmd->color_g;
    checksum += cmd->color_b;
    
    // Add text length
    checksum += cmd->text_length;
    
    // Add text bytes
    for (int i = 0; i < cmd->text_length && i < PROTOCOL_MAX_TEXT_LENGTH; i++) {
        checksum += (uint8_t)cmd->text[i];
    }
    
    // Add font name bytes
    for (int i = 0; i < 32 && cmd->font_name[i] != '\0'; i++) {
        checksum += (uint8_t)cmd->font_name[i];
    }
    
    return checksum;
}

void DisplayManager::processGifCommand(GifCommand* cmd) {
    if (!cmd) return;
    
    // Check if command is for this screen
    if (cmd->screen_id != my_screen_id) {
        std::cout << "GIF command for screen " << (int)cmd->screen_id 
                  << " ignored (this is screen " << (int)my_screen_id << ")" << std::endl;
        return;
    }
    
    // Calculate checksum for this command
    uint32_t checksum = calculateGifChecksum(cmd);
    
    // Check if this is a duplicate command
    if (command_cache.gif_checksums[cmd->element_id] == checksum && checksum != 0) {
        std::cout << "GIF command ID=" << (int)cmd->element_id << " is duplicate (checksum=" 
                  << checksum << "), skipping processing" << std::endl;
        serial_protocol.sendResponse(cmd->screen_id, RESP_OK);
        return;
    }
    
    std::cout << "Processing GIF command: ID=" << (int)cmd->element_id << " " << cmd->filename 
              << " at (" << cmd->x_pos << "," << cmd->y_pos 
              << ") size " << cmd->width << "x" << cmd->height 
              << " (checksum=" << checksum << ")" << std::endl;
    
    std::string filename(cmd->filename);
    bool success = addGifElement(filename, cmd->x_pos, cmd->y_pos, cmd->width, cmd->height, cmd->element_id);
    
    if (success) {
        // Update cache with new checksum
        command_cache.gif_checksums[cmd->element_id] = checksum;
        std::cout << "GIF loaded successfully, cache updated" << std::endl;
        serial_protocol.sendResponse(cmd->screen_id, RESP_OK);
    } else {
        std::cout << "Failed to load GIF" << std::endl;
        serial_protocol.sendResponse(cmd->screen_id, RESP_FILE_NOT_FOUND);
    }
}

void DisplayManager::processTextCommand(TextCommand* cmd) {
    if (!cmd) return;
    
    // Check if command is for this screen
    if (cmd->screen_id != my_screen_id) {
        std::cout << "TEXT command for screen " << (int)cmd->screen_id 
                  << " ignored (this is screen " << (int)my_screen_id << ")" << std::endl;
        return;
    }
    
    // Calculate checksum for this command
    uint32_t checksum = calculateTextChecksum(cmd);
    
    // Check if this is a duplicate command
    if (command_cache.text_checksums[cmd->element_id] == checksum && checksum != 0) {
        std::cout << "TEXT command ID=" << (int)cmd->element_id << " is duplicate (checksum=" 
                  << checksum << "), skipping processing" << std::endl;
        serial_protocol.sendResponse(cmd->screen_id, RESP_OK);
        return;
    }
    
    std::string text(cmd->text, cmd->text_length);
    std::string font_name(cmd->font_name);
    
    // If no font specified, use default
    if (font_name.empty()) {
        font_name = "fonts/5x7.bdf";
    }
    
    std::cout << "Processing TEXT command: ID=" << (int)cmd->element_id << " '" << text 
              << "' with font: " << font_name << " blink=" << cmd->blink_interval_ms 
              << "ms (checksum=" << checksum << ")" << std::endl;
    
    // Convert RGB to 8-bit color index using fast lookup
    uint8_t color_index = ColorPalette::rgbTo8bitFast(cmd->color_r, cmd->color_g, cmd->color_b);
    
    // Use font_size = 1 (no scaling, use native BDF font size)
    bool success = addTextElement(text, cmd->x_pos, cmd->y_pos, 1, color_index, font_name, 
                                  cmd->element_id, cmd->blink_interval_ms);
    
    if (success) {
        // Update cache with new checksum
        command_cache.text_checksums[cmd->element_id] = checksum;
        std::cout << "Text element added successfully, cache updated" << std::endl;
        serial_protocol.sendResponse(cmd->screen_id, RESP_OK);
    } else {
        std::cout << "Failed to add text element" << std::endl;
        serial_protocol.sendResponse(cmd->screen_id, RESP_INVALID_PARAMS);
    }

    canvas->SetPixel(1, 511, 255, 0, 0);
}

void DisplayManager::processClearCommand(ClearCommand* cmd) {
    if (!cmd) return;
    
    // Check if command is for this screen
    if (cmd->screen_id != my_screen_id) {
        std::cout << "CLEAR command for screen " << (int)cmd->screen_id 
                  << " ignored (this is screen " << (int)my_screen_id << ")" << std::endl;
        return;
    }
    
    clearScreen();
    serial_protocol.sendResponse(cmd->screen_id, RESP_OK);
}

void DisplayManager::processClearTextCommand(ClearCommand* cmd) {
    if (!cmd) return;
    
    // Check if command is for this screen
    if (cmd->screen_id != my_screen_id) {
        std::cout << "CLEAR_TEXT command for screen " << (int)cmd->screen_id 
                  << " ignored (this is screen " << (int)my_screen_id << ")" << std::endl;
        return;
    }
    
    std::cout << "Processing CLEAR_TEXT command" << std::endl;
    clearText();
    serial_protocol.sendResponse(cmd->screen_id, RESP_OK);
}

void DisplayManager::processDeleteElementCommand(DeleteElementCommand* cmd) {
    if (!cmd) return;
    
    // Check if command is for this screen
    if (cmd->screen_id != my_screen_id) {
        std::cout << "DELETE_ELEMENT command for screen " << (int)cmd->screen_id 
                  << " ignored (this is screen " << (int)my_screen_id << ")" << std::endl;
        return;
    }
    
    std::cout << "Processing DELETE_ELEMENT command: element_id=" << (int)cmd->element_id << std::endl;
    
    bool found = false;
    for (auto it = elements.begin(); it != elements.end(); ++it) {
        if (it->element_id == cmd->element_id) {
            // Clear cache for this element
            if (it->type == DisplayElement::GIF) {
                command_cache.gif_checksums[it->element_id] = 0;
            } else if (it->type == DisplayElement::TEXT) {
                command_cache.text_checksums[it->element_id] = 0;
            }
            
            std::cout << "Deleting element ID=" << (int)cmd->element_id 
                      << " type=" << (it->type == DisplayElement::GIF ? "GIF" : "TEXT") << std::endl;
            
            elements.erase(it);
            display_dirty = true;
            found = true;
            break;
        }
    }
    
    if (found) {
        std::cout << "Element deleted successfully. Remaining elements: " << elements.size() << std::endl;
        serial_protocol.sendResponse(cmd->screen_id, RESP_OK);
    } else {
        std::cout << "Element ID=" << (int)cmd->element_id << " not found" << std::endl;
        serial_protocol.sendResponse(cmd->screen_id, RESP_ERROR);
    }
}

void DisplayManager::processBrightnessCommand(BrightnessCommand* cmd) {
    if (!cmd) return;
    
    // Check if command is for this screen
    if (cmd->screen_id != my_screen_id) {
        std::cout << "BRIGHTNESS command for screen " << (int)cmd->screen_id 
                  << " ignored (this is screen " << (int)my_screen_id << ")" << std::endl;
        return;
    }
    
    setBrightness(cmd->brightness);
    serial_protocol.sendResponse(cmd->screen_id, RESP_OK);
}

void DisplayManager::processStatusCommand(StatusCommand* cmd) {
    if (!cmd) return;
    
    // Check if command is for this screen
    if (cmd->screen_id != my_screen_id) {
        std::cout << "STATUS command for screen " << (int)cmd->screen_id 
                  << " ignored (this is screen " << (int)my_screen_id << ")" << std::endl;
        return;
    }
    
    std::string status = getStatus();
    serial_protocol.sendResponse(cmd->screen_id, RESP_OK, 
                               (const uint8_t*)status.c_str(), status.length());
}

void DisplayManager::resetCache() {
    // Clear command cache
    for (int i = 0; i < 256; i++) {
        command_cache.gif_checksums[i] = 0;
        command_cache.text_checksums[i] = 0;
    }
    std::cout << "Command cache reset" << std::endl;
}

void DisplayManager::addDiagnosticElements() {
    // Draw full green matrix with "ProGames" text in center
    std::cout << "Drawing diagnostic pattern: Full green matrix with ProGames..." << std::endl;
    
    // Clear canvas first
    canvas->Clear();
    
    // Fill entire screen with green
    Color8 green = ColorPalette::getColor(3);    // Green
    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            canvas->SetPixel(x, y, green.r/5, green.g/5, green.b/5);
        }
    }
    
    // Draw "ProGames" text in center using white color
    std::string text = "ProGames";
    
    // Choose font size based on screen width (smaller for narrow screens)
    uint8_t font_size;
    if (SCREEN_WIDTH < 100) {
        // For narrow vertical screens (64px wide), use smaller font
        font_size = 1;
        text = "Pro\nGames";  // Split into two lines for narrow screen
    } else {
        font_size = 4;
    }
    
    // Estimate text width (approximately 6 pixels per character * font_size)
    int text_width = 8 * 6 * font_size;  // "ProGames" or longest line
    int text_height = 8 * font_size;
    
    // Calculate center position
    int center_x = (SCREEN_WIDTH - text_width) / 2;
    int center_y = (SCREEN_HEIGHT - text_height) / 2;
    

    for (int y = 0; y < SCREEN_HEIGHT-10; y++) {
            canvas->SetPixel(0, y, 100, green.g/5, green.b/5);
    }

    // For vertical screens, draw text vertically centered
    if (SCREEN_WIDTH < 100) {
        // Draw "Pro" and "Games" on separate lines for narrow screen
        center_x = (SCREEN_WIDTH - 3 * 6 * font_size) / 2;  // "Pro" = 3 chars
        int line_height = 8 * font_size + 2;  // Add small gap
        int center_y_line1 = (SCREEN_HEIGHT - 2 * line_height) / 2;
        
        std::cout << "Drawing 'Pro' at (" << center_x << "," << center_y_line1 << ")" << std::endl;
        for (int i=0; i<50; i++) {
            drawString("Pro", 0, i * 10, font_size, 1);
        }
        
        
        center_x = (SCREEN_WIDTH - 5 * 6 * font_size) / 2;  // "Games" = 5 chars
        int center_y_line2 = center_y_line1 + line_height;
        
        std::cout << "Drawing 'Games' at (" << center_x << "," << center_y_line2 << ")" << std::endl;
        drawString("Games", center_x, center_y_line2, font_size, 1);
    } else {
        std::cout << "Drawing 'ProGames' at center (" << center_x << "," << center_y << ")" << std::endl;
        drawString(text, center_x, center_y, font_size, 1);  // White text (color index 1 = white)
    }
    
    // Force immediate display
    canvas = matrix->SwapOnVSync(canvas, 1);
    
    std::cout << "Diagnostic pattern drawn - green matrix with ProGames in center" << std::endl;
}
