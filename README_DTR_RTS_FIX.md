# Fix dla problemu restartu ESP32 - Podsumowanie

## 📋 Problem który został rozwiązany

Gdy ESP32 się restartuje, ekran LED się nie odświeża. Na konsoli widać:
```
Buffer size: 418, calling processBuffer()
No SOF found in buffer, clearing 418 bytes of garbage
```

**Przyczyny:**
1. ESP32 wysyła garbage data podczas bootowania (418+ bajtów)
2. GPIO0 był w stanie LOW → ESP32 wchodził w bootloader mode
3. Brak automatycznego przesyłania stanu wyświetlacza po restarcie

## ✅ Rozwiązania które zostały wdrożone

### 0. Usunięcie CLOCAL - kluczowa poprawka! ⚠️

**Problem:** Flaga `CLOCAL` w konfiguracji portu szeregowego **blokowała kontrolę DTR/RTS**!

`CLOCAL` = "ignore modem control lines" - system ignorował nasze `ioctl()` wywołania.

**Rozwiązanie:**
```c
tty.c_cflag &= ~CLOCAL;  // Don't ignore modem control lines!
```

Teraz DTR/RTS działają poprawnie! Zobacz `CLOCAL_PROBLEM.md` dla szczegółów.

### 1. Poprawiona sekwencja DTR/RTS (direct logic!)

**Kluczowa zmiana:** **Ta płytka** ma **bezpośrednią logikę** (bez inwersji):
- `RTS=HIGH` → `GPIO0=HIGH` → ✅ **Normal Mode**
- `RTS=LOW` → `GPIO0=LOW` → Bootloader Mode

⚠️ **Uwaga:** To różni się od typowych komercyjnych płytek (NodeMCU, DevKit), które mają odwróconą logikę!

**Nowa sekwencja:**
```cpp
1. RTS=HIGH (50ms)    → Ustaw GPIO0=HIGH PRZED resetem
2. DTR=LOW (100ms)    → Trzymaj ESP32 w reset
3. DTR=HIGH           → Puść reset, ESP32 widzi GPIO0=HIGH
4. Czekaj 1 sekundę   → ESP32 bootuje się w normal mode
```

### 2. Wykrywanie restartu ESP32

- Automatyczne wykrywanie >200 bajtów garbage data
- Grace period 2 sekundy (ignorowanie wszystkich danych)
- Agresywne czyszczenie UART buffers

### 3. Skrypt testowy

Nowy plik `test_dtr_rts.sh` do testowania sekwencji DTR/RTS bez uruchamiania pełnego programu.

## 🚀 Jak używać

### Krok 1: Test podstawowy

**Opcja A - Szybki test (zamyka port po 10 sekundach):**
```bash
cd /home/erwinek/liv
sudo ./test_dtr_rts.sh /dev/ttyUSB0
```

**Opcja B - Reset i trzymaj (POLECANE!):**
```bash
sudo ./reset_esp32_and_hold.sh /dev/ttyUSB0
```
Ten skrypt:
- Wykonuje pełną sekwencję reset
- Trzyma port otwarty przez 30 sekund
- ESP32 pozostaje w normal mode
- ✅ **Możesz zmierzyć GPIO0 multimetrem podczas działania skryptu**

⚠️ **Ważne:** Po zamknięciu portu, DTR/RTS wracają do LOW! Dlatego główny program trzyma port otwarty.

### Krok 2: Uruchom program
```bash
make
sudo ./bin/led-image-viewer
```

Powinieneś zobaczyć (z nowymi logami debugowania):
```
Serial config: Hardware flow control OFF, Modem control lines ENABLED
Initializing ESP32 reset sequence (DIRECT logic - no inversion)...
Setting RTS=HIGH (GPIO0 will be HIGH for normal mode)
Current modem status before RTS change: 0x...
RTS requested: HIGH, actual: HIGH (status=0x...)  ← ✅ Weryfikacja!
Asserting reset (DTR=LOW, EN=LOW)
DTR set to LOW
Releasing reset (DTR=HIGH, EN=HIGH) - ESP32 starting in normal mode
DTR set to HIGH
Keeping RTS=HIGH during boot (GPIO0=HIGH)
Final state: DTR=HIGH (EN=HIGH), RTS=HIGH (GPIO0=HIGH)
Waiting for ESP32 to boot...
ESP32 should be running in normal mode
```

**Ważne:** Sprawdź czy widzisz `RTS requested: HIGH, actual: HIGH` - to oznacza że RTS faktycznie jest ustawione!

### Krok 3: Testuj restart
1. Wyślij komendę wyświetlenia
2. Wciśnij przycisk RESET na ESP32
3. Sprawdź czy pojawi się: `ESP32 RESTART DETECTED`

## 📁 Pliki które zostały zmodyfikowane/utworzone

| Plik | Typ | Opis |
|------|-----|------|
| `SerialProtocol.h` | Kod | Dodano metody `setDTR()`, `setRTS()`, `initESP32ResetSequence()` |
| `SerialProtocol.cpp` | Kod | ⚠️ Usunięto CLOCAL + implementacja sekwencji i wykrywania restartu |
| `ESP32_RESTART_FIX.md` | Docs | Szczegółowa dokumentacja techniczna |
| `QUICK_START_DTR_RTS.md` | Docs | Przewodnik szybkiego startu |
| `DTR_RTS_LOGIC_NOTE.md` | Docs | Wyjaśnienie DIRECT vs INVERTED logic |
| `CLOCAL_PROBLEM.md` | Docs | ⚠️ Wyjaśnienie problemu z CLOCAL (WAŻNE!) |
| `SOLUTION_GPIO0_PROBLEM.md` | Docs | Rozwiązanie problemu GPIO0=LOW po zamknięciu portu |
| `README_DTR_RTS_FIX.md` | Docs | Ten plik - podsumowanie |
| `test_dtr_rts.sh` | Skrypt | Test podstawowy (zamyka po 10s) |
| `reset_esp32_and_hold.sh` | Skrypt | ⭐ Reset i trzymaj 30s (POLECANY do testów!) |
| `hold_esp32_normal_mode.sh` | Skrypt | Tylko trzymaj DTR/RTS HIGH |

## ⚙️ Konfiguracja

### Jeśli ESP32 nie startuje poprawnie:

**Opcja A: Zwiększ czasy**
W `SerialProtocol.cpp::initESP32ResetSequence()`:
```cpp
usleep(1000000); // Zmień na 2000000 (2 sekundy)
```

**Opcja B: Wyłącz sekwencję** (jeśli brak DTR/RTS)
W `SerialProtocol.cpp::init()` zakomentuj:
```cpp
// initESP32ResetSequence();
```

**Opcja C: Odwróć logikę** (jeśli używasz typowej komercyjnej płytki NodeMCU/DevKit)
Zmień w `initESP32ResetSequence()`:
```cpp
setRTS(false);  // Zamiast setRTS(true) - dla typowych płytek z inwersją
```
Zobacz `DTR_RTS_LOGIC_NOTE.md` dla wyjaśnienia różnic między płytkami.

## ⚠️ WAŻNA INFORMACJA: Zamykanie portu szeregowego

### Dlaczego GPIO0 jest LOW po zakończeniu skryptu testowego?

Gdy program zamyka port szeregowy, linie DTR i RTS **automatycznie wracają do stanu domyślnego** (LOW):
```
Program działa:      DTR=HIGH, RTS=HIGH → GPIO0=HIGH ✅
Program zamyka port: DTR=LOW,  RTS=LOW  → GPIO0=LOW  ❌
```

**To jest normalne zachowanie systemu operacyjnego!**

### Rozwiązania:

1. **Użyj `reset_esp32_and_hold.sh`** - trzyma port otwarty przez 30 sekund
   ```bash
   sudo ./reset_esp32_and_hold.sh /dev/ttyUSB0
   # GPIO0 pozostanie HIGH przez 30 sekund - możesz to zmierzyć!
   ```

2. **Uruchom główny program zaraz po teście** - główny program trzyma port otwarty
   ```bash
   sudo ./bin/led-image-viewer
   # Ten program NIE zamyka portu dopóki nie zostanie zakończony
   ```

3. **Użyj hold script** jeśli chcesz tylko utrzymać stan bez resetu
   ```bash
   sudo ./hold_esp32_normal_mode.sh /dev/ttyUSB0
   # Trzyma DTR/RTS HIGH do momentu Ctrl+C
   ```

## 🔧 Diagnostyka

### ESP32 wchodzi w bootloader mode:
```
waiting for download
```
**Przyczyna:** GPIO0 jest LOW podczas restartu

**Sprawdź najpierw czy RTS działa:**
1. Uruchom program i sprawdź logi
2. Szukaj linii: `RTS requested: HIGH, actual: HIGH`
3. Jeśli widzisz `actual: LOW` mimo `requested: HIGH` → RTS nie działa!

**Możliwe rozwiązania:**
1. ✅ **CLOCAL był problemem** - już naprawione w kodzie!
2. Jeśli nadal nie działa - sprawdź czy konwerter USB-Serial ma RTS
3. Dla typowych płytek NodeMCU/DevKit - odwróć logikę: użyj `setRTS(false)`

### Nic się nie dzieje:
**Sprawdź:**
1. Czy konwerter USB-Serial ma DTR/RTS? (nie wszystkie mają!)
2. Czy linie są podłączone: DTR→EN, RTS→GPIO0?
3. Czy masz uprawnienia: `sudo chmod 666 /dev/ttyUSB0`

### ESP32 się resetuje ale nie boot:
**Zwiększ czasy w sekwencji** - ESP32 może potrzebować więcej czasu

## 📊 Statystyki zmian

- **Linii kodu dodanych:** ~150
- **Nowych funkcji:** 4 (`setDTR`, `setRTS`, `initESP32ResetSequence`, `detectESP32Restart`)
- **Czas rozwiązania:** Około 2 godziny
- **Wersja:** 1.0 (2025-10-19)

## ⚠️ Uwagi techniczne

1. **Ta płytka ma DIRECT logic** - RTS=HIGH daje GPIO0=HIGH (różni się od większości komercyjnych płytek!)
2. **Typowe płytki (NodeMCU, DevKit) mają INVERTED logic** - RTS=LOW daje GPIO0=HIGH
3. **Nie wszystkie konwertery mają DTR/RTS** - tanie CH340/CP2102 czasem nie mają
4. **GPIO0 musi być ustawione PRZED resetem** - to kluczowe!
5. **Grace period zapobiega zapętleniu** - garbage data nie blokuje już systemu

**Ważne:** Zobacz `DTR_RTS_LOGIC_NOTE.md` dla szczegółowego wyjaśnienia różnic!

## 📝 TODO - Co jeszcze trzeba zrobić

⚠️ **ESP32 musi wysyłać ponownie stan wyświetlacza po restarcie!**

Obecnie po restarcie:
1. ✅ Raspberry Pi wykrywa restart
2. ✅ Grace period czyści garbage
3. ❌ ESP32 NIE wysyła stanu wyświetlacza
4. ❌ Ekran pozostaje pusty

**Rozwiązania po stronie ESP32:**
- Zapisywać stan w EEPROM/Flash
- Przesyłać ponownie po `setup()`
- Lub: Raspberry Pi przechowuje i wysyła ponownie

Szczegóły w `ESP32_RESTART_FIX.md`.

## 🎯 Test akceptacyjny

Program działa poprawnie gdy:
- [x] ESP32 bootuje się w normal mode (nie bootloader)
- [x] Przy starcie widać `ESP32 should be running in normal mode`
- [x] Po restart ESP32 widać `ESP32 RESTART DETECTED`
- [x] Grace period działa (2 sekundy ignorowania danych)
- [ ] **Ekran odświeża się po restarcie ESP32** (wymaga zmian w ESP32!)

## 📞 Support

Jeśli masz problemy:
1. Uruchom `sudo ./test_dtr_rts.sh` i sprawdź output
2. Przeczytaj `QUICK_START_DTR_RTS.md`
3. Zobacz szczegóły w `ESP32_RESTART_FIX.md`
4. Sprawdź czy Twój konwerter ma DTR/RTS: `dmesg | grep -i usb`

---

**Wersja:** 1.0  
**Data:** 2025-10-19  
**Status:** ✅ Kompletne po stronie Raspberry Pi | ⚠️ Wymaga zmian w ESP32

