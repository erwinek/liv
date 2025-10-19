# LEDMatrix - Biblioteka Arduino dla ESP32

Biblioteka do sterowania matrycą LED za pomocą ESP32 i Raspberry Pi przez komunikację szeregową.

## 📦 Instalacja

### Arduino IDE

1. Skopiuj folder `LEDMatrix` do katalogu bibliotek Arduino:
   - Windows: `Documents\Arduino\libraries\`
   - Linux: `~/Arduino/libraries/`
   - macOS: `~/Documents/Arduino/libraries/`

2. Zrestartuj Arduino IDE

### PlatformIO

Dodaj do `platformio.ini`:
```ini
lib_deps = 
    file://LEDMatrix
```

## 🚀 Szybki start

```cpp
#include <LEDMatrix.h>

LEDMatrix matrix(Serial);  // Użyj Serial, Serial1, lub Serial2

void setup() {
    // Inicjalizacja komunikacji szeregowej (1 Mbps)
    matrix.begin(1000000);
    
    // Wyświetl tekst
    matrix.displayText("Hello World", 0, 0, 
                      16,                          // fontSize (obecnie nieużywany)
                      255, 255, 255,               // RGB - biały
                      "ComicNeue-Regular-20.bdf",  // czcionka BDF
                      1);                          // element_id
    
    // Załaduj animowany GIF
    matrix.loadGif("animated.gif", 0, 20, 64, 32, 2);
}

void loop() {
    // Twoja logika...
}
```

## 📚 API Reference

### Inicjalizacja

#### `LEDMatrix(HardwareSerial &serial, uint8_t screenId = 1)`
Konstruktor biblioteki.

**Parametry:**
- `serial` - Port szeregowy (Serial, Serial1, Serial2)
- `screenId` - ID ekranu (domyślnie 1)

**Przykład:**
```cpp
LEDMatrix matrix(Serial);     // Domyślny ID = 1
LEDMatrix matrix(Serial2, 5); // ID = 5
```

#### `void begin(uint32_t baudrate = 1000000)`
Inicjalizacja portu szeregowego.

**Parametry:**
- `baudrate` - Prędkość transmisji (domyślnie 1 Mbps)

**Przykład:**
```cpp
matrix.begin();          // 1 Mbps (domyślnie)
matrix.begin(115200);    // 115200 bps
```

### Wyświetlanie tekstu

#### `void displayText(...)`
Wyświetla tekst na matrycy LED.

```cpp
void displayText(const char* text,           // Tekst do wyświetlenia
                 uint16_t x, uint16_t y,     // Pozycja (x, y)
                 uint8_t fontSize,           // Rozmiar czcionki (nieużywany)
                 uint8_t r, uint8_t g, uint8_t b,  // Kolor RGB
                 const char* fontName,       // Nazwa czcionki BDF
                 uint8_t elementId,          // Unikalny ID elementu
                 uint16_t blinkIntervalMs = 0);  // Częstotliwość migania (opcjonalny)
```

**Parametry:**
- `text` - Tekst do wyświetlenia (max 31 znaków)
- `x, y` - Pozycja na ekranie
- `fontSize` - Rozmiar czcionki (obecnie nieużywany, zostaw 16)
- `r, g, b` - Kolor RGB (0-255 każdy)
- `fontName` - Nazwa pliku czcionki BDF (np. "ComicNeue-Regular-20.bdf")
- `elementId` - Unikalny ID elementu (0-255), używany do aktualizacji/usuwania
- `blinkIntervalMs` - **NOWE!** Częstotliwość migania w ms (0 = brak migania, 1-1000 = miga)

**Przykłady:**

```cpp
// Tekst bez migania
matrix.displayText("Status OK", 10, 10, 16, 0, 255, 0, 
                  "ComicNeue-Regular-16.bdf", 1);

// Tekst migający co pół sekundy (alarm)
matrix.displayText("ALARM!", 20, 20, 16, 255, 0, 0,
                  "ComicNeue-Regular-20.bdf", 2, 500);

// Tekst migający szybko (250ms)
matrix.displayText("WARNING", 30, 30, 16, 255, 165, 0,
                  "ComicNeue-Regular-16.bdf", 3, 250);
```

### Ładowanie GIF

#### `void loadGif(...)`
Ładuje i wyświetla animowany GIF.

```cpp
void loadGif(const char* filename,          // Nazwa pliku GIF
             uint16_t x, uint16_t y,        // Pozycja (x, y)
             uint16_t width, uint16_t height,  // Rozmiar
             uint8_t elementId);            // Unikalny ID elementu
```

**Parametry:**
- `filename` - Nazwa pliku GIF (max 63 znaki, ścieżka względna do katalogu `gifs/`)
- `x, y` - Pozycja na ekranie
- `width, height` - Rozmiar wyświetlanego GIF-a
- `elementId` - Unikalny ID elementu (0-255)

**Przykład:**
```cpp
// Załaduj animowany emotikon
matrix.loadGif("smile.gif", 0, 0, 32, 32, 10);

// Załaduj większą animację
matrix.loadGif("loading.gif", 50, 50, 64, 64, 11);
```

### Sterowanie ekranem

#### `void clearScreen()`
Czyści cały ekran (usuwa wszystkie elementy - tekst i GIF-y).

```cpp
matrix.clearScreen();
```

#### `void clearText()`
Czyści tylko elementy tekstowe, pozostawia GIF-y.

```cpp
matrix.clearText();
```

#### `void setBrightness(uint8_t brightness)`
Ustawia jasność ekranu.

**Parametry:**
- `brightness` - Jasność (0-100), gdzie 100 = maksymalna jasność

```cpp
matrix.setBrightness(50);   // 50% jasności
matrix.setBrightness(100);  // 100% jasności
```

### Pomocnicze

#### `void setScreenId(uint8_t screenId)`
Zmienia ID ekranu.

```cpp
matrix.setScreenId(2);  // Zmień na ekran 2
```

#### `uint8_t getScreenId() const`
Zwraca aktualny ID ekranu.

```cpp
uint8_t id = matrix.getScreenId();
```

## 🎨 Czcionki

Biblioteka używa czcionek BDF (Bitmap Distribution Format). Dostępne czcionki w katalogu `fonts/`:

- `ComicNeue-Regular-16.bdf` - 16px, styl regularny
- `ComicNeue-Regular-20.bdf` - 20px, styl regularny
- `ComicNeue-Bold-20.bdf` - 20px, pogrubiony
- (i inne...)

## 💡 Miganie tekstu (NOWOŚĆ v1.1.0)

Biblioteka obsługuje miganie tekstu poprzez parametr `blinkIntervalMs`:

```cpp
// Tekst bez migania (domyślnie)
matrix.displayText("Static", 0, 0, 16, 255, 255, 255, 
                  "ComicNeue-Regular-16.bdf", 1);

// Tekst migający co sekundę
matrix.displayText("Blink 1s", 0, 20, 16, 255, 0, 0,
                  "ComicNeue-Regular-16.bdf", 2, 1000);

// Tekst migający co pół sekundy
matrix.displayText("Blink 0.5s", 0, 40, 16, 0, 255, 0,
                  "ComicNeue-Regular-16.bdf", 3, 500);

// Szybkie miganie (250ms)
matrix.displayText("Fast blink", 0, 60, 16, 255, 165, 0,
                  "ComicNeue-Regular-16.bdf", 4, 250);
```

**Szczegóły:** Zobacz `BLINK_FEATURE.md` dla pełnej dokumentacji.

## 📡 Protokół komunikacji

### Format pakietu

```
[SOF] [SCREEN_ID] [COMMAND] [PAYLOAD_LEN] [PAYLOAD...] [CHECKSUM] [EOF]
```

- `SOF` = 0xAA (Start of Frame)
- `EOF` = 0x55 (End of Frame)
- `CHECKSUM` = XOR wszystkich bajtów payload

### Komendy

| Kod | Nazwa | Opis |
|-----|-------|------|
| 0x01 | LOAD_GIF | Załaduj i wyświetl GIF |
| 0x02 | DISPLAY_TEXT | Wyświetl tekst |
| 0x03 | CLEAR_SCREEN | Wyczyść cały ekran |
| 0x04 | SET_BRIGHTNESS | Ustaw jasność |
| 0x05 | GET_STATUS | Pobierz status (nieużywane) |
| 0x06 | CLEAR_TEXT | Wyczyść tylko tekst |

## 🔧 Przykłady

### Przykład 1: Prosty wyświetlacz statusu

```cpp
#include <LEDMatrix.h>

LEDMatrix matrix(Serial);

void setup() {
    matrix.begin(1000000);
    matrix.clearScreen();
    
    // Nagłówek
    matrix.displayText("System Status", 5, 0, 16, 255, 255, 255,
                      "ComicNeue-Bold-20.bdf", 1);
    
    // Status OK
    matrix.displayText("OK", 10, 25, 16, 0, 255, 0,
                      "ComicNeue-Regular-20.bdf", 2);
}

void loop() {
    delay(1000);
}
```

### Przykład 2: System alarmowy

```cpp
#include <LEDMatrix.h>

LEDMatrix matrix(Serial);

void showAlarm() {
    matrix.clearScreen();
    
    // Migający alarm
    matrix.displayText("ALARM!", 20, 20, 16, 255, 0, 0,
                      "ComicNeue-Bold-20.bdf", 1, 500);
    
    // Animowany GIF ostrzeżenia
    matrix.loadGif("warning.gif", 0, 50, 32, 32, 2);
}

void showNormal() {
    matrix.clearScreen();
    
    // Normalny status (nie miga)
    matrix.displayText("Status OK", 10, 20, 16, 0, 255, 0,
                      "ComicNeue-Regular-20.bdf", 1);
}

void setup() {
    matrix.begin(1000000);
}

void loop() {
    if (/* warunek alarmu */) {
        showAlarm();
    } else {
        showNormal();
    }
    delay(100);
}
```

### Przykład 3: Zegar z migającym separatorem

```cpp
#include <LEDMatrix.h>

LEDMatrix matrix(Serial);

void updateClock() {
    // Pobierz aktualny czas (z RTC lub innego źródła)
    int hour = 12;
    int minute = 34;
    int second = 56;
    
    char time_str[16];
    sprintf(time_str, "%02d:%02d:%02d", hour, minute, second);
    
    // Wyświetl czas z migającymi dwukropkami
    matrix.displayText(time_str, 10, 20, 16, 255, 255, 0,
                      "ComicNeue-Regular-20.bdf", 1, 1000);
}

void setup() {
    matrix.begin(1000000);
}

void loop() {
    updateClock();
    delay(1000);
}
```

### Przykład 4: Wiele elementów z różnymi efektami

```cpp
#include <LEDMatrix.h>

LEDMatrix matrix(Serial);

void setup() {
    matrix.begin(1000000);
    matrix.clearScreen();
    
    // Element 1: Nagłówek (nie miga)
    matrix.displayText("Dashboard", 5, 0, 16, 255, 255, 255,
                      "ComicNeue-Bold-16.bdf", 1);
    
    // Element 2: Status OK (wolne miganie)
    matrix.displayText("OK", 5, 20, 16, 0, 255, 0,
                      "ComicNeue-Regular-16.bdf", 2, 1000);
    
    // Element 3: Temperatura (nie miga)
    matrix.displayText("25C", 40, 20, 16, 255, 165, 0,
                      "ComicNeue-Regular-16.bdf", 3);
    
    // Element 4: Alert (szybkie miganie)
    matrix.displayText("ALERT", 5, 40, 16, 255, 0, 0,
                      "ComicNeue-Regular-16.bdf", 4, 300);
    
    // Element 5: Animowany GIF
    matrix.loadGif("loading.gif", 50, 35, 16, 16, 5);
}

void loop() {
    // Aktualizuj dane w razie potrzeby...
    delay(100);
}
```

## 🎯 Element ID

Każdy element (tekst lub GIF) musi mieć unikalny `elementId` (0-255). Jest to używane do:

1. **Aktualizacji** - wysłanie nowej komendy z tym samym ID zaktualizuje element
2. **Identyfikacji** - pozwala na zarządzanie konkretnymi elementami
3. **Usuwania** - możesz usunąć element wysyłając pustą komendę z danym ID

```cpp
// Utwórz element z ID=5
matrix.displayText("Hello", 10, 10, 16, 255, 255, 255,
                  "ComicNeue-Regular-16.bdf", 5);

// Zaktualizuj element z ID=5
matrix.displayText("World", 10, 10, 16, 255, 0, 0,
                  "ComicNeue-Regular-16.bdf", 5);
```

## 🔌 Podłączenie sprzętu

### ESP32 → Raspberry Pi

```
ESP32           Raspberry Pi
-----           ------------
GND     ----→   GND
TX      ----→   RX
RX      ----→   TX
```

### Porty szeregowe ESP32

- `Serial` - USB (do debugowania lub komunikacji przez USB)
- `Serial1` - GPIO 9/10 (RX/TX, domyślnie)
- `Serial2` - GPIO 16/17 (RX/TX, domyślnie)

**Przykład z Serial2:**
```cpp
LEDMatrix matrix(Serial2);

void setup() {
    matrix.begin(1000000);
    // Serial2 używa GPIO16 (RX) i GPIO17 (TX)
}
```

## ⚙️ Wymagania

- **ESP32** (dowolny model)
- **Arduino IDE** 1.8.x lub nowszy / **PlatformIO**
- **Raspberry Pi** z zainstalowanym `led-image-viewer`
- **Port szeregowy** 1 Mbps (domyślnie)

## 📝 Changelog

### v1.1.0 (2025-10-19)
- ✨ **NOWOŚĆ:** Dodano obsługę migania tekstu (`blinkIntervalMs`)
- 🔧 Zaktualizowano strukturę `TextCommand` z 75 do 77 bajtów
- 📚 Dodano dokumentację `BLINK_FEATURE.md`

### v1.0.0
- 🎉 Pierwsza publiczna wersja
- Podstawowe funkcje: displayText, loadGif, clearScreen, setBrightness

## 🐛 Rozwiązywanie problemów

### Nic się nie wyświetla
1. Sprawdź połączenie TX/RX (TX ESP32 → RX RPi)
2. Sprawdź czy `led-image-viewer` działa na RPi
3. Sprawdź baudrate (musi być 1 Mbps na obu stronach)

### Znaki specjalne się nie wyświetlają
1. Czcionki BDF mają ograniczony zestaw znaków
2. Użyj czcionek z pełnym wsparciem UTF-8

### Tekst nie miga
1. Sprawdź czy ustawiłeś `blinkIntervalMs > 0`
2. Sprawdź czy używasz najnowszej wersji biblioteki
3. Zobacz `BLINK_FEATURE.md` dla szczegółów

## 📄 Licencja

MIT License - używaj dowolnie!

## 👥 Autor

Generated with ❤️ for ESP32 LED Matrix projects

---

**Dokumentacja:** [README.md](README.md) | [BLINK_FEATURE.md](BLINK_FEATURE.md)  
**Wersja:** 1.1.0  
**Data:** 2025-10-19
