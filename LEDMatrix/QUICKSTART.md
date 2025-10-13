# Quick Start Guide - LEDMatrix Library

## Szybki start w 5 minut

### 1. Instalacja (2 minuty)

**Opcja A: Jako biblioteka**
```
1. Skopiuj folder LEDMatrix do Arduino/libraries/
2. Uruchom ponownie Arduino IDE
3. Gotowe!
```

**Opcja B: Standalone**
```
1. Otwórz plik LEDMatrix_Standalone.ino
2. Gotowe!
```

### 2. Połączenia (1 minuta)

```
ESP32 TX (GPIO 17) → RX matrycy
ESP32 GND         → GND matrycy
```

### 3. Pierwszy program (2 minuty)

```cpp
#include "LEDMatrix.h"

LEDMatrix matrix(Serial1);

void setup() {
    matrix.begin(1000000);
    matrix.clearScreen();
    matrix.displayText("HELLO", 0, 0, 2, 255, 255, 0);
}

void loop() {
    // Gotowe!
}
```

## Podstawowe funkcje

### Wyświetl tekst
```cpp
matrix.displayText("HELLO", x, y, fontSize, r, g, b);
```

### Wyczyść ekran
```cpp
matrix.clearScreen();
```

### Ustaw jasność
```cpp
matrix.setBrightness(80); // 0-100
```

### Załaduj GIF
```cpp
matrix.loadGif("anim/1.gif", x, y, width, height);
```

## Przykłady

### Przykład 1: Zmiana kolorów
```cpp
void loop() {
    matrix.displayText("RED", 0, 0, 2, 255, 0, 0);
    delay(1000);
    matrix.displayText("GREEN", 0, 0, 2, 0, 255, 0);
    delay(1000);
    matrix.displayText("BLUE", 0, 0, 2, 0, 0, 255);
    delay(1000);
}
```

### Przykład 2: Animacja
```cpp
uint8_t hue = 0;

void loop() {
    uint8_t r = 255 * sin(hue * PI / 180);
    uint8_t g = 255 * sin((hue + 120) * PI / 180);
    uint8_t b = 255 * sin((hue + 240) * PI / 180);
    
    matrix.displayText("RAINBOW", 0, 0, 2, r, g, b);
    
    hue = (hue + 5) % 360;
    delay(50);
}
```

## Parametry funkcji

### displayText()
```cpp
displayText(text, x, y, fontSize, r, g, b)
```
- `text` - Tekst (max 31 znaków)
- `x`, `y` - Pozycja (0-191)
- `fontSize` - Rozmiar czcionki (1-8)
- `r`, `g`, `b` - Kolor RGB (0-255)

### loadGif()
```cpp
loadGif(filename, x, y, width, height)
```
- `filename` - Nazwa pliku (max 63 znaki)
- `x`, `y` - Pozycja (0-191)
- `width`, `height` - Rozmiar (0-192)

## Debug

Otwórz Serial Monitor (115200 bps) aby zobaczyć wysyłane pakiety:

```
[MTRX TX] AA 01 02 28 00 00 00 00 02 FF FF 00 05 48 45 4C 4C 4F ...
```

## Rozwiązywanie problemów

| Problem | Rozwiązanie |
|---------|-------------|
| Brak komunikacji | Sprawdź połączenia TX→RX |
| Zniekształcony tekst | Zmniejsz baudrate lub dodaj delay() |
| Brak wyświetlania | Wywołaj clearScreen() i setBrightness(80) |

## Następne kroki

1. Przejrzyj przykłady w folderze `examples`
2. Przeczytaj pełną dokumentację w `README.md`
3. Eksperymentuj z różnymi parametrami

## Wsparcie

- Dokumentacja: `README.md`
- Instalacja: `INSTALLATION.md`
- Przykłady: `examples/`

Powodzenia! 🚀

