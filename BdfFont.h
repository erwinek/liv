#ifndef BDF_FONT_H
#define BDF_FONT_H

#include <string>
#include <vector>
#include <map>
#include <cstdint>

struct BdfChar {
    uint32_t encoding;
    int16_t width;
    int16_t height;
    int16_t x_offset;
    int16_t y_offset;
    std::vector<uint8_t> bitmap;
};

class BdfFont {
public:
    BdfFont();
    ~BdfFont();
    
    bool loadFromFile(const std::string& filename);
    const BdfChar* getChar(uint32_t encoding) const;
    int getCharWidth() const { return char_width; }
    int getCharHeight() const { return char_height; }
    int getFontAscent() const { return font_ascent; }
    int getFontDescent() const { return font_descent; }
    
private:
    std::map<uint32_t, BdfChar> chars;
    int char_width;
    int char_height;
    int font_ascent;
    int font_descent;
    
    bool parseBdfFile(const std::string& filename);
    BdfChar parseChar(std::ifstream& file);
    std::vector<uint8_t> parseBitmap(std::ifstream& file, int width, int height);
};

#endif // BDF_FONT_H
