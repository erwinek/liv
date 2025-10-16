# Szybka naprawa - Matryca pokazuje tylko ekran kontrolny

## Problem
Matryca LED pokazuje tylko ekran kontrolny (pusty ekran lub logo), zamiast wyświetlać tekst.

## Szybkie rozwiązanie (5 minut)

### Krok 1: Uruchom test diagnostyczny

1. Otwórz Arduino IDE
2. Otwórz: `File → Examples → LEDMatrix → DiagnosticTest`
3. Wgraj na ESP32
4. Otwórz Serial Monitor (115200 bps)

### Krok 2: Sprawdź wyniki

W Serial Monitor powinieneś zobaczyć:
```
[TEST 1] Serial1 Initialization
✓ Serial1 initialized

[TEST 2] Sending Simple Packet
Packet sent: AA 01 03 00 00 55
✓ Packet sent and flushed
```

**Jeśli widzisz te komunikaty:**
- ✅ Pakiet jest wysyłany
- ❌ Problem z połączeniem sprzętowym

**Jeśli NIE widzisz tych komunikatów:**
- ❌ Problem z kodem lub ESP32

### Krok 3: Sprawdź połączenia

```
ESP32                    LED Matrix
────                     ──────────
GPIO 17 (TX)  ────────>  RX
GND           ────────>  GND
```

**UWAGA:** 
- TX ESP32 → RX Matrycy (NIE TX→TX!)
- GND musi być wspólne

### Krok 4: Spróbuj innego baudrate

Jeśli połączenia są OK, spróbuj zmienić baudrate:

1. Otwórz: `File → Examples → LEDMatrix → InteractiveTest`
2. Wgraj na ESP32
3. Otwórz Serial Monitor (115200 bps)
4. Wpisz `4` (Change baudrate)
5. Wybierz `1` (115200 bps)
6. Wpisz `7` (Run all tests)

### Krok 5: Testuj różne baudrates

W InteractiveTest możesz łatwo testować różne baudrates:
- `4` → `1` = 115200 bps
- `4` → `2` = 230400 bps
- `4` → `3` = 460800 bps
- `4` → `4` = 921600 bps
- `4` → `5` = 1000000 bps

Po każdej zmianie baudrate:
- Wpisz `7` (Run all tests)
- Sprawdź czy matryca reaguje

## Najczęstsze przyczyny i rozwiązania

### 1. Złe połączenia (80% przypadków)

**Problem:** TX połączone z TX zamiast RX

**Rozwiązanie:**
```
❌ BŁĘDNIE:  ESP32 TX → Matrix TX
❌ BŁĘDNIE:  ESP32 RX → Matrix RX
✅ POPRAWNIE: ESP32 TX → Matrix RX
```

### 2. Zły baudrate (15% przypadków)

**Problem:** Matryca używa innego baudrate

**Rozwiązanie:**
- Spróbuj 115200 bps (najczęściej działa)
- Spróbuj 230400 bps
- Spróbuj 460800 bps
- Spróbuj 921600 bps
- Spróbuj 1000000 bps (domyślny)

### 3. Matryca nie jest włączona (3% przypadków)

**Problem:** Matryca nie jest zasilana

**Rozwiązanie:**
- Sprawdź czy matryca jest włączona
- Sprawdź zasilanie
- Sprawdź czy matryca pokazuje ekran startowy

### 4. Matryca potrzebuje resetu (2% przypadków)

**Problem:** Matryca jest w stanie zawieszenia

**Rozwiązanie:**
- Odłącz i podłącz zasilanie matrycy
- Naciśnij przycisk reset (jeśli jest)
- Poczekaj 10 sekund po włączeniu

## Test krok po kroku

### Test 1: Podstawowe połączenie

```cpp
void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial1.begin(115200); // Niski baudrate dla testu
    delay(500);
    
    // Wyślij prosty pakiet
    Serial1.write(0xAA); // SOF
    Serial1.write(0x01); // Screen ID
    Serial1.write(0x03); // CMD_CLEAR_SCREEN
    Serial1.write(0x00); // Payload length
    Serial1.write(0x00); // Checksum
    Serial1.write(0x55); // EOF
    Serial1.flush();
    
    Serial.println("Test packet sent!");
}

void loop() {
    // Nic nie rób
}
```

**Jeśli to działa:**
- ✅ Połączenie jest OK
- ✅ Baudrate jest OK
- Możesz używać biblioteki

**Jeśli to NIE działa:**
- ❌ Sprawdź połączenia
- ❌ Spróbuj innego baudrate

### Test 2: Test z biblioteką

```cpp
#include "LEDMatrix.h"

LEDMatrix matrix(Serial1);

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    matrix.begin(115200); // Niski baudrate
    delay(500);
    
    matrix.clearScreen();
    delay(500);
    
    matrix.setBrightness(80);
    delay(500);
    
    matrix.displayText("TEST", 0, 0, 2, 255, 0, 0);
}

void loop() {
    // Nic nie rób
}
```

## Checklist

Przed zgłoszeniem problemu sprawdź:

- [ ] ESP32 TX → Matrix RX (NIE TX→TX!)
- [ ] GND → GND
- [ ] Matryca jest włączona
- [ ] Matryca pokazuje ekran startowy
- [ ] Serial Monitor pokazuje pakiety [MTRX TX]
- [ ] Spróbowałeś różnych baudrates (115200, 230400, 460800, 921600, 1000000)
- [ ] Odłączyłeś i podłączyłeś wszystko ponownie
- [ ] Zasilanie matrycy jest stabilne

## Jeśli nic nie pomaga

1. Sprawdź dokumentację matrycy:
   - Jaki jest domyślny baudrate?
   - Czy potrzebuje specjalnej sekwencji inicjalizacji?
   - Czy ma tryb testowy?

2. Skontaktuj się z producentem matrycy

3. Sprawdź czy masz najnowszą wersję oprogramowania matrycy

## Dodatkowe zasoby

- `TROUBLESHOOTING.md` - Szczegółowy przewodnik rozwiązywania problemów
- `LEDMatrix/examples/DiagnosticTest/` - Test diagnostyczny
- `LEDMatrix/examples/InteractiveTest/` - Interaktywny test

---

**Powodzenia!** 🚀


