#include "SerialProtocol.h"
#include <iostream>
int main() {
    TextCommand cmd;
    std::cout << "Field offsets:" << std::endl;
    std::cout << "  screen_id: " << (char*)&cmd.screen_id - (char*)&cmd << std::endl;
    std::cout << "  command: " << (char*)&cmd.command - (char*)&cmd << std::endl;
    std::cout << "  x_pos: " << (char*)&cmd.x_pos - (char*)&cmd << std::endl;
    std::cout << "  y_pos: " << (char*)&cmd.y_pos - (char*)&cmd << std::endl;
    std::cout << "  font_size: " << (char*)&cmd.font_size - (char*)&cmd << std::endl;
    std::cout << "  color_r: " << (char*)&cmd.color_r - (char*)&cmd << std::endl;
    std::cout << "  color_g: " << (char*)&cmd.color_g - (char*)&cmd << std::endl;
    std::cout << "  color_b: " << (char*)&cmd.color_b - (char*)&cmd << std::endl;
    std::cout << "  text_length: " << (char*)&cmd.text_length - (char*)&cmd << std::endl;
    std::cout << "  text: " << (char*)&cmd.text - (char*)&cmd << std::endl;
    std::cout << "Total size: " << sizeof(TextCommand) << std::endl;
    return 0;
}
