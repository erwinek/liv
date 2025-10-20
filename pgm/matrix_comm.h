#ifndef MATRIX_COMM_H
#define MATRIX_COMM_H

#include <Arduino.h>
#include <LEDMatrix.h>

// Typy komend dla matrycy LED
enum MatrixCommandType {
    MATRIX_CMD_LOAD_GIF,
    MATRIX_CMD_DISPLAY_TEXT,
    MATRIX_CMD_CLEAR_SCREEN,
    MATRIX_CMD_CLEAR_TEXT,
    MATRIX_CMD_DELETE_ELEMENT,
    MATRIX_CMD_SET_BRIGHTNESS
};

// Struktura komendy
struct MatrixCommand {
    MatrixCommandType type;
    
    // Parametry dla różnych komend
    union {
        struct {
            char filename[64];
            uint16_t x, y, width, height;
            uint8_t elementId;
        } gif;
        
        struct {
            char text[32];
            uint16_t x, y;
            uint8_t fontSize;
            uint8_t r, g, b;
            char fontName[32];
            uint8_t elementId;
            uint16_t blinkInterval;
        } text;
        
        struct {
            uint8_t elementId;
        } deleteElement;
        
        struct {
            uint8_t brightness;
        } brightness;
    };
};

// Handle dla high-priority task
extern TaskHandle_t MatrixCommTaskHandle;

// Inicjalizacja (wywołaj w setup())
void MatrixCommInit(LEDMatrix* matrix);

// Funkcje wysyłające komendy (non-blocking, używają kolejki)
void MatrixComm_LoadGif(const char* filename, uint16_t x, uint16_t y, 
                        uint16_t width, uint16_t height, uint8_t elementId);
void MatrixComm_DisplayText(const char* text, uint16_t x, uint16_t y,
                            uint8_t fontSize, uint8_t r, uint8_t g, uint8_t b,
                            const char* fontName, uint8_t elementId, 
                            uint16_t blinkInterval = 0);
void MatrixComm_ClearScreen();
void MatrixComm_ClearText();
void MatrixComm_DeleteElement(uint8_t elementId);
void MatrixComm_SetBrightness(uint8_t brightness);

#endif // MATRIX_COMM_H

