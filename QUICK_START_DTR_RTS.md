# Quick Start - DTR/RTS dla ESP32

## Szybki test sekwencji DTR/RTS

### Opcja 1: Test z automatycznym zamknięciem
Podstawowy test który pokazuje sekwencję:
```bash
cd /home/erwinek/liv
sudo ./test_dtr_rts.sh /dev/ttyUSB0
```

Ten skrypt:
- Wykona sekwencję reset ESP32
- Pokaże stan linii DTR/RTS
- Odczyta komunikaty bootloadera ESP32
- Trzyma port otwarty przez 10 sekund, potem zamyka

⚠️ **Uwaga:** Po zamknięciu portu, DTR/RTS wracają do LOW (GPIO0=LOW)

### Opcja 2: Reset i trzymaj (POLECANE do testów!)
Wykonaj reset i pozostaw ESP32 w normal mode:
```bash
cd /home/erwinek/liv
sudo ./reset_esp32_and_hold.sh /dev/ttyUSB0
```

Ten skrypt:
- Wykona pełną sekwencję reset
- Pozostawi DTR=HIGH, RTS=HIGH
- Trzyma port otwarty przez 30 sekund
- ESP32 pozostaje w normal mode podczas działania skryptu
- **Idealny do weryfikacji że GPIO0=HIGH po sekwencji**

### Opcja 3: Tylko trzymaj stan
Jeśli ESP32 już działa, po prostu utrzymuj stan:
```bash
cd /home/erwinek/liv
sudo ./hold_esp32_normal_mode.sh /dev/ttyUSB0
```

Ten skrypt po prostu trzyma DTR=HIGH, RTS=HIGH (bez resetu).

## Pełny test z programem

### ⚠️ WAŻNA POPRAWKA: Problem CLOCAL został rozwiązany!

Jeśli wcześniej GPIO0 było LOW mimo uruchomienia programu, to dlatego że flaga `CLOCAL` blokowała kontrolę DTR/RTS. **To zostało naprawione!**

Zobacz `CLOCAL_PROBLEM.md` dla szczegółów.

### Teraz uruchom program:

1. Skompiluj:
```bash
cd /home/erwinek/liv
make
```

2. Uruchom:
```bash
sudo ./bin/led-image-viewer
```

3. Sprawdź logi - powinny pokazać:
```
Serial config: Hardware flow control OFF, Modem control lines ENABLED
Initializing ESP32 reset sequence (DIRECT logic - no inversion)...
Setting RTS=HIGH (GPIO0 will be HIGH for normal mode)
Current modem status before RTS change: 0x...
RTS requested: HIGH, actual: HIGH (status=0x...)  ← ✅ Sprawdź to!
Asserting reset (DTR=LOW, EN=LOW)
DTR set to LOW
Releasing reset (DTR=HIGH, EN=HIGH) - ESP32 starting in normal mode
DTR set to HIGH
Keeping RTS=HIGH during boot (GPIO0=HIGH)
Final state: DTR=HIGH (EN=HIGH), RTS=HIGH (GPIO0=HIGH)
Waiting for ESP32 to boot...
ESP32 should be running in normal mode
```

**Klucz do weryfikacji:** Sprawdź linię `RTS requested: HIGH, actual: HIGH`
- Jeśli widzisz `actual: HIGH` → ✅ RTS działa!
- Jeśli widzisz `actual: LOW` → ❌ Problem z hardwarem lub sterownikiem

## ⚠️ WAŻNE: Problem z zamykaniem portu szeregowego

Gdy program/skrypt zamyka port szeregowy (`ser.close()` w Python, `close(fd)` w C), linie DTR i RTS **wracają do stanu domyślnego** (zazwyczaj LOW).

**Konsekwencje:**
- DTR=LOW → EN=LOW → ESP32 w reset ❌
- RTS=LOW → GPIO0=LOW → ESP32 w bootloader mode (jeśli się uruchomi) ❌

**Rozwiązania:**
1. **Nie zamykaj portu** - trzymaj go otwarty (skrypt `reset_esp32_and_hold.sh`)
2. **Zaraz po zamknięciu uruchom główny program** który znowu ustawi prawidłowe linie
3. **Użyj głównego programu** (`led-image-viewer`) który trzyma port otwarty przez cały czas

**To dlatego:** Główny program `led-image-viewer` wykonuje sekwencję reset przy starcie i **nie zamyka portu** dopóki nie zostanie zakończony.

## ⚠️ WAŻNE: Direct Logic (bez inwersji)!

**Ta płytka** używa **bezpośredniej logiki** (bez tranzystorów inwertujących):
- **RTS=HIGH** (software) → **GPIO0=HIGH** (hardware) → ✅ Normal Mode
- **RTS=LOW** (software) → **GPIO0=LOW** (hardware) → Bootloader Mode

**Różnica od typowych płytek:** Większość komercyjnych płytek ESP32 (NodeMCU, DevKit) ma odwróconą logikę (RTS=LOW→GPIO0=HIGH), ale ta płytka ma bezpośrednie połączenie.

**Rozwiązanie:** Sekwencja trzyma **RTS=HIGH przez cały czas**, co zapewnia GPIO0=HIGH.

Zobacz `DTR_RTS_LOGIC_NOTE.md` dla szczegółowego wyjaśnienia.

## Jeśli ESP32 nie startuje

### Opcja 1: Zwiększ czasy w sekwencji
Edytuj `SerialProtocol.cpp::initESP32ResetSequence()`:
```cpp
usleep(1000000); // Zwiększ z 1000000 na 2000000 (2 sekundy zamiast 1)
// lub
usleep(100000);  // Zwiększ czas reset z 100000 na 200000 (200ms)
```

### Opcja 2: Wyłącz sekwencję reset (jeśli brak DTR/RTS)
Zakomentuj w `SerialProtocol.cpp::init()`:
```cpp
// Initialize ESP32 with proper DTR/RTS sequence
// initESP32ResetSequence();  // <-- Dodaj // na początku
```

### Opcja 3: Sprawdź hardware
```bash
# Sprawdź czy konwerter USB-Serial ma DTR/RTS
dmesg | grep -i usb
lsusb -v | grep -i "serial\|uart"

# Sprawdź uprawnienia
ls -la /dev/ttyUSB0
sudo chmod 666 /dev/ttyUSB0  # jeśli potrzeba
```

## Testowanie z przyciskiem RESET na ESP32

1. Uruchom program
2. Wyślij komendę wyświetlenia (GIF/tekst)
3. Wciśnij przycisk RESET na ESP32
4. Sprawdź logi:
```
=== ESP32 RESTART DETECTED (XXX bytes of garbage) ===
=== Entering 2 second grace period ===
...
ESP32 restart grace period ended - resuming normal operation
```

## Diagnostyka

### Problemy z DTR/RTS:
```bash
# Test DTR/RTS z minicom
minicom -D /dev/ttyUSB0 -b 1000000
# W minicom: Ctrl+A > O > Serial port setup
# Sprawdź czy Hardware Flow Control = Yes/No
```

### ESP32 bootuje się w trybie bootloader:
- Sprawdź RTS - powinno być HIGH
- Zwiększ czasy w sekwencji
- Sprawdź połączenie GPIO0 z RTS

### ESP32 się nie resetuje:
- Sprawdź DTR - powinno zmieniać się LOW/HIGH
- Sprawdź połączenie EN z DTR
- Sprawdź kondensator na linii reset (100nF typowo)

## Zobacz też

- `ESP32_RESTART_FIX.md` - pełna dokumentacja
- `SerialProtocol.h` - deklaracje funkcji
- `SerialProtocol.cpp` - implementacja

---

**Tip:** Jeśli wszystko działa bez sekwencji DTR/RTS (ESP32 uruchamia się poprawnie), możesz ją wyłączyć dla szybszego startu programu.

