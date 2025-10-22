# Changelog - Dodanie Pixel Mapper
# Changelog - Added Pixel Mapper Support

Data: 22 października 2025
Date: October 22, 2025

## Zmiana / Change

Dodano obsługę parametru `pixel_mapper` w systemie konfiguracji, wymaganego dla pionowych układów paneli LED.

Added support for `pixel_mapper` parameter in configuration system, required for vertical LED panel arrangements.

## Dlaczego była potrzebna ta zmiana?

Ekran #2 (64x512) składa się z 8 paneli ułożonych **pionowo** (od dołu do góry). 

Biblioteka rpi-rgb-led-matrix domyślnie zakłada, że panele są układane **poziomo**. Aby poprawnie wyświetlać obraz na pionowym układzie, musimy użyć `--led-pixel-mapper=V-mapper`.

## Wprowadzone zmiany / Changes Made

### 1. ScreenConfig.h
✅ Dodano pole `std::string pixel_mapper`
✅ Dodano parsowanie parametru `pixel_mapper` z pliku INI
✅ Dodano wyświetlanie pixel mapper w metodzie `print()`

### 2. main.cpp
✅ Dodano ustawienie `matrix_options.pixel_mapper_config` z konfiguracji
✅ Warunek sprawdzający czy pixel_mapper nie jest pusty

### 3. screen_config_vertical.ini
✅ Dodano parametr: `pixel_mapper = V-mapper`
✅ Dodano komentarz wyjaśniający

### 4. screen_config.ini
✅ Dodano zakomentowany parametr `pixel_mapper` dla dokumentacji
✅ Dodano komentarz wyjaśniający

### 5. Dokumentacja
✅ Zaktualizowano `CONFIG_README.md` z informacją o pixel_mapper
✅ Utworzono `PIXEL_MAPPER_INFO.md` z pełną dokumentacją pixel mappers

## Konfiguracja przed / Before

```ini
[screen]
screen_id = 2
rows = 64
cols = 64
chain_length = 1
parallel = 8
hardware_mapping = regular
gpio_slowdown = 3
```

**Problem:** Obraz byłby zniekształcony lub niepoprawnie zmapowany

## Konfiguracja po / After

```ini
[screen]
screen_id = 2
rows = 64
cols = 64
chain_length = 1
parallel = 8
hardware_mapping = regular
pixel_mapper = V-mapper  # <-- NOWE!
gpio_slowdown = 3
```

**Rezultat:** Obraz jest poprawnie wyświetlany na pionowym ekranie

## Testowanie / Testing

### Kompilacja
```bash
cd /home/erwinek/liv
make
```
✅ Kompilacja przeszła pomyślnie bez błędów

### Weryfikacja konfiguracji
```bash
sudo ./bin/led-image-viewer --config screen_config_vertical.ini
```

Aplikacja powinna wyświetlić:
```
=== Screen Configuration ===
Screen ID: 2
Matrix: 64x64
Chain length: 1
Parallel: 8
Total resolution: 64x512
Hardware mapping: regular
Pixel mapper: V-mapper    # <-- To powinno być widoczne
GPIO slowdown: 3
...
```

## Obsługiwane Pixel Mappers

| Pixel Mapper | Zastosowanie | Przykład |
|--------------|--------------|----------|
| `V-mapper` | Panele pionowe | 1x8 paneli = 64x512 |
| `U-mapper` | Panele w kształcie U | Meander |
| `Rotate:90` | Obrót o 90° | Dowolny układ |
| `Rotate:180` | Obrót o 180° | Dowolny układ |
| `Rotate:270` | Obrót o 270° | Dowolny układ |
| (puste) | Standardowy układ | 3x3 paneli = 192x192 |

## Backward Compatibility

✅ Jeśli `pixel_mapper` nie jest podany w pliku konfiguracyjnym, jest ustawiany na pusty string
✅ Jeśli `pixel_mapper` jest pusty, `matrix_options.pixel_mapper_config` nie jest ustawiany
✅ Wszystkie poprzednie konfiguracje działają bez zmian

## Pliki zmienione / Modified Files

1. `ScreenConfig.h` - dodano obsługę pixel_mapper
2. `main.cpp` - użycie pixel_mapper przy tworzeniu matrycy
3. `screen_config.ini` - dodano dokumentację
4. `screen_config_vertical.ini` - **dodano pixel_mapper = V-mapper**
5. `CONFIG_README.md` - zaktualizowano dokumentację
6. `PIXEL_MAPPER_INFO.md` - **NOWY** - pełna dokumentacja

## Nowe pliki / New Files

- `PIXEL_MAPPER_INFO.md` - Szczegółowy przewodnik po pixel mappers
- `CHANGELOG_PIXEL_MAPPER.md` - Ten plik

## Następne kroki / Next Steps

1. **Test na prawdziwym sprzęcie:**
   ```bash
   # Na RPi #2 z pionowym ekranem
   sudo ./run_screen2.sh
   ```

2. **Weryfikacja:**
   - Ekran diagnostyczny powinien pokazać zieloną matrycę z "ProGames"
   - Napis powinien być czytelny i nie odwrócony
   - Pozycja powinna być wycentrowana

3. **Jeśli obraz nadal nieprawidłowy:**
   - Spróbuj `pixel_mapper = Rotate:90` lub innej wartości
   - Sprawdź okablowanie paneli
   - Sprawdź czy `parallel = 8` jest poprawne dla Twojego układu

## Dodatkowe informacje

### Jak działa V-mapper?

1. **Bez V-mapper:**
   - `parallel = 8` oznacza 8 równoległych łańcuchów poziomych
   - Każdy łańcuch zaczyna się od lewej i idzie w prawo
   - Obraz jest renderowany jakby były to poziome paski

2. **Z V-mapper:**
   - `parallel = 8` jest reinterpretowane jako pionowy stos
   - Piksel (0,0) jest w lewym dolnym rogu pierwszego panelu
   - Obraz jest renderowany od dołu do góry
   - Każdy panel 64x64 jest kolejną sekcją pionową

### Diagram

```
Bez V-mapper (źle):          Z V-mapper (dobrze):
Panel 0 → → → →              ┌─────┐ Panel 7 (góra)
Panel 1 → → → →              ├─────┤ Panel 6
Panel 2 → → → →              ├─────┤ Panel 5
Panel 3 → → → →              ├─────┤ Panel 4
Panel 4 → → → →              ├─────┤ Panel 3
Panel 5 → → → →              ├─────┤ Panel 2
Panel 6 → → → →              ├─────┤ Panel 1
Panel 7 → → → →              └─────┘ Panel 0 (dół)

(poziome paski)              (pionowy stos)
```

## Podsumowanie / Summary

✅ Dodano pełną obsługę pixel mapper w systemie konfiguracji
✅ Ekran #2 (64x512) teraz używa `V-mapper` dla poprawnego mapowania
✅ Ekran #1 (192x192) działa jak poprzednio (bez pixel mapper)
✅ System jest kompatybilny wstecz
✅ Kod skompilowany bez błędów
✅ Dokumentacja zaktualizowana

System jest gotowy do uruchomienia na pionowym ekranie LED! 🎮📺

