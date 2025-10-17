/*
 * ESP32 Demo - Wyświetlanie tekstu na matrycy LED
 * 
 * Ten przykład pokazuje jak wyświetlić tekst "Esp32" 
 * na środku matrycy LED 96x96 przy użyciu biblioteki LEDMatrix.
 * 
 * Połączenie:
 * - ESP32 TX -> /dev/ttyUSB0 (do led-image-viewer)
 * - ESP32 RX <- /dev/ttyUSB0 (z led-image-viewer)
 * - Baud rate: 1000000 bps
 */

#include <LEDMatrix.h>

// Użyj Serial dla komunikacji z matrycą LED (/dev/ttyUSB0)
LEDMatrix matrix(Serial, 1);

bool commandsSent = false;
unsigned long lastSendMs = 0;

void setup() {
  // Inicjalizuj Serial z baud rate dla matrycy LED
  Serial.begin(1000000);
  delay(100);
  matrix.clearScreen();
  delay(500);
}

uint16_t counter = 0;

void loop() {
  // Co 5 sekund wyślij ponownie GIF + tekst, aby viewer mógł je przechwycić po restarcie
  unsigned long now = millis();
  if(counter<999) counter++;
  else counter = 111;
  
  // Wyświetl tekst z element_id=1 - będzie automatycznie aktualizowany bez migotania
  matrix.displayText(String(counter).c_str(), 60, 64, 2, 255, 255, 255, "fonts/ComicNeue-Bold-48.bdf", 1);
  
  if (!commandsSent || now - lastSendMs > 5000) {
    // Załaduj GIF z element_id=0
    matrix.loadGif("anim/1.gif", 0, 0, 192, 192, 0);
    commandsSent = true;
    lastSendMs = now;
  }
  delay(100);
}

