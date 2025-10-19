# Changelog - LEDMatrix Library

Wszystkie istotne zmiany w projekcie LEDMatrix są dokumentowane w tym pliku.

Format bazuje na [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
a numeracja wersji zgodna z [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.1.0] - 2025-10-19

### ✨ Dodano
- **Funkcja migania tekstu** - nowy parametr `blinkIntervalMs` w funkcji `displayText()`
  - Zakres: 0-1000 ms (0 = brak migania, 1-1000 = częstotliwość migania)
  - Każdy element tekstowy może mieć własną częstotliwość migania
  - Obsługa po stronie Raspberry Pi (led-image-viewer)
- Nowa dokumentacja `BLINK_FEATURE.md` opisująca funkcję migania
- Nowy przykład `BlinkingText.ino` pokazujący różne częstotliwości migania
- Nowy przykład `AlarmSystem.ino` demonstrujący system alarmowy z miganiem

### 🔧 Zmieniono
- Struktura `TextCommand` rozszerzona z 75 do 77 bajtów (dodano `blink_interval_ms`)
- Funkcja `displayText()` otrzymała opcjonalny parametr `blinkIntervalMs` (domyślnie 0)
- Zaktualizowano `README.md` z pełną dokumentacją nowej funkcji
- Zaktualizowano `library.properties` do wersji 1.1.0

### 🐛 Naprawiono
- Ograniczenie wartości `blinkIntervalMs` do 1000ms
- Prawidłowe kodowanie little-endian dla `blink_interval_ms` w payloadzie
- **KRYTYCZNE:** Miganie tekstu nie było rozpoznawane jako animowana zawartość w `DisplayManager::render()`, przez co miganie nie działało (dodano sprawdzenie `element.blink_interval_ms > 0` do wykrywania animowanej zawartości)

### 📚 Dokumentacja
- Rozszerzone README z sekcją o miganiu tekstu
- Nowy dokument `BLINK_FEATURE.md` z szczegółowym opisem funkcji
- Nowy plik `keywords.txt` dla podświetlania składni w Arduino IDE
- Zaktualizowane przykłady użycia

### ⚙️ Kompatybilność
- **Kompatybilność wsteczna:** TAK - parametr `blinkIntervalMs` jest opcjonalny
- **Wymaga aktualizacji led-image-viewer:** TAK - do wersji z obsługą migania
- Stary kod ESP32 będzie działał bez zmian (domyślnie `blinkIntervalMs = 0`)

## [1.0.0] - 2025-XX-XX

### 🎉 Pierwsza publiczna wersja

### ✨ Funkcje
- Wyświetlanie tekstu z czcionkami BDF
- Ładowanie i wyświetlanie animowanych GIF-ów
- Czyszczenie ekranu (wszystko lub tylko tekst)
- Ustawianie jasności ekranu (0-100%)
- Obsługa wielu ekranów (ID ekranu)
- Komunikacja przez UART do 1 Mbps
- Protokół z ramką SOF/EOF i sumą kontrolną XOR

### 📡 Protokół
- Format: `[SOF][SCREEN_ID][CMD][LEN][PAYLOAD][CHECKSUM][EOF]`
- SOF = 0xAA, EOF = 0x55
- Komendy: LOAD_GIF, DISPLAY_TEXT, CLEAR_SCREEN, SET_BRIGHTNESS, CLEAR_TEXT

### 📚 Dokumentacja
- Pełne README z API reference
- Przykłady: BasicUsage, SimpleDemo, AdvancedDemo
- Instrukcja instalacji (INSTALLATION.md)
- Quick start guide (QUICKSTART.md)

### 🔌 Hardware
- Wsparcę dla ESP32 (wszystkie modele)
- Serial, Serial1, Serial2
- Połączenie z Raspberry Pi przez UART

---

## Legenda

- ✨ `Dodano` - nowe funkcje
- 🔧 `Zmieniono` - zmiany w istniejących funkcjach
- ⚠️ `Deprecated` - funkcje oznaczone jako przestarzałe
- 🐛 `Naprawiono` - poprawki błędów
- 🗑️ `Usunięto` - usunięte funkcje
- 🔒 `Bezpieczeństwo` - poprawki bezpieczeństwa
- 📚 `Dokumentacja` - zmiany w dokumentacji
- ⚙️ `Kompatybilność` - informacje o kompatybilności

---

## Plan rozwoju (Roadmap)

### [1.2.0] - Planowane
- [ ] Obsługa gradientów kolorów
- [ ] Płynne przejścia między tekstami (fade in/out)
- [ ] Efekty przewijania poziomego/pionowego
- [ ] Obsługa większej ilości czcionek

### [1.3.0] - Planowane
- [ ] Obsługa obrazków statycznych (PNG, JPEG)
- [ ] Konfigurowalne efekty przejść
- [ ] Obsługa emoji i znaków Unicode

### [2.0.0] - W przyszłości
- [ ] Dwukierunkowa komunikacja (potwierdzenia)
- [ ] Bardziej zaawansowany protokół (CRC16 zamiast XOR)
- [ ] Obsługa kilku wyświetlaczy jednocześnie
- [ ] Web interface dla konfiguracji

