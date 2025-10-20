/*
 * DeleteElement Example
 * 
 * Demonstruje użycie funkcji deleteElement() do precyzyjnego
 * zarządzania pojedynczymi elementami na ekranie.
 * 
 * Przykład pokazuje:
 * - Dodawanie wielu elementów z różnymi ID
 * - Selektywne usuwanie pojedynczych elementów
 * - Rotację komunikatów
 * - Dynamiczne menu
 */

#include <LEDMatrix.h>

// Utwórz obiekt matrycy LED (używając Serial, screen_id=1)
LEDMatrix matrix(Serial, 1);

// Element IDs
#define BACKGROUND_GIF   0
#define TITLE_TEXT       1
#define MESSAGE_TEXT     2
#define STATUS_TEXT      3
#define MENU_CURSOR      10

void setup() {
    // Inicjalizacja Serial @ 1Mbps
    matrix.begin(1000000);
    delay(100);
    
    // Przykład 1: Podstawowe użycie
    basicExample();
    delay(5000);
    
    // Przykład 2: Rotujące komunikaty
    rotatingMessages();
    delay(2000);
    
    // Przykład 3: Dynamiczne menu
    dynamicMenu();
}

void loop() {
    // Możesz dodać własną logikę tutaj
    delay(100);
}

// PRZYKŁAD 1: Podstawowe dodawanie i usuwanie elementów
void basicExample() {
    matrix.clearScreen();
    
    // Dodaj tło (GIF)
    matrix.loadGif("anim/background.gif", 0, 0, 192, 192, BACKGROUND_GIF);
    
    // Dodaj tytuł
    matrix.displayText("DELETE DEMO", 30, 20, 2, 255, 255, 0, 
                      "fonts/9x18B.bdf", TITLE_TEXT);
    
    // Dodaj 3 komunikaty
    matrix.displayText("Message 1", 40, 60, 2, 255, 0, 0, 
                      "fonts/9x18B.bdf", 2);
    delay(1000);
    
    matrix.displayText("Message 2", 40, 90, 2, 0, 255, 0, 
                      "fonts/9x18B.bdf", 3);
    delay(1000);
    
    matrix.displayText("Message 3", 40, 120, 2, 0, 0, 255, 
                      "fonts/9x18B.bdf", 4);
    delay(2000);
    
    // Usuń środkowy komunikat (ID=3)
    matrix.deleteElement(3);
    delay(1500);
    
    // Usuń pierwszy komunikat (ID=2)
    matrix.deleteElement(2);
    delay(1500);
    
    // Usuń ostatni komunikat (ID=4)
    matrix.deleteElement(4);
    delay(500);
    
    // Tytuł i GIF nadal widoczne!
}

// PRZYKŁAD 2: Rotujące komunikaty
void rotatingMessages() {
    matrix.clearScreen();
    
    const char* messages[] = {
        "READY...",
        "SET...",
        "GO!"
    };
    
    for(int i = 0; i < 3; i++) {
        // Usuń poprzedni komunikat (jeśli istnieje)
        if(i > 0) {
            matrix.deleteElement(MESSAGE_TEXT);
        }
        
        // Wyświetl nowy komunikat z tym samym ID
        matrix.displayText(messages[i], 60, 90, 2, 255, 255, 0, 
                          "fonts/9x18B.bdf", MESSAGE_TEXT);
        
        delay(1000);
    }
    
    // Usuń ostatni komunikat
    matrix.deleteElement(MESSAGE_TEXT);
}

// PRZYKŁAD 3: Dynamiczne menu z kursorem
void dynamicMenu() {
    matrix.clearScreen();
    
    // Wyświetl tytuł
    matrix.displayText("MENU", 70, 10, 2, 255, 255, 0, 
                      "fonts/9x18B.bdf", TITLE_TEXT);
    
    // Wyświetl opcje menu (stałe)
    matrix.displayText("Start Game", 30, 50, 2, 255, 255, 255, 
                      "fonts/9x18B.bdf", 20);
    matrix.displayText("Settings", 30, 80, 2, 255, 255, 255, 
                      "fonts/9x18B.bdf", 21);
    matrix.displayText("Exit", 30, 110, 2, 255, 255, 255, 
                      "fonts/9x18B.bdf", 22);
    
    // Symulacja poruszania się po menu (w prawdziwej aplikacji: reakcja na przyciski)
    int selectedOption = 0;  // 0=Start Game, 1=Settings, 2=Exit
    int menuPositions[] = {50, 80, 110};  // Y pozycje opcji
    
    for(int i = 0; i < 6; i++) {  // 6 ruchów dla demonstracji
        // Usuń stary kursor
        matrix.deleteElement(MENU_CURSOR);
        
        // Wyświetl kursor przy wybranej opcji
        matrix.displayText(">", 10, menuPositions[selectedOption], 2, 
                          255, 255, 0, "fonts/9x18B.bdf", MENU_CURSOR);
        
        delay(800);
        
        // Przejdź do następnej opcji (cyklicznie)
        selectedOption = (selectedOption + 1) % 3;
    }
    
    // Symulacja wyboru (opcja "Exit")
    matrix.deleteElement(MENU_CURSOR);
    selectedOption = 2;  // Exit
    matrix.displayText(">", 10, menuPositions[selectedOption], 2, 
                      0, 255, 0, "fonts/9x18B.bdf", MENU_CURSOR, 300);  // Migający
    
    delay(2000);
}

/*
 * DODATKOWE PRZYKŁADY UŻYCIA:
 * 
 * 1. Powiadomienia tymczasowe:
 *    matrix.displayText("Coin!", 50, 150, 2, 255, 128, 0, "fonts/9x18B.bdf", 100, 300);
 *    delay(2000);
 *    matrix.deleteElement(100);
 * 
 * 2. Countdown:
 *    for(int i = 3; i > 0; i--) {
 *        matrix.deleteElement(50);
 *        matrix.displayText(String(i).c_str(), 90, 90, 2, 255, 0, 0, 
 *                          "fonts/ComicNeue-Bold-48.bdf", 50);
 *        delay(1000);
 *    }
 * 
 * 3. Status indicator:
 *    // Wyświetl status
 *    matrix.displayText("Connecting...", 40, 140, 2, 255, 255, 0, "fonts/9x18B.bdf", 99);
 *    
 *    // Po nawiązaniu połączenia
 *    matrix.deleteElement(99);
 *    matrix.displayText("Connected!", 40, 140, 2, 0, 255, 0, "fonts/9x18B.bdf", 99);
 */

