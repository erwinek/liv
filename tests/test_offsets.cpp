#include "SerialProtocol.h"
#include <iostream>
int main() {
    TextCommand cmd;
    std::cout << "Field offsets:" << std::endl;
    std::cout << "  screen_id: offset " << (char*)&cmd.screen_id - (char*)&cmd << std::endl;
    std::cout << "  command: offset " << (char*)&cmd.command - (char*)&cmd << std::endl;
    std::cout << "  x_pos: offset " << (char*)&cmd.x_pos - (char*)&cmd << std::endl;
    std::cout << "  y_pos: offset " << (char*)&cmd.y_pos - (char*)&cmd << std::endl;
    std::cout << "  font_size: offset " << (char*)&cmd.font_size - (char*)&cmd << std::endl;
    std::cout << "  color_r: offset " << (char*)&cmd.color_r - (char*)&cmd << std::endl;
    std::cout << "  color_g: offset " << (char*)&cmd.color_g - (char*)&cmd << std::endl;
    std::cout << "  color_b: offset " << (char*)&cmd.color_b - (char*)&cmd << std::endl;
    std::cout << "  text_length: offset " << (char*)&cmd.text_length - (char*)&cmd << std::endl;
    std::cout << "  text: offset " << (char*)&cmd.text - (char*)&cmd << std::endl;
    return 0;
}
