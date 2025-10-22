# Changelog - System Konfiguracji Ekranów
# Changelog - Screen Configuration System

Data: 22 października 2025
Date: October 22, 2025

## Dodane funkcje / Added Features

### 1. System konfiguracji ekranów przez pliki INI
- Nowy plik `ScreenConfig.h` - parser plików konfiguracyjnych INI
- Wsparcie dla różnych rozmiarów i konfiguracji ekranów LED
- Dynamiczne ustawianie rozdzielczości ekranu

### 2. Dwa przykładowe pliki konfiguracyjne

#### `screen_config.ini` - Ekran #1 (192x192, ID=1)
- 9 paneli 64x64px (3×3)
- Rozdzielczość: 192×192 pikseli
- Domyślna konfiguracja

#### `screen_config_vertical.ini` - Ekran #2 (64x512, ID=2)
- 8 paneli 64x64px ułożonych pionowo (1×8)
- Rozdzielczość: 64×512 pikseli
- Konfiguracja dla ekranu pionowego

### 3. Skrypty uruchomieniowe
- `run_screen1.sh` - uruchomienie ekranu #1
- `run_screen2.sh` - uruchomienie ekranu #2

### 4. Dokumentacja
- `CONFIG_README.md` - kompletny przewodnik konfiguracji

## Zmiany w kodzie / Code Changes

### main.cpp
- Dodano parsowanie argumentu `--config <plik>`
- Dynamiczne ładowanie konfiguracji z pliku
- Wyświetlanie szczegółów konfiguracji przy starcie
- Przekazywanie portu szeregowego z konfiguracji do DisplayManager

### DisplayManager.h / DisplayManager.cpp
- Zmiana `SCREEN_WIDTH` i `SCREEN_HEIGHT` ze stałych na zmienne
- Dynamiczne ustawianie rozmiaru ekranu na podstawie matrycy
- Dodanie parametru `serial_port` do metody `init()`
- Aktualizacja `getStatus()` do używania dynamicznych wymiarów

### ScreenConfig.h (NOWY)
- Klasa `ScreenConfig` z parserem INI
- Wsparcie dla wszystkich parametrów konfiguracyjnych:
  - `screen_id` - ID ekranu (1-255)
  - `rows`, `cols` - wymiary pojedynczego panelu
  - `chain_length`, `parallel` - konfiguracja matrycy
  - `hardware_mapping` - mapowanie sprzętowe
  - `gpio_slowdown` - opóźnienie GPIO
  - `serial_port`, `serial_baudrate` - konfiguracja portu szeregowego
  - `show_diagnostics` - ekran testowy przy starcie

## Ekran diagnostyczny / Diagnostic Screen

### Zmieniony wygląd
- **Przed:** kolorowe kwadraty w rogach, biała ramka, tekst "LED"
- **Teraz:** pełna zielona matryca z napisem "ProGames" na środku
  - Działa na dowolnej rozdzielczości (tekst automatycznie centrowany)
  - Obsługa ekranów 192×192, 64×512 i innych

## Kompatybilność wsteczna / Backward Compatibility

✅ Aplikacja działa bez pliku konfiguracyjnego (używa domyślnych ustawień)
✅ Zachowana kompatybilność z poprzednimi argumentami linii poleceń
✅ Flaga `--no-diagnostics` nadal działa

## Użycie / Usage

### Uruchomienie z konfiguracją
```bash
# Ekran #1 (192x192)
sudo ./bin/led-image-viewer --config screen_config.ini

# Ekran #2 (64x512) 
sudo ./bin/led-image-viewer --config screen_config_vertical.ini

# Lub za pomocą skryptów
sudo ./run_screen1.sh
sudo ./run_screen2.sh
```

### Uruchomienie bez konfiguracji (domyślne ustawienia)
```bash
sudo ./bin/led-image-viewer
```

### Wyłączenie ekranu diagnostycznego
```bash
sudo ./bin/led-image-viewer --config screen_config.ini --no-diagnostics
```

## Scenariusz dwuekranowy / Dual Screen Scenario

System obsługuje 2 ekrany LED na 2 Raspberry Pi 4:

**RPi #1:**
- Ekran 192×192 (3×3 panele)
- ID=1
- Port: /dev/ttyUSB0
- Komenda: `sudo ./run_screen1.sh`

**RPi #2:**
- Ekran 64×512 (1×8 paneli pionowo)
- ID=2  
- Port: /dev/ttyUSB0
- Komenda: `sudo ./run_screen2.sh`

Każdy ekran ma unikalny ID, dzięki czemu ESP32 może je rozróżnić podczas komunikacji.

## Testowanie / Testing

System został pomyślnie skompilowany:
```
✅ main.cpp - zaktualizowany i skompilowany
✅ DisplayManager.cpp - zaktualizowany i skompilowany
✅ DisplayManager.h - zaktualizowany i skompilowany
✅ ScreenConfig.h - nowy plik, skompilowany
✅ Brak błędów kompilacji
```

## Dalsze kroki / Next Steps

1. Przetestować na prawdziwym sprzęcie:
   - Uruchomić z `screen_config.ini` na RPi z matrycą 192×192
   - Uruchomić z `screen_config_vertical.ini` na RPi z matrycą 64×512

2. Dostosować parametry jeśli potrzeba:
   - `gpio_slowdown` - jeśli obraz jest zniekształcony
   - `hardware_mapping` - jeśli układ paneli jest inny

3. Zaktualizować ESP32 aby rozpoznawało `screen_id` w komunikacji

## Pliki / Files

### Nowe pliki
- `ScreenConfig.h` - parser konfiguracji
- `screen_config.ini` - konfiguracja ekranu #1
- `screen_config_vertical.ini` - konfiguracja ekranu #2
- `CONFIG_README.md` - dokumentacja
- `run_screen1.sh` - skrypt uruchomieniowy ekranu #1
- `run_screen2.sh` - skrypt uruchomieniowy ekranu #2
- `CHANGELOG_CONFIG_SYSTEM.md` - ten plik

### Zmodyfikowane pliki
- `main.cpp` - dodano obsługę konfiguracji
- `DisplayManager.cpp` - dynamiczne wymiary ekranu, konfigurowalny port szeregowy
- `DisplayManager.h` - zmieniony interfejs
- `DisplayManager.cpp::addDiagnosticElements()` - nowy wygląd ekranu testowego

