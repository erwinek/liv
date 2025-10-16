# ESP32 Demo - Wyświetlanie tekstu na matrycy LED

## Opis

Ten przykład pokazuje jak wyświetlić tekst "Esp32" na środku matrycy LED 96x96 przy użyciu biblioteki LEDMatrix.

## Wymagania

- ESP32 Dev Module lub kompatybilna płytka
- Połączenie szeregowe między ESP32 a urządzeniem z led-image-viewer
- arduino-cli zainstalowane i skonfigurowane
- Biblioteka LEDMatrix

## Połączenie sprzętowe

### Wariant 1: ESP32 -> USB-Serial -> Raspberry Pi
```
ESP32 Serial1 (TX=GPIO17, RX=GPIO16) <-> USB-Serial Adapter <-> Raspberry Pi USB
```

### Wariant 2: ESP32 -> Raspberry Pi GPIO
```
ESP32 TX (GPIO17) -> Raspberry Pi RX (GPIO15)
ESP32 RX (GPIO16) -> Raspberry Pi TX (GPIO14)
ESP32 GND         -> Raspberry Pi GND
```

## Kompilacja i upload

### 1. Kompilacja
```bash
arduino-cli compile --fqbn esp32:esp32:esp32 --library /home/erwinek/liv/LEDMatrix /home/erwinek/liv/LEDMatrix/examples/Esp32Demo
```

### 2. Upload na ESP32
```bash
arduino-cli upload -p /dev/ttyUSB0 --fqbn esp32:esp32:esp32 /home/erwinek/liv/LEDMatrix/examples/Esp32Demo
```

### 3. Monitor szeregowy (debug)
```bash
arduino-cli monitor -p /dev/ttyUSB0 -c baudrate=115200
```

## Konfiguracja led-image-viewer

Na urządzeniu docelowym (np. Raspberry Pi) uruchom led-image-viewer:

```bash
cd /home/erwinek/liv
sudo ./bin/led-image-viewer /dev/ttyUSB0 1000000
```

**Uwaga:**  
- ESP32 używa tego samego portu UART co do programowania (/dev/ttyUSB0)
- Uruchom led-image-viewer PRZED lub PO uruchomieniu ESP32  
- ESP32 wysyła komendy co 5 sekund, więc nawet jeśli pierwsze pakiety są zagłuszone przez boot messages, kolejne będą działać

## Protokół komunikacji

ESP32 wysyła komendy do led-image-viewer przy użyciu protokołu szeregowego:

- **Baud rate:** 1000000 bps
- **Format pakietu:** SOF + Screen_ID + Command + Payload_Length + Payload + Checksum + EOF
- **TextCommand:** Zawiera pozycję (x, y), rozmiar czcionki, kolor RGB i tekst

## Modyfikacja przykładu

Możesz zmodyfikować tekst, pozycję, kolor i rozmiar czcionki w funkcji `setup()`:

```cpp
// Wyświetl własny tekst
matrix.displayText("Twoj tekst", x, y, fontSize, r, g, b);

// Opcjonalnie użyj custom BDF font
matrix.displayText("Tekst", 10, 30, 2, 255, 0, 0, "fonts/ComicNeue-Regular-20.bdf");
```

## Debug

Komunikaty debug są wysyłane przez Serial (USB) z prędkością 115200 bps. Możesz monitorować je używając:

```bash
arduino-cli monitor -p /dev/ttyUSB0 -c baudrate=115200
```

lub dowolnego terminala szeregowego (np. screen, minicom, putty).

## Rozwiązywanie problemów

### ESP32 nie łączy się
- Sprawdź czy port szeregowy jest poprawny: `ls -la /dev/ttyUSB*`
- Upewnij się, że użytkownik ma uprawnienia do portu: `sudo usermod -a -G dialout $USER`
- Spróbuj nacisnąć przycisk BOOT podczas uploadu

### led-image-viewer nie odbiera komend
- Sprawdź czy led-image-viewer jest uruchomiony
- Zweryfikuj połączenie sprzętowe (TX->RX, RX->TX, GND->GND)
- Upewnij się, że baud rate jest zgodny (1000000 bps)
- Sprawdź logi led-image-viewer: `tail -f logs/led_output.log`

### Tekst nie wyświetla się na matrycy
- Sprawdź pozycję tekstu (x, y) - czy mieści się w wymiarach matrycy (96x96)
- Zweryfikuj kolor (nie używaj czarnego: 0,0,0)
- Upewnij się, że ekran został wyczyszczony przed wyświetleniem

