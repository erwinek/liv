/*
 * Minimal Test - Wyślij pojedynczy bajt do sprawdzenia komunikacji
 */

void setup() {
  Serial.begin(1000000);
  delay(3000); // Czekaj na led-image-viewer
  
  // Wyślij marker bajty
  Serial.write(0xAA); // SOF
  Serial.write(0x55); // EOF  
  Serial.flush();
}

void loop() {
  delay(1000);
}

