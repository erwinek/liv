#!/bin/bash
# Pełny test diagnostyczny
# Usage: sudo ./full_debug_test.sh

DEVICE=${1:-/dev/ttyUSB0}

echo "========================================"
echo "PEŁNY TEST DIAGNOSTYCZNY"
echo "Device: $DEVICE"
echo "========================================"
echo ""
echo "Test 1: Program C z ioctl"
echo "----------------------------------------"

# Uruchom test ioctl
./test_ioctl_rts "$DEVICE" 2>&1 | tee test_results.txt

echo ""
echo ""
echo "========================================"
echo "WYNIKI DIAGNOSTYKI"
echo "========================================"
echo ""
echo "Proszę odpowiedz na następujące pytania:"
echo ""
echo "1. Podczas 'MEASUREMENT TIME' (RTS=HIGH):"
echo "   Ile voltów pokazywał multimetr na GPIO0? ______V"
echo ""
echo "2. Po zamknięciu portu (RTS wraca do LOW):"
echo "   Ile voltów pokazywał multimetr na GPIO0? ______V"
echo ""
echo "3. Czy podczas toggle test GPIO0 mrugało? (TAK/NIE) ______"
echo ""
echo "4. Co pokazał test ioctl dla RTS=LOW?"
grep -A3 "Setting RTS=LOW" test_results.txt | head -5
echo ""
echo "5. Co pokazał test ioctl dla RTS=HIGH?"
grep -A3 "Setting RTS=HIGH" test_results.txt | head -5
echo ""
echo "========================================"
echo "Analiza:"
echo "========================================"
echo ""
echo "Jeśli GPIO0:"
echo "  • Podczas RTS=HIGH było ~3.3V → DIRECT logic"
echo "  • Podczas RTS=HIGH było ~0V → INVERTED logic"
echo "  • Nie zmieniało się wcale → RTS nie jest połączone z GPIO0"
echo ""
echo "Jeśli toggle test nie działał → problem z połączeniem hardware"
echo ""

