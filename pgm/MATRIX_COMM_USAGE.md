# MatrixComm - High Priority Serial Communication

## Problem

W aplikacji `pgm.ino` po uderzeniu (lub innych intensywnych operacjach) komunikaty przez UART do matrycy LED mogą być opóźnione, ponieważ główny `loop()` ma **niski priorytet (1)**.

## Rozwiązanie

**MatrixComm** - dedykowany high-priority task (priorytet: `configMAX_PRIORITIES - 2`) działający na **Core 0**, który obsługuje całą komunikację Serial z matrycą LED.

### Architektura

```
┌─────────────────────────────────────────────────────┐
│  loop() - Core 1, Priority 1                        │
│  ├─ Obsługa gry, sensory, LEDs                     │
│  └─ MatrixComm_DisplayText() → Queue               │
└─────────────────────────────────────────────────────┘
                       ↓ (Queue)
┌─────────────────────────────────────────────────────┐
│  MatrixCommTask - Core 0, Priority (MAX-2)          │
│  ├─ Czeka na komendy z kolejki                     │
│  ├─ Wysyła przez Serial (1Mbps)                    │
│  └─ Bardzo szybki response (wysoki priorytet!)     │
└─────────────────────────────────────────────────────┘
```

### Korzyści

✅ **Wysoki priorytet** - komunikacja Serial nie jest blokowana przez inne operacje  
✅ **Oddzielny rdzeń (Core 0)** - nie konkuruje z loop() (Core 1)  
✅ **Non-blocking** - komendy idą do kolejki, loop() nie czeka  
✅ **Niezawodne** - nawet przy intensywnych operacjach (np. uderzenie, LED effects)  
✅ **Kolejka 20 komend** - burst komend jest obsługiwany

## Jak użyć w pgm.ino

### 1. Dodaj nagłówek

```cpp
#include "matrix_comm.h"  // Dodaj na początku pliku
```

### 2. Inicjalizacja w setup()

```cpp
void setup() {
    // ... istniejący kod ...
    
    matrix.begin(1000000);  // Normalna inicjalizacja
    
    // Inicjalizuj high-priority task dla komunikacji
    MatrixCommInit(&matrix);
    
    // Od teraz używaj MatrixComm_* zamiast matrix.*
}
```

### 3. Zamień wywołania matrix.* na MatrixComm_*

**Przed:**
```cpp
matrix.clearScreen();
matrix.displayText("ProGames", 0, 0, 2, 255, 255, 255, "fonts/ComicNeue-Bold-48.bdf", 1);
matrix.loadGif("anim/1.gif", 0, 0, 192, 192, 0);
```

**Po:**
```cpp
MatrixComm_ClearScreen();
MatrixComm_DisplayText("ProGames", 0, 0, 2, 255, 255, 255, "fonts/ComicNeue-Bold-48.bdf", 1);
MatrixComm_LoadGif("anim/1.gif", 0, 0, 192, 192, 0);
```

### 4. Przykład w funkcji Choinka()

```cpp
void Choinka() {
    // ... 
    
    switch(LedEffectCnt) {
        case 0:
            // Zamiast:
            // matrix.loadGif("anim/1.gif", 0, 0, 192, 192, 0);
            // matrix.displayText("*$* Insert Coin *$*", 10, 170, 2, 255, 255, 0, "fonts/9x18B.bdf", 1, 400);
            
            // Użyj:
            MatrixComm_LoadGif("anim/1.gif", 0, 0, 192, 192, 0);
            MatrixComm_DisplayText("*$* Insert Coin *$*", 10, 170, 2, 255, 255, 0, 
                                  "fonts/9x18B.bdf", 1, 400);
            MatrixComm_DisplayText("PRO-GAMES POLAND", 25, 5, 2, 255, 255, 0, 
                                  "fonts/9x18B.bdf", 2, 0);
            MatrixComm_DisplayText("* Monster 3in1 *", 40, 25, 2, 255, 255, 0, 
                                  "fonts/7x13.bdf", 3, 0);
            break;
    }
}
```

### 5. Przykład w funkcji Pomiar()

```cpp
bool Pomiar() {
    // ...
    
    if ((TimeSens0Stop > 0) && (TimeSens0Start > 0)) {
        // Zamiast:
        // matrix.clearScreen();
        // matrix.loadGif("anim/8.gif", 0, 0, 192, 192, 0);
        
        // Użyj:
        MatrixComm_ClearScreen();
        MatrixComm_LoadGif("anim/8.gif", 0, 0, 192, 192, 0);
        
        // ... reszta kodu ...
    }
}
```

## API

### MatrixComm_LoadGif()
```cpp
MatrixComm_LoadGif(filename, x, y, width, height, elementId);
```

### MatrixComm_DisplayText()
```cpp
MatrixComm_DisplayText(text, x, y, fontSize, r, g, b, fontName, elementId, blinkInterval);
```

### MatrixComm_ClearScreen()
```cpp
MatrixComm_ClearScreen();
```

### MatrixComm_ClearText()
```cpp
MatrixComm_ClearText();
```

### MatrixComm_DeleteElement()
```cpp
MatrixComm_DeleteElement(elementId);
```

### MatrixComm_SetBrightness()
```cpp
MatrixComm_SetBrightness(brightness);  // 0-100
```

## Różnice vs bezpośrednie wywołania

| Aspekt | matrix.* (stary) | MatrixComm_* (nowy) |
|--------|------------------|---------------------|
| Priorytet | 1 (loop) | MAX-2 (wysoki) |
| Core | 1 | 0 |
| Blocking | Tak (delay 5ms) | Nie (kolejka) |
| Opóźnienia | Możliwe przy intensywnych operacjach | Minimalne |
| Burst komend | Może gubić | Kolejka 20 komend |

## Diagnostyka

W Serial Monitor zobaczysz:
```
MatrixComm task started with high priority on Core 0
```

Sprawdź priorytety tasków:
```cpp
void printTaskInfo() {
    Serial.printf("loop() priority: %d, core: %d\n", 
                  uxTaskPriorityGet(NULL), xPortGetCoreID());
    Serial.printf("MatrixComm priority: %d\n", 
                  uxTaskPriorityGet(MatrixCommTaskHandle));
    Serial.printf("TaskPcf priority: %d\n", 
                  uxTaskPriorityGet(TaskPcfHandle));
}

// W setup():
printTaskInfo();
```

Wyjście:
```
loop() priority: 1, core: 1
MatrixComm priority: 23   (configMAX_PRIORITIES-2 = 25-2)
TaskPcf priority: 24      (configMAX_PRIORITIES-1 = 25-1)
```

## Kolejność priorytetów

```
Najwyższy:  TaskPcf        (24) - obsługa PCF8574, przyciski, coin
            MatrixComm     (23) - komunikacja Serial z matrycą ← NOWE!
            ...
Najniższy:  loop()         (1)  - główna logika gry
```

## Ważne uwagi

### 1. Kolejka vs bezpośrednie wywołania
```cpp
// ❌ Źle - można przegapić komendę jeśli kolejka pełna
for(int i=0; i<100; i++) {
    MatrixComm_DisplayText(...);  // Po 20 iteracjach kolejka pełna!
}

// ✅ Dobrze - ograniczona liczba komend
MatrixComm_ClearScreen();
MatrixComm_DisplayText("SCORE", ...);
MatrixComm_DisplayText("RECORD", ...);
```

### 2. Rozmiar kolejki
Domyślnie: **20 komend**

Jeśli potrzebujesz więcej, zmień w `matrix_comm.cpp`:
```cpp
matrixCommandQueue = xQueueCreate(50, sizeof(MatrixCommand));  // 20 → 50
```

### 3. Stack size
Domyślnie: **8KB**

Jeśli widzisz stack overflow, zwiększ w `matrix_comm.cpp`:
```cpp
xTaskCreatePinnedToCore(
    MatrixCommTask,
    "MatrixComm",
    16384,  // 8192 → 16384 (16KB)
    // ...
```

## Migracja krok po kroku

### Krok 1: Dodaj pliki
- Skopiuj `matrix_comm.h` do folderu `pgm/`
- Skopiuj `matrix_comm.cpp` do folderu `pgm/`

### Krok 2: Dodaj include
```cpp
#include "matrix_comm.h"
```

### Krok 3: Inicjalizuj w setup()
```cpp
MatrixCommInit(&matrix);
```

### Krok 4: Znajdź i zamień (Find & Replace)
```
Znajdź:    matrix.clearScreen()
Zamień na: MatrixComm_ClearScreen()

Znajdź:    matrix.displayText(
Zamień na: MatrixComm_DisplayText(

Znajdź:    matrix.loadGif(
Zamień na: MatrixComm_LoadGif(

Znajdź:    matrix.clearText()
Zamień na: MatrixComm_ClearText()

Znajdź:    matrix.deleteElement(
Zamień na: MatrixComm_DeleteElement(
```

### Krok 5: Testuj
Upload do ESP32 i obserwuj:
- Czy komunikaty są szybsze?
- Czy nie ma opóźnień po uderzeniu?
- Czy wszystkie komendy dochodzą?

## Troubleshooting

### Problem: Komendy nie dochodzą
**Rozwiązanie:** Zwiększ rozmiar kolejki (domyślnie 20)

### Problem: Stack overflow
**Rozwiązanie:** Zwiększ stack size taska (domyślnie 8KB)

### Problem: Task nie startuje
**Rozwiązanie:** Sprawdź Serial Monitor, powinno być:
```
MatrixComm task started with high priority on Core 0
```

## Alternatywne podejście (jeśli MatrixComm za skomplikowane)

Możesz też po prostu podnieść priorytet loop():

```cpp
void setup() {
    // ... istniejący kod ...
    
    // Podnieś priorytet loop()
    vTaskPrioritySet(NULL, configMAX_PRIORITIES - 3);  // NULL = current task (loop)
    
    Serial.printf("loop() priority set to: %d\n", uxTaskPriorityGet(NULL));
}
```

**Ale:** MatrixComm jest lepsze, bo:
- ✅ Oddzielny Core (0 vs 1) - prawdziwa wielowątkowość
- ✅ Non-blocking (kolejka)
- ✅ loop() może mieć niski priorytet (niższe zużycie CPU)

## Autor
Implementacja: 2025-10-20
Optymalizacja komunikacji UART @ 1Mbps

