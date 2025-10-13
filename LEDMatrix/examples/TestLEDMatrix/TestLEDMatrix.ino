/*
 * TestLEDMatrix - Przykładowy projekt testowy
 * Testowanie biblioteki LEDMatrix
 * 
 * Ten projekt demonstruje podstawowe funkcje biblioteki:
 * - Wyświetlanie tekstu
 * - Ładowanie animacji GIF
 * - Czyszczenie ekranu
 * - Zmiana jasności
 * 
 * Wymagania:
 * - ESP32
 * - Matryca LED z obsługą protokołu
 * - Połączenie serial (domyślnie Serial1)
 */

#include "LEDMatrix.h"

// Tworzenie obiektu matrycy LED
// Serial1 - port szeregowy (GPIO 9 - RX, GPIO 10 - TX)
LEDMatrix matrix(Serial1, 1); // ID ekranu = 1

// Zmienne pomocnicze
unsigned long lastUpdate = 0;
uint8_t testStep = 0;
uint8_t brightness = 80;

void setup() {
    // Inicjalizacja Serial do debugowania
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n\n========================================");
    Serial.println("LED Matrix Test Program");
    Serial.println("========================================\n");
    
    // Inicjalizacja matrycy LED
    Serial.println("Initializing LED Matrix...");
    matrix.begin(1000000); // Baudrate: 1 Mbps
    delay(500);
    
    Serial.println("LED Matrix initialized!");
    Serial.println("Starting test sequence...\n");
    
    // Ustaw jasność
    matrix.setBrightness(brightness);
    delay(500);
    
    // Wyczyść ekran
    matrix.clearScreen();
    delay(500);
}

void loop() {
    unsigned long currentTime = millis();
    
    // Zmiana testu co 3 sekundy
    if (currentTime - lastUpdate > 3000) {
        lastUpdate = currentTime;
        
        Serial.print("Test Step: ");
        Serial.println(testStep);
        
        switch (testStep) {
            case 0:
                // Test 1: Wyświetl tekst "HELLO"
                Serial.println("Test 1: Displaying 'HELLO'");
                matrix.displayText("HELLO", 0, 0, 2, 255, 255, 0); // Żółty
                break;
                
            case 1:
                // Test 2: Wyświetl tekst "WORLD"
                Serial.println("Test 2: Displaying 'WORLD'");
                matrix.displayText("WORLD", 0, 50, 2, 0, 255, 255); // Cyan
                break;
                
            case 2:
                // Test 3: Wyświetl tekst "ESP32"
                Serial.println("Test 3: Displaying 'ESP32'");
                matrix.displayText("ESP32", 0, 0, 3, 255, 0, 255); // Magenta
                break;
                
            case 3:
                // Test 4: Wyczyść ekran
                Serial.println("Test 4: Clearing screen");
                matrix.clearScreen();
                break;
                
            case 4:
                // Test 5: Załaduj animację GIF
                Serial.println("Test 5: Loading GIF animation");
                matrix.loadGif("anim/1.gif", 0, 0, 96, 96);
                break;
                
            case 5:
                // Test 6: Zmień jasność
                Serial.println("Test 6: Changing brightness");
                brightness = (brightness == 80) ? 50 : 80;
                matrix.setBrightness(brightness);
                break;
                
            case 6:
                // Test 7: Wyświetl tekst z różnymi kolorami
                Serial.println("Test 7: Multi-color text");
                matrix.displayText("RED", 0, 0, 2, 255, 0, 0); // Czerwony
                delay(1000);
                matrix.displayText("GREEN", 0, 0, 2, 0, 255, 0); // Zielony
                delay(1000);
                matrix.displayText("BLUE", 0, 0, 2, 0, 0, 255); // Niebieski
                break;
                
            case 7:
                // Test 8: Wyczyść i zacznij od nowa
                Serial.println("Test 8: Restarting test sequence");
                matrix.clearScreen();
                testStep = -1; // Zostanie zwiększone do 0
                break;
        }
        
        testStep++;
        if (testStep > 7) {
            testStep = 0;
        }
    }
    
    // Krótkie opóźnienie
    delay(10);
}

