# Przewodnik instalacji - LEDMatrix Library

## Opis

Biblioteka LEDMatrix umożliwia łatwą komunikację z matrycą LED poprzez port szeregowy ESP32.

## Metoda 1: Instalacja jako biblioteka Arduino

### Krok 1: Pobierz bibliotekę

Skopiuj folder `LEDMatrix` do katalogu bibliotek Arduino:

**Windows:**
```
C:\Users\[NazwaUżytkownika]\Documents\Arduino\libraries\LEDMatrix\
```

**macOS:**
```
~/Documents/Arduino/libraries/LEDMatrix/
```

**Linux:**
```
~/Arduino/libraries/LEDMatrix/
```

### Krok 2: Uruchom ponownie Arduino IDE

Zamknij i ponownie otwórz Arduino IDE.

### Krok 3: Otwórz przykład

1. Otwórz Arduino IDE
2. Przejdź do: **File → Examples → LEDMatrix → TestLEDMatrix**
3. Skompiluj i wgraj na ESP32

## Metoda 2: Użycie projektu standalone

Jeśli nie chcesz instalować biblioteki, możesz użyć projektu standalone:

### Krok 1: Otwórz plik

Otwórz plik `LEDMatrix_Standalone.ino` w Arduino IDE.

### Krok 2: Skompiluj i wgraj

1. Wybierz płytkę: **Tools → Board → ESP32 Dev Module**
2. Wybierz port: **Tools → Port → COMx** (Windows) lub `/dev/ttyUSB0` (Linux)
3. Kliknij **Upload**

## Konfiguracja sprzętowa

### Połączenia

```
ESP32          Matryca LED
------         -----------
GPIO 17 (TX) → RX
GND         → GND
VIN         → VIN (jeśli wymagane)
```

### Ustawienia portu szeregowego

- **Baudrate**: 1000000 bps (1 Mbps)
- **Parity**: None
- **Stop bits**: 1
- **Data bits**: 8

## Przykłady użycia

### Przykład 1: Podstawowy

```cpp
#include "LEDMatrix.h"

LEDMatrix matrix(Serial1);

void setup() {
    matrix.begin(1000000);
    matrix.clearScreen();
    matrix.displayText("HELLO", 0, 0, 2, 255, 255, 0);
}

void loop() {
    // Twój kod
}
```

### Przykład 2: Animacja kolorów

```cpp
#include "LEDMatrix.h"

LEDMatrix matrix(Serial1);
uint8_t hue = 0;

void setup() {
    matrix.begin(1000000);
}

void loop() {
    uint8_t r = 255 * sin(hue * PI / 180);
    uint8_t g = 255 * sin((hue + 120) * PI / 180);
    uint8_t b = 255 * sin((hue + 240) * PI / 180);
    
    matrix.displayText("RAINBOW", 0, 0, 2, r, g, b);
    
    hue = (hue + 5) % 360;
    delay(50);
}
```

## Rozwiązywanie problemów

### Problem: "LEDMatrix.h: No such file or directory"

**Rozwiązanie:**
- Upewnij się, że biblioteka jest w folderze `libraries`
- Uruchom ponownie Arduino IDE
- Sprawdź nazwę folderu (musi być dokładnie `LEDMatrix`)

### Problem: Brak komunikacji z matrycą

**Rozwiązanie:**
- Sprawdź połączenia (TX → RX)
- Zweryfikuj baudrate (1000000)
- Sprawdź czy matryca jest zasilana
- Upewnij się, że wybrałeś właściwy port szeregowy

### Problem: Zniekształcony tekst

**Rozwiązanie:**
- Zmniejsz baudrate (np. 115200)
- Dodaj opóźnienia między komendami
- Sprawdź jakość połączenia

### Problem: Błąd kompilacji

**Rozwiązanie:**
- Upewnij się, że używasz ESP32
- Zainstaluj ESP32 Board Support Package
- Sprawdź wersję Arduino IDE (minimum 1.8.0)

## Testowanie

Po zainstalowaniu biblioteki możesz przetestować ją używając przykładów:

1. **TestLEDMatrix** - Pełny test wszystkich funkcji
2. **SimpleDemo** - Prosty przykład animacji
3. **AdvancedDemo** - Zaawansowane funkcje

## Wsparcie

W razie problemów:
1. Sprawdź dokumentację w pliku `README.md`
2. Przejrzyj przykłady w folderze `examples`
3. Sprawdź Serial Monitor (115200 bps) dla debugowania

## Licencja

Ten kod jest udostępniany na licencji MIT.

## Wersja

1.0.0

