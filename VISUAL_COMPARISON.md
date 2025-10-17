# Wizualna Porównanie - Poprawka Descenderów

## Problem z literą "p" w "Esp32"

### ❌ PRZED POPRAWKĄ (Błędne wyrównanie)

```
Linia bazowa (Y=79) ────────────────────────────
     E    s    p    3    2
    ███  ███  ███  ███  ███
    █    █    █    █      █
    ███  ███  █    ███    █
    █      █  █      █  ███
    ███  ███  █    ███    █
                ↑
         Descender "p" NIE schodzi
         poniżej linii bazowej!
         (kończy się na Y=71)
```

**Problem:** Litera "p" była renderowana za wysoko, descender nie sięgał poniżej linii bazowej.

---

### ✅ PO POPRAWCE (Prawidłowe wyrównanie)

```
Linia bazowa (Y=79) ────────────────────────────
     E    s    p    3    2
    ███  ███  ███  ███  ███
    █    █    █    █      █
    ███  ███  █    ███    █
    █      █  █      █  ███
    ███  ███  █    ███    █
             ─█─
              █     ← Descender "p" PRAWIDŁOWO
             ─█─       schodzi poniżej linii
              █        bazowej! (do Y≈86)
             ─█─
```

**Rozwiązanie:** Litera "p" jest teraz prawidłowo wyrównana, descender schodzi poniżej linii bazowej.

---

## Szczegóły Techniczne

### Parametry Fontu ComicNeue-Bold-48.bdf

| Litera | ASCII | BBX | y_offset | Typ |
|--------|-------|-----|----------|-----|
| **E** | 69 | `24 33 4 0` | **0** | Normalny znak |
| **s** | 115 | `18 24 1 0` | **0** | Normalny znak |
| **p** | 112 | `20 32 3 -8` | **-8** | **Descender!** |
| **3** | 51 | `20 32 1 0` | **0** | Normalny znak |
| **2** | 50 | `20 32 2 0` | **0** | Normalny znak |

### Obliczenia dla y_offset = -8

#### ❌ Błędna formuła (przed poprawką):
```cpp
py = baseline_y + y_offset - height + row + 1
py = 79 + (-8) - 32 + row + 1
py = 40 + row
```
- Góra (row=0): y = 40
- Dół (row=31): y = 71 ← **ZA WYSOKO!**

#### ✅ Prawidłowa formuła (po poprawce):
```cpp
py = baseline_y - y_offset - height + row
py = 79 - (-8) - 32 + row
py = 55 + row
```
- Góra (row=0): y = 55
- Dół (row=31): y = 86 ← **PRAWIDŁOWO!**

### Różnica: 15 pikseli przesunięcia

---

## Inne Litery z Descenderami

Poprawka dotyczy wszystkich liter z descenderami:

| Litera | Nazwa | Descender |
|--------|-------|-----------|
| **p** | p | ✅ Poprawione |
| **g** | g | ✅ Poprawione |
| **y** | y | ✅ Poprawione |
| **q** | q | ✅ Poprawione |
| **j** | j | ✅ Poprawione |

### Test "happy" (2 descenders)

```
Linia bazowa ─────────────────────────────
    h    a    p    p    y
   ██   ███  ███  ███  ███
   ██   █ █  █    █      █
   ██   ███  █    █      █
   ██   █ █  █    █      █
   ██   █ █  █    █      █
        ─────█─────█─────█─
             █     █     █  ← Descenders "p", "p", "y"
             █     █    ██     schodzą poniżej linii!
```

### Test "gpqyj" (wszystkie descenders)

```
Linia bazowa ─────────────────────────────────────
    g    p    q    y    j
   ███  ███  ███  ███   ██
   █    █    █      █   ██
   ███  █    ███    █   ██
   █ █  █    █ █    █   ██
   ███  █    ███    █   ██
   ──█───█────█─────█───██─
     █   █    █     █   ██  ← Wszystkie descenders
    ██   █   ██    ██   ██     prawidłowo poniżej
   ███  ─█─  ███  ███  ───     linii bazowej!
```

---

## Weryfikacja Wizualna

Aby zweryfikować poprawkę wizualnie:

```bash
# Uruchom test descenderów
cd /home/erwinek/liv
python3 test_descenders.py
```

Powinno się zobaczyć na matrycy LED:
1. **"Esp32"** - litera "p" z descenderem poniżej linii
2. **"happy"** - litery "p" i "y" z descenderami
3. **"gpqyj"** - wszystkie descenders prawidłowo wyrównane
4. **"Egypt"** - mieszane: normalne i descenders
5. **"HELLO"** - wielkie litery bez descenderów (dla porównania)

---

## Wynik

✅ **Wszystkie litery z descenderami są teraz prawidłowo wyrównane!**

Litera "p" w napisie "Esp32" schodzi poniżej linii bazowej, tak jak powinno to być w prawidłowej typografii.

