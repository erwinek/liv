# Rozwiązywanie problemów - LED Matrix

## Problem: Matryca pokazuje tylko ekran kontrolny

### Diagnoza

Jeśli matryca pokazuje tylko ekran kontrolny (pusty ekran lub logo), oznacza to, że **nie otrzymuje prawidłowych pakietów**.

### Krok 1: Sprawdź Serial Monitor

Otwórz Serial Monitor (115200 bps) i sprawdź czy widzisz komunikaty:

```
[MTRX TX] AA 01 03 00 00 55
[MTRX TX] AA 01 04 01 50 55 55
[MTRX TX] AA 01 02 28 00 00 00 00 02 FF 00 00 04 54 45 53 54 ...
```

**Jeśli widzisz te komunikaty:**
- ✅ Pakiet jest wysyłany
- ❌ Problem z połączeniem sprzętowym

**Jeśli NIE widzisz tych komunikatów:**
- ❌ Problem z kodem lub inicjalizacją

### Krok 2: Sprawdź połączenia

```
ESP32                    LED Matrix
────                     ──────────
GPIO 17 (TX)  ────────>  RX
GND           ────────>  GND
```

**UWAGA:** 
- TX ESP32 → RX Matrycy (NIE TX→TX!)
- GND musi być wspólne

### Krok 3: Sprawdź baudrate

Domyślny baudrate to **1000000 bps (1 Mbps)**.

Jeśli to nie działa, spróbuj zmienić na **115200 bps**:

```cpp
matrix.begin(115200); // Zamiast 1000000
```

### Krok 4: Test z niższym baudrate

Stwórz prosty test:

```cpp
void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial1.begin(115200); // Niski baudrate
    delay(500);
    
    // Wyślij prosty pakiet ręcznie
    Serial1.write(0xAA); // SOF
    Serial1.write(0x01); // Screen ID
    Serial1.write(0x03); // CMD_CLEAR_SCREEN
    Serial1.write(0x00); // Payload length
    Serial1.write(0x00); // Checksum (0 XOR 0 = 0)
    Serial1.write(0x55); // EOF
    Serial1.flush();
    
    Serial.println("Packet sent!");
}
```

### Krok 5: Sprawdź czy matryca działa

Podłącz matrycę do komputera przez USB i sprawdź czy:
- Matryca się włącza
- Pokazuje ekran startowy
- Czy ma swoje własne menu/test

### Krok 6: Sprawdź dokumentację matrycy

Sprawdź w dokumentacji matrycy:
- Jaki jest domyślny baudrate?
- Czy potrzebuje specjalnej sekwencji inicjalizacji?
- Czy ma tryb testowy?

## Częste problemy i rozwiązania

### Problem 1: Brak komunikatów w Serial Monitor

**Przyczyna:** Problem z inicjalizacją

**Rozwiązanie:**
```cpp
void setup() {
    Serial.begin(115200);
    delay(2000); // Dłuższe opóźnienie
    
    Serial.println("Starting...");
    
    // Sprawdź czy Serial1 jest dostępny
    if (Serial1) {
        Serial.println("Serial1 OK");
    } else {
        Serial.println("Serial1 ERROR!");
    }
}
```

### Problem 2: Złe połączenia

**Przyczyna:** TX połączone z TX zamiast RX

**Rozwiązanie:**
- Odłącz wszystko
- Podłącz TX ESP32 → RX Matrycy
- Podłącz GND → GND
- Upewnij się, że zasilanie jest OK

### Problem 3: Zły baudrate

**Przyczyna:** Matryca używa innego baudrate

**Rozwiązanie:**
Spróbuj różnych baudrates:
```cpp
// 115200 bps
matrix.begin(115200);

// 230400 bps
matrix.begin(230400);

// 460800 bps
matrix.begin(460800);

// 921600 bps
matrix.begin(921600);

// 1000000 bps (domyślny)
matrix.begin(1000000);
```

### Problem 4: Matryca nie reaguje

**Przyczyna:** Matryca potrzebuje resetu lub specjalnej sekwencji

**Rozwiązanie:**
```cpp
void setup() {
    // Reset matrycy (jeśli ma pin RESET)
    pinMode(RESET_PIN, OUTPUT);
    digitalWrite(RESET_PIN, LOW);
    delay(100);
    digitalWrite(RESET_PIN, HIGH);
    delay(500);
    
    // Inicjalizacja
    matrix.begin(1000000);
}
```

### Problem 5: Tylko ekran kontrolny

**Przyczyna:** Pakiet jest wysyłany, ale matryca go nie rozpoznaje

**Rozwiązanie:**
1. Sprawdź czy ID ekranu jest poprawne (domyślnie 1)
2. Sprawdź czy suma kontrolna jest poprawna
3. Sprawdź czy protokół jest zgodny z dokumentacją

## Test diagnostyczny

Uruchom ten kod i sprawdź wyniki:

```cpp
void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("=== LED Matrix Diagnostic Test ===");
    
    // Test 1: Inicjalizacja
    Serial.println("\n[TEST 1] Initializing Serial1...");
    Serial1.begin(1000000);
    delay(500);
    Serial.println("✓ Serial1 initialized");
    
    // Test 2: Wyślij prosty pakiet
    Serial.println("\n[TEST 2] Sending test packet...");
    Serial1.write(0xAA); // SOF
    Serial1.write(0x01); // Screen ID
    Serial1.write(0x03); // CMD_CLEAR_SCREEN
    Serial1.write(0x00); // Payload length
    Serial1.write(0x00); // Checksum
    Serial1.write(0x55); // EOF
    Serial1.flush();
    Serial.println("✓ Packet sent");
    
    // Test 3: Sprawdź czy dane są wysyłane
    Serial.println("\n[TEST 3] Checking Serial1...");
    if (Serial1.availableForWrite() > 0) {
        Serial.println("✓ Serial1 is ready");
    } else {
        Serial.println("✗ Serial1 error!");
    }
    
    Serial.println("\n=== Diagnostic Complete ===");
    Serial.println("Check Serial Monitor for results");
}

void loop() {
    // Nic nie rób
}
```

## Co sprawdzić w Serial Monitor

Po uruchomieniu powinieneś zobaczyć:

```
=== LED Matrix Diagnostic Test ===

[TEST 1] Initializing Serial1...
✓ Serial1 initialized

[TEST 2] Sending test packet...
✓ Packet sent

[TEST 3] Checking Serial1...
✓ Serial1 is ready

=== Diagnostic Complete ===
Check Serial Monitor for results
```

## Kontakt z producentem matrycy

Jeśli nic nie pomaga, skontaktuj się z producentem matrycy i zapytaj o:
- Domyślny baudrate
- Format protokołu
- Przykładowy kod inicjalizacji
- Dokumentację protokołu komunikacji

## Dodatkowe zasoby

- [ESP32 Serial Documentation](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/uart.html)
- [Arduino Serial Reference](https://www.arduino.cc/reference/en/language/functions/communication/serial/)

---

**Powodzenia!** 🚀


