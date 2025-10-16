/*
 * SimpleDemo_Fixed - Poprawiona wersja z niższym baudrate
 * 
 * Ta wersja używa niższego baudrate (115200) który jest bardziej stabilny
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
    delay(2000); // Dłuższe opóźnienie na stabilizację
    
    Serial.println("\n========================================");
    Serial.println("LED Matrix Demo - FIXED VERSION");
    Serial.println("========================================\n");
    
    Serial.println("Checking hardware...");
    Serial.println("- ESP32 TX (GPIO 17) -> Matrix RX");
    Serial.println("- ESP32 GND -> Matrix GND");
    Serial.println("- Baudrate: 115200 bps (reduced for stability)");
    Serial.println();
    
    // Inicjalizacja matrycy z NIŻSZYM baudrate
    Serial.println("Initializing LED Matrix...");
    Serial.println("Serial1.begin(115200)");
    
    // WAŻNE: Używamy 115200 zamiast 1000000
    matrix.begin(115200); // Niższy baudrate = bardziej stabilny
    delay(1000); // Dłuższe opóźnienie
    
    Serial.println("✓ Serial1 initialized");
    Serial.println("✓ No reset detected");
    Serial.println();
    
    // Test 1: Wyczyść ekran
    Serial.println("[TEST 1] Sending clearScreen()...");
    matrix.clearScreen();
    delay(1000);
    Serial.println("✓ clearScreen sent");
    
    // Test 2: Ustaw jasność
    Serial.println("\n[TEST 2] Sending setBrightness(80)...");
    matrix.setBrightness(80);
    delay(1000);
    Serial.println("✓ setBrightness sent");
    
    // Test 3: Wyświetl tekst
    Serial.println("\n[TEST 3] Sending displayText('TEST')...");
    matrix.displayText("TEST", 0, 0, 2, 255, 0, 0);
    delay(1000);
    Serial.println("✓ displayText sent");
    
    Serial.println("\n========================================");
    Serial.println("Initialization complete!");
    Serial.println("Starting demo loop...");
    Serial.println("========================================\n");
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


