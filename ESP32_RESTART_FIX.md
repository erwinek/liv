# Fix dla problemu restartu ESP32

## Problem
Gdy ESP32 się restartuje, ekran LED nie odświeża się, a na konsoli widać tylko:
```
Buffer size: 418, calling processBuffer()
No SOF found in buffer, clearing 418 bytes of garbage
Received byte: 0x0
Buffer size: 1, calling processBuffer()
No SOF found in buffer, clearing 1 bytes of garbage
```

**Przyczyna:**
- ESP32 podczas bootowania wysyła komunikaty bootloadera i garbage data (0x0, debug messages, itp.)
- Raspberry Pi odbiera te śmieci (418+ bajtów), ale nie może znaleźć SOF (0xAA)
- Ekran nie odświeża się, bo ESP32 nie wysyła ponownie stanu wyświetlacza po restarcie

## Rozwiązanie

### Zmiany po stronie Raspberry Pi (ZROBIONE ✓)

Dodano inteligentne wykrywanie restartu ESP32 i okres karencji (grace period):

1. **Prawidłowa inicjalizacja DTR/RTS** (`initESP32ResetSequence()`) ✅ NOWE!
   - Przy starcie programu wykonywana jest właściwa sekwencja reset ESP32:
     - DTR LOW + RTS LOW (reset + boot mode) - 100ms
     - DTR HIGH + RTS LOW (bootloader mode) - 50ms  
     - DTR HIGH + RTS HIGH (normal mode) - 50ms
     - DTR LOW (reset pulse) - 50ms
     - DTR HIGH (release reset) + czekanie 500ms na boot
   - ESP32 uruchamia się w trybie normalnym z czystym stanem
   - Obie linie (DTR=HIGH, RTS=HIGH) = normalna praca

2. **Wykrywanie restartu** (`detectESP32Restart()`)
   - Gdy odbierzemy >200 bajtów garbage data (ESP32 boot = 500-1000 bajtów)
   - System wykrywa, że prawdopodobnie nastąpił restart ESP32

3. **Okres karencji (Grace Period)** - 2 sekundy
   - Przez 2 sekundy po wykryciu restartu:
     - Wszystkie odebrane dane są ignorowane
     - UART jest agresywnie czyszczony
     - Brak przetwarzania pakietów
   - Po zakończeniu:
     - System wraca do normalnej pracy
     - UART jest ostatni raz czyszczony
     - Buffer jest resetowany

4. **Redukcja debug output**
   - Zmniejszono ilość logów dla bajtów 0x0 (aby nie zaśmiecać konsoli)

### Co się teraz stanie:

**Przed zmianami:**
```
Received byte: 0x0      <- 418x powtórzone
Buffer size: 418
No SOF found, clearing 418 bytes of garbage
[powtarza się w nieskończoność]
```

**Po zmianach (przy starcie programu):**
```
Serial protocol initialized on /dev/ttyUSB0 at 1000000 bps
Initializing ESP32 reset sequence...
DTR set to LOW
RTS set to LOW
DTR set to HIGH
RTS set to HIGH
DTR set to LOW
DTR set to HIGH
ESP32 reset sequence complete (DTR=HIGH, RTS=HIGH)
Waiting for ESP32 to boot...
ESP32 should be ready for communication
UART buffers flushed
```

**Po zmianach (gdy ESP32 się zrestartuje podczas pracy):**
```
Buffer size: 418
No SOF found, clearing 418 bytes of garbage
=== ESP32 RESTART DETECTED (418 bytes of garbage) ===
=== Entering 2 second grace period ===
ESP32 restart grace period: ignoring 256 bytes (remaining: 1950 ms)
ESP32 restart grace period: ignoring 128 bytes (remaining: 1820 ms)
...
ESP32 restart grace period ended - resuming normal operation
[system czeka na prawidłowe pakiety od ESP32]
```

## Co JESZCZE TRZEBA ZROBIĆ

### ⚠️ Zmiany po stronie ESP32 (TO DO!)

ESP32 **MUSI** wysłać ponownie stan wyświetlacza po zakończeniu bootowania!

**Opcja 1: Wysyłanie "READY" signal**
```cpp
void setup() {
  Serial.begin(1000000);
  delay(100); // Poczekaj na stabilizację UART
  
  // Wyślij sygnał ready
  uint8_t packet[] = {0xAA, 0x00, 0x05, 0x00, 0x00, 0x55}; // GET_STATUS command
  Serial.write(packet, sizeof(packet));
  Serial.flush();
  
  // Następnie wyślij ponownie ostatni stan wyświetlacza
  resendLastDisplayState();
}
```

**Opcja 2: Zapisywanie stanu w EEPROM/Flash**
```cpp
// Przed każdą zmianą wyświetlacza:
saveDisplayStateToFlash(currentState);

// Po restarcie w setup():
void setup() {
  Serial.begin(1000000);
  delay(100);
  
  DisplayState state = loadDisplayStateFromFlash();
  if (state.valid) {
    resendDisplayCommands(state);
  }
}
```

**Opcja 3: Raspberry Pi przechowuje stan i wysyła ponownie**
- Raspberry Pi zapisuje ostatni wysłany stan
- Po wykryciu restartu ESP32 (po grace period), automatycznie wysyła ponownie
- Wymaga modyfikacji DisplayManager.cpp

## Testowanie

1. Skompiluj kod: `make`
2. Uruchom: `sudo ./bin/led-image-viewer`
3. Wyślij komendę wyświetlenia GIF/tekstu
4. Zrestartuj ESP32 (przycisk RESET)
5. Obserwuj konsolę:
   - Powinno pokazać "ESP32 RESTART DETECTED"
   - Grace period przez 2 sekundy
   - Potem "resuming normal operation"
6. **Wyślij ponownie komendę z ESP32 lub aplikacji** - ekran powinien się odświeżyć

## Pliki zmodyfikowane

- `SerialProtocol.h` - dodano:
  - Zmienne dla wykrywania restartu ESP32
  - Metody `setDTR()`, `setRTS()`, `initESP32ResetSequence()`
  - Metody `detectESP32Restart()`, `getCurrentTimeUs()`
- `SerialProtocol.cpp` - implementacja:
  - Prawidłowej sekwencji reset DTR/RTS przy inicjalizacji
  - Wykrywania restartu ESP32 (garbage detection)
  - Grace period (2 sekundy ignorowania danych)
  - Funkcji kontroli linii szeregowych

## Jak działa sekwencja DTR/RTS?

W ESP32 linie DTR i RTS są podłączone do pinów kontrolnych:
- **DTR** -> **EN** (Enable/Reset pin) - przez kondensator/tranzystor
- **RTS** -> **GPIO0** (Boot mode pin) - przez kondensator/tranzystor

### Logika stanów (DIRECT - bez inwersji!):
⚠️ **UWAGA**: Ta płytka używa **bezpośredniej logiki** (bez tranzystorów inwertujących):

| RTS (software) | GPIO0 (hardware) | DTR (software) | EN (hardware) | Rezultat |
|----------------|------------------|----------------|---------------|----------|
| HIGH | **HIGH** | LOW | **LOW** | Reset + Normal mode ready |
| HIGH | **HIGH** | HIGH | **HIGH** | **Normal Mode (aplikacja)** ✅ |
| LOW | **LOW** | LOW | **LOW** | Reset + Bootloader ready |
| LOW | **LOW** | HIGH | **HIGH** | Bootloader Mode (czeka na flash) |

**Ważne:** Większość komercyjnych płytek używa **odwróconej logiki** (RTS=LOW→GPIO0=HIGH), ale ta płytka ma **bezpośrednie połączenie** (RTS=HIGH→GPIO0=HIGH).

### Sekwencja inicjalizacji (POTWIERDZONA dla tej płytki):
1. **RTS=HIGH** (50ms) - Ustaw GPIO0=HIGH dla normal mode (PRZED resetem!)
2. **DTR=LOW** (100ms) - Trzymaj ESP32 w reset (EN=LOW)
3. **DTR=HIGH** - Puść reset (EN=HIGH), ESP32 sprawdza GPIO0=HIGH → **Normal Mode** ✅
4. **Czekaj 1 sekundę** - ESP32 bootuje się w trybie aplikacji
5. **Stan końcowy: DTR=HIGH (EN=HIGH), RTS=HIGH (GPIO0=HIGH)**

**Kluczowe:** GPIO0 musi być ustawione **PRZED** zwolnieniem resetu (EN)!

To zapewnia czyste uruchomienie ESP32 w trybie normalnym bez wchodzenia w bootloader.

Zobacz `DTR_RTS_LOGIC_NOTE.md` dla szczegółów o różnicach między płytkami.

## Parametry do dostrojenia

W `SerialProtocol.h`:
```cpp
static constexpr uint64_t RESTART_GRACE_PERIOD_US = 2000000; // 2 sekundy
```

W `SerialProtocol.cpp::detectESP32Restart()`:
```cpp
if (garbage_bytes > 200) {  // Próg wykrywania restartu
```

W `SerialProtocol.cpp::initESP32ResetSequence()`:
```cpp
usleep(50000);   // 50ms - stabilizacja GPIO0
usleep(100000);  // 100ms - czas trzymania w reset
usleep(1000000); // 1000ms (1 sekunda) - czekanie na pełny boot ESP32
```

Możesz zmienić te wartości jeśli:
- ESP32 bootuje się dłużej -> zwiększ ostatnie `usleep(1000000)` np. do 2000000 (2 sekundy)
- ESP32 nie uruchamia się stabilnie -> zwiększ czas trzymania w reset (100ms -> 200ms)
- Zbyt często wykrywa restart -> zwiększ próg garbage_bytes (200 -> 300)
- Zbyt rzadko wykrywa restart -> zmniejsz próg garbage_bytes (200 -> 150)

## Następne kroki

1. ✅ Zaimplementować prawidłową sekwencję DTR/RTS (DONE)
2. ✅ Zaimplementować wykrywanie restartu (DONE)
3. ✅ Dodać grace period (DONE)
4. ⚠️ **ESP32: Wysyłać ponownie stan po restarcie** (TODO!)
5. 🔄 Opcjonalnie: Raspberry Pi przechowuje i wysyła ponownie stan (TODO)

## Uwagi techniczne

- **Sprzętowe połączenie**: Upewnij się, że Twój konwerter USB-Serial ma wyprowadzone linie DTR i RTS
- **Nie wszystkie konwertery**: Tanie konwertery CH340/CP2102 czasem nie mają DTR/RTS!
- **Sprawdź schemat**: Potwierdź że DTR→EN i RTS→GPIO0 w Twoim układzie
- **Jeśli nie działa**: Możesz wyłączyć sekwencję reset zakomentowując `initESP32ResetSequence()` w `SerialProtocol.cpp::init()`

---

Data: 2025-10-19
Autor: AI Assistant

