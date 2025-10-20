# Protokół z Preambułą - Reliable Frame Synchronization

## Problem

Ramki były gubione pomimo:
- ✅ 5ms opóźnienia między ramkami
- ✅ Poprawionego wykrywania ramek
- ✅ Zwiększonego bufora

**Przyczyna:** Na drucie był dużo randomowego ruchu (śmieci), a pojedynczy bajt SOF (0xAA) był niewystarczająco unikalny do niezawodnej synchronizacji.

## Rozwiązanie: Preambuła jak w Ethernet

### Nowy format ramki

**Przed zmianą:**
```
[SOF][screen_id][command][payload_length][payload...][checksum][EOF]
0xAA  ...        ...      ...             ...         ...       0x55
```
- **Problem:** 0xAA często występuje w śmieciach na linii

**Po zmianie (v1.2.0):**
```
[PREAMBLE_1][PREAMBLE_2][PREAMBLE_3][SOF][screen_id][command][payload_length][payload...][checksum][EOF]
   0xAA        0x55        0xAA      0x55  ...        ...      ...             ...         ...       0xAA
```
- **Wzór synchronizacyjny:** `0xAA 0x55 0xAA 0x55` (4 bajty)
- **Znacznie bardziej unikalny** - mała szansa na przypadkowe wystąpienie w śmieciach

### Inspiracja: Ethernet Preamble

W Ethernet:
- **Preamble:** 7 bajtów (0x55 0x55 0x55 0x55 0x55 0x55 0x55)
- **SFD (Start Frame Delimiter):** 1 bajt (0xD5)
- **Razem:** 8 bajtów synchronizacji

Nasza implementacja:
- **Preamble:** 3 bajty (0xAA 0x55 0xAA)
- **SOF:** 1 bajt (0x55)
- **Razem:** 4 bajty synchronizacji - wystarczające dla RS232 @ 1Mbps

## Zmiany w kodzie

### 1. Stałe protokołu

**Plik:** `SerialProtocol.h` i `LEDMatrix.h`

```cpp
// Przed:
#define PROTOCOL_SOF 0xAA  // Start of Frame
#define PROTOCOL_EOF 0x55  // End of Frame

// Po:
#define PROTOCOL_PREAMBLE_1 0xAA  // Preamble byte 1
#define PROTOCOL_PREAMBLE_2 0x55  // Preamble byte 2
#define PROTOCOL_PREAMBLE_3 0xAA  // Preamble byte 3
#define PROTOCOL_SOF 0x55         // Start of Frame (after preamble)
#define PROTOCOL_EOF 0xAA         // End of Frame
```

**Uwaga:** SOF i EOF **zamienione miejscami** - teraz SOF=0x55, EOF=0xAA

### 2. Nadajnik (ESP32)

**Plik:** `LEDMatrix/LEDMatrix.cpp`

```cpp
void LEDMatrix::sendPacket(uint8_t command, const uint8_t* payload, uint8_t payloadLength) {
    // Wysyłaj preambułę (3 bajty)
    _serial->write(PROTOCOL_PREAMBLE_1);    // 0xAA
    _serial->write(PROTOCOL_PREAMBLE_2);    // 0x55
    _serial->write(PROTOCOL_PREAMBLE_3);    // 0xAA
    
    // Wysyłaj właściwą ramkę
    _serial->write(PROTOCOL_SOF);           // 0x55
    _serial->write(_screenId);
    _serial->write(command);
    _serial->write(payloadLength);
    
    if (payload && payloadLength > 0) {
        _serial->write(payload, payloadLength);
    }
    
    _serial->write(checksum);
    _serial->write(PROTOCOL_EOF);           // 0xAA
    _serial->flush();
    delay(5);  // 5ms delay between frames
}
```

### 3. Odbiornik (Raspberry Pi)

**Plik:** `SerialProtocol.cpp`

#### Zwiększony bufor
```cpp
// Przed: 512 bajtów
// Po: 2048 bajtów
if (rx_buffer.size() > 2048) {
    rx_buffer.erase(rx_buffer.begin(), rx_buffer.begin() + 512);
}
```

#### Wykrywanie preambuły

```cpp
void SerialProtocol::processBuffer() {
    while (!rx_buffer.empty()) {
        // Szukaj sekwencji: [0xAA][0x55][0xAA][0x55]
        size_t preamble_position = rx_buffer.size();
        for (size_t i = 0; i + 3 < rx_buffer.size(); i++) {
            if (rx_buffer[i]   == PROTOCOL_PREAMBLE_1 &&  // 0xAA
                rx_buffer[i+1] == PROTOCOL_PREAMBLE_2 &&  // 0x55
                rx_buffer[i+2] == PROTOCOL_PREAMBLE_3 &&  // 0xAA
                rx_buffer[i+3] == PROTOCOL_SOF) {          // 0x55
                preamble_position = i;
                break;
            }
        }
        
        if (preamble_position == rx_buffer.size()) {
            // Brak preambuły - wyrzuć śmieci (zachowaj ostatnie 3 bajty)
            if (rx_buffer.size() > 3) {
                rx_buffer.erase(rx_buffer.begin(), 
                              rx_buffer.begin() + (rx_buffer.size() - 3));
            }
            return;
        }
        
        // Wyrzuć śmieci przed preambułą
        if (preamble_position > 0) {
            rx_buffer.erase(rx_buffer.begin(), 
                          rx_buffer.begin() + preamble_position);
        }
        
        // Przetwarzaj ramkę (preambuła na pozycji 0-3)
        // ... parsowanie ...
    }
}
```

## Korzyści

### ✅ Znacznie lepsza synchronizacja
- Wzór `0xAA 0x55 0xAA 0x55` jest bardzo unikalny
- Mała szansa na przypadkowe wystąpienie w śmieciach
- Odporność na burst noise

### ✅ Większy bufor (2048 bajtów)
- 4x większy niż poprzednio (512 bajtów)
- Lepsze radzenie sobie z szybkimi sekwencjami komend
- Odporność na chwilowe opóźnienia w przetwarzaniu

### ✅ Zachowanie ostatnich 3 bajtów
Przy czyszczeniu śmieci:
```cpp
// Zachowaj ostatnie 3 bajty na wypadek częściowo odebranej preambuły
// Np. jeśli odebrano [0xAA][0x55][0xAA] ale jeszcze nie [0x55]
if (garbage_size > 3) {
    rx_buffer.erase(rx_buffer.begin(), 
                  rx_buffer.begin() + (garbage_size - 3));
}
```

## Kompatybilność

### ⚠️ Breaking Change!

To jest **zmiana łamiąca kompatybilność wsteczną**:
- Stara wersja ESP32 (bez preambuły) NIE będzie działać z nowym odbiornikiem
- Nowy ESP32 (z preambułą) NIE będzie działać ze starym odbiornikiem

### Wymagane aktualizacje

**Aby system działał, MUSISZ zaktualizować OBA komponenty:**

1. **ESP32 (Arduino):**
   - Zaktualizuj bibliotekę `LEDMatrix` do wersji **1.2.0** lub nowszej
   - Wgraj nowy sketch do ESP32

2. **Raspberry Pi (C++):**
   - Zakompiluj i uruchom nowy `led-image-viewer`
   - Restart aplikacji

## Rozmiar ramki

### Przed zmianą
```
Minimalna ramka (clearScreen):
[SOF][ID][CMD][LEN][CHECKSUM][EOF] = 6 bajtów

Maksymalna ramka (TEXT):
[SOF][ID][CMD][LEN][PAYLOAD(77)][CHECKSUM][EOF] = 83 bajty
```

### Po zmianie
```
Minimalna ramka (clearScreen):
[PRE1][PRE2][PRE3][SOF][ID][CMD][LEN][CHECKSUM][EOF] = 9 bajtów (+3 bajty)

Maksymalna ramka (TEXT):
[PRE1][PRE2][PRE3][SOF][ID][CMD][LEN][PAYLOAD(77)][CHECKSUM][EOF] = 86 bajtów (+3 bajty)
```

**Overhead:** +3 bajty (3.6% dla maksymalnej ramki)
**Zysk:** Znacznie lepsza niezawodność!

## Wydajność

### Przy 1Mbps (1 000 000 bit/s = 125 000 bajt/s)

- **Wysłanie 1 ramki (86 bajtów):**
  - Transmisja: 86 / 125000 = 0.688 ms
  - Delay: 5 ms
  - **Razem: ~5.7 ms**

- **Sekwencja 10 ramek:**
  - Przed: ~57 ms
  - Po: ~57 ms (preambuła nie wpływa znacząco)

### Odporność na szum

**Prawdopodobieństwo przypadkowej preambuły:**

Zakładając losowe dane:
- Szansa na `0xAA`: 1/256
- Szansa na `0x55 0xAA 0x55`: (1/256)³ = 1/16 777 216

**Bardzo mała szansa na fałszywe wykrycie!**

## Testowanie

### Test 1: Ramka w czystym środowisku
```cpp
matrix.clearScreen();
```
**Wynik:** Działa natychmiast

### Test 2: Ramka ze śmieciami przed
```
Śmieci: [0x12][0x34][0x56]...[0xAA][0x55][0xAA][0x55][ramka]
```
**Wynik:** Śmieci są ignorowane, ramka wykryta poprawnie

### Test 3: Fałszywa preambuła
```
[0xAA][0x55][0xAA][0x12]... <- nie jest to prawidłowa preambuła (0x12 zamiast 0x55)
```
**Wynik:** Ignorowane, szukamy dalej

### Test 4: Sekwencja ramek
```cpp
for(int i=0; i<10; i++) {
    matrix.clearScreen();
    delay(10);
}
```
**Wynik:** Wszystkie 10 ramek dostarczonych poprawnie

## Debug

Przy problemach, sprawdź logi:
```
*** PREAMBLE+SOF found at position X [0xAA 0x55 0xAA 0x55]
Preamble verified, SOF at position 3
Packet header: screen_id=1 command=3 payload_length=0
Complete valid packet found, parsing...
>>> RESPONSE SENT: screen_id=1 code=0 bytes_written=9 (including preamble)
```

## Wersje

- **LEDMatrix (ESP32):** 1.2.0
- **led-image-viewer (RasPi):** 2025-10-20

## Powiązane dokumenty

- `FRAME_LOSS_FIX.md` - Poprzednie próby naprawy
- `PROTOCOL.md` - Pełna specyfikacja protokołu
- `LEDMatrix/CHANGELOG.md` - Historia zmian

## Autor

Implementacja preambuły: 2025-10-20
Inspiracja: IEEE 802.3 Ethernet Preamble

