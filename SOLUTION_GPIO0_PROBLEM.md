# Rozwiązanie problemu: GPIO0=LOW po zakończeniu skryptu

## 🎯 Problem

Po uruchomieniu `test_dtr_rts.sh`, GPIO0 jest LOW zamiast HIGH.

## 💡 Przyczyna

**Zamykanie portu szeregowego resetuje linie DTR/RTS do LOW!**

Gdy program wywołuje `close()` na porcie szeregowym, system operacyjny automatycznie:
1. Ustawia DTR=LOW (EN=LOW)
2. Ustawia RTS=LOW (GPIO0=LOW)

**To normalne zachowanie!** Dzieje się tak w każdym programie który zamyka port.

## ✅ Rozwiązanie

### Opcja 1: Użyj skryptu który TRZYMA port otwarty (POLECANE!)

```bash
sudo ./reset_esp32_and_hold.sh /dev/ttyUSB0
```

**Co robi:**
- Wykonuje pełną sekwencję reset ESP32
- DTR=HIGH, RTS=HIGH
- **Trzyma port otwarty przez 30 sekund**
- GPIO0 pozostaje HIGH przez te 30 sekund
- Możesz zmierzyć GPIO0 multimetrem!

**Logi:**
```
[1/4] Setting RTS=HIGH (GPIO0=HIGH for normal mode)
[2/4] Asserting reset (DTR=LOW, EN=LOW)
[3/4] Releasing reset (DTR=HIGH, EN=HIGH)
[4/4] Waiting for ESP32 to boot (1 second)...
✅ ESP32 Reset Complete - Normal Mode Active
Current state:
  DTR: True (EN=HIGH - ESP32 running)
  RTS: True (GPIO0=HIGH - normal mode)
Holding port open to maintain state
[Auto-close in 30 seconds... Ctrl+C to exit now]
```

### Opcja 2: Uruchom główny program

Główny program `led-image-viewer` wykonuje tę samą sekwencję i **nie zamyka portu** dopóki nie zostanie zakończony:

```bash
sudo ./bin/led-image-viewer
```

**Przewaga:** Port pozostaje otwarty przez cały czas działania programu.

### Opcja 3: Tylko utrzymuj stan (bez resetu)

Jeśli ESP32 już działa i chcesz tylko trzymać linie HIGH:

```bash
sudo ./hold_esp32_normal_mode.sh /dev/ttyUSB0
```

## 📊 Porównanie skryptów

| Skrypt | Reset? | Czas trzymania | Przeznaczenie |
|--------|--------|----------------|---------------|
| `test_dtr_rts.sh` | ✅ | 10 sekund | Szybki test, pokazuje sekwencję |
| `reset_esp32_and_hold.sh` | ✅ | 30 sekund | ⭐ **POLECANY do testów GPIO0** |
| `hold_esp32_normal_mode.sh` | ❌ | Do Ctrl+C | Tylko utrzymanie stanu |
| `led-image-viewer` | ✅ | Do zakończenia | Główny program (produkcja) |

## 🧪 Jak zweryfikować że działa

### Podczas działania skryptu `reset_esp32_and_hold.sh`:

1. **Opcja A - Multimetr:**
   ```
   Podłącz multimetr do GPIO0
   Uruchom: sudo ./reset_esp32_and_hold.sh
   Powinieneś zmierzyć ~3.3V przez 30 sekund
   ```

2. **Opcja B - Monitor szeregowy:**
   ```
   Uruchom skrypt i obserwuj komunikaty ESP32
   Powinny pokazać boot w normal mode (aplikacja)
   NIE powinno być "waiting for download"
   ```

3. **Opcja C - LED na płytce:**
   ```
   Większość płytek ma LED na GPIO0
   Obserwuj czy LED gaśnie (HIGH) czy świeci (LOW)
   ```

## 🔍 Dodatkowe informacje

### Dlaczego główny program działa poprawnie?

```c++
// W SerialProtocol::init():
initESP32ResetSequence();  // Ustawia DTR=HIGH, RTS=HIGH
// ... port pozostaje otwarty ...
// Program działa przez godziny/dni
// Port NIE jest zamykany dopóki program nie zakończy się
```

### Co się dzieje gdy zamkniesz port?

```
PRZED close():  DTR=HIGH, RTS=HIGH → GPIO0=HIGH ✅
close()
PO close():     DTR=LOW,  RTS=LOW  → GPIO0=LOW  ❌
```

### Czy to bug?

**NIE!** To zamierzone zachowanie systemu operacyjnego:
- Zapobiega "wiszącym" sygnałom na linii
- Reset urządzeń przy odłączeniu
- Standard RS-232 / USB-Serial

## 📚 Zobacz też

- `README_DTR_RTS_FIX.md` - Kompletne podsumowanie
- `QUICK_START_DTR_RTS.md` - Przewodnik użytkownika
- `DTR_RTS_LOGIC_NOTE.md` - Wyjaśnienie DIRECT vs INVERTED logic
- `ESP32_RESTART_FIX.md` - Szczegóły techniczne

---

**TL;DR:** Użyj `sudo ./reset_esp32_and_hold.sh` aby GPIO0 pozostało HIGH przez 30 sekund! 🎯

