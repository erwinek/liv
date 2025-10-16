/*
 * LED Matrix Controller - Standalone Version
 * Prosty projekt Arduino do sterowania matrycą LED z ESP32
 * 
 * Ten projekt zawiera cały kod w jednym pliku - nie wymaga instalacji biblioteki
 * 
 * Połączenia:
 * - ESP32 TX (GPIO 17) -> RX matrycy
 * - ESP32 GND -> GND matrycy
 * - ESP32 VIN -> VIN matrycy (jeśli wymagane)
 * 
 * Autor: Generated
 * Wersja: 1.0
 */

// ============================================================================
// KONFIGURACJA
// ============================================================================

// Port szeregowy (Serial1 dla ESP32)
#define MATRIX_SERIAL Serial1

// ID ekranu
#define SCREEN_ID 1

// Baudrate
#define BAUD_RATE 1000000

// ============================================================================
// STAŁE PROTOKOŁU
// ============================================================================

#define PROTOCOL_SOF 0xAA
#define PROTOCOL_EOF 0x55

// Komendy
#define CMD_LOAD_GIF 0x01
#define CMD_DISPLAY_TEXT 0x02
#define CMD_CLEAR_SCREEN 0x03
#define CMD_SET_BRIGHTNESS 0x04
#define CMD_GET_STATUS 0x05

// ============================================================================
// ZMIENNE GLOBALNE
// ============================================================================

unsigned long lastUpdate = 0;
uint8_t demoStep = 0;
uint8_t brightness = 80;

// ============================================================================
// FUNKCJE POMOCNICZE
// ============================================================================

// Obliczanie sumy kontrolnej (XOR)
uint8_t calculateChecksum(const uint8_t* data, uint8_t length) {
    uint8_t checksum = 0;
    for (uint8_t i = 0; i < length; i++) {
        checksum ^= data[i];
    }
    return checksum;
}

// Wysyłanie pakietu do matrycy
void sendPacket(uint8_t command, const uint8_t* payload, uint8_t payloadLength) {
    // Debug - wyświetl pakiet
    Serial.print("\n[MTRX TX] ");
    Serial.print(PROTOCOL_SOF, HEX);
    Serial.print(" ");
    Serial.print(SCREEN_ID, HEX);
    Serial.print(" ");
    Serial.print(command, HEX);
    Serial.print(" ");
    Serial.print(payloadLength, HEX);
    Serial.print(" ");
    
    // Wyślij pakiet
    MATRIX_SERIAL.write(PROTOCOL_SOF);
    MATRIX_SERIAL.write(SCREEN_ID);
    MATRIX_SERIAL.write(command);
    MATRIX_SERIAL.write(payloadLength);
    
    // Wyślij payload
    if (payload && payloadLength > 0) {
        MATRIX_SERIAL.write(payload, payloadLength);
        
        // Debug - wyświetl payload
        for (uint8_t i = 0; i < payloadLength && i < 20; i++) {
            if (payload[i] < 0x10) Serial.print("0");
            Serial.print(payload[i], HEX);
            Serial.print(" ");
        }
        if (payloadLength > 20) Serial.print("... ");
    }
    
    // Wyślij sumę kontrolną
    uint8_t checksum = calculateChecksum(payload, payloadLength);
    MATRIX_SERIAL.write(checksum);
    if (checksum < 0x10) Serial.print("0");
    Serial.print(checksum, HEX);
    Serial.print(" ");
    
    // Wyślij EOF
    MATRIX_SERIAL.write(PROTOCOL_EOF);
    Serial.println(PROTOCOL_EOF, HEX);
    
    // Flush
    MATRIX_SERIAL.flush();
}

// ============================================================================
// FUNKCJE API MATRYCY
// ============================================================================

// Wyczyść ekran
void clearScreen() {
    sendPacket(CMD_CLEAR_SCREEN, nullptr, 0);
}

// Ustaw jasność
void setBrightness(uint8_t brightness) {
    if (brightness > 100) brightness = 100;
    
    uint8_t payload[1];
    payload[0] = brightness;
    
    sendPacket(CMD_SET_BRIGHTNESS, payload, 1);
}

// Wyświetl tekst
void displayText(const char* text, uint16_t x, uint16_t y, 
                 uint8_t fontSize, uint8_t r, uint8_t g, uint8_t b) {
    uint8_t textLen = strlen(text);
    if (textLen > 31) textLen = 31;
    
    // Przygotuj payload
    uint8_t payload[40];
    
    // X Position (little-endian)
    payload[0] = x & 0xFF;
    payload[1] = (x >> 8) & 0xFF;
    
    // Y Position (little-endian)
    payload[2] = y & 0xFF;
    payload[3] = (y >> 8) & 0xFF;
    
    // Font Size
    payload[4] = fontSize;
    
    // Kolor RGB
    payload[5] = r;
    payload[6] = g;
    payload[7] = b;
    
    // Długość tekstu
    payload[8] = textLen;
    
    // Kopiuj tekst i wypełnij zerami
    memset(&payload[9], 0, 32);
    strncpy((char*)&payload[9], text, 31);
    
    sendPacket(CMD_DISPLAY_TEXT, payload, 40);
}

// Załaduj GIF
void loadGif(const char* filename, uint16_t x, uint16_t y, 
             uint16_t width, uint16_t height) {
    uint8_t payload[70];
    
    // X Position (little-endian)
    payload[0] = x & 0xFF;
    payload[1] = (x >> 8) & 0xFF;
    
    // Y Position (little-endian)
    payload[2] = y & 0xFF;
    payload[3] = (y >> 8) & 0xFF;
    
    // Width (little-endian)
    payload[4] = width & 0xFF;
    payload[5] = (width >> 8) & 0xFF;
    
    // Height (little-endian)
    payload[6] = height & 0xFF;
    payload[7] = (height >> 8) & 0xFF;
    
    // Kopiuj nazwę pliku i wypełnij zerami
    memset(&payload[8], 0, 64);
    strncpy((char*)&payload[8], filename, 63);
    
    sendPacket(CMD_LOAD_GIF, payload, 70);
}

// ============================================================================
// SETUP
// ============================================================================

void setup() {
    // Inicjalizacja Serial do debugowania
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n========================================");
    Serial.println("LED Matrix Controller - Standalone");
    Serial.println("========================================\n");
    
    // Inicjalizacja matrycy
    Serial.println("Initializing LED Matrix...");
    MATRIX_SERIAL.begin(BAUD_RATE);
    delay(500);
    
    // Wyczyść ekran
    Serial.println("Clearing screen...");
    clearScreen();
    delay(500);
    
    // Ustaw jasność
    Serial.println("Setting brightness...");
    setBrightness(brightness);
    delay(500);
    
    Serial.println("Ready!\n");
    Serial.println("Starting demo sequence...\n");
}

// ============================================================================
// LOOP
// ============================================================================

void loop() {
    unsigned long currentTime = millis();
    
    // Zmiana demo co 3 sekundy
    if (currentTime - lastUpdate > 3000) {
        lastUpdate = currentTime;
        
        Serial.print("Demo Step: ");
        Serial.println(demoStep);
        
        switch (demoStep) {
            case 0:
                Serial.println("Displaying: HELLO (Yellow)");
                displayText("HELLO", 0, 0, 2, 255, 255, 0);
                break;
                
            case 1:
                Serial.println("Displaying: WORLD (Cyan)");
                displayText("WORLD", 0, 50, 2, 0, 255, 255);
                break;
                
            case 2:
                Serial.println("Displaying: ESP32 (Magenta)");
                displayText("ESP32", 0, 0, 3, 255, 0, 255);
                break;
                
            case 3:
                Serial.println("Clearing screen...");
                clearScreen();
                break;
                
            case 4:
                Serial.println("Loading GIF: anim/1.gif");
                loadGif("anim/1.gif", 0, 0, 96, 96);
                break;
                
            case 5:
                Serial.println("Changing brightness...");
                brightness = (brightness == 80) ? 50 : 80;
                setBrightness(brightness);
                break;
                
            case 6:
                Serial.println("Displaying: RED");
                displayText("RED", 0, 0, 2, 255, 0, 0);
                delay(1000);
                Serial.println("Displaying: GREEN");
                displayText("GREEN", 0, 0, 2, 0, 255, 0);
                delay(1000);
                Serial.println("Displaying: BLUE");
                displayText("BLUE", 0, 0, 2, 0, 0, 255);
                break;
                
            case 7:
                Serial.println("Restarting demo sequence...");
                clearScreen();
                demoStep = -1; // Zostanie zwiększone do 0
                break;
        }
        
        demoStep++;
        if (demoStep > 7) {
            demoStep = 0;
        }
    }
    
    delay(10);
}

// ============================================================================
// KONIEC
// ============================================================================


