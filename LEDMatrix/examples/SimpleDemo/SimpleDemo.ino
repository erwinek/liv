/*
 * SimpleDemo - Prosty przykład użycia biblioteki LEDMatrix
 * 
 * Ten przykład demonstruje podstawowe funkcje biblioteki:
 * - Wyświetlanie tekstu
 * - Zmiana kolorów
 * - Animacja
 */

#include "LEDMatrix.h"

// Utworzenie obiektu matrycy
LEDMatrix matrix(Serial1);

// Zmienne do animacji
unsigned long lastUpdate = 0;
uint8_t colorStep = 0;
uint8_t textIndex = 0;

// Tablica tekstów do wyświetlenia
const char* texts[] = {
    "HELLO",
    "WORLD",
    "ESP32",
    "LED",
    "MATRIX"
};

void setup() {
    // Inicjalizacja Serial do debugowania
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("Simple LED Matrix Demo");
    Serial.println("======================");
    
    // Inicjalizacja matrycy
    Serial.println("Initializing LED Matrix...");
    matrix.begin(1000000); // 1 Mbps
    delay(500);
    
    // Wyczyść ekran
    matrix.clearScreen();
    delay(500);
    
    // Ustaw jasność
    matrix.setBrightness(80);
    delay(500);
    
    Serial.println("Ready!");
}

void loop() {
    unsigned long currentTime = millis();
    
    // Aktualizuj co 2 sekundy
    if (currentTime - lastUpdate > 2000) {
        lastUpdate = currentTime;
        
        // Wybierz tekst
        const char* text = texts[textIndex];
        textIndex = (textIndex + 1) % 5;
        
        // Oblicz kolor (przejście przez kolory)
        uint8_t r = 255 * (sin(colorStep * PI / 180) + 1) / 2;
        uint8_t g = 255 * (sin((colorStep + 120) * PI / 180) + 1) / 2;
        uint8_t b = 255 * (sin((colorStep + 240) * PI / 180) + 1) / 2;
        
        // Wyświetl tekst
        Serial.print("Displaying: ");
        Serial.println(text);
        matrix.displayText(text, 0, 0, 2, r, g, b);
        
        // Zwiększ krok koloru
        colorStep = (colorStep + 10) % 360;
    }
    
    delay(10);
}

