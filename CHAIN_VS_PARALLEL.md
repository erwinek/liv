# Chain Length vs Parallel - Wyjaśnienie
# Chain Length vs Parallel - Explanation

## Podstawowe pojęcia / Basic Concepts

### Chain Length (Długość łańcucha)
**Co to jest:** Liczba paneli połączonych **szeregowo** w jednym łańcuchu

**Jak to działa:**
```
GPIO → Panel 1 → Panel 2 → Panel 3 → ... → Panel N
       (data flows through all panels in sequence)
```

**Przykład:**
- 8 paneli pionowych = `chain_length = 8`
- 3 panele poziome = `chain_length = 3`

### Parallel (Równoległe łańcuchy)
**Co to jest:** Liczba **niezależnych łańcuchów** podłączonych równolegle do różnych pinów GPIO

**Jak to działa:**
```
GPIO Pin 1 → Chain 1 (Panel 1 → Panel 2 → Panel 3)
GPIO Pin 2 → Chain 2 (Panel 4 → Panel 5 → Panel 6)
GPIO Pin 3 → Chain 3 (Panel 7 → Panel 8 → Panel 9)
```

**Ograniczenia:**
- RPi obsługuje maksymalnie 3 równoległe łańcuchy (`parallel = 1, 2, or 3`)
- Każdy łańcuch wymaga osobnego zestawu pinów GPIO

## Przykłady konfiguracji

### ✅ Ekran 192x192 (9 paneli = 3×3)
```ini
rows = 64
cols = 64
chain_length = 3    # 3 panele poziomo w każdym łańcuchu
parallel = 3        # 3 łańcuchy pionowo
```

**Układ fizyczny:**
```
Chain 1: [Panel 1] → [Panel 2] → [Panel 3]
Chain 2: [Panel 4] → [Panel 5] → [Panel 6]
Chain 3: [Panel 7] → [Panel 8] → [Panel 9]
```

**Rozdzielczość:** 64×3 = 192px (width), 64×3 = 192px (height)

### ✅ Ekran 64x512 (8 paneli pionowo)
```ini
rows = 64
cols = 64
chain_length = 8    # 8 paneli w jednym łańcuchu
parallel = 1        # 1 łańcuch
pixel_mapper = V-mapper  # Mapuje łańcuch pionowo
```

**Układ fizyczny:**
```
[Panel 8] (góra)
    ↑
[Panel 7]
    ↑
[Panel 6]
    ↑
[Panel 5]
    ↑
[Panel 4]
    ↑
[Panel 3]
    ↑
[Panel 2]
    ↑
[Panel 1] (dół)
    ↑
  GPIO
```

**Rozdzielczość:** 64×1 = 64px (width), 64×8 = 512px (height)

### ✅ Ekran 256x64 (4 panele poziomo)
```ini
rows = 64
cols = 64
chain_length = 4    # 4 panele w jednym łańcuchu
parallel = 1        # 1 łańcuch
```

**Układ fizyczny:**
```
GPIO → [Panel 1] → [Panel 2] → [Panel 3] → [Panel 4]
```

**Rozdzielczość:** 64×4 = 256px (width), 64×1 = 64px (height)

### ❌ BŁĄD: parallel = 8
```ini
rows = 64
cols = 64
chain_length = 1
parallel = 8        # ❌ ZŁE! Maksimum to 3
```

**Błąd:** `Parallel outside usable range (1..3 allowed)`

**Rozwiązanie:** Użyj `chain_length = 8, parallel = 1` zamiast tego

## Kiedy używać chain_length, a kiedy parallel?

### Użyj `chain_length` gdy:
- ✅ Masz panele połączone **szeregowo** (jeden za drugim)
- ✅ Masz JEDEN łańcuch
- ✅ Wszystkie panele są podłączone do JEDNEGO pinu GPIO (plus masa i zasilanie)

### Użyj `parallel` gdy:
- ✅ Masz **wiele niezależnych łańcuchów**
- ✅ Każdy łańcuch jest podłączony do INNEGO pinu GPIO
- ✅ Maksymalnie 3 łańcuchy

## Wzór na obliczenie rozdzielczości

```
Szerokość = cols × chain_length
Wysokość = rows × parallel
```

**Przykład 1:** 3×3 panele
- cols = 64, rows = 64
- chain_length = 3, parallel = 3
- Rozdzielczość = 64×3 = 192px × 64×3 = 192px ✅

**Przykład 2:** 8 paneli pionowo
- cols = 64, rows = 64
- chain_length = 8, parallel = 1
- Rozdzielczość = 64×8 = 512px × 64×1 = 64px ❌ (źle!)
- **Z V-mapper:** 64×1 = 64px × 64×8 = 512px ✅ (dobrze!)

## Pixel Mapper i jego rola

### Bez pixel mapper
`chain_length × parallel` określa jak panele są **fizycznie połączone**

### Z pixel mapper (np. V-mapper)
Pixel mapper **przekształca** sposób w jaki dane są mapowane na panele

**V-mapper:**
- Bierze poziomy łańcuch i "obraca" go pionowo
- `chain_length = 8` z V-mapper = 8 paneli pionowo (nie poziomo)

## Typowe błędy

### Błąd 1: Za duża wartość parallel
```ini
parallel = 8  # ❌ Maksimum to 3!
```
**Rozwiązanie:** Użyj `chain_length` zamiast `parallel`

### Błąd 2: Zapomnienie o V-mapper dla pionowych
```ini
chain_length = 8
parallel = 1
# ❌ Brak pixel_mapper = V-mapper
```
**Efekt:** Obraz będzie rozciągnięty poziomo zamiast pionowo

### Błąd 3: Odwrotne wartości
```ini
# Dla 3×3 paneli
chain_length = 3
parallel = 3  # ✅ Dobrze

# Pomyłka:
chain_length = 9
parallel = 1  # ❌ To da 576×64, nie 192×192!
```

## Testowanie konfiguracji

1. **Uruchom aplikację:**
   ```bash
   sudo ./bin/led-image-viewer --config your_config.ini
   ```

2. **Sprawdź rozdzielczość w logach:**
   ```
   Total resolution: 64x512  # To powinno być poprawne
   ```

3. **Sprawdź ekran diagnostyczny:**
   - Zielona matryca + "ProGames"
   - Jeśli tekst jest nieczytelny lub zniekształcony → błędna konfiguracja

## Podsumowanie

| Układ | chain_length | parallel | pixel_mapper | Rozdzielczość |
|-------|--------------|----------|--------------|---------------|
| 3×3 panele | 3 | 3 | (brak) | 192×192 |
| 8 pionowo | 8 | 1 | V-mapper | 64×512 |
| 4 poziomo | 4 | 1 | (brak) | 256×64 |
| 2×2 panele | 2 | 2 | (brak) | 128×128 |
| 1 panel | 1 | 1 | (brak) | 64×64 |

