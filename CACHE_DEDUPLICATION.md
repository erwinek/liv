# Cache Deduplikacji Komend

## Data: 2025-10-17

## Problem
System przetwarzał te same komendy wielokrotnie, co powodowało:
- Niepotrzebne ładowanie GIF-ów z dysku
- Zbędne przetwarzanie tekstu
- Marnowanie zasobów CPU

## Rozwiązanie
Dodano mechanizm cache'owania który zapamiętuje checksumę ostatnich parametrów dla każdego `element_id` (0-255).

### Jak działa:
1. Gdy nadejdzie komenda GIF lub TEXT, system oblicza jej checksumę
2. Porównuje z poprzednią checksumą dla tego samego `element_id`
3. Jeśli checksumы są identyczne → komenda jest pomijana
4. Jeśli checksumы się różnią → komenda jest przetwarzana i cache aktualizowany

### Checksum obliczana z:
- **GIF:** element_id + x + y + width + height + filename
- **TEXT:** element_id + x + y + color (RGB) + text_length + text + font_name

## Implementacja

### Struktura cache (DisplayManager.h):
```cpp
struct CommandCache {
    uint32_t gif_checksums[256];   // One per element_id
    uint32_t text_checksums[256];
};
```

### Funkcje (DisplayManager.cpp):
- `calculateGifChecksum()` - oblicza checksumę GIF
- `calculateTextChecksum()` - oblicza checksumę TEXT
- `processGifCommand()` - sprawdza cache przed przetwarzaniem
- `processTextCommand()` - sprawdza cache przed przetwarzaniem
- `resetCache()` - resetuje wszystkie checksumы

### Automatyczne czyszczenie cache:
- `clearScreen()` - resetuje cały cache
- `clearText()` - resetuje cache dla usuniętych elementów TEXT
- `removeElement()` - resetuje cache dla usuwanego elementu

## Korzyści
✅ Brak ponownego ładowania identycznych GIF-ów  
✅ Brak zbędnego przetwarzania tekstu  
✅ Niskie zużycie pamięci (tylko 2KB)  
✅ Szybka checksuma O(n) gdzie n = długość parametrów  
✅ Automatyczne zarządzanie przy usuwaniu elementów  

## Przykład działania

### Przed cache:
```
ESP32 wysyła: loadGif("anim/1.gif", 0, 0, 192, 192, id=0)
RPi: Ładuje GIF... ✓

ESP32 wysyła ponownie: loadGif("anim/1.gif", 0, 0, 192, 192, id=0)
RPi: Ładuje GIF ponownie... ✓ (niepotrzebne!)
```

### Po cache:
```
ESP32 wysyła: loadGif("anim/1.gif", 0, 0, 192, 192, id=0)
RPi: Checksum=12345, ładuje GIF... ✓

ESP32 wysyła ponownie: loadGif("anim/1.gif", 0, 0, 192, 192, id=0)
RPi: Checksum=12345 (duplikat), pomijam ✓
```

## Logi

### Przy pierwszej komendzie:
```
Processing GIF command: ID=0 anim/1.gif at (0,0) size 192x192 (checksum=12345)
GIF loaded successfully, cache updated
```

### Przy duplikacie:
```
GIF command ID=0 is duplicate (checksum=12345), skipping processing
```

## Znane ograniczenia
1. **Prosta checksuma** - suma bajtów, może mieć kolizje (bardzo rzadkie)
2. **Brak timeout** - cache ważny do restartu aplikacji lub clear
3. **Brak statystyk** - nie zliczamy ile duplikatów zostało pominiętych

## Możliwe ulepszenia
- Użycie CRC32 zamiast prostej sumy (lepsza jakość)
- Dodanie timeout dla cache (np. 60 sekund)
- Statystyki cache hit/miss
- Persistent cache (zapis do pliku)

## Kompilacja
```bash
cd /home/erwinek/liv
make led-image-viewer
```

## Dodatkowe usprawnienia
W `SerialProtocol::init()` dodano automatyczne czyszczenie bufora UART:
```cpp
tcflush(serial_fd, TCIOFLUSH);
```
To pomaga przy inicjalizacji, czyści ewentualne śmieci z bufora.

## Pliki zmienione
- `DisplayManager.h` - dodano CommandCache, metody checksum i resetCache
- `DisplayManager.cpp` - implementacja cache'owania
- `SerialProtocol.cpp` - tcflush() przy init()

## Status
✅ Zaimplementowane i przetestowane kompilacją  
⏳ Do przetestowania w runtime przez użytkownika

