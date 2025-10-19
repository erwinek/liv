#include "../SerialProtocol.h"
#include <iostream>
#include <cstddef>

int main() {
    std::cout << "TextCommand structure analysis:" << std::endl;
    std::cout << "=================================" << std::endl;
    std::cout << "screen_id:         offset=" << offsetof(TextCommand, screen_id) << " size=1" << std::endl;
    std::cout << "command:           offset=" << offsetof(TextCommand, command) << " size=1" << std::endl;
    std::cout << "element_id:        offset=" << offsetof(TextCommand, element_id) << " size=1" << std::endl;
    std::cout << "x_pos:             offset=" << offsetof(TextCommand, x_pos) << " size=2" << std::endl;
    std::cout << "y_pos:             offset=" << offsetof(TextCommand, y_pos) << " size=2" << std::endl;
    std::cout << "color_r:           offset=" << offsetof(TextCommand, color_r) << " size=1" << std::endl;
    std::cout << "color_g:           offset=" << offsetof(TextCommand, color_g) << " size=1" << std::endl;
    std::cout << "color_b:           offset=" << offsetof(TextCommand, color_b) << " size=1" << std::endl;
    std::cout << "text_length:       offset=" << offsetof(TextCommand, text_length) << " size=1" << std::endl;
    std::cout << "text:              offset=" << offsetof(TextCommand, text) << " size=32" << std::endl;
    std::cout << "font_name:         offset=" << offsetof(TextCommand, font_name) << " size=32" << std::endl;
    std::cout << "blink_interval_ms: offset=" << offsetof(TextCommand, blink_interval_ms) << " size=2" << std::endl;
    std::cout << "=================================" << std::endl;
    std::cout << "Total sizeof(TextCommand) = " << sizeof(TextCommand) << " bytes" << std::endl;
    std::cout << "Expected: 77 bytes" << std::endl;
    
    if (sizeof(TextCommand) == 77) {
        std::cout << "✓ Structure size is CORRECT!" << std::endl;
    } else {
        std::cout << "✗ ERROR: Structure size is WRONG!" << std::endl;
    }
    
    return 0;
}

