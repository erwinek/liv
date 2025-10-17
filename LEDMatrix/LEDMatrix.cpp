/*
 * LEDMatrix Library - Implementation
 * Biblioteka do sterowania matrycą LED z ESP32
 */

#include "LEDMatrix.h"

// Konstruktor
LEDMatrix::LEDMatrix(HardwareSerial &serial, uint8_t screenId) {
    _serial = &serial;
    _screenId = screenId;
}

// Inicjalizacja portu szeregowego
void LEDMatrix::begin(uint32_t baudrate) {
    _serial->begin(baudrate);
    delay(100); // Krótkie opóźnienie na inicjalizację
}

// Wyczyść ekran
void LEDMatrix::clearScreen() {
    sendPacket(CMD_CLEAR_SCREEN, nullptr, 0);
}

// Ustaw jasność
void LEDMatrix::setBrightness(uint8_t brightness) {
    if (brightness > 100) brightness = 100;
    
    uint8_t payload[1];
    payload[0] = brightness;
    
    sendPacket(CMD_SET_BRIGHTNESS, payload, 1);
}

// Wyświetl tekst
void LEDMatrix::displayText(const char* text, uint16_t x, uint16_t y, 
                            uint8_t fontSize, uint8_t r, uint8_t g, uint8_t b,
                            const char* fontName) {
    // Ograniczenie długości tekstu
    uint8_t textLen = strlen(text);
    if (textLen > 31) textLen = 31;
    
    // TextCommand structure (zgodna z led-image-viewer):
    // screen_id (1) + command (1) + x_pos (2) + y_pos (2) + font_size (1) +
    // color_r (1) + color_g (1) + color_b (1) + text_length (1) + 
    // text (32) + font_name (32) = 75 bytes
    uint8_t payload[75];
    memset(payload, 0, 75);
    
    // screen_id
    payload[0] = _screenId;
    
    // command  
    payload[1] = CMD_DISPLAY_TEXT;
    
    // X Position (little-endian)
    payload[2] = x & 0xFF;
    payload[3] = (x >> 8) & 0xFF;
    
    // Y Position (little-endian)
    payload[4] = y & 0xFF;
    payload[5] = (y >> 8) & 0xFF;
    
    // Font Size
    payload[6] = fontSize;
    
    // Kolor RGB
    payload[7] = r;
    payload[8] = g;
    payload[9] = b;
    
    // Długość tekstu
    payload[10] = textLen;
    
    // Kopiuj tekst i wypełnij zerami (32 bytes)
    strncpy((char*)&payload[11], text, 31);
    
    // Kopiuj nazwę czcionki (32 bytes) - jeśli podana
    if (fontName && strlen(fontName) > 0) {
        strncpy((char*)&payload[43], fontName, 31);
    }
    
    sendPacket(CMD_DISPLAY_TEXT, payload, 75);
}

// Załaduj GIF
void LEDMatrix::loadGif(const char* filename, uint16_t x, uint16_t y, 
                        uint16_t width, uint16_t height) {
    // GifCommand structure (zgodna z led-image-viewer):
    // screen_id (1) + command (1) + x (2) + y (2) + width (2) + height (2) + filename (64) = 74 bytes
    uint8_t payload[74];
    memset(payload, 0, 74);
    
    // screen_id
    payload[0] = _screenId;
    
    // command
    payload[1] = CMD_LOAD_GIF;
    
    // X Position (little-endian)
    payload[2] = x & 0xFF;
    payload[3] = (x >> 8) & 0xFF;
    
    // Y Position (little-endian)
    payload[4] = y & 0xFF;
    payload[5] = (y >> 8) & 0xFF;
    
    // Width (little-endian)
    payload[6] = width & 0xFF;
    payload[7] = (width >> 8) & 0xFF;
    
    // Height (little-endian)
    payload[8] = height & 0xFF;
    payload[9] = (height >> 8) & 0xFF;
    
    // Kopiuj nazwę pliku (64 bytes)
    strncpy((char*)&payload[10], filename, 63);
    
    sendPacket(CMD_LOAD_GIF, payload, 74);
}

// Ustaw ID ekranu
void LEDMatrix::setScreenId(uint8_t screenId) {
    _screenId = screenId;
}

// Pobierz ID ekranu
uint8_t LEDMatrix::getScreenId() const {
    return _screenId;
}

// Wysyłanie pakietu
void LEDMatrix::sendPacket(uint8_t command, const uint8_t* payload, uint8_t payloadLength) {
    // Debug - wyświetl pakiet (zakomentowane, bo koliduje z komunikacją przez ten sam Serial)
    // printPacket(command, payload, payloadLength);
    
    // Buduj i wysyłaj pakiet
    _serial->write(PROTOCOL_SOF);           // Start of Frame
    _serial->write(_screenId);              // Screen ID
    _serial->write(command);                // Command
    _serial->write(payloadLength);          // Payload Length
    
    // Wysyłaj payload (jeśli istnieje)
    if (payload && payloadLength > 0) {
        _serial->write(payload, payloadLength);
    }
    
    // Oblicz i wyślij sumę kontrolną (tylko z payload, zgodnie z SerialProtocol)
    uint8_t checksum = 0;
    if (payload && payloadLength > 0) {
        for (uint8_t i = 0; i < payloadLength; i++) {
            checksum ^= payload[i];
        }
    }
    _serial->write(checksum);
    
    _serial->write(PROTOCOL_EOF);           // End of Frame
    
    // Flush - upewnij się, że wszystkie dane zostały wysłane
    _serial->flush();
}

// Obliczanie sumy kontrolnej (XOR)
uint8_t LEDMatrix::calculateChecksum(const uint8_t* data, uint8_t length) {
    uint8_t checksum = 0;
    for (uint8_t i = 0; i < length; i++) {
        checksum ^= data[i];
    }
    return checksum;
}

// Debug - wyświetl pakiet w hex
void LEDMatrix::printPacket(uint8_t command, const uint8_t* payload, uint8_t payloadLength) {
    Serial.print("\n[MTRX TX] ");
    Serial.print(PROTOCOL_SOF, HEX);
    Serial.print(" ");
    Serial.print(_screenId, HEX);
    Serial.print(" ");
    Serial.print(command, HEX);
    Serial.print(" ");
    Serial.print(payloadLength, HEX);
    Serial.print(" ");
    
    // Wyświetl payload
    for (uint8_t i = 0; i < payloadLength && i < 20; i++) { // Ograniczenie do 20 bajtów dla czytelności
        if (payload[i] < 0x10) Serial.print("0");
        Serial.print(payload[i], HEX);
        Serial.print(" ");
    }
    
    if (payloadLength > 20) {
        Serial.print("... ");
    }
    
    // Wyświetl sumę kontrolną
    uint8_t checksum = calculateChecksum(payload, payloadLength);
    if (checksum < 0x10) Serial.print("0");
    Serial.print(checksum, HEX);
    Serial.print(" ");
    
    Serial.println(PROTOCOL_EOF, HEX);
}

