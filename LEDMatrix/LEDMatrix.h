/*
 * LEDMatrix Library
 * Biblioteka do sterowania matrycą LED z ESP32
 * Protokół komunikacji serial
 * 
 * Autor: Generated
 * Wersja: 1.1.0
 * 
 * OBSŁUGA WIELU EKRANÓW:
 * Wszystkie metody przyjmują opcjonalny parametr screen_id.
 * - Jeśli screen_id = 0 (wartość domyślna), użyty zostanie _screenId z konstruktora
 * - Jeśli screen_id > 0, komenda zostanie wysłana do określonego ekranu
 * 
 * PRZYKŁAD UŻYCIA:
 * LEDMatrix matrix(Serial1, 1);  // Domyślny ekran = 1
 * matrix.displayText("Hello", 0, 0, 1, 255, 0, 0, "font", 1);     // Użyje ekranu 1 (domyślny)
 * matrix.displayText("World", 0, 0, 1, 255, 0, 0, "font", 2, 0, 2);  // Wysyła do ekranu 2
 */

#ifndef LEDMatrix_h
#define LEDMatrix_h

#include "Arduino.h"

// Stałe protokołu - Preamble for reliable synchronization (like Ethernet)
#define PROTOCOL_PREAMBLE_1 0xAA  // Preamble byte 1
#define PROTOCOL_PREAMBLE_2 0x55  // Preamble byte 2
#define PROTOCOL_PREAMBLE_3 0xAA  // Preamble byte 3
#define PROTOCOL_SOF 0x55         // Start of Frame (after preamble)
#define PROTOCOL_EOF 0xAA         // End of Frame
#define PROTOCOL_MAX_PAYLOAD 150

// ID ekranu (domyślnie 1)
#define PROTOCOL_SCREEN_ID 1

// Komendy
#define CMD_LOAD_GIF 0x01
#define CMD_DISPLAY_TEXT 0x02
#define CMD_CLEAR_SCREEN 0x03
#define CMD_SET_BRIGHTNESS 0x04
#define CMD_GET_STATUS 0x05
#define CMD_CLEAR_TEXT 0x06
#define CMD_DELETE_ELEMENT 0x07

class LEDMatrix {
public:
    // Konstruktor
    LEDMatrix(HardwareSerial &serial, uint8_t screenId = PROTOCOL_SCREEN_ID);
    
    // Inicjalizacja
    void begin(uint32_t baudrate = 1000000);
    
    // Podstawowe funkcje
    void clearScreen(uint8_t screen_id = 0);
    void clearText(uint8_t screen_id = 0);  // Clear only text elements, keep GIFs
    void deleteElement(uint8_t elementId, uint8_t screen_id = 0);  // Delete specific element by ID
    void setBrightness(uint8_t brightness, uint8_t screen_id = 0);
    
    // Wyświetlanie tekstu
    void displayText(const char* text, uint16_t x, uint16_t y, 
                     uint8_t fontSize, 
                     uint8_t r, uint8_t g, uint8_t b,
                     const char* fontName, uint8_t elementId,
                     uint16_t blinkIntervalMs = 0, uint8_t screen_id = 0);
    
    // Ładowanie GIF
    void loadGif(const char* filename, uint16_t x, uint16_t y, 
                 uint16_t width, uint16_t height, uint8_t elementId, uint8_t screen_id = 0);
    
    // Funkcje pomocnicze
    void setScreenId(uint8_t screenId);
    uint8_t getScreenId() const;
    
private:
    HardwareSerial* _serial;
    uint8_t _screenId;
    
    // Wysyłanie pakietu
    void sendPacket(uint8_t command, const uint8_t* payload, uint8_t payloadLength, uint8_t screen_id = 0);
    
    // Obliczanie sumy kontrolnej (XOR)
    uint8_t calculateChecksum(const uint8_t* data, uint8_t length);
    
    // Debug
    void printPacket(uint8_t command, const uint8_t* payload, uint8_t payloadLength);
};

#endif

