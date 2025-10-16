#include "SerialProtocol.h"
#include <iostream>
int main() {
    TextCommand cmd;
    std::cout << "Field offsets and sizes:" << std::endl;
    std::cout << "  screen_id: offset " << (char*)&cmd.screen_id - (char*)&cmd << ", size " << sizeof(cmd.screen_id) << std::endl;
    std::cout << "  command: offset " << (char*)&cmd.command - (char*)&cmd << ", size " << sizeof(cmd.command) << std::endl;
    std::cout << "  x_pos: offset " << (char*)&cmd.x_pos - (char*)&cmd << ", size " << sizeof(cmd.x_pos) << std::endl;
    std::cout << "  y_pos: offset " << (char*)&cmd.y_pos - (char*)&cmd << ", size " << sizeof(cmd.y_pos) << std::endl;
    std::cout << "  font_size: offset " << (char*)&cmd.font_size - (char*)&cmd << ", size " << sizeof(cmd.font_size) << std::endl;
    std::cout << "  color_r: offset " << (char*)&cmd.color_r - (char*)&cmd << ", size " << sizeof(cmd.color_r) << std::endl;
    std::cout << "  color_g: offset " << (char*)&cmd.color_g - (char*)&cmd << ", size " << sizeof(cmd.color_g) << std::endl;
    std::cout << "  color_b: offset " << (char*)&cmd.color_b - (char*)&cmd << ", size " << sizeof(cmd.color_b) << std::endl;
    std::cout << "  text_length: offset " << (char*)&cmd.text_length - (char*)&cmd << ", size " << sizeof(cmd.text_length) << std::endl;
    std::cout << "  text: offset " << (char*)&cmd.text - (char*)&cmd << ", size " << sizeof(cmd.text) << std::endl;
    return 0;
}
