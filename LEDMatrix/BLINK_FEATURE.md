# Funkcja migania tekstu w bibliotece LEDMatrix

## Przegląd

Biblioteka LEDMatrix wspiera funkcję migania tekstu poprzez opcjonalny parametr `blinkIntervalMs` w funkcji `displayText()`.

## Parametr blinkIntervalMs

- **Typ:** `uint16_t`
- **Zakres:** 0-1000 milisekund
- **Domyślna wartość:** 0 (brak migania)
- **Opis:**
  - `0` - tekst nie miga (wyświetla się stale)
  - `1-1000` - częstotliwość migania w milisekundach (np. 500 = miga co 500ms)

## Użycie

### Przykład 1: Tekst bez migania (domyślny)

```cpp
#include <LEDMatrix.h>

LEDMatrix matrix(Serial);

void setup() {
    matrix.begin(1000000);
    
    // Tekst bez migania (standardowe użycie)
    matrix.displayText("Hello World", 0, 0, 
                      16,              // fontSize (nieużywany obecnie)
                      255, 0, 0,       // RGB (czerwony)
                      "ComicNeue-Regular-20.bdf",  // czcionka
                      1);              // elementId
}
```

### Przykład 2: Tekst z miganiem (500ms)

```cpp
#include <LEDMatrix.h>

LEDMatrix matrix(Serial);

void setup() {
    matrix.begin(1000000);
    
    // Tekst migający co 500ms
    matrix.displayText("ALERT!", 0, 0, 
                      16,              // fontSize
                      255, 0, 0,       // RGB (czerwony)
                      "ComicNeue-Regular-20.bdf",
                      1,               // elementId
                      500);            // blinkIntervalMs - miga co 500ms
}
```

### Przykład 3: Wiele elementów z różną częstotliwością migania

```cpp
#include <LEDMatrix.h>

LEDMatrix matrix(Serial);

void setup() {
    matrix.begin(1000000);
    
    // Element 1: Tekst stały
    matrix.displayText("Status:", 0, 0, 
                      16, 255, 255, 255,  // biały
                      "ComicNeue-Regular-16.bdf",
                      1,
                      0);  // nie miga
    
    // Element 2: Szybkie miganie (300ms)
    matrix.displayText("WARNING", 50, 0, 
                      16, 255, 165, 0,    // pomarańczowy
                      "ComicNeue-Regular-16.bdf",
                      2,
                      300);  // szybkie miganie
    
    // Element 3: Wolne miganie (1000ms)
    matrix.displayText("INFO", 0, 20, 
                      16, 0, 255, 0,      // zielony
                      "ComicNeue-Regular-16.bdf",
                      3,
                      1000); // wolne miganie
}
```

## Jak to działa

1. ESP32 wysyła komendę tekstową z parametrem `blinkIntervalMs`
2. Raspberry Pi odbiera komendę i zapisuje parametr migania
3. DisplayManager co kilka milisekund sprawdza czy minął czas migania
4. Jeśli tak, przełącza stan widoczności tekstu (visible/hidden)
5. Tekst jest rysowany tylko gdy `blink_visible == true`

## Struktura pakietu (TextCommand)

```
Offset | Rozmiar | Pole
-------|---------|------------------
0      | 1       | screen_id
1      | 1       | command
2      | 1       | element_id
3      | 2       | x_pos (little-endian)
5      | 2       | y_pos (little-endian)
7      | 1       | color_r
8      | 1       | color_g
9      | 1       | color_b
10     | 1       | text_length
11     | 32      | text
43     | 32      | font_name
75     | 2       | blink_interval_ms (little-endian)
-------|---------|------------------
Razem: 77 bajtów
```

## Uwagi

1. **Ograniczenie wartości:** Wartości powyżej 1000ms są automatycznie ograniczane do 1000ms
2. **Niezależność elementów:** Każdy element tekstowy może mieć własną częstotliwość migania
3. **Kompatybilność wsteczna:** Parametr jest opcjonalny (domyślnie 0), więc stary kod nadal działa
4. **Wydajność:** Miganie nie wpływa na wydajność - sprawdzanie odbywa się tylko co kilka ms
5. **Synchronizacja:** Każdy element ma własny timer, więc elementy migają niezależnie

## Przykładowe zastosowania

- **Alarmy/Ostrzeżenia:** Szybkie miganie (200-300ms) dla krytycznych alertów
- **Powiadomienia:** Średnie miganie (500ms) dla standardowych powiadomień
- **Status:** Wolne miganie (800-1000ms) dla informacji statusowych
- **Nagłówki:** Brak migania (0ms) dla stałych napisów

## Historia wersji

- **v1.1.0** - Dodano obsługę migania tekstu (2025-10-19)
- **v1.0.0** - Pierwsza wersja biblioteki

