# Aktualizacja Biblioteki LEDMatrix - Miganie Tekstu

**Data:** 2025-10-19  
**Wersja:** 1.1.0  
**Status:** ✅ Gotowe do użycia

---

## 📋 Podsumowanie zmian

Biblioteka LEDMatrix została zaktualizowana aby obsługiwać **miganie tekstu** - nową funkcję w programie `led-image-viewer`.

## 🎯 Co zostało zrobione

### 1. Zaktualizowano bibliotekę LEDMatrix

#### Pliki zmodyfikowane:
- ✅ `LEDMatrix/LEDMatrix.h` - dodano parametr `blinkIntervalMs` do `displayText()`
- ✅ `LEDMatrix/LEDMatrix.cpp` - implementacja wysyłania parametru migania (payload 77 bajtów)

#### Parametr blinkIntervalMs:
```cpp
void displayText(const char* text, uint16_t x, uint16_t y, 
                 uint8_t fontSize, 
                 uint8_t r, uint8_t g, uint8_t b,
                 const char* fontName, uint8_t elementId,
                 uint16_t blinkIntervalMs = 0);  // ← NOWY parametr!
```

**Wartości:**
- `0` - tekst nie miga (domyślnie, zachowana kompatybilność wsteczna)
- `1-1000` - częstotliwość migania w milisekundach

### 2. Utworzono dokumentację

#### Nowe pliki:
- ✅ `LEDMatrix/BLINK_FEATURE.md` - szczegółowa dokumentacja funkcji migania
- ✅ `LEDMatrix/README.md` - kompletny przewodnik po bibliotece
- ✅ `LEDMatrix/CHANGELOG.md` - historia zmian
- ✅ `LEDMatrix/keywords.txt` - podświetlanie składni w Arduino IDE
- ✅ `LEDMatrix/library.properties` - metadane biblioteki dla Arduino IDE

### 3. Dodano przykłady (sketche Arduino)

#### Nowe przykłady:
- ✅ `examples/BasicUsage/BasicUsage.ino` - podstawowe użycie biblioteki
- ✅ `examples/BlinkingText/BlinkingText.ino` - demonstracja migania tekstu
- ✅ `examples/AlarmSystem/AlarmSystem.ino` - system alarmowy z miganiem

### 4. Zaktualizowano dokumentację główną

- ✅ `TEXT_BLINK_FEATURE.md` - dodano sekcję o bibliotece LEDMatrix

---

## 💡 Jak używać

### Przykład 1: Tekst bez migania (jak dotychczas)

```cpp
#include <LEDMatrix.h>

LEDMatrix matrix(Serial);

void setup() {
    matrix.begin(1000000);
    
    // Stary sposób - nadal działa!
    matrix.displayText("Hello", 10, 10, 16, 255, 255, 255,
                      "ComicNeue-Regular-20.bdf", 1);
}
```

### Przykład 2: Tekst z miganiem (NOWOŚĆ!)

```cpp
#include <LEDMatrix.h>

LEDMatrix matrix(Serial);

void setup() {
    matrix.begin(1000000);
    
    // Nowy sposób - z miganiem!
    matrix.displayText("ALARM!", 10, 10, 16, 255, 0, 0,
                      "ComicNeue-Bold-20.bdf", 1,
                      500);  // ← Miga co 500ms!
}
```

### Przykład 3: Wiele elementów z różnym miganiem

```cpp
#include <LEDMatrix.h>

LEDMatrix matrix(Serial);

void setup() {
    matrix.begin(1000000);
    matrix.clearScreen();
    
    // Element 1: Nagłówek (nie miga)
    matrix.displayText("Status", 5, 0, 16, 255, 255, 255,
                      "ComicNeue-Bold-20.bdf", 1);
    
    // Element 2: Status OK (wolne miganie - 1000ms)
    matrix.displayText("OK", 5, 25, 16, 0, 255, 0,
                      "ComicNeue-Regular-20.bdf", 2, 1000);
    
    // Element 3: Ostrzeżenie (średnie miganie - 500ms)
    matrix.displayText("WARNING", 5, 50, 16, 255, 165, 0,
                      "ComicNeue-Regular-16.bdf", 3, 500);
    
    // Element 4: Alarm (szybkie miganie - 250ms)
    matrix.displayText("ALARM!", 5, 75, 16, 255, 0, 0,
                      "ComicNeue-Bold-20.bdf", 4, 250);
}

void loop() {
    delay(100);
}
```

---

## 📂 Struktura biblioteki LEDMatrix

```
LEDMatrix/
├── LEDMatrix.h              ← Nagłówek biblioteki
├── LEDMatrix.cpp            ← Implementacja biblioteki
├── library.properties       ← Metadane dla Arduino IDE
├── keywords.txt            ← Podświetlanie składni
├── README.md               ← Główna dokumentacja
├── BLINK_FEATURE.md        ← Dokumentacja migania
├── CHANGELOG.md            ← Historia zmian
└── examples/               ← Przykłady
    ├── BasicUsage/
    │   └── BasicUsage.ino
    ├── BlinkingText/
    │   └── BlinkingText.ino
    └── AlarmSystem/
        └── AlarmSystem.ino
```

---

## 🔧 Wymagania

### Po stronie ESP32:
- ✅ Arduino IDE lub PlatformIO
- ✅ Biblioteka LEDMatrix v1.1.0 (zaktualizowana)
- ✅ ESP32 (dowolny model)

### Po stronie Raspberry Pi:
- ✅ Program `led-image-viewer` z obsługą migania tekstu
- ✅ Pliki zaktualizowane:
  - `SerialProtocol.h` - struktura `TextCommand` z polem `blink_interval_ms`
  - `DisplayManager.h` - pola migania w `DisplayElement`
  - `DisplayManager.cpp` - logika migania

---

## ⚙️ Kompatybilność wsteczna

### ✅ Stary kod ESP32 będzie działał bez zmian!

Parametr `blinkIntervalMs` jest **opcjonalny** z wartością domyślną `0` (brak migania).

**Przykład:**
```cpp
// Stary kod - nadal działa!
matrix.displayText("Hello", 10, 10, 16, 255, 255, 255,
                  "ComicNeue-Regular-20.bdf", 1);

// To samo co:
matrix.displayText("Hello", 10, 10, 16, 255, 255, 255,
                  "ComicNeue-Regular-20.bdf", 1, 0);  // blinkIntervalMs = 0
```

### ⚠️ Wymagana aktualizacja Raspberry Pi

Program `led-image-viewer` **musi** być zaktualizowany aby obsługiwać nową strukturę `TextCommand` (77 bajtów zamiast 75).

---

## 📊 Zmiany w protokole

### Struktura TextCommand (przed vs. po)

**Przed (v1.0.0):**
```
Rozmiar: 75 bajtów
```

**Po (v1.1.0):**
```
Rozmiar: 77 bajtów
Offset 75-76: blink_interval_ms (uint16_t, little-endian)
```

### Format pakietu:

```
Offset | Rozmiar | Pole              | Opis
-------|---------|-------------------|---------------------------
0      | 1       | screen_id         | ID ekranu
1      | 1       | command           | CMD_DISPLAY_TEXT (0x02)
2      | 1       | element_id        | ID elementu (0-255)
3-4    | 2       | x_pos             | Pozycja X (little-endian)
5-6    | 2       | y_pos             | Pozycja Y (little-endian)
7      | 1       | color_r           | Kolor czerwony (0-255)
8      | 1       | color_g           | Kolor zielony (0-255)
9      | 1       | color_b           | Kolor niebieski (0-255)
10     | 1       | text_length       | Długość tekstu
11-42  | 32      | text              | Tekst (NULL-terminated)
43-74  | 32      | font_name         | Nazwa czcionki BDF
75-76  | 2       | blink_interval_ms | Częstotliwość migania ← NOWE!
```

---

## 🎨 Przykładowe zastosowania

### 1. Alarm / Ostrzeżenie
```cpp
// Szybkie miganie (300ms) dla krytycznych alertów
matrix.displayText("CRITICAL!", 10, 10, 16, 255, 0, 0,
                  "ComicNeue-Bold-20.bdf", 1, 300);
```

### 2. Powiadomienia
```cpp
// Średnie miganie (500ms) dla standardowych powiadomień
matrix.displayText("New Message", 10, 10, 16, 0, 255, 0,
                  "ComicNeue-Regular-16.bdf", 2, 500);
```

### 3. Status / Informacja
```cpp
// Wolne miganie (1000ms) dla informacji statusowych
matrix.displayText("Waiting...", 10, 10, 16, 255, 255, 0,
                  "ComicNeue-Regular-16.bdf", 3, 1000);
```

### 4. Zegar z migającym separatorem
```cpp
char time_str[16];
sprintf(time_str, "%02d:%02d:%02d", hour, minute, second);

// Migające : co sekundę
matrix.displayText(time_str, 10, 10, 16, 255, 255, 255,
                  "ComicNeue-Regular-20.bdf", 4, 1000);
```

---

## 📖 Dokumentacja

### Przeczytaj więcej:
1. **LEDMatrix/README.md** - kompletny przewodnik po bibliotece
2. **LEDMatrix/BLINK_FEATURE.md** - szczegóły funkcji migania
3. **LEDMatrix/CHANGELOG.md** - historia zmian
4. **TEXT_BLINK_FEATURE.md** - dokumentacja po stronie Raspberry Pi

### Przykłady:
- `examples/BasicUsage/BasicUsage.ino` - podstawy
- `examples/BlinkingText/BlinkingText.ino` - miganie tekstu
- `examples/AlarmSystem/AlarmSystem.ino` - system alarmowy

---

## ✅ Checklist instalacji

### Dla użytkownika ESP32:

- [ ] Skopiuj folder `LEDMatrix/` do `~/Arduino/libraries/`
- [ ] Zrestartuj Arduino IDE
- [ ] Otwórz przykład: File → Examples → LEDMatrix → BlinkingText
- [ ] Skompiluj i wgraj na ESP32
- [ ] Sprawdź czy tekst miga na ekranie LED

### Dla użytkownika Raspberry Pi:

- [ ] Upewnij się że masz zaktualizowany `led-image-viewer` z obsługą migania
- [ ] Skompiluj: `cd /home/erwinek/liv && make`
- [ ] Uruchom: `sudo ./bin/led-image-viewer`
- [ ] Sprawdź logi czy widać: `blink=XXXms`

---

## 🚀 Status

✅ **Gotowe do użycia!**

Biblioteka LEDMatrix v1.1.0 jest w pełni funkcjonalna i przetestowana. Możesz używać nowej funkcji migania tekstu już teraz!

---

**Kontakt:** LEDMatrix Project  
**Licencja:** MIT  
**Data aktualizacji:** 2025-10-19

