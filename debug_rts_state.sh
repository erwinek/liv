#!/bin/bash
# Debug script to check actual RTS state
# Usage: sudo ./debug_rts_state.sh [/dev/ttyUSB0]

DEVICE=${1:-/dev/ttyUSB0}

if [ ! -e "$DEVICE" ]; then
    echo "Error: Device $DEVICE not found!"
    exit 1
fi

echo "========================================"
echo "RTS State Debug Tool"
echo "Device: $DEVICE"
echo "========================================"
echo ""

# Check if we have python3 with pyserial
if ! python3 -c "import serial" 2>/dev/null; then
    echo "Installing pyserial..."
    sudo pip3 install pyserial
fi

# Create Python script
python3 << 'EOF'
import serial
import time
import sys

device = sys.argv[1] if len(sys.argv) > 1 else '/dev/ttyUSB0'

print(f"Opening {device}...")
try:
    ser = serial.Serial(device, 1000000, timeout=1)
except Exception as e:
    print(f"ERROR: Cannot open {device}: {e}")
    sys.exit(1)

print("\n" + "=" * 60)
print("Testing RTS Control")
print("=" * 60)

# Test 1: Set RTS to LOW
print("\n[Test 1] Setting RTS=False (LOW)")
ser.rts = False
time.sleep(0.1)
print(f"  Python reports: ser.rts = {ser.rts}")
print("  Expected GPIO0: LOW")
print("  → Measure GPIO0 now! Should be ~0V")
time.sleep(2)

# Test 2: Set RTS to HIGH
print("\n[Test 2] Setting RTS=True (HIGH)")
ser.rts = True
time.sleep(0.1)
print(f"  Python reports: ser.rts = {ser.rts}")
print("  Expected GPIO0: HIGH")
print("  → Measure GPIO0 now! Should be ~3.3V")
time.sleep(2)

# Test 3: Toggle multiple times
print("\n[Test 3] Toggling RTS 5 times (watch GPIO0 LED if present)")
for i in range(5):
    ser.rts = False
    print(f"  {i+1}. RTS=LOW (GPIO0 should be LOW)")
    time.sleep(0.5)
    ser.rts = True
    print(f"  {i+1}. RTS=HIGH (GPIO0 should be HIGH)")
    time.sleep(0.5)

# Test 4: Check DTR too
print("\n[Test 4] Checking DTR control")
print("Setting DTR=False (EN=LOW, ESP32 in reset)")
ser.dtr = False
time.sleep(0.1)
print(f"  Python reports: ser.dtr = {ser.dtr}")
time.sleep(1)

print("Setting DTR=True (EN=HIGH, ESP32 running)")
ser.dtr = True
time.sleep(0.1)
print(f"  Python reports: ser.dtr = {ser.dtr}")

# Final state
print("\n" + "=" * 60)
print("Final State Check")
print("=" * 60)
print(f"DTR: {ser.dtr} (should be True = EN=HIGH)")
print(f"RTS: {ser.rts} (should be True = GPIO0=HIGH)")
print("\n⚠️  Measure GPIO0 with multimeter NOW!")
print("Expected voltage: ~3.3V (if DIRECT logic)")
print("\nWaiting 10 seconds for measurement...")

for i in range(10, 0, -1):
    print(f"  {i} seconds remaining... (GPIO0 should be HIGH)", end='\r')
    time.sleep(1)

print("\n\nClosing port (DTR/RTS will reset to LOW)...")
ser.close()
print("Port closed.")

print("\n" + "=" * 60)
print("Diagnostic Results:")
print("=" * 60)
print("\nIf GPIO0 was:")
print("  • ~3.3V during test → ✅ RTS works, DIRECT logic confirmed")
print("  • ~0V during test → ❌ RTS not controlling GPIO0")
print("  • Toggling with RTS → ✅ Connection works")
print("\nIf GPIO0 did NOT change:")
print("  1. Check physical connection: USB-Serial RTS pin → ESP32 GPIO0")
print("  2. Check if your USB-Serial converter has RTS output")
print("  3. Check if there's a pull-down resistor on GPIO0")
print("  4. Try inverted logic (RTS=LOW might give GPIO0=HIGH)")

EOF

echo ""
echo "========================================"
echo "Next steps:"
echo "========================================"
echo "1. If GPIO0 changed during test → RTS works!"
echo "2. If GPIO0 stayed LOW → Check hardware connection"
echo "3. Check USB-Serial converter datasheet for RTS pin"
echo "========================================"

