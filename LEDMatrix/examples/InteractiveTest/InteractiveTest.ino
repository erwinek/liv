/*
 * InteractiveTest - Interaktywny test komunikacji
 * 
 * Użyj Serial Monitor aby testować różne ustawienia
 */

// ============================================================================
// KONFIGURACJA
// ============================================================================

uint32_t currentBaudrate = 1000000;
uint8_t screenId = 1;

// ============================================================================
// FUNKCJE POMOCNICZE
// ============================================================================

void printMenu() {
    Serial.println("\n╔═══════════════════════════════════════════════════════════╗");
    Serial.println("║         LED Matrix Interactive Test                     ║");
    Serial.println("╚═══════════════════════════════════════════════════════════╝");
    Serial.println();
    Serial.println("Current settings:");
    Serial.print("  Baudrate: ");
    Serial.println(currentBaudrate);
    Serial.print("  Screen ID: ");
    Serial.println(screenId);
    Serial.println();
    Serial.println("Commands:");
    Serial.println("  1 - Test clear screen");
    Serial.println("  2 - Test brightness");
    Serial.println("  3 - Test display text");
    Serial.println("  4 - Change baudrate");
    Serial.println("  5 - Change screen ID");
    Serial.println("  6 - Send custom packet");
    Serial.println("  7 - Run all tests");
    Serial.println("  h - Show this menu");
    Serial.println();
    Serial.print("Enter command: ");
}

void sendPacket(uint8_t command, uint8_t* payload, uint8_t payloadLength) {
    // Reinitialize Serial1 with current baudrate
    Serial1.end();
    delay(100);
    Serial1.begin(currentBaudrate);
    delay(500);
    
    Serial.print("\n[MTRX TX] ");
    Serial.print("0xAA ");
    Serial.print(screenId, HEX);
    Serial.print(" 0x");
    Serial.print(command, HEX);
    Serial.print(" 0x");
    Serial.print(payloadLength, HEX);
    Serial.print(" ");
    
    // Calculate checksum
    uint8_t checksum = 0;
    if (payload && payloadLength > 0) {
        for (uint8_t i = 0; i < payloadLength; i++) {
            checksum ^= payload[i];
            if (payload[i] < 0x10) Serial.print("0");
            Serial.print(payload[i], HEX);
            Serial.print(" ");
        }
    }
    
    Serial.print("0x");
    if (checksum < 0x10) Serial.print("0");
    Serial.print(checksum, HEX);
    Serial.println(" 0x55");
    
    // Send packet
    Serial1.write(0xAA); // SOF
    Serial1.write(screenId); // Screen ID
    Serial1.write(command); // Command
    Serial1.write(payloadLength); // Payload length
    
    if (payload && payloadLength > 0) {
        Serial1.write(payload, payloadLength);
    }
    
    Serial1.write(checksum); // Checksum
    Serial1.write(0x55); // EOF
    Serial1.flush();
    
    Serial.println("✓ Packet sent");
}

void testClearScreen() {
    Serial.println("\n[TEST] Clear Screen");
    sendPacket(0x03, nullptr, 0);
}

void testBrightness() {
    Serial.println("\n[TEST] Set Brightness");
    
    uint8_t brightness = 80;
    uint8_t payload[1];
    payload[0] = brightness;
    
    Serial.print("Setting brightness to: ");
    Serial.println(brightness);
    
    sendPacket(0x04, payload, 1);
}

void testDisplayText() {
    Serial.println("\n[TEST] Display Text");
    
    const char* text = "HELLO";
    uint8_t payload[40];
    
    // X=0, Y=0
    payload[0] = 0x00;
    payload[1] = 0x00;
    payload[2] = 0x00;
    payload[3] = 0x00;
    
    // Font size = 2
    payload[4] = 0x02;
    
    // RGB = Yellow
    payload[5] = 0xFF; // R
    payload[6] = 0xFF; // G
    payload[7] = 0x00; // B
    
    // Text length
    payload[8] = strlen(text);
    
    // Copy text
    memset(&payload[9], 0, 32);
    strncpy((char*)&payload[9], text, 31);
    
    Serial.print("Displaying text: ");
    Serial.println(text);
    
    sendPacket(0x02, payload, 40);
}

void changeBaudrate() {
    Serial.println("\n[CONFIG] Change Baudrate");
    Serial.println();
    Serial.println("Available baudrates:");
    Serial.println("  1 - 115200");
    Serial.println("  2 - 230400");
    Serial.println("  3 - 460800");
    Serial.println("  4 - 921600");
    Serial.println("  5 - 1000000 (current)");
    Serial.print("\nSelect baudrate: ");
    
    while (!Serial.available()) {
        delay(10);
    }
    
    int choice = Serial.parseInt();
    
    switch (choice) {
        case 1: currentBaudrate = 115200; break;
        case 2: currentBaudrate = 230400; break;
        case 3: currentBaudrate = 460800; break;
        case 4: currentBaudrate = 921600; break;
        case 5: currentBaudrate = 1000000; break;
        default:
            Serial.println("Invalid choice, keeping current baudrate");
            return;
    }
    
    Serial.println();
    Serial.print("Baudrate changed to: ");
    Serial.println(currentBaudrate);
}

void changeScreenId() {
    Serial.println("\n[CONFIG] Change Screen ID");
    Serial.print("Current Screen ID: ");
    Serial.println(screenId);
    Serial.print("Enter new Screen ID (1-255): ");
    
    while (!Serial.available()) {
        delay(10);
    }
    
    int newId = Serial.parseInt();
    
    if (newId >= 1 && newId <= 255) {
        screenId = newId;
        Serial.println();
        Serial.print("Screen ID changed to: ");
        Serial.println(screenId);
    } else {
        Serial.println("\nInvalid ID, keeping current value");
    }
}

void sendCustomPacket() {
    Serial.println("\n[CUSTOM] Send Custom Packet");
    Serial.println("Enter packet data as hex (e.g., AA 01 03 00 00 55)");
    Serial.print("Packet: ");
    
    // Wait for input
    while (!Serial.available()) {
        delay(10);
    }
    
    String input = Serial.readStringUntil('\n');
    input.trim();
    
    // Parse hex values
    int start = 0;
    int end = input.indexOf(' ');
    
    while (end != -1 || start < input.length()) {
        if (end == -1) end = input.length();
        
        String hexByte = input.substring(start, end);
        hexByte.trim();
        
        if (hexByte.length() > 0) {
            uint8_t byteValue = strtol(hexByte.c_str(), NULL, 16);
            Serial1.write(byteValue);
            Serial.print(hexByte);
            Serial.print(" ");
        }
        
        start = end + 1;
        end = input.indexOf(' ', start);
    }
    
    Serial1.flush();
    Serial.println();
    Serial.println("✓ Custom packet sent");
}

void runAllTests() {
    Serial.println("\n[ALL TESTS] Running all tests...");
    Serial.println();
    
    testClearScreen();
    delay(1000);
    
    testBrightness();
    delay(1000);
    
    testDisplayText();
    delay(1000);
    
    Serial.println("\n✓ All tests complete!");
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n");
    Serial.println("╔═══════════════════════════════════════════════════════════╗");
    Serial.println("║                                                           ║");
    Serial.println("║         LED Matrix Interactive Test                      ║");
    Serial.println("║                                                           ║");
    Serial.println("╚═══════════════════════════════════════════════════════════╝");
    
    printMenu();
}

void loop() {
    if (Serial.available()) {
        char command = Serial.read();
        
        // Clear input buffer
        while (Serial.available()) {
            Serial.read();
        }
        
        Serial.println(command);
        
        switch (command) {
            case '1':
                testClearScreen();
                break;
            case '2':
                testBrightness();
                break;
            case '3':
                testDisplayText();
                break;
            case '4':
                changeBaudrate();
                break;
            case '5':
                changeScreenId();
                break;
            case '6':
                sendCustomPacket();
                break;
            case '7':
                runAllTests();
                break;
            case 'h':
            case 'H':
                printMenu();
                break;
            default:
                Serial.println("Unknown command. Type 'h' for help.");
                break;
        }
        
        Serial.println();
        Serial.print("Enter command: ");
    }
    
    delay(10);
}


