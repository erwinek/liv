/*
 * DiagnosticTest - Test diagnostyczny komunikacji z matrycą LED
 * 
 * Ten program pomaga zdiagnozować problemy z komunikacją
 */

// ============================================================================
// KONFIGURACJA - ZMIEŃ TUTAJ JEŚLI TRZEBA
// ============================================================================

// Baudrate - spróbuj różnych wartości jeśli 1000000 nie działa
#define BAUD_RATE 1000000

// ID ekranu (domyślnie 1)
#define SCREEN_ID 1

// ============================================================================
// TESTY
// ============================================================================

void test1_SerialInit() {
    Serial.println("\n[TEST 1] Serial1 Initialization");
    Serial.println("==================================");
    
    Serial.print("Baudrate: ");
    Serial.println(BAUD_RATE);
    
    Serial1.begin(BAUD_RATE);
    delay(500);
    
    Serial.println("✓ Serial1.begin() called");
    Serial.println("✓ Waiting 500ms for initialization");
    
    // Sprawdź czy Serial1 jest dostępny
    if (Serial1) {
        Serial.println("✓ Serial1 is available");
    } else {
        Serial.println("✗ Serial1 NOT available!");
    }
    
    Serial.println();
}

void test2_SendSimplePacket() {
    Serial.println("[TEST 2] Sending Simple Packet");
    Serial.println("==================================");
    
    // Wyślij prosty pakiet: CMD_CLEAR_SCREEN
    Serial.println("Sending: SOF, ScreenID, Command, Length, Checksum, EOF");
    
    Serial1.write(0xAA); // SOF
    Serial1.write(SCREEN_ID); // Screen ID
    Serial1.write(0x03); // CMD_CLEAR_SCREEN
    Serial1.write(0x00); // Payload length = 0
    Serial1.write(0x00); // Checksum = 0 (0 XOR 0 = 0)
    Serial1.write(0x55); // EOF
    
    Serial1.flush();
    
    Serial.println("Packet sent: AA 01 03 00 00 55");
    Serial.println("✓ Packet sent and flushed");
    Serial.println();
}

void test3_SendBrightnessPacket() {
    Serial.println("[TEST 3] Sending Brightness Packet");
    Serial.println("==================================");
    
    uint8_t brightness = 80;
    
    Serial.print("Setting brightness to: ");
    Serial.println(brightness);
    
    // Wyślij pakiet: CMD_SET_BRIGHTNESS
    Serial1.write(0xAA); // SOF
    Serial1.write(SCREEN_ID); // Screen ID
    Serial1.write(0x04); // CMD_SET_BRIGHTNESS
    Serial1.write(0x01); // Payload length = 1
    Serial1.write(brightness); // Brightness value
    Serial1.write(brightness); // Checksum (brightness XOR 0 = brightness)
    Serial1.write(0x55); // EOF
    
    Serial1.flush();
    
    Serial.print("Packet sent: AA ");
    Serial.print(SCREEN_ID, HEX);
    Serial.print(" 04 01 ");
    Serial.print(brightness, HEX);
    Serial.print(" ");
    Serial.print(brightness, HEX);
    Serial.println(" 55");
    Serial.println("✓ Packet sent and flushed");
    Serial.println();
}

void test4_SendTextPacket() {
    Serial.println("[TEST 4] Sending Text Packet");
    Serial.println("==================================");
    
    const char* text = "TEST";
    uint8_t textLen = strlen(text);
    
    Serial.print("Text: ");
    Serial.println(text);
    
    // Przygotuj payload
    uint8_t payload[40];
    
    // X=0, Y=0
    payload[0] = 0x00;
    payload[1] = 0x00;
    payload[2] = 0x00;
    payload[3] = 0x00;
    
    // Font size = 2
    payload[4] = 0x02;
    
    // RGB = Red
    payload[5] = 0xFF; // R
    payload[6] = 0x00; // G
    payload[7] = 0x00; // B
    
    // Text length
    payload[8] = textLen;
    
    // Copy text
    memset(&payload[9], 0, 32);
    strncpy((char*)&payload[9], text, 31);
    
    // Calculate checksum
    uint8_t checksum = 0;
    for (int i = 0; i < 40; i++) {
        checksum ^= payload[i];
    }
    
    // Wyślij pakiet
    Serial1.write(0xAA); // SOF
    Serial1.write(SCREEN_ID); // Screen ID
    Serial1.write(0x02); // CMD_DISPLAY_TEXT
    Serial1.write(0x28); // Payload length = 40
    Serial1.write(payload, 40); // Payload
    Serial1.write(checksum); // Checksum
    Serial1.write(0x55); // EOF
    
    Serial1.flush();
    
    Serial.print("Packet sent: AA ");
    Serial.print(SCREEN_ID, HEX);
    Serial.println(" 02 28 [40 bytes payload] [checksum] 55");
    Serial.println("✓ Packet sent and flushed");
    Serial.println();
}

void test5_CheckConnections() {
    Serial.println("[TEST 5] Hardware Connections Check");
    Serial.println("==================================");
    Serial.println("ESP32          →  LED Matrix");
    Serial.println("────────────────────────────────");
    Serial.println("GPIO 17 (TX)   →  RX");
    Serial.println("GND            →  GND");
    Serial.println("────────────────────────────────");
    Serial.println("Baudrate: 1000000 bps (1 Mbps)");
    Serial.println();
    Serial.println("⚠️  CHECK:");
    Serial.println("  1. TX ESP32 → RX Matrix (NOT TX→TX!)");
    Serial.println("  2. GND is connected");
    Serial.println("  3. Matrix is powered");
    Serial.println("  4. Matrix is turned on");
    Serial.println();
}

void test6_AlternativeBaudrates() {
    Serial.println("[TEST 6] Alternative Baudrates");
    Serial.println("==================================");
    Serial.println("If current baudrate doesn't work, try:");
    Serial.println("  - 115200 bps (slow, reliable)");
    Serial.println("  - 230400 bps");
    Serial.println("  - 460800 bps");
    Serial.println("  - 921600 bps");
    Serial.println("  - 1000000 bps (current)");
    Serial.println();
    Serial.println("To change baudrate:");
    Serial.println("  Edit #define BAUD_RATE at top of file");
    Serial.println();
}

void setup() {
    // Inicjalizacja Serial do debugowania
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n");
    Serial.println("╔═══════════════════════════════════════════════════════════╗");
    Serial.println("║                                                           ║");
    Serial.println("║           LED Matrix Diagnostic Test                     ║");
    Serial.println("║                                                           ║");
    Serial.println("╚═══════════════════════════════════════════════════════════╝");
    Serial.println();
    
    // Uruchom wszystkie testy
    test1_SerialInit();
    delay(1000);
    
    test2_SendSimplePacket();
    delay(1000);
    
    test3_SendBrightnessPacket();
    delay(1000);
    
    test4_SendTextPacket();
    delay(1000);
    
    test5_CheckConnections();
    
    test6_AlternativeBaudrates();
    
    Serial.println("╔═══════════════════════════════════════════════════════════╗");
    Serial.println("║                                                           ║");
    Serial.println("║           Tests Complete!                                ║");
    Serial.println("║                                                           ║");
    Serial.println("║  Check the LED Matrix for changes                        ║");
    Serial.println("║                                                           ║");
    Serial.println("║  If nothing happens:                                     ║");
    Serial.println("║    1. Check connections (TX→RX, GND→GND)                 ║");
    Serial.println("║    2. Try different baudrate                             ║");
    Serial.println("║    3. Check if matrix is powered                         ║");
    Serial.println("║    4. Check matrix documentation                         ║");
    Serial.println("║                                                           ║");
    Serial.println("╚═══════════════════════════════════════════════════════════╝");
    Serial.println();
}

void loop() {
    // Nic nie rób - testy są jednorazowe
    delay(1000);
}


