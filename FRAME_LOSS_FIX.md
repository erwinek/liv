# Naprawa Gubienia Ramek - Frame Loss Fix

## Problem

Podczas wysyłania wielu komend pod rząd z ESP32 do Raspberry Pi, ramki były gubione:
- `clearScreen()` nie działał
- Migające napisy znikały
- Komendy wysyłane 3x nadal nie docierały

### Przykład problematycznego kodu:
```cpp
matrix.loadGif("anim/1.gif", 0, 0, 192, 192, 0);           // 80 bajtów
matrix.displayText("...", 10, 170, 2, 255, 255, 0, ...);   // 83 bajty
matrix.displayText("...", 25, 5, 2, 255, 255, 0, ...);     // 83 bajty  
matrix.displayText("...", 40, 25, 2, 255, 255, 0, ...);    // 83 bajty
```

Wszystkie 4 ramki (łącznie ~330 bajtów) były wysyłane praktycznie jednocześnie.

## Przyczyna

### 1. Przeplatanie się ramek przy wysokim baudrate (1Mbps)
Nawet z `flush()`, gdy ramki są wysyłane jedna za drugą bez opóźnienia, mogą się przeplatać w buforze UART:
```
Ramka 1: [SOF][...][EOF]
Ramka 2:                [SOF][...][EOF]
                           ↑
                    Może nadejść zanim Ramka 1 
                    zostanie w pełni przetworzona
```

### 2. Fałszywe wykrywanie SOF w payloadzie (naprawione)
Poprzednia implementacja sprawdzała czy wewnątrz payloadu jest bajt `0xAA` (SOF) i traktowała to jako uszkodzoną ramkę. Ale payload **może legalnie zawierać 0xAA**:
- Kolor RGB gdzie R, G lub B = 170 (0xAA) ← **częsty przypadek!**
- Współrzędne X, Y = 170
- Znak 'ª' w tekście

To powodowało wyrzucanie **poprawnych** ramek!

## Rozwiązanie

### 1. Opóźnienie między ramkami (ESP32)
**Plik:** `LEDMatrix/LEDMatrix.cpp`

Dodano 5ms opóźnienie po każdej ramce w funkcji `sendPacket()`:

```cpp
_serial->write(PROTOCOL_EOF);
_serial->flush();

// Critical delay to prevent frame overlap at high baudrate
// At 1Mbps, even with flush(), back-to-back frames can interfere
// 5ms gives receiver time to process the frame
delay(5);
```

**Dlaczego 5ms?**
- Przy 1Mbps: 80 bajtów = ~0.64ms transmisji
- 5ms daje czas na:
  - Pełną transmisję ramki
  - Przetworzenie przez odbiorcę
  - Wysłanie odpowiedzi (jeśli potrzeba)

### 2. Poprawione wykrywanie uszkodzonych ramek (Raspberry Pi)
**Plik:** `SerialProtocol.cpp`

Usunięto fałszywe sprawdzanie SOF wewnątrz payloadu. Teraz protokół:

1. ✅ Znajduje SOF na początku bufora
2. ✅ Czyta header i oblicza rozmiar pakietu  
3. ✅ Czeka na kompletny pakiet
4. ✅ Sprawdza czy EOF jest na właściwej pozycji
5. ✅ Jeśli EOF się nie zgadza → ten SOF był fałszywy, szukaj następnego

**Bazujemy tylko na strukturze ramki (SOF...EOF), nie na zawartości payloadu!**

### 3. Szczegółowe debugowanie (Raspberry Pi)

Dodano logowanie dla diagnozowania problemów:
```cpp
// Każdy odebrany bajt
std::cout << "RX[" << rx_buffer.size() << "]: 0x" << hex << (int)byte << std::endl;

// Wykrycie SOF
std::cout << "*** SOF received (0xAA) at buffer position " << rx_buffer.size() << std::endl;

// Wysłana odpowiedź
std::cout << ">>> RESPONSE SENT: screen_id=" << (int)screen_id << " code=" << (int)code << std::endl;
```

## Rezultat

✅ **clearScreen() działa poprawnie**
✅ **Migające napisy są widoczne**  
✅ **Niezawodna komunikacja nawet przy wielu komendach**
✅ **Brak fałszywych alarmów o uszkodzonych ramkach**

## Testy

### Test 1: Wiele komend pod rząd
```cpp
matrix.clearScreen();
delay(10);
matrix.loadGif("anim/1.gif", 0, 0, 192, 192, 0);
matrix.displayText("Insert Coin", 10, 170, 2, 255, 255, 0, "fonts/9x18B.bdf", 1, 400);
matrix.displayText("PRO-GAMES", 25, 5, 2, 255, 255, 0, "fonts/9x18B.bdf", 2, 0);
matrix.displayText("Monster 3in1", 40, 25, 2, 255, 255, 0, "fonts/7x13.bdf", 3, 0);
```
**Wynik:** Wszystkie komendy są wykonywane poprawnie

### Test 2: Szybka sekwencja clearScreen()
```cpp
for(int i=0; i<10; i++) {
    matrix.clearScreen();
    delay(10);
}
```
**Wynik:** Ekran jest czyszczony przy każdej iteracji

### Test 3: Kolory zawierające 0xAA
```cpp
// Kolor (170, 255, 0) - żółto-zielony
matrix.displayText("TEST", 50, 50, 2, 170, 255, 0, "fonts/9x18B.bdf", 1);
```
**Wynik:** Tekst jest wyświetlany poprawnie (wcześniej ramka była odrzucana)

## Wersje

- **LEDMatrix:** 1.1.1
- **led-image-viewer:** Z poprawkami z 2025-10-20

## Pliki zmienione

### ESP32 (Arduino)
- `LEDMatrix/LEDMatrix.cpp` - dodano 5ms delay
- `LEDMatrix/CHANGELOG.md` - zaktualizowano

### Raspberry Pi (C++)
- `SerialProtocol.cpp` - poprawiono wykrywanie ramek, dodano debugowanie
- `SerialProtocol.h` - bez zmian

## Uwagi

### Wydajność
5ms opóźnienie między ramkami:
- Dla 4 ramek = 20ms dodatkowego czasu
- Nieznaczny wpływ na responsywność
- **Wielokrotnie lepsze niż gubienie ramek i 3x retransmisja!**

### Alternatywne rozwiązania (odrzucone)
1. **Zwiększenie opóźnienia do 10ms** - niepotrzebne, 5ms wystarczy
2. **Czekanie na odpowiedź** - bardziej złożone, niepotrzebne przy 5ms delay
3. **Zmniejszenie baudrate** - niepotrzebne, problem rozwiązany

## Autor
Naprawa wykonana: 2025-10-20

