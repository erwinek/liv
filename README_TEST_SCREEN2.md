# Test Screen #2 - XVnx.gif Display

## ✅ DZIAŁA! / WORKING!

System do testowania ekranu #2 (64x512 pionowy) przez skrzyżowane porty szeregowe.

## Konfiguracja portów / Port Configuration

**Porty szeregowe są SKRZYŻOWANE (TX↔RX):**
- `led-image-viewer` słucha na: **/dev/ttyUSB0** (RX)
- `test_screen2_simple.py` wysyła przez: **/dev/ttyUSB1** (TX)

**Uwaga:** Numery portów USB mogą się zmieniać przy reconnect! Sprawdź `ls /dev/ttyUSB*`

## ✅ Potwierdzenie działania:

Data testu: 22 października 2025, 19:40
Status: **DZIAŁA POPRAWNIE**

```
GIF element added successfully. Total elements: 2
GIF loaded successfully, cache updated
```

## Uruchomienie / Running

### 1. Uruchom led-image-viewer dla ekranu #2:
```bash
cd /home/erwinek/liv
sudo ./run_screen2.sh
```

Lub bezpośrednio:
```bash
sudo ./bin/led-image-viewer --config screen_config_vertical.ini
```

### 2. Wyślij komendę GIF przez port testowy:
```bash
python3 test_screen2_simple.py
```

## Co robi test_screen2_simple.py?

1. Otwiera `/dev/ttyUSB1` (port wysyłający)
2. Tworzy pakiet zgodny z protokołem (PREAMBLE + SOF + GIF command + EOF)
3. Wysyła komendę LOAD_GIF:
   - Plik: `anim/XVnx.gif`
   - Pozycja: (0, 0)
   - Rozmiar: 64x128
   - Screen ID: 2
4. Zamyka port

## Weryfikacja / Verification

### Sprawdź logi:
```bash
tail -f /tmp/led_screen2.log
```

### Szukaj w logach:
```
✅ "GIF element added successfully" - GIF załadowany poprawnie
✅ "GIF loaded successfully, cache updated" - GIF na ekranie
✅ "RESPONSE SENT: code=0" - Odpowiedź OK
❌ "Bounds check FAILED" - Poza granicami ekranu
❌ "Failed to load GIF" - Błąd ładowania pliku
```

## Struktura pakietu / Packet Structure

```
[PREAMBLE: 0xAA 0x55 0xAA] 
[SOF: 0x55]
[Header: screen_id, command, payload_length]
[Payload: GifCommand structure - 75 bytes]
  - screen_id (1 byte)
  - command (1 byte)  
  - element_id (1 byte)
  - x_pos (2 bytes, little-endian)
  - y_pos (2 bytes, little-endian)
  - width (2 bytes, little-endian)
  - height (2 bytes, little-endian)
  - filename (64 bytes, null-terminated)
[Checksum: XOR of payload]
[EOF: 0xAA]
```

## Ekran #2 Specyfikacja / Screen #2 Specs

- Rozdzielczość: **64x512** pikseli
- Orientacja: **Pionowa** (8 paneli 64x64 od dołu do góry)
- Pixel mapper: **V-mapper** (mapuje łańcuch pionowo)
- Screen ID: **2**

## Troubleshooting

### Problem: "Failed to open serial port"
```bash
# Sprawdź które porty istnieją
ls -la /dev/ttyUSB*

# Upewnij się że masz uprawnienia
sudo chmod 666 /dev/ttyUSB1
sudo chmod 666 /dev/ttyUSB2
```

### Problem: "position out of bounds"
- Pozycja y + height nie może przekraczać 512
- Pozycja x + width nie może przekraczać 64
- Przykład OK: (0,0) 64x128
- Przykład ŹLE: (0,400) 64x128 → 400+128=528 > 512

### Problem: Packet nie dociera
1. Sprawdź czy led-image-viewer działa: `ps aux | grep led-image-viewer`
2. Sprawdź logi: `tail -f /tmp/led_screen2.log`
3. Upewnij się że porty są poprawne w config: `screen_config_vertical.ini`

## Pliki / Files

- `test_screen2_simple.py` - Skrypt testowy
- `screen_config_vertical.ini` - Konfiguracja ekranu #2
- `run_screen2.sh` - Skrypt uruchomieniowy
- `/tmp/led_screen2.log` - Logi aplikacji

## Status

✅ Protocol implementation - DZIAŁA
✅ Cross-connected ports (ttyUSB1 ↔ ttyUSB2) - DZIAŁA  
✅ Packet parsing - DZIAŁA
✅ GIF loading - DZIAŁA
✅ Display on Screen #2 - DZIAŁA

**Data testu: 22 października 2025**

