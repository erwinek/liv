/*
 * Przykład użycia biblioteki LEDMatrix z dwoma ekranami LED
 * 
 * Ten przykład pokazuje, jak kontrolować 2 ekrany LED jednocześnie
 * używając pojedynczej instancji biblioteki LEDMatrix.
 */

#include <LEDMatrix.h>

// Utworzenie instancji biblioteki
// Domyślny ekran to 1
LEDMatrix matrix(Serial1, 1);

void setup() {
    // Inicjalizacja portu szeregowego do debugowania
    Serial.begin(115200);
    
    // Inicjalizacja komunikacji z ekranami LED
    matrix.begin(1000000);  // 1Mbps
    
    delay(1000);
    
    Serial.println("=== Dual Screen LED Matrix Demo ===");
    
    // Wyczyść oba ekrany
    Serial.println("Czyszczenie ekranów...");
    matrix.clearScreen(1);  // Wyczyść ekran 1
    matrix.clearScreen(2);  // Wyczyść ekran 2
    
    delay(500);
    
    // Ustaw jasność dla obu ekranów
    Serial.println("Ustawianie jasności...");
    matrix.setBrightness(80, 1);  // 80% dla ekranu 1
    matrix.setBrightness(80, 2);  // 80% dla ekranu 2
    
    delay(500);
    
    // Wyświetl tekst na ekranie 1
    Serial.println("Wyświetlanie tekstu na ekranie 1...");
    matrix.displayText("Screen 1", 10, 10, 1, 255, 0, 0, "6x10", 1, 0, 1);
    
    delay(500);
    
    // Wyświetl tekst na ekranie 2
    Serial.println("Wyświetlanie tekstu na ekranie 2...");
    matrix.displayText("Screen 2", 10, 10, 1, 0, 255, 0, "6x10", 1, 0, 2);
    
    delay(500);
    
    // Załaduj GIF na ekran 1
    Serial.println("Ładowanie GIF na ekran 1...");
    matrix.loadGif("/gifs/icon.gif", 0, 30, 32, 32, 10, 1);
    
    delay(500);
    
    // Załaduj GIF na ekran 2
    Serial.println("Ładowanie GIF na ekran 2...");
    matrix.loadGif("/gifs/heart.gif", 0, 30, 32, 32, 10, 2);
    
    Serial.println("Setup zakończony!");
}

void loop() {
    // Przykład: Zmiana tekstu co 5 sekund
    static unsigned long lastUpdate = 0;
    static int counter = 0;
    
    if (millis() - lastUpdate > 5000) {
        lastUpdate = millis();
        counter++;
        
        // Aktualizuj licznik na ekranie 1
        char buffer1[32];
        snprintf(buffer1, sizeof(buffer1), "Count: %d", counter);
        matrix.displayText(buffer1, 10, 50, 1, 255, 255, 0, "6x10", 20, 0, 1);
        
        // Aktualizuj licznik na ekranie 2 (z opóźnieniem)
        char buffer2[32];
        snprintf(buffer2, sizeof(buffer2), "Count: %d", counter * 2);
        matrix.displayText(buffer2, 10, 50, 1, 0, 255, 255, "6x10", 20, 0, 2);
        
        Serial.print("Zaktualizowano oba ekrany. Counter: ");
        Serial.println(counter);
    }
}


