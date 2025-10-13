/*
 * LEDMatrix Library
 * Biblioteka do sterowania matrycą LED z ESP32
 * Protokół komunikacji serial
 * 
 * Autor: Generated
 * Wersja: 1.0.0
 */

#ifndef LEDMatrix_h
#define LEDMatrix_h

#include "Arduino.h"

// Stałe protokołu
#define PROTOCOL_SOF 0xAA
#define PROTOCOL_EOF 0x55
#define PROTOCOL_MAX_PAYLOAD 150

// ID ekranu (domyślnie 1)
#define PROTOCOL_SCREEN_ID 1

// Komendy
#define CMD_LOAD_GIF 0x01
#define CMD_DISPLAY_TEXT 0x02
#define CMD_CLEAR_SCREEN 0x03
#define CMD_SET_BRIGHTNESS 0x04
#define CMD_GET_STATUS 0x05

class LEDMatrix {
public:
    // Konstruktor
    LEDMatrix(HardwareSerial &serial, uint8_t screenId = PROTOCOL_SCREEN_ID);
    
    // Inicjalizacja
    void begin(uint32_t baudrate = 1000000);
    
    // Podstawowe funkcje
    void clearScreen();
    void setBrightness(uint8_t brightness);
    
    // Wyświetlanie tekstu
    void displayText(const char* text, uint16_t x = 0, uint16_t y = 0, 
                     uint8_t fontSize = 2, 
                     uint8_t r = 255, uint8_t g = 255, uint8_t b = 255);
    
    // Ładowanie GIF
    void loadGif(const char* filename, uint16_t x = 0, uint16_t y = 0, 
                 uint16_t width = 96, uint16_t height = 96);
    
    // Funkcje pomocnicze
    void setScreenId(uint8_t screenId);
    uint8_t getScreenId() const;
    
private:
    HardwareSerial* _serial;
    uint8_t _screenId;
    
    // Wysyłanie pakietu
    void sendPacket(uint8_t command, const uint8_t* payload, uint8_t payloadLength);
    
    // Obliczanie sumy kontrolnej (XOR)
    uint8_t calculateChecksum(const uint8_t* data, uint8_t length);
    
    // Debug
    void printPacket(uint8_t command, const uint8_t* payload, uint8_t payloadLength);
};

#endif

