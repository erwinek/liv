#include "SerialProtocol.h"
#include <iostream>
int main() {
    std::cout << "sizeof(TextCommand) = " << sizeof(TextCommand) << std::endl;
    std::cout << "PROTOCOL_MAX_TEXT_LENGTH = " << PROTOCOL_MAX_TEXT_LENGTH << std::endl;
    return 0;
}
