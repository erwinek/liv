# Pixel Mapper - Informacje
# Pixel Mapper - Information

## Co to jest Pixel Mapper?

Pixel mapper to funkcja biblioteki rpi-rgb-led-matrix, która mapuje współrzędne pikseli na fizyczne pozycje LEDów na panelach. Jest niezbędny w niektórych konfiguracjach, gdzie panele są ułożone w nietypowy sposób.

## Kiedy używać Pixel Mapper?

### V-mapper (Vertical Mapper)
**Zastosowanie:** Panele ułożone pionowo (od dołu do góry)

**Przykład:** 8 paneli 64x64px ułożonych pionowo = 64x512px

**Konfiguracja:**
```ini
chain_length = 1
parallel = 8
pixel_mapper = V-mapper
```

**Jak to działa:**
- Biblioteka rpi-rgb-led-matrix domyślnie zakłada, że panele są układane poziomo
- Gdy ustawisz `parallel = 8`, biblioteka spodziewa się 8 równoległych łańcuchów poziomych
- `V-mapper` zmienia sposób mapowania, tak aby panele były traktowane jako pionowy stos

### U-mapper (U-shaped Mapper)
**Zastosowanie:** Panele ułożone w kształcie litery U (meander)

**Przykład:** Łańcuch paneli idący w prawo, potem w dół, potem w lewo

### Rotate:X (Rotation Mapper)
**Zastosowanie:** Obrót całego obrazu o X stopni

**Dostępne wartości:**
- `Rotate:90` - obrót o 90° w prawo
- `Rotate:180` - obrót o 180°
- `Rotate:270` - obrót o 270° w prawo (90° w lewo)

## Przykłady konfiguracji

### Ekran pionowy 64x512 (8 paneli w jednym łańcuchu)
```ini
[screen]
screen_id = 2
rows = 64
cols = 64
chain_length = 8    # 8 paneli w łańcuchu
parallel = 1        # 1 łańcuch
pixel_mapper = V-mapper
```

### Ekran pionowy 128x256 (8 paneli w 2 łańcuchach po 4)
```ini
[screen]
screen_id = 3
rows = 64
cols = 64
chain_length = 4    # 4 panele w każdym łańcuchu
parallel = 2        # 2 równoległe łańcuchy
pixel_mapper = V-mapper
```

### Ekran standardowy 192x192 (9 paneli 3x3)
```ini
[screen]
screen_id = 1
rows = 64
cols = 64
chain_length = 3
parallel = 3
# pixel_mapper = (puste - nie potrzebne)
```

### Ekran obrócony o 90°
```ini
[screen]
rows = 64
cols = 64
chain_length = 3
parallel = 3
pixel_mapper = Rotate:90
```

## Jak sprawdzić czy potrzebujesz pixel mapper?

1. **Uruchom aplikację bez pixel mapper:**
   ```bash
   sudo ./bin/led-image-viewer --config your_config.ini
   ```

2. **Sprawdź ekran diagnostyczny:**
   - Powinieneś zobaczyć zieloną matrycę z napisem "ProGames"
   
3. **Jeśli obraz jest nieprawidłowy:**
   - Zniekształcony lub pocięty → spróbuj `V-mapper`
   - Do góry nogami → spróbuj `Rotate:180`
   - Obrócony → spróbuj `Rotate:90` lub `Rotate:270`

## Kombinowanie pixel mappers

Można łączyć pixel mappery używając dwukropka:

```ini
pixel_mapper = Rotate:90;V-mapper
```

To najpierw zastosuje V-mapper, a potem obróci o 90°.

## Debug

Jeśli pixel mapper nie działa:

1. Sprawdź logi przy starcie - aplikacja wyświetla:
   ```
   Pixel mapper: V-mapper
   ```

2. Spróbuj różnych wariantów:
   ```ini
   # Wariant 1
   pixel_mapper = V-mapper
   
   # Wariant 2
   pixel_mapper = Rotate:90
   
   # Wariant 3
   pixel_mapper = U-mapper
   ```

3. Sprawdź dokumentację biblioteki:
   ```bash
   cd /path/to/rpi-rgb-led-matrix
   ./examples-api-use/demo --help
   ```

## Więcej informacji

Oficjalna dokumentacja rpi-rgb-led-matrix:
https://github.com/hzeller/rpi-rgb-led-matrix

Pixel mappers:
https://github.com/hzeller/rpi-rgb-led-matrix#remapping-coordinates

