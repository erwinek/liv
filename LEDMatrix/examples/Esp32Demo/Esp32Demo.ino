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

void setup() {
  // Inicjalizuj Serial z baud rate dla matrycy LED
  Serial.begin(1000000);
  delay(100);
}

void loop() {
  // Wysyłaj komendy co 5 sekund
  if (!commandsSent || millis() % 5000 < 100) {
    matrix.clearScreen();
    delay(500);
    matrix.displayText("Esp32", 23, 41, 2, 0, 255, 0); // Zielony tekst
    commandsSent = true;
    delay(5000);
  }
  delay(100);
}

