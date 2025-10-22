# Screen Configuration Guide
# Przewodnik konfiguracji ekranów

## Przegląd / Overview

System LED Display Controller obsługuje różne konfiguracje ekranów LED poprzez pliki konfiguracyjne INI.

The LED Display Controller system supports different LED screen configurations through INI configuration files.

## Dostępne konfiguracje / Available Configurations

### 1. Ekran 192x192 (ID=1) - Domyślny
**Plik:** `screen_config.ini`

Konfiguracja dla ekranu składającego się z 9 paneli 64x64px (3x3):
- Rozdzielczość: 192x192 pikseli
- Panele: 3 poziomo × 3 pionowo
- Screen ID: 1

**Użycie:**
```bash
sudo ./bin/led-image-viewer --config screen_config.ini
```

### 2. Ekran 64x512 (ID=2) - Pionowy
**Plik:** `screen_config_vertical.ini`

Konfiguracja dla ekranu składającego się z 8 paneli 64x64px ułożonych pionowo:
- Rozdzielczość: 64x512 pikseli
- Panele: 8 paneli w JEDNYM łańcuchu (szeregowo połączone)
- Screen ID: 2
- **chain_length: 8** (8 paneli w łańcuchu)
- **parallel: 1** (1 łańcuch)
- **Pixel mapper: V-mapper** (wymagane dla układu pionowego)

**Użycie:**
```bash
sudo ./bin/led-image-viewer --config screen_config_vertical.ini
```

## Uruchamianie bez konfiguracji

Jeśli nie podasz pliku konfiguracyjnego, aplikacja użyje ustawień domyślnych (192x192, ID=1):
```bash
sudo ./bin/led-image-viewer
```

If you don't provide a configuration file, the application will use default settings (192x192, ID=1):

## Parametry konfiguracji / Configuration Parameters

### [screen]

| Parametr | Opis / Description | Przykład |
|----------|-------------------|----------|
| `screen_id` | ID ekranu (1-255) / Screen ID | `1` lub `2` |
| `rows` | Wysokość pojedynczego panelu / Panel height | `64` |
| `cols` | Szerokość pojedynczego panelu / Panel width | `64` |
| `chain_length` | Liczba paneli poziomo / Panels horizontally | `3` lub `1` |
| `parallel` | Liczba paneli pionowo / Panels vertically | `3` lub `8` |
| `hardware_mapping` | Mapowanie sprzętowe / Hardware mapping | `regular` |
| `pixel_mapper` | Mapowanie pikseli / Pixel mapping | puste lub `V-mapper` |
| `gpio_slowdown` | Spowolnienie GPIO (0-4) / GPIO slowdown | `3` |
| `serial_port` | Port szeregowy ESP32 / ESP32 serial port | `/dev/ttyUSB0` |
| `serial_baudrate` | Prędkość transmisji / Baud rate | `1000000` |
| `show_diagnostics` | Ekran testowy przy starcie / Show test screen | `true` lub `false` |

## Obliczanie całkowitej rozdzielczości / Calculating Total Resolution

```
Szerokość = cols × chain_length
Wysokość = rows × parallel

Przykład 1: 64 × 3 = 192px (szerokość), 64 × 3 = 192px (wysokość)
Przykład 2: 64 × 1 = 64px (szerokość), 64 × 8 = 512px (wysokość)
```

## Tworzenie własnej konfiguracji / Creating Custom Configuration

1. Skopiuj jeden z przykładowych plików konfiguracyjnych:
   ```bash
   cp screen_config.ini my_screen.ini
   ```

2. Edytuj parametry według swojej konfiguracji sprzętowej:
   ```bash
   nano my_screen.ini
   ```

3. Uruchom z nową konfiguracją:
   ```bash
   sudo ./bin/led-image-viewer --config my_screen.ini
   ```

## Typowe konfiguracje / Common Configurations

### Pojedynczy panel 64x64
```ini
rows = 64
cols = 64
chain_length = 1
parallel = 1
```
Rozdzielczość: 64×64

### 2×2 panele (128x128)
```ini
rows = 64
cols = 64
chain_length = 2
parallel = 2
```
Rozdzielczość: 128×128

### 4×4 panele (256x256)
```ini
rows = 64
cols = 64
chain_length = 4
parallel = 4
```
Rozdzielczość: 256×256

### Pasek poziomy (4 panele)
```ini
rows = 64
cols = 64
chain_length = 4
parallel = 1
```
Rozdzielczość: 256×64

### Pasek pionowy (4 panele)
```ini
rows = 64
cols = 64
chain_length = 4
parallel = 1
pixel_mapper = V-mapper
```
Rozdzielczość: 64×256
**Uwaga:** Dla układów pionowych użyj `chain_length = N` (N paneli w łańcuchu), `parallel = 1` i `pixel_mapper = V-mapper`

## Dodatkowe opcje linii poleceń / Additional Command Line Options

### Wyłączenie ekranu diagnostycznego
```bash
sudo ./bin/led-image-viewer --config screen_config.ini --no-diagnostics
```

### Ładowanie GIF-ów przy starcie (tryb kompatybilności wstecznej)
```bash
sudo ./bin/led-image-viewer anim/1.gif anim/2.gif anim/3.gif anim/4.gif
```

## Dwuekranowy system / Dual Screen System

Aby uruchomić 2 ekrany na 2 Raspberry Pi 4:

### Raspberry Pi #1 (Ekran 192x192, ID=1)
```bash
sudo ./bin/led-image-viewer --config screen_config.ini
```

### Raspberry Pi #2 (Ekran 64x512, ID=2)
```bash
sudo ./bin/led-image-viewer --config screen_config_vertical.ini
```

**Ważne:** Każdy ekran musi mieć unikalny `screen_id` aby ESP32 mogło je rozróżnić podczas komunikacji szeregowej.

**Important:** Each screen must have a unique `screen_id` so the ESP32 can distinguish them during serial communication.

## Rozwiązywanie problemów / Troubleshooting

### Błąd: "Failed to create RGB matrix"
- Sprawdź czy parametry `rows`, `cols`, `chain_length`, `parallel` są poprawne
- Upewnij się że uruchamiasz z `sudo`

### Błąd: "Failed to initialize serial protocol"
- Sprawdź czy ESP32 jest podłączone
- Sprawdź czy `serial_port` jest poprawny (użyj `ls /dev/ttyUSB*`)
- Dodaj uprawnienia: `sudo chmod 666 /dev/ttyUSB0`

### Nieprawidłowy obraz na ekranie
- Sprawdź `hardware_mapping` (spróbuj `regular`, `adafruit-hat`, `regular-pi1`)
- Dostosuj `gpio_slowdown` (zwiększ jeśli obraz jest zniekształcony)
- **Dla pionowych ekranów:** Użyj `pixel_mapper = V-mapper`
- Dla innych układów pixel mapper obsługuje: `V-mapper`, `U-mapper`, `Rotate:90`, `Rotate:180`, `Rotate:270`

## Zobacz także / See Also

- `PROTOCOL.md` - Dokumentacja protokołu komunikacyjnego
- `README.md` - Ogólna dokumentacja projektu
- `LEDMatrix/README.md` - Biblioteka ESP32

