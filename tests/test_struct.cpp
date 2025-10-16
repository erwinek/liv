#include "SerialProtocol.h"
#include <iostream>

int main() {
    std::cout << "sizeof(GifCommand) = " << sizeof(GifCommand) << std::endl;
    std::cout << "Expected: 74 (2+2+2+2+2+2+64)" << std::endl;
    
    // Test structure layout
    GifCommand cmd;
    cmd.screen_id = 1;
    cmd.command = 2;
    cmd.x_pos = 100;
    cmd.y_pos = 200;
    cmd.width = 64;
    cmd.height = 64;
    
    uint8_t* bytes = (uint8_t*)&cmd;
    std::cout << "Bytes: ";
    for (int i = 0; i < 16; i++) {
        std::cout << "0x" << std::hex << (int)bytes[i] << std::dec << " ";
    }
    std::cout << std::endl;
    
    return 0;
}
