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

void loop() {
  // Co 5 sekund wyślij ponownie GIF + tekst, aby viewer mógł je przechwycić po restarcie
  unsigned long now = millis();
  if (!commandsSent || now - lastSendMs > 5000) {
    //matrix.clearScreen();
    delay(50);
    matrix.loadGif("anim/2.gif", 0, 0, 64, 64);
    delay(100);
    matrix.displayText("Esp32", 23, 41, 2, 0, 255, 0, "fonts/ComicNeue-Bold-48.bdf");
    commandsSent = true;
    lastSendMs = now;
  }
  delay(100);
}

