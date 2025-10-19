/*
 * LEDMatrix Library - Alarm System Example
 * 
 * Ten przykład pokazuje system alarmowy z trzema stanami:
 * - NORMAL: Status OK (zielony, nie miga)
 * - WARNING: Ostrzeżenie (pomarańczowy, wolne miganie)
 * - ALARM: Alarm (czerwony, szybkie miganie)
 * 
 * Przełączanie stanów: przycisk podłączony do GPIO 0
 * 
 * Autor: LEDMatrix Library
 * Data: 2025-10-19
 */

#include <LEDMatrix.h>

// Utwórz obiekt LEDMatrix
LEDMatrix matrix(Serial);

// Pin przycisku (użyj przycisku BOOT na ESP32)
#define BUTTON_PIN 0

// Stany systemu
enum SystemState {
    STATE_NORMAL,
    STATE_WARNING,
    STATE_ALARM
};

SystemState currentState = STATE_NORMAL;
unsigned long lastButtonPress = 0;
const unsigned long DEBOUNCE_TIME = 200; // 200ms debounce

void setup() {
    // Inicjalizacja LEDMatrix
    matrix.begin(1000000);
    
    // Konfiguracja przycisku
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    
    delay(1000);
    
    // Wyświetl początkowy stan
    displayState(STATE_NORMAL);
}

void loop() {
    // Sprawdź przycisk (naciśnięcie = LOW)
    if (digitalRead(BUTTON_PIN) == LOW) {
        unsigned long now = millis();
        
        // Debouncing
        if (now - lastButtonPress > DEBOUNCE_TIME) {
            lastButtonPress = now;
            
            // Przełącz na następny stan
            currentState = (SystemState)((currentState + 1) % 3);
            displayState(currentState);
        }
    }
    
    delay(10);
}

// Wyświetl odpowiedni ekran dla danego stanu
void displayState(SystemState state) {
    // Wyczyść poprzedni stan
    matrix.clearScreen();
    
    // Nagłówek (zawsze bez migania)
    matrix.displayText("System Status", 5, 0,
                      16,
                      255, 255, 255,               // Biały
                      "ComicNeue-Bold-16.bdf",
                      1);
    
    switch (state) {
        case STATE_NORMAL:
            // Stan normalny - zielony, nie miga
            matrix.displayText("STATUS: OK", 10, 25,
                              16,
                              0, 255, 0,            // Zielony
                              "ComicNeue-Regular-20.bdf",
                              2);                   // Nie miga (domyślnie 0)
            
            matrix.displayText("All systems normal", 5, 50,
                              16,
                              255, 255, 255,        // Biały
                              "ComicNeue-Regular-16.bdf",
                              3);
            break;
            
        case STATE_WARNING:
            // Ostrzeżenie - pomarańczowy, wolne miganie
            matrix.displayText("WARNING", 15, 25,
                              16,
                              255, 165, 0,          // Pomarańczowy
                              "ComicNeue-Bold-20.bdf",
                              2,
                              800);                 // Wolne miganie (800ms)
            
            matrix.displayText("Check system", 10, 50,
                              16,
                              255, 255, 0,          // Żółty
                              "ComicNeue-Regular-16.bdf",
                              3);
            
            // Opcjonalnie: GIF ostrzeżenia
            // matrix.loadGif("warning.gif", 70, 25, 32, 32, 4);
            break;
            
        case STATE_ALARM:
            // Alarm - czerwony, szybkie miganie
            matrix.displayText("ALARM!", 25, 25,
                              16,
                              255, 0, 0,            // Czerwony
                              "ComicNeue-Bold-20.bdf",
                              2,
                              300);                 // Szybkie miganie (300ms)
            
            matrix.displayText("IMMEDIATE ACTION", 5, 50,
                              16,
                              255, 0, 0,            // Czerwony
                              "ComicNeue-Regular-16.bdf",
                              3,
                              500);                 // Miganie (500ms)
            
            // Opcjonalnie: GIF alarmu
            // matrix.loadGif("alarm.gif", 70, 25, 32, 32, 4);
            break;
    }
    
    // Informacja o przycisku (zawsze wyświetlana, nie miga)
    matrix.displayText("Press BOOT to change", 2, 80,
                      16,
                      128, 128, 128,                // Szary
                      "ComicNeue-Regular-12.bdf",
                      10);
}

