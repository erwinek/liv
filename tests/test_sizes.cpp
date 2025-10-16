#include "SerialProtocol.h"
#include <iostream>
int main() {
    TextCommand cmd;
    std::cout << "Field sizes:" << std::endl;
    std::cout << "  screen_id: " << sizeof(cmd.screen_id) << " bytes" << std::endl;
    std::cout << "  command: " << sizeof(cmd.command) << " bytes" << std::endl;
    std::cout << "  x_pos: " << sizeof(cmd.x_pos) << " bytes" << std::endl;
    std::cout << "  y_pos: " << sizeof(cmd.y_pos) << " bytes" << std::endl;
    std::cout << "  font_size: " << sizeof(cmd.font_size) << " bytes" << std::endl;
    std::cout << "  color_r: " << sizeof(cmd.color_r) << " bytes" << std::endl;
    std::cout << "  color_g: " << sizeof(cmd.color_g) << " bytes" << std::endl;
    std::cout << "  color_b: " << sizeof(cmd.color_b) << " bytes" << std::endl;
    std::cout << "  text_length: " << sizeof(cmd.text_length) << " bytes" << std::endl;
    std::cout << "  text: " << sizeof(cmd.text) << " bytes" << std::endl;
    std::cout << "Total: " << sizeof(TextCommand) << " bytes" << std::endl;
    return 0;
}
