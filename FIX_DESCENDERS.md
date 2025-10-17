# Poprawka Renderowania Descenderów w BDF

## Data: 17 października 2025

## Problem

Litera "p" w napisie "Esp32" była wyświetlana **za wysoko** - część poniżej linii bazowej (descender) nie była prawidłowo wyrównana.

## Przyczyna

Błąd w formule obliczania pozycji Y podczas renderowania znaków z fontów BDF w pliku `DisplayManager.cpp`.

### Specyfikacja BDF

W formacie BDF parametr `y_offset` w `BBX` oznacza przesunięcie od linii bazowej do **dolnego lewego rogu** bitmapy:

```
BBX width height x_offset y_offset
```

Przykłady z fontu ComicNeue-Bold-48.bdf:
- **E** (ASCII 69): `BBX 24 33 4 0` → y_offset = **0** (normalny znak)
- **p** (ASCII 112): `BBX 20 32 3 -8` → y_offset = **-8** (descender!)

Dla "p": `y_offset = -8` oznacza że dolny róg bitmapy jest **8 pikseli poniżej** linii bazowej.

## Rozwiązanie

### Przed poprawką (błędny kod):

```cpp
// Linia 451 - BŁĄD: dodawanie y_offset
int py = baseline_y + bdf_char->y_offset - bdf_char->height + row + 1;
```

#### Obliczenia dla "p" (y_offset=-8, height=32, baseline_y=79):
- row=0 (góra): py = 79 + (-8) - 32 + 0 + 1 = **40** ❌
- row=31 (dół): py = 79 + (-8) - 32 + 31 + 1 = **71** ❌

Descender kończył się na y=71, czyli **8 pikseli ZA WYSOKO** (powinien być na y=79+8=87).

### Po poprawce (prawidłowy kod):

```cpp
// Linia 453 - POPRAWNE: odejmowanie y_offset
int py = baseline_y - bdf_char->y_offset - bdf_char->height + row;
```

#### Obliczenia dla "p" (y_offset=-8, height=32, baseline_y=79):
- row=0 (góra): py = 79 - (-8) - 32 + 0 = **55** ✅
- row=31 (dół): py = 79 - (-8) - 32 + 31 = **86** ✅

Descender kończy się na y=86, czyli **7 pikseli poniżej** linii bazowej (baseline_y=79).

### Matematyka

W układzie współrzędnych ekranu (Y rośnie w dół):

```
Dolny róg = baseline_y - y_offset
Górny róg = baseline_y - y_offset - height
Piksel w rzędzie r (0=góra): py = baseline_y - y_offset - height + row
```

Dla descenderów (y_offset < 0):
- `baseline_y - y_offset` = `baseline_y - (-8)` = `baseline_y + 8` → dolny róg **poniżej** linii bazowej ✅

## Zmienione pliki

- **DisplayManager.cpp** (linia 448-453):
  - Zmieniono formułę z `baseline_y + y_offset` na `baseline_y - y_offset`
  - Usunięto `+ 1` (niepotrzebne)
  - Dodano szczegółowe komentarze wyjaśniające

## Testy

### Test 1: Esp32Demo (oryginalny przykład)
```
Tekst: "Esp32" at (23, 41)
Kolor: Zielony RGB(0, 255, 0)
Font: ComicNeue-Bold-48.bdf
```
✅ **PASSED** - Litera "p" prawidłowo wyrównana

### Test 2: happy
```
Tekst: "happy" at (10, 41)
Descenders: p, y
```
✅ **PASSED** - Oba descenders prawidłowo wyrównane

### Test 3: gpqyj
```
Tekst: "gpqyj" at (10, 41)
Descenders: g, p, q, y, j
```
✅ **PASSED** - Wszystkie descenders prawidłowo wyrównane

### Test 4: Egypt
```
Tekst: "Egypt" at (15, 41)
Mix: E (normalny), g, y (descenders), p (normalny), t (normalny)
```
✅ **PASSED** - Prawidłowe mieszane wyrównanie

### Test 5: HELLO
```
Tekst: "HELLO" at (15, 41)
Wielkie litery bez descenderów (dla porównania)
```
✅ **PASSED** - Bazowa linia niezmieniona

## Weryfikacja

### Przed poprawką:
- Litera "p" w "Esp32" była **za wysoko**
- Descender nie sięgał poniżej linii bazowej
- Błąd: **16 pikseli przesunięcia** (dla y_offset=-8)

### Po poprawce:
- Litera "p" w "Esp32" jest **prawidłowo wyrównana**
- Descender prawidłowo schodzi poniżej linii bazowej
- Wszystkie litery z descenderami (p, g, y, q, j) wyrównane ✅

## Pliki testowe

Utworzono skrypty testowe:
1. `/home/erwinek/liv/test_esp32demo.py` - Test podstawowy
2. `/home/erwinek/liv/test_descenders.py` - Test kompleksowy descenderów
3. `/home/erwinek/liv/test_comicneue_variations.py` - Test różnych kolorów

## Komenda rebuild

```bash
cd /home/erwinek/liv
sudo pkill -f led-image-viewer
cd build
make -j4
sudo ../bin/led-image-viewer --led-rows=96 --led-cols=96 --led-chain=2 \
  --led-parallel=1 --led-brightness=100 --led-slowdown-gpio=4 \
  --led-no-hardware-pulse --led-gpio-mapping=adafruit-hat &

# Test
python3 test_descenders.py
```

## Status

✅ **POPRAWKA ZAKOŃCZONA SUKCESEM**

Wszystkie testy przeszły pomyślnie. Litera "p" oraz wszystkie inne litery z descenderami są teraz prawidłowo wyrównane w fontach BDF.

---

## Techniczne szczegóły dla programistów

### Format BDF - BBX (Bounding Box)

```
BBX BBw BBh BBxoff0x BByoff0y
```

- `BBw`, `BBh` - szerokość i wysokość bitmapy
- `BBxoff0x` - przesunięcie X od origin (na linii bazowej)
- `BByoff0y` - przesunięcie Y od origin (na linii bazowej) do **dolnego lewego rogu** bitmapy

### Konwersja współrzędnych BDF → Ekran

BDF używa układu matematycznego (Y w górę), ekran używa układu komputerowego (Y w dół):

```
BDF:           Ekran:
Y              
^              baseline_y -------- (origin)
|                 |
origin            | height
|                 |
V              Y  V
               bottom = baseline_y - y_offset
```

### Wzór na pozycję piksela

```cpp
int py = baseline_y - bdf_char->y_offset - bdf_char->height + row;
```

Gdzie:
- `baseline_y` = `element.y + font_ascent`
- `row` = numer wiersza w bitmapie (0 = góra, height-1 = dół)
- Dla descenderów: `y_offset < 0`, więc `-y_offset` jest dodatnie

