/*
 * LEDMatrix Library - Basic Usage Example
 * 
 * Ten przykład pokazuje podstawowe użycie biblioteki LEDMatrix:
 * - Inicjalizacja połączenia szeregowego
 * - Wyświetlanie prostego tekstu
 * - Ładowanie animowanego GIF-a
 * 
 * Autor: LEDMatrix Library
 * Data: 2025-10-19
 */

#include <LEDMatrix.h>

// Utwórz obiekt LEDMatrix używając Serial (USB)
// Możesz też użyć Serial1 lub Serial2 dla komunikacji przez GPIO
LEDMatrix matrix(Serial);

void setup() {
    // Inicjalizacja komunikacji szeregowej z prędkością 1 Mbps
    matrix.begin(1000000);
    
    // Krótkie opóźnienie na uruchomienie systemu
    delay(1000);
    
    // Wyczyść ekran przed rozpoczęciem
    matrix.clearScreen();
    
    // Wyświetl tekst powitalny
    // Parametry: tekst, x, y, fontSize, R, G, B, czcionka, elementId
    matrix.displayText("Hello World!", 10, 10, 
                      16,                          // fontSize (nieużywany)
                      255, 255, 255,               // Biały kolor (RGB)
                      "ComicNeue-Regular-20.bdf",  // Czcionka BDF
                      1);                          // Element ID = 1
    
    // Wyświetl drugi tekst w innym kolorze
    matrix.displayText("LED Matrix", 10, 35,
                      16,
                      0, 255, 0,                   // Zielony kolor
                      "ComicNeue-Regular-16.bdf",
                      2);                          // Element ID = 2
    
    // Załaduj animowany GIF (opcjonalnie)
    // Parametry: plik, x, y, szerokość, wysokość, elementId
    // matrix.loadGif("smile.gif", 0, 0, 32, 32, 3);
}

void loop() {
    // W tym przykładzie nie ma dynamicznych aktualizacji
    // Tekst pozostaje na ekranie
    
    delay(1000);
}

