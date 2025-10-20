# Delete Element Feature - Usuwanie pojedynczych elementów

## Opis

Nowa komenda **DELETE_ELEMENT** pozwala na precyzyjne usuwanie pojedynczych elementów graficznych (GIF lub TEXT) z ekranu po ich `element_id`.

## Motywacja

Poprzednio dostępne były tylko:
- `clearScreen()` - usuwa WSZYSTKIE elementy (GIF + TEXT)
- `clearText()` - usuwa tylko teksty, pozostawia GIFy

**Problem:** Nie można było usunąć konkretnego elementu bez wpływu na pozostałe.

**Rozwiązanie:** `deleteElement(elementId)` - usuwa dokładnie ten element, który chcesz.

## API

### ESP32 (Arduino)

```cpp
// Usuń element o konkretnym ID
matrix.deleteElement(elementId);

// Parametry:
//   elementId (uint8_t) - ID elementu do usunięcia (0-255)
```

### Przykład użycia

```cpp
// Wyświetl GIF w tle
matrix.loadGif("anim/background.gif", 0, 0, 192, 192, 0);

// Dodaj tekst powitalny
matrix.displayText("Welcome!", 50, 50, 2, 255, 255, 0, "fonts/9x18B.bdf", 1);

// Dodaj migający tekst
matrix.displayText("INSERT COIN", 40, 100, 2, 255, 0, 0, "fonts/9x18B.bdf", 2, 500);

// Po 5 sekundach - usuń tylko migający tekst (ID=2)
delay(5000);
matrix.deleteElement(2);  // Usuwa "INSERT COIN"
                          // GIF i "Welcome!" nadal widoczne!

// Usuń tekst powitalny (ID=1)
delay(2000);
matrix.deleteElement(1);  // Usuwa "Welcome!"
                          // Tylko GIF nadal widoczny
```

### Scenariusze użycia

#### 1. Dynamiczne menu
```cpp
// Wyświetl opcje menu
matrix.displayText("1. Start Game", 10, 50, 2, 255, 255, 255, "fonts/9x18B.bdf", 10);
matrix.displayText("2. Settings", 10, 80, 2, 255, 255, 255, "fonts/9x18B.bdf", 11);
matrix.displayText("3. Exit", 10, 110, 2, 255, 255, 255, "fonts/9x18B.bdf", 12);

// Podświetl wybraną opcję
matrix.displayText(">", 0, 50, 2, 255, 255, 0, "fonts/9x18B.bdf", 20);

// Użytkownik nacisnął "dół" - przesuń kursor
matrix.deleteElement(20);  // Usuń stary kursor
matrix.displayText(">", 0, 80, 2, 255, 255, 0, "fonts/9x18B.bdf", 20);  // Nowy kursor
```

#### 2. Animowany countdown
```cpp
for(int i = 3; i > 0; i--) {
    matrix.deleteElement(100);  // Usuń poprzednią cyfrę
    
    String countdown = String(i);
    matrix.displayText(countdown.c_str(), 90, 90, 2, 255, 0, 0, 
                      "fonts/ComicNeue-Bold-48.bdf", 100);
    delay(1000);
}
matrix.deleteElement(100);  // Usuń "0"
matrix.displayText("GO!", 70, 90, 2, 0, 255, 0, "fonts/ComicNeue-Bold-48.bdf", 101);
```

#### 3. Rotujące komunikaty
```cpp
const char* messages[] = {"READY", "SET", "GO!"};
uint8_t msgId = 50;

for(int i = 0; i < 3; i++) {
    if(i > 0) matrix.deleteElement(msgId);  // Usuń poprzedni komunikat
    
    matrix.displayText(messages[i], 60, 90, 2, 255, 255, 0, 
                      "fonts/9x18B.bdf", msgId);
    delay(1000);
}
```

#### 4. Powiadomienia tymczasowe
```cpp
void showNotification(const char* text, uint16_t duration_ms) {
    const uint8_t NOTIFICATION_ID = 200;
    
    // Usuń poprzednie powiadomienie jeśli istnieje
    matrix.deleteElement(NOTIFICATION_ID);
    
    // Wyświetl nowe
    matrix.displayText(text, 20, 150, 2, 255, 128, 0, "fonts/9x18B.bdf", 
                      NOTIFICATION_ID, 300);  // Migające
    
    // Zaplanuj usunięcie
    delay(duration_ms);
    matrix.deleteElement(NOTIFICATION_ID);
}

// Użycie:
showNotification("Coin inserted!", 2000);
showNotification("Game starting...", 3000);
```

## Protokół

### Format ramki

```
[PREAMBLE][SOF][screen_id][CMD_DELETE_ELEMENT][payload_length][element_id][checksum][EOF]
[AA 55 AA][55][    01    ][        07        ][      01      ][element_id][checksum][AA]
```

### Payload

```cpp
typedef struct {
    uint8_t screen_id;
    uint8_t command;
    uint8_t element_id;    // ID elementu do usunięcia (0-255)
} __attribute__((packed)) DeleteElementCommand;
```

**Rozmiar payloadu:** 1 bajt (tylko element_id)  
**Całkowity rozmiar ramki:** 9 bajtów (z preambułą)

### Odpowiedź

- **RESP_OK (0x00)** - Element usunięty pomyślnie
- **RESP_ERROR (0x01)** - Element o podanym ID nie został znaleziony

## Implementacja

### Raspberry Pi (C++)

#### DisplayManager::processDeleteElementCommand()

```cpp
void DisplayManager::processDeleteElementCommand(DeleteElementCommand* cmd) {
    bool found = false;
    for (auto it = elements.begin(); it != elements.end(); ++it) {
        if (it->element_id == cmd->element_id) {
            // Wyczyść cache
            if (it->type == DisplayElement::GIF) {
                command_cache.gif_checksums[it->element_id] = 0;
            } else if (it->type == DisplayElement::TEXT) {
                command_cache.text_checksums[it->element_id] = 0;
            }
            
            // Usuń element
            elements.erase(it);
            display_dirty = true;
            found = true;
            break;
        }
    }
    
    if (found) {
        serial_protocol.sendResponse(cmd->screen_id, RESP_OK);
    } else {
        serial_protocol.sendResponse(cmd->screen_id, RESP_ERROR);
    }
}
```

### ESP32 (Arduino)

#### LEDMatrix::deleteElement()

```cpp
void LEDMatrix::deleteElement(uint8_t elementId) {
    uint8_t payload[1];
    payload[0] = elementId;
    
    sendPacket(CMD_DELETE_ELEMENT, payload, 1);
}
```

## Różnice między komendami czyszczenia

| Komenda | Co usuwa | Przykład użycia |
|---------|----------|-----------------|
| `clearScreen()` | WSZYSTKIE elementy (GIF + TEXT) | Reset całego ekranu przed nową sceną |
| `clearText()` | Tylko elementy TEXT | Zmiana tekstów, zachowanie GIF w tle |
| `deleteElement(id)` | Jeden konkretny element | Precyzyjne zarządzanie pojedynczymi elementami |

## Zalety

✅ **Precyzja** - Usuń tylko to, co chcesz  
✅ **Wydajność** - Nie trzeba odświeżać całego ekranu  
✅ **Elastyczność** - Dynamiczne zarządzanie treścią  
✅ **Czytelność kodu** - Jasne intencje

## Wersje

- **LEDMatrix (ESP32):** 1.2.0
- **led-image-viewer (RasPi):** 2025-10-20

## Kompatybilność

Wymaga:
- Biblioteki LEDMatrix v1.2.0 lub nowszej (ESP32)
- led-image-viewer z obsługą DELETE_ELEMENT (RasPi)

## Testowanie

### Test 1: Podstawowe usuwanie
```cpp
// Dodaj 3 elementy
matrix.displayText("Text 1", 10, 10, 2, 255, 0, 0, "fonts/9x18B.bdf", 1);
matrix.displayText("Text 2", 10, 40, 2, 0, 255, 0, "fonts/9x18B.bdf", 2);
matrix.displayText("Text 3", 10, 70, 2, 0, 0, 255, "fonts/9x18B.bdf", 3);

delay(2000);

// Usuń środkowy
matrix.deleteElement(2);  
// Widoczne: "Text 1" i "Text 3"
```

### Test 2: Usuwanie nieistniejącego elementu
```cpp
matrix.deleteElement(99);  
// Odpowiedź: RESP_ERROR (element nie istnieje)
// Bez wpływu na wyświetlane elementy
```

### Test 3: Mieszane typy elementów
```cpp
matrix.loadGif("anim/1.gif", 0, 0, 96, 96, 10);
matrix.displayText("Hello", 100, 50, 2, 255, 255, 0, "fonts/9x18B.bdf", 11);

delay(2000);

matrix.deleteElement(10);  // Usuwa GIF
// Widoczny: tylko "Hello"

delay(2000);

matrix.deleteElement(11);  // Usuwa TEXT
// Ekran pusty
```

## Debugowanie

Logi w konsoli Raspberry Pi:
```
Processing DELETE_ELEMENT command: element_id=2
Deleting element ID=2 type=TEXT
Element deleted successfully. Remaining elements: 2

Processing DELETE_ELEMENT command: element_id=99
Element ID=99 not found
>>> RESPONSE SENT: screen_id=1 code=1 (ERROR)
```

## Powiązane dokumenty

- `PROTOCOL.md` - Pełna specyfikacja protokołu
- `PREAMBLE_PROTOCOL.md` - Protokół z preambułą
- `LEDMatrix/README.md` - Dokumentacja biblioteki

## Autor

Implementacja: 2025-10-20

