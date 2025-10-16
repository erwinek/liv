#include "BdfFont.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>

BdfFont::BdfFont() : char_width(5), char_height(7), font_ascent(7), font_descent(0) {
}

BdfFont::~BdfFont() {
}

bool BdfFont::loadFromFile(const std::string& filename) {
    return parseBdfFile(filename);
}

const BdfChar* BdfFont::getChar(uint32_t encoding) const {
    auto it = chars.find(encoding);
    if (it != chars.end()) {
        return &it->second;
    }
    return nullptr;
}

bool BdfFont::parseBdfFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open BDF file: " << filename << std::endl;
        return false;
    }
    
    std::string line;
    while (std::getline(file, line)) {
        if (line.find("FONTBOUNDINGBOX") == 0) {
            std::istringstream iss(line);
            std::string token;
            iss >> token >> char_width >> char_height;
            std::cout << "Font bounding box: " << char_width << "x" << char_height << std::endl;
        }
        else if (line.find("FONT_ASCENT") == 0) {
            std::istringstream iss(line);
            std::string token;
            iss >> token >> font_ascent;
            std::cout << "Font ascent: " << font_ascent << std::endl;
        }
        else if (line.find("FONT_DESCENT") == 0) {
            std::istringstream iss(line);
            std::string token;
            iss >> token >> font_descent;
            std::cout << "Font descent: " << font_descent << std::endl;
        }
        else if (line.find("STARTCHAR") == 0) {
            // Parse character
            BdfChar ch = parseChar(file);
            if (ch.encoding != 0xFFFFFFFF) { // Valid character
                chars[ch.encoding] = ch;
            }
        }
    }
    
    file.close();
    std::cout << "Loaded " << chars.size() << " characters from BDF file" << std::endl;
    return true;
}

BdfChar BdfFont::parseChar(std::ifstream& file) {
    BdfChar ch;
    ch.encoding = 0xFFFFFFFF; // Invalid encoding
    
    std::string line;
    while (std::getline(file, line)) {
        if (line.find("ENCODING") == 0) {
            std::istringstream iss(line);
            std::string token;
            iss >> token >> ch.encoding;
        }
        else if (line.find("DWIDTH") == 0) {
            std::istringstream iss(line);
            std::string token;
            iss >> token >> ch.dwidth;  // DWIDTH determines character advancement
        }
        else if (line.find("BBX") == 0) {
            std::istringstream iss(line);
            std::string token;
            int16_t x_offset, y_offset;
            iss >> token >> ch.width >> ch.height >> x_offset >> y_offset;  // BBX determines bitmap size
            ch.x_offset = x_offset;
            ch.y_offset = y_offset;
        }
        else if (line.find("BITMAP") == 0) {
            ch.bitmap = parseBitmap(file, ch.width, ch.height);
            break;
        }
        else if (line.find("ENDCHAR") == 0) {
            break;
        }
    }
    
    // Debug: print character info for letters L, E, D
    if (ch.encoding == 'L' || ch.encoding == 'E' || ch.encoding == 'D') {
        std::cout << "BDF: Parsed char '" << (char)ch.encoding << "' (ASCII " << ch.encoding 
                  << ") size " << ch.width << "x" << ch.height 
                  << " offset (" << ch.x_offset << "," << ch.y_offset << ")"
                  << " dwidth=" << ch.dwidth
                  << " bitmap size " << ch.bitmap.size() << " bytes" << std::endl;
        
        // Print first few bytes of bitmap
        for (size_t i = 0; i < std::min(ch.bitmap.size(), size_t(5)); i++) {
            std::cout << "  bitmap[" << i << "] = 0x" << std::hex << (int)ch.bitmap[i] << std::dec << std::endl;
        }
    }
    
    return ch;
}

std::vector<uint8_t> BdfFont::parseBitmap(std::ifstream& file, int width, int height) {
    std::vector<uint8_t> bitmap;
    std::string line;
    
    // Calculate bytes per row (width in bits / 8, rounded up)
    int bytes_per_row = (width + 7) / 8;
    
    for (int row = 0; row < height; row++) {
        if (std::getline(file, line)) {
            // Parse hex string
            std::istringstream iss(line);
            std::string hex_str;
            iss >> hex_str;
            
            // Convert hex string to bytes
            for (int byte = 0; byte < bytes_per_row; byte++) {
                if (byte * 2 + 1 < hex_str.length()) {
                    std::string byte_str = hex_str.substr(byte * 2, 2);
                    uint8_t byte_val = static_cast<uint8_t>(std::stoul(byte_str, nullptr, 16));
                    bitmap.push_back(byte_val);
                } else {
                    bitmap.push_back(0);
                }
            }
        } else {
            // Fill with zeros if line is missing
            for (int byte = 0; byte < bytes_per_row; byte++) {
                bitmap.push_back(0);
            }
        }
    }
    
    return bitmap;
}
