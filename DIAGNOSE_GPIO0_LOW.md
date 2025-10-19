# Diagnoza: GPIO0 nadal LOW

## 🔍 Krok po kroku diagnostyka

GPIO0 jest nadal LOW mimo wszystkich poprawek. Musimy zdiagnozować gdzie jest problem.

### Krok 1: Test z programem C (bezpośredni ioctl)

```bash
cd /home/erwinek/liv
sudo ./test_ioctl_rts /dev/ttyUSB0
```

**Co sprawdza:**
- Czy ioctl(TIOCMSET) działa
- Czy ioctl(TIOCMGET) zwraca poprawny stan
- Czy CLOCAL jest wyłączone
- Daje 10 sekund na zmierzenie GPIO0

**Szukaj w outputcie:**
```
CLOCAL: DISABLED (GOOD: should be DISABLED)  ← Musi być DISABLED!
RTS is HIGH  ← Musi być SUCCESS!
→ GPIO0 should be HIGH now (~3.3V)  ← Zmierz teraz!
```

**Możliwe rezultaty:**

#### A) ioctl pokazuje SUCCESS, ale GPIO0 jest LOW
→ **Problem hardware**: RTS nie jest połączone z GPIO0

#### B) ioctl pokazuje FAILED
→ **Problem driver/system**: ioctl nie działa

#### C) ioctl pokazuje SUCCESS i GPIO0 jest HIGH
→ **✅ Działa!** Problem był w głównym programie

---

### Krok 2: Test z Python (debug_rts_state.sh)

```bash
sudo ./debug_rts_state.sh /dev/ttyUSB0
```

**Co sprawdza:**
- Czy pyserial poprawnie kontroluje RTS
- Toggle test (mruganie)
- 10 sekund na pomiar

**Obserwuj GPIO0:**
- Powinno się zmieniać LOW/HIGH przy toggle
- Jeśli nie zmienia się → problem hardware

---

### Krok 3: Test głównego programu

```bash
sudo ./bin/led-image-viewer 2>&1 | grep -A5 "RTS"
```

**Szukaj linii:**
```
Serial config: Hardware flow control OFF, Modem control lines ENABLED
RTS requested: HIGH, actual: HIGH (status=0x...)
```

**Możliwe przypadki:**

#### Przypadek 1: `actual: LOW` mimo `requested: HIGH`
```
RTS requested: HIGH, actual: LOW ⚠️ MISMATCH!
```
→ **Problem w kodzie głównego programu**

#### Przypadek 2: `actual: HIGH` ale GPIO0 jest LOW
```
RTS requested: HIGH, actual: HIGH (status=0x6)
```
→ **Problem hardware: RTS software HIGH, ale GPIO0 fizycznie LOW**

---

## 🔧 Możliwe problemy i rozwiązania

### Problem 1: RTS nie jest fizycznie połączone z GPIO0

**Sprawdź:**
1. Czy Twój konwerter USB-Serial ma wyprowadzony pin RTS?
2. Czy RTS jest połączone z GPIO0 na ESP32?
3. Czy są jakieś tranzystory/rezystory w połączeniu?

**Test:**
```bash
# Sprawdź pinout Twojego konwertera
lsusb -v | grep -A10 "Serial"
dmesg | grep -i "ftdi\|cp210\|ch340"
```

Niektóre tanie konwertery **nie mają RTS**!

### Problem 2: GPIO0 ma pull-down resistor

ESP32 ma wewnętrzny pull-down na GPIO0. Jeśli RTS ma za słaby output, może nie przełączyć GPIO0 na HIGH.

**Rozwiązanie:**
- Sprawdź schemat płytki
- Może potrzeba zewnętrznego pull-up na GPIO0
- Lub mocniejszego drivera na RTS

### Problem 3: Odwrócona logika (mimo testów)

Może być dodatkowy stopień inwersji który nie został wykryty.

**Test:**
Zmień w kodzie:
```cpp
// W initESP32ResetSequence():
setRTS(false);  // Zamiast setRTS(true)
```

### Problem 4: GPIO0 jest używane przez coś innego

Sprawdź czy GPIO0 nie jest:
- Podłączone do czegoś innego na płytce
- Sterowane przez firmware ESP32 (jeśli bootuje się wcześniej)
- Ma konflikt z innym peryferiom

### Problem 5: Konwerter USB-Serial nie obsługuje modem control

Niektóre konwertery ignorują modem control całkowicie.

**Sprawdź:**
```bash
sudo dmesg | tail -20
# Szukaj komunikatów o USB-Serial
```

**Typowe konwertery:**
- ✅ FTDI FT232 - ma RTS/DTR
- ✅ CP2102 (niektóre wersje) - ma RTS/DTR
- ❌ CH340 (tanie wersje) - często brak RTS/DTR
- ✅ Prolific PL2303 - ma RTS/DTR

---

## 📊 Macierz diagnostyczna

| ioctl test | Python test | Główny program | GPIO0 | Diagnoza |
|------------|-------------|----------------|-------|----------|
| ✅ SUCCESS | ✅ Zmienia się | ✅ actual: HIGH | ❌ LOW | Hardware: RTS nie dociera do GPIO0 |
| ✅ SUCCESS | ✅ Zmienia się | ❌ actual: LOW | ❌ LOW | Bug w głównym programie |
| ❌ FAILED | ❌ Nie zmienia | - | ❌ LOW | Driver/system problem |
| ✅ SUCCESS | ✅ Zmienia się | ✅ actual: HIGH | ✅ HIGH | **DZIAŁA!** |

---

## 🎯 Następne kroki

1. **Uruchom test ioctl:**
   ```bash
   sudo ./test_ioctl_rts /dev/ttyUSB0
   ```

2. **Zmierz GPIO0 multimetrem** podczas testu (masz 10 sekund)

3. **Wyślij wyniki:**
   - Co pokazuje ioctl test? (SUCCESS czy FAILED?)
   - Ile voltów pokazuje multimetr na GPIO0?
   - Czy GPIO0 się zmienia przy toggle test?

4. **Sprawdź konwerter USB-Serial:**
   ```bash
   lsusb
   dmesg | grep -i "usb\|serial" | tail -20
   ```

---

## 📝 Informacje do zebrania

Proszę podaj:
1. Model konwertera USB-Serial (output z `lsusb`)
2. Output z `sudo ./test_ioctl_rts /dev/ttyUSB0`
3. Napięcie na GPIO0 podczas testu (multimetrem)
4. Czy GPIO0 mruga podczas toggle test?
5. Model płytki ESP32

To pomoże zdiagnozować dokładny problem!

---

**TL;DR:** 
1. Uruchom `sudo ./test_ioctl_rts /dev/ttyUSB0`
2. Zmierz GPIO0 podczas testu
3. Podaj wyniki 🔬

