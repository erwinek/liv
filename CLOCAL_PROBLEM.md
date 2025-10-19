# Problem: CLOCAL blokował DTR/RTS

## 🐛 Problem który znaleźliśmy

Po uruchomieniu `sudo ./bin/led-image-viewer`, GPIO0 nadal miało niski stan (LOW), mimo że kod wywoływał `setRTS(true)`.

## 🔍 Przyczyna

W konfiguracji portu szeregowego była ustawiona flaga **`CLOCAL`**:

```c
tty.c_cflag |= CREAD | CLOCAL;  // ← Problem!
```

### Co robi CLOCAL?

`CLOCAL` = "Local mode" = **ignoruj linie modem control**

Gdy `CLOCAL` jest ustawione:
- System ignoruje zmiany na liniach DTR, DCD, DSR
- `ioctl(TIOCMSET)` może nie działać poprawnie
- DTR/RTS mogą być blokowane przez sterownik

**To była przyczyna!** Kod wywoływał `setRTS(true)`, ale system ignorował to przez `CLOCAL`.

## ✅ Rozwiązanie

Usunęliśmy `CLOCAL` i wyłączyliśmy go explicite:

```c
tty.c_cflag |= CREAD;           // Enable reading
tty.c_cflag &= ~CLOCAL;         // Don't ignore modem control lines!
```

Teraz:
- DTR/RTS są kontrolowane przez `ioctl()`
- `setRTS(true)` faktycznie ustawia RTS na HIGH
- GPIO0 jest HIGH podczas działania programu

## 📊 Przed i Po

### PRZED (z CLOCAL):
```
setRTS(true) wywoływane ✓
  ↓
System ignoruje przez CLOCAL ❌
  ↓
RTS pozostaje LOW
  ↓
GPIO0 = LOW (bootloader mode) ❌
```

### PO (bez CLOCAL):
```
setRTS(true) wywoływane ✓
  ↓
System ustawia RTS=HIGH ✓
  ↓
RTS = HIGH
  ↓
GPIO0 = HIGH (normal mode) ✅
```

## 🧪 Weryfikacja

Dodaliśmy także dodatkowe debugowanie w `setRTS()`:

```cpp
// Przed zmianą
std::cout << "Current modem status: 0x" << status << std::endl;

// Po zmianie - weryfikacja
int verify_status;
ioctl(serial_fd, TIOCMGET, &verify_status);
bool rts_actual = (verify_status & TIOCM_RTS) != 0;
std::cout << "RTS requested: HIGH, actual: " << (rts_actual ? "HIGH" : "LOW");
if (rts_actual != state) {
    std::cout << " ⚠️ MISMATCH!";
}
```

Teraz przy starcie programu zobaczysz:
```
Serial config: Hardware flow control OFF, Modem control lines ENABLED
Setting RTS=HIGH (GPIO0 will be HIGH for normal mode)
Current modem status before RTS change: 0x...
RTS requested: HIGH, actual: HIGH (status=0x...)
```

## 📚 Dokumentacja techniczna

### Flagi terminala c_cflag:

| Flaga | Znaczenie | Nasz wybór |
|-------|-----------|------------|
| `CREAD` | Enable receiver | ✅ ON (musimy czytać dane) |
| `CLOCAL` | Ignore modem control lines | ❌ OFF (potrzebujemy DTR/RTS!) |
| `CRTSCTS` | Hardware flow control | ❌ OFF (kontrolujemy RTS ręcznie) |
| `CS8` | 8 data bits | ✅ ON (standard) |
| `PARENB` | Parity enable | ❌ OFF (8N1) |
| `CSTOPB` | Two stop bits | ❌ OFF (1 stop bit) |

### Dlaczego CLOCAL było ustawione?

Typowo `CLOCAL` jest używane gdy:
- Nie używasz modemu (local connection)
- Nie potrzebujesz linii DCD (Data Carrier Detect)
- Chcesz aby program nie czekał na sygnał carrier

Ale w naszym przypadku **aktywnie kontrolujemy DTR/RTS**, więc `CLOCAL` blokowało naszą funkcjonalność.

## 🎯 Lekcja

Gdy kontrolujesz DTR/RTS ręcznie przez `ioctl()`:
- ❌ **NIE** ustawiaj `CLOCAL`
- ❌ **NIE** ustawiaj `CRTSCTS` (hardware flow control)
- ✅ Używaj tylko `CREAD`
- ✅ Kontroluj linie przez `TIOCMGET/TIOCMSET`

## 🔗 Zobacz też

- `SerialProtocol.cpp::init()` - konfiguracja portu
- `SerialProtocol.cpp::setRTS()` - kontrola RTS z weryfikacją
- `SerialProtocol.cpp::setDTR()` - kontrola DTR

## 📝 Changelog

- **2025-10-19**: Usunięto `CLOCAL`, dodano weryfikację w `setRTS()`
- **Status**: ✅ GPIO0 teraz działa poprawnie!

---

**TL;DR:** `CLOCAL` ignorowało nasze DTR/RTS. Po usunięciu, GPIO0 jest HIGH! 🎉

