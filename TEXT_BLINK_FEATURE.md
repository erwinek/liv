# Funkcja Migania Tekstu

## 📋 Opis

Dodano funkcję migania tekstu do programu `led-image-viewer`. Tekst może migać z określoną częstotliwością od 1ms do 1000ms.

## 🔧 Jak działa

### Struktura TextCommand

Do struktury `TextCommand` dodano nowe pole:
```c
uint16_t blink_interval_ms;  // Częstotliwość migania w ms (0=brak, 1-1000=częstotliwość)
```

### Wartości parametru blink_interval_ms:

- **0** - Tekst nie miga (domyślnie, normalny tekst)
- **1-1000** - Tekst miga z podaną częstotliwością w milisekundach
  - 500ms = miganie co pół sekundy (2x na sekundę)
  - 1000ms = miganie co sekundę
  - 250ms = szybkie miganie (4x na sekundę)

## 📡 Protokół Szeregowy

### Format pakietu TextCommand:

```
Offset | Pole              | Typ      | Opis
-------|-------------------|----------|------------------------------
0      | screen_id         | uint8_t  | ID ekranu
1      | command           | uint8_t  | CMD_DISPLAY_TEXT (0x02)
2      | element_id        | uint8_t  | Unikalny ID elementu (0-255)
3-4    | x_pos             | uint16_t | Pozycja X
5-6    | y_pos             | uint16_t | Pozycja Y
7      | color_r           | uint8_t  | Kolor czerwony (0-255)
8      | color_g           | uint8_t  | Kolor zielony (0-255)
9      | color_b           | uint8_t  | Kolor niebieski (0-255)
10     | text_length       | uint8_t  | Długość tekstu
11-42  | text[32]          | char[]   | Tekst (max 32 znaki)
43-74  | font_name[32]     | char[]   | Nazwa czcionki
75-76  | blink_interval_ms | uint16_t | Częstotliwość migania (0-1000ms)
```

**Rozmiar struktury:** 77 bajtów

## 💻 Przykłady użycia

### ESP32 (Arduino) - z biblioteką LEDMatrix

**Najprostsze rozwiązanie - biblioteka LEDMatrix już zawiera wsparcie dla migania!**

```cpp
#include <LEDMatrix.h>

LEDMatrix matrix(Serial);  // Użyj Serial, Serial1, lub Serial2

void setup() {
    matrix.begin(1000000);  // 1Mbps
    
    // Przykład 1: Tekst migający co pół sekundy
    matrix.displayText("ALARM!", 50, 50, 
                      16,                              // fontSize (nieużywany)
                      255, 0, 0,                       // RGB (czerwony)
                      "ComicNeue-Regular-20.bdf",      // czcionka
                      1,                               // element_id
                      500);                            // blinkIntervalMs - miga co 500ms!
    
    // Przykład 2: Tekst migający szybko (250ms)
    matrix.displayText("WARNING", 50, 80, 
                      16, 255, 165, 0,                 // pomarańczowy
                      "ComicNeue-Regular-16.bdf",
                      2,
                      250);                            // szybkie miganie
    
    // Przykład 3: Tekst bez migania (parametr domyślny = 0)
    matrix.displayText("Normal Text", 50, 110, 
                      16, 255, 255, 255,               // biały
                      "ComicNeue-Regular-16.bdf",
                      3);                              // brak parametru = 0 = nie miga
}

void loop() {
    // Twoja logika aplikacji...
}
```

**Dokumentacja:** Zobacz `LEDMatrix/BLINK_FEATURE.md` dla więcej przykładów.

### ESP32 (Arduino) - bez biblioteki (bezpośredni protokół)

```cpp
#include "SerialProtocol.h"

void sendBlinkingText(const char* text, uint16_t x, uint16_t y, uint16_t blink_ms) {
    TextCommand cmd;
    cmd.screen_id = 0;
    cmd.command = CMD_DISPLAY_TEXT;
    cmd.element_id = 1;
    cmd.x_pos = x;
    cmd.y_pos = y;
    cmd.color_r = 255;
    cmd.color_g = 0;
    cmd.color_b = 0;  // Czerwony kolor
    cmd.text_length = strlen(text);
    strncpy(cmd.text, text, PROTOCOL_MAX_TEXT_LENGTH);
    strncpy(cmd.font_name, "fonts/ComicNeue-Regular-20.bdf", 32);
    cmd.blink_interval_ms = blink_ms;  // 500ms = miganie co pół sekundy
    
    // Wyślij pakiet przez UART
    sendPacket(CMD_DISPLAY_TEXT, (uint8_t*)&cmd, sizeof(TextCommand));
}

void setup() {
    Serial.begin(1000000);
    
    // Przykład 1: Tekst migający co pół sekundy
    sendBlinkingText("ALARM!", 50, 50, 500);
    
    // Przykład 2: Tekst migający szybko (250ms)
    sendBlinkingText("WARNING", 50, 80, 250);
    
    // Przykład 3: Tekst bez migania
    sendBlinkingText("Normal Text", 50, 110, 0);
}
```

### Python

```python
import serial
import struct

def send_blinking_text(ser, text, x, y, blink_ms):
    # Struktura: screen_id, command, element_id, x, y, r, g, b, text_len, text[32], font[32], blink_ms
    packet = struct.pack('<BBBHHBBBs32s32sH',
        0,                           # screen_id
        0x02,                        # CMD_DISPLAY_TEXT
        1,                           # element_id
        x, y,                        # pozycja
        255, 0, 0,                   # kolor (czerwony)
        len(text),                   # text_length
        text.encode('utf-8').ljust(32, b'\x00'),  # text
        b'fonts/ComicNeue-Regular-20.bdf\x00'.ljust(32, b'\x00'),  # font_name
        blink_ms                     # blink_interval_ms
    )
    
    # Oblicz checksum i wyślij z ramką protokołu
    # ... (kod do wysłania pakietu)

# Przykłady
ser = serial.Serial('/dev/ttyUSB0', 1000000)

# Tekst migający co sekundę
send_blinking_text(ser, "HELLO", 50, 50, 1000)

# Tekst migający szybko
send_blinking_text(ser, "FAST", 50, 80, 250)

# Tekst bez migania
send_blinking_text(ser, "STATIC", 50, 110, 0)
```

## 🎯 Przypadki użycia

### 1. Alarm/Ostrzeżenie
```cpp
sendBlinkingText("ALARM!", 50, 50, 500);  // Miganie co 0.5s
```

### 2. Powiadomienie
```cpp
sendBlinkingText("New Message", 10, 100, 1000);  // Miganie co 1s
```

### 3. Status krytyczny
```cpp
sendBlinkingText("ERROR", 80, 80, 250);  // Szybkie miganie (4x/s)
```

### 4. Migająca ramka czasowa
```cpp
// Migająca godzina
char time_str[32];
sprintf(time_str, "%02d:%02d:%02d", hour, minute, second);
sendBlinkingText(time_str, 60, 60, 1000);  // Miganie co sekundę
```

## ⚙️ Implementacja wewnętrzna

### Struktura DisplayElement

Dodano pola:
```cpp
uint16_t blink_interval_ms;  // Częstotliwość migania w ms
bool blink_visible;          // Aktualny stan widoczności
uint64_t last_blink_time;    // Czas ostatniego przełączenia
```

### Logika migania

1. **updateTextElement()** - sprawdza czy upłynął czas migania i przełącza `blink_visible`
2. **drawTextElement()** - sprawdza `blink_visible` i pomija rysowanie jeśli tekst jest ukryty
3. **Automatyczne odświeżanie** - display jest oznaczany jako dirty przy każdym migni
ęciu

## 📊 Wydajność

- **CPU:** Minimalne obciążenie - tylko sprawdzenie czasu co ramkę
- **Refresh rate:** Utrzymuje 30Hz przy wielu migających tekstach
- **Dokładność:** Timing bazuje na `getCurrentTimeUs()` (mikrosekundy)

## 🔍 Diagnostyka

W logach programu zobaczysz:
```
Processing TEXT command: ID=1 'ALARM!' with font: fonts/ComicNeue-Regular-20.bdf blink=500ms
Element ID=1 added. Total elements: 1 blink=500ms
```

## 📝 Uwagi

1. **Zakres wartości:** 0-1000ms. Wartości >1000ms zostaną zaakceptowane ale nie są zalecane
2. **Wiele elementów:** Każdy element tekstowy może mieć swoją własną częstotliwość migania
3. **Aktualizacja:** Wysłanie nowej komendy z tym samym element_id zmienia częstotliwość migania
4. **Wyłączanie:** Ustaw `blink_interval_ms=0` aby wyłączyć miganie dla istniejącego elementu

## 🆕 Zmiany w kodzie

### Pliki zmodyfikowane:

#### Raspberry Pi (led-image-viewer):
- `SerialProtocol.h` - dodano pole `blink_interval_ms` do `TextCommand`
- `DisplayManager.h` - dodano pola migania do `DisplayElement`, zaktualizowano sygnaturę `addTextElement()`
- `DisplayManager.cpp` - implementacja logiki migania w `updateTextElement()` i `drawTextElement()`

#### ESP32 (biblioteka LEDMatrix):
- `LEDMatrix/LEDMatrix.h` - dodano parametr `blinkIntervalMs` do funkcji `displayText()`
- `LEDMatrix/LEDMatrix.cpp` - zaktualizowano payload z 75 do 77 bajtów, dodano wysyłanie `blink_interval_ms`
- `LEDMatrix/BLINK_FEATURE.md` - nowa dokumentacja dla biblioteki LEDMatrix

### Kompatybilność wsteczna:
- **Biblioteka LEDMatrix:** Parametr `blinkIntervalMs` jest opcjonalny (domyślnie 0), stary kod nadal działa
- **Raspberry Pi:** Obsługuje zarówno stare pakiety (75 bajtów) jak i nowe (77 bajtów)
- **Zalecenie:** Użyj najnowszej wersji biblioteki LEDMatrix aby uzyskać pełne wsparcie dla migania

## 🐛 Rozwiązywanie problemów

### Tekst nie miga
- ✅ **NAPRAWIONE (2025-10-19):** Problem z wykrywaniem animowanej zawartości - tekst z miganiem nie był rozpoznawany jako animowany, przez co renderowanie było pomijane. Zobacz `BLINK_BUG_FIX.md` dla szczegółów.
- Sprawdź czy `blink_interval_ms > 0`
- Sprawdź logi: `blink=0ms` oznacza że miganie jest wyłączone
- Upewnij się że ESP32 wysyła poprawną strukturę (77 bajtów)
- Upewnij się że używasz najnowszej wersji `led-image-viewer` (po 2025-10-19)

### Miganie jest zbyt szybkie/wolne
- Dostosuj wartość `blink_interval_ms` (w milisekundach)
- 500ms = 2x na sekundę (typowo dla alarmów)
- 1000ms = 1x na sekundę (typowo dla notyfikacji)

### Tekst miga ale nie w odpowiednim tempie
- Sprawdź wydajność systemu
- Zbyt wiele elementów animowanych może wpłynąć na timing

---

**Data:** 2025-10-19  
**Wersja:** 1.0  
**Status:** ✅ Zaimplementowane i przetestowane

