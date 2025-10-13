# LEDMatrix Library

Biblioteka Arduino do sterowania matrycą LED z ESP32 za pomocą protokołu komunikacji szeregowej.

## Opis

Biblioteka LEDMatrix umożliwia łatwą komunikację z matrycą LED poprzez port szeregowy ESP32. Obsługuje następujące funkcje:

- Wyświetlanie tekstu z konfigurowalnymi parametrami (pozycja, rozmiar czcionki, kolor)
- Ładowanie i odtwarzanie animacji GIF
- Czyszczenie ekranu
- Kontrola jasności

## Instalacja

### Metoda 1: Instalacja ręczna

1. Pobierz bibliotekę i rozpakuj do folderu `libraries` w Twoim folderze Arduino:
   ```
   Arduino/libraries/LEDMatrix/
   ```

2. Uruchom ponownie Arduino IDE

### Metoda 2: Instalacja jako biblioteka lokalna

1. Skopiuj folder `LEDMatrix` do Twojego projektu
2. W Arduino IDE: Sketch → Include Library → Add .ZIP Library

## Szybki start

```cpp
#include "LEDMatrix.h"

// Utwórz obiekt matrycy (używając Serial1)
LEDMatrix matrix(Serial1, 1); // ID ekranu = 1

void setup() {
    // Inicjalizacja portu szeregowego
    matrix.begin(1000000); // Baudrate: 1 Mbps
    
    // Wyczyść ekran
    matrix.clearScreen();
    
    // Wyświetl tekst
    matrix.displayText("HELLO", 0, 0, 2, 255, 255, 0); // Żółty tekst
}

void loop() {
    // Twój kod
}
```

## API

### Konstruktor

```cpp
LEDMatrix(HardwareSerial &serial, uint8_t screenId = 1)
```

- `serial` - Referencja do portu szeregowego (np. Serial1)
- `screenId` - ID ekranu (domyślnie 1)

### Metody

#### begin()
```cpp
void begin(uint32_t baudrate = 1000000)
```
Inicjalizuje port szeregowy z określonym baudrate (domyślnie 1 Mbps).

#### clearScreen()
```cpp
void clearScreen()
```
Czyści ekran matrycy LED.

#### setBrightness()
```cpp
void setBrightness(uint8_t brightness)
```
Ustawia jasność ekranu (0-100).

#### displayText()
```cpp
void displayText(const char* text, uint16_t x = 0, uint16_t y = 0, 
                 uint8_t fontSize = 2, 
                 uint8_t r = 255, uint8_t g = 255, uint8_t b = 255)
```
Wyświetla tekst na ekranie.

Parametry:
- `text` - Tekst do wyświetlenia (max 31 znaków)
- `x` - Pozycja X (domyślnie 0)
- `y` - Pozycja Y (domyślnie 0)
- `fontSize` - Rozmiar czcionki (domyślnie 2)
- `r`, `g`, `b` - Składowe koloru RGB (0-255)

#### loadGif()
```cpp
void loadGif(const char* filename, uint16_t x = 0, uint16_t y = 0, 
             uint16_t width = 96, uint16_t height = 96)
```
Ładuje i wyświetla animację GIF.

Parametry:
- `filename` - Nazwa pliku GIF (max 63 znaki)
- `x`, `y` - Pozycja wyświetlania
- `width`, `height` - Rozmiar animacji

#### setScreenId()
```cpp
void setScreenId(uint8_t screenId)
```
Ustawia ID ekranu.

#### getScreenId()
```cpp
uint8_t getScreenId() const
```
Zwraca aktualne ID ekranu.

## Przykłady

### Przykład 1: Podstawowe wyświetlanie tekstu

```cpp
#include "LEDMatrix.h"

LEDMatrix matrix(Serial1);

void setup() {
    matrix.begin(1000000);
    matrix.clearScreen();
    
    // Wyświetl tekst na żółto
    matrix.displayText("HELLO", 0, 0, 2, 255, 255, 0);
}

void loop() {
    // Kod
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
    // Konwersja HSV na RGB
    uint8_t r = 255 * sin(hue * PI / 180);
    uint8_t g = 255 * sin((hue + 120) * PI / 180);
    uint8_t b = 255 * sin((hue + 240) * PI / 180);
    
    matrix.displayText("RAINBOW", 0, 0, 2, r, g, b);
    
    hue = (hue + 5) % 360;
    delay(50);
}
```

### Przykład 3: Ładowanie GIF

```cpp
#include "LEDMatrix.h"

LEDMatrix matrix(Serial1);

void setup() {
    matrix.begin(1000000);
    matrix.setBrightness(80);
    
    // Załaduj animację
    matrix.loadGif("anim/1.gif", 0, 0, 96, 96);
}

void loop() {
    // Kod
}
```

## Protokół komunikacji

Biblioteka używa następującego protokołu:

```
[SOF][ScreenID][Command][PayloadLength][Payload][Checksum][EOF]
```

- **SOF** (Start of Frame): 0xAA
- **ScreenID**: ID ekranu (domyślnie 1)
- **Command**: Typ komendy (0x01-0x05)
- **PayloadLength**: Długość danych (0-150)
- **Payload**: Dane komendy
- **Checksum**: Suma kontrolna XOR payload
- **EOF** (End of Frame): 0x55

### Dostępne komendy

- `0x01` - CMD_LOAD_GIF
- `0x02` - CMD_DISPLAY_TEXT
- `0x03` - CMD_CLEAR_SCREEN
- `0x04` - CMD_SET_BRIGHTNESS
- `0x05` - CMD_GET_STATUS

## Wymagania sprzętowe

- **Mikrokontroler**: ESP32
- **Port szeregowy**: HardwareSerial (Serial1, Serial2)
- **Baudrate**: 1000000 bps (1 Mbps)
- **Połączenie**: TX → RX matrycy, GND wspólne

## Rozwiązywanie problemów

### Problem: Brak komunikacji z matrycą

**Rozwiązanie:**
- Sprawdź połączenia (TX → RX, GND)
- Zweryfikuj baudrate (domyślnie 1000000)
- Sprawdź czy ID ekranu jest poprawne
- Upewnij się, że matryca jest zasilana

### Problem: Zniekształcony tekst

**Rozwiązanie:**
- Zmniejsz baudrate (np. 115200)
- Dodaj opóźnienia między komendami
- Sprawdź jakość połączenia

### Problem: Brak wyświetlania

**Rozwiązanie:**
- Wyczyść ekran: `matrix.clearScreen()`
- Sprawdź jasność: `matrix.setBrightness(80)`
- Zweryfikuj poprawność nazw plików GIF

## Debug

Biblioteka automatycznie wyświetla pakiety w formacie hex na Serial (115200 bps):

```
[MTRX TX] AA 01 02 28 00 00 00 00 02 FF FF 00 05 48 45 4C 4C 4F ...
```

## Licencja

Ten kod jest udostępniany na licencji MIT. Możesz go swobodnie używać i modyfikować.

## Autor

Generated - 2024

## Wersja

1.0.0

## Changelog

### v1.0.0 (2024)
- Pierwsza wersja biblioteki
- Podstawowe funkcje wyświetlania tekstu
- Obsługa GIF
- Kontrola jasności
- Czyszczenie ekranu

