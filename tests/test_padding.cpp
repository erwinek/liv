#include "SerialProtocol.h"
#include <iostream>
int main() {
    TextCommand cmd;
    std::cout << "Total size: " << sizeof(TextCommand) << " bytes" << std::endl;
    std::cout << "Expected: 11 + 32 = 43 bytes" << std::endl;
    std::cout << "Padding: " << sizeof(TextCommand) - 43 << " bytes" << std::endl;
    return 0;
}
