/*
 * LEDMatrix Library - Blinking Text Example
 * 
 * Ten przykład pokazuje jak używać funkcji migania tekstu:
 * - Tekst bez migania (normalny)
 * - Tekst z wolnym miganiem (1000ms)
 * - Tekst z szybkim miganiem (250ms)
 * - Tekst ze średnim miganiem (500ms)
 * 
 * Autor: LEDMatrix Library
 * Data: 2025-10-19
 */

#include <LEDMatrix.h>

// Utwórz obiekt LEDMatrix
LEDMatrix matrix(Serial);

void setup() {
    // Inicjalizacja z prędkością 1 Mbps
    matrix.begin(1000000);
    
    delay(1000);
    matrix.clearScreen();
    
    // 1. Nagłówek - tekst bez migania (domyślnie blinkIntervalMs = 0)
    matrix.displayText("Blink Demo", 5, 0,
                      16,
                      255, 255, 255,               // Biały
                      "ComicNeue-Bold-20.bdf",
                      1);                          // Nie miga (parametr blinkIntervalMs pominięty)
    
    // 2. Wolne miganie - co sekundę (1000ms)
    matrix.displayText("Slow", 5, 25,
                      16,
                      0, 255, 0,                   // Zielony
                      "ComicNeue-Regular-16.bdf",
                      2,
                      1000);                       // Miga co 1 sekundę
    
    // 3. Średnie miganie - co pół sekundy (500ms)
    matrix.displayText("Medium", 5, 45,
                      16,
                      255, 165, 0,                 // Pomarańczowy
                      "ComicNeue-Regular-16.bdf",
                      3,
                      500);                        // Miga co pół sekundy
    
    // 4. Szybkie miganie - co 250ms
    matrix.displayText("Fast", 5, 65,
                      16,
                      255, 0, 0,                   // Czerwony
                      "ComicNeue-Regular-16.bdf",
                      4,
                      250);                        // Szybkie miganie
    
    // 5. Bardzo szybkie miganie - co 100ms (dla efektu uwagi)
    matrix.displayText("ALERT!", 70, 25,
                      16,
                      255, 0, 255,                 // Magenta
                      "ComicNeue-Bold-20.bdf",
                      5,
                      100);                        // Bardzo szybkie miganie
}

void loop() {
    // Wszystkie efekty migania są obsługiwane automatycznie
    // przez led-image-viewer na Raspberry Pi
    
    delay(1000);
}

