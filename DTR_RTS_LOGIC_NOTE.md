# ⚠️ WAŻNA NOTATKA - Logika DTR/RTS

## ✅ Twoja płytka używa INVERTED LOGIC (typowe dla ESP32)

Po testach z multimetrem **POTWIERDZONO**, że płytka ma **układ inwertujący** (standardowy dla większości płytek ESP32):

### Logika która działa u Ciebie (POTWIERDZONA TESTAMI):
```
RTS=LOW  -> GPIO0=HIGH -> Normal Mode ✅  ← Potwierdzono multimetrem!
RTS=HIGH -> GPIO0=LOW  -> Bootloader Mode
DTR=HIGH -> EN=HIGH    -> Running
DTR=LOW  -> EN=LOW     -> Reset
```

### Prawidłowa sekwencja dla Twojej płytki:
```cpp
1. RTS=LOW (50ms)     → GPIO0=HIGH (ustawione PRZED resetem!) ← INVERTED!
2. DTR=LOW (100ms)    → EN=LOW (trzymaj w reset)
3. DTR=HIGH           → EN=HIGH (puść reset, ESP32 widzi GPIO0=HIGH)
4. RTS=LOW            → GPIO0=HIGH (trzymaj przez boot) ← INVERTED!
```

**Stan końcowy: DTR=HIGH, RTS=LOW → EN=HIGH, GPIO0=HIGH → Normal Mode** ✅

---

## ✅ Potwierdzenie: Ta płytka jest TYPOWA!

**Twoja płytka używa tego samego układu co większość komercyjnych płytek ESP32!**

Komercyjne płytki (NodeMCU, DevKit, itp.) używają **INVERTED LOGIC** przez układ tranzystorów:

```
RTS=LOW  -> GPIO0=HIGH -> Normal Mode  ← TWOJA PŁYTKA TAK SAMO!
RTS=HIGH -> GPIO0=LOW  -> Bootloader Mode
```

**Dlaczego początkowo myśleliśmy że to DIRECT?**
- Nie mierzyliśmy GPIO0 multimetrem podczas działania programu
- Mierzyliśmy dopiero PO zamknięciu portu (gdy RTS wraca do LOW)
- Błędnie interpretowaliśmy wyniki

---

## Jak to wykryliśmy

### Test 1 (z RTS=LOW):
```
Po sekwencji: EN=1, GPIO0=0
Rezultat: ESP32 w bootloader mode ❌
```

### Test 2 (z RTS=HIGH):
```
Po sekwencji: EN=1, GPIO0=1
Rezultat: ESP32 w normal mode ✅
```

**Wniosek:** Twoja płytka nie ma układu inwertującego.

---

## Co to znaczy dla kodu

Kod został dostosowany do **DIRECT LOGIC**:
- `setRTS(true)` = `RTS=HIGH` = `GPIO0=HIGH` = Normal Mode
- `setRTS(false)` = `RTS=LOW` = `GPIO0=LOW` = Bootloader Mode

**Jeśli używasz typowej komercyjnej płytki**, musisz odwrócić logikę z powrotem:
```cpp
setRTS(false);  // RTS=LOW dla typowych płytek
```

---

## Schemat ideowy

### Twoja płytka (DIRECT):
```
USB-Serial DTR ──────────► EN (ESP32)
USB-Serial RTS ──────────► GPIO0 (ESP32)
```

### Typowe płytki (INVERTED):
```
USB-Serial DTR ─┬─[Q1]─► EN (ESP32)
                │
USB-Serial RTS ─┴─[Q2]─► GPIO0 (ESP32)

Q1, Q2 = NPN/PNP transistors (inverted logic)
```

---

## TL;DR

✅ **Twoja konfiguracja:** `RTS=HIGH` dla Normal Mode  
📋 **Typowe płytki:** `RTS=LOW` dla Normal Mode  
🔧 **Kod jest już poprawiony** dla Twojej płytki!

---

Data: 2025-10-19  
Status: ✅ Potwierdzone działanie z DIRECT LOGIC

