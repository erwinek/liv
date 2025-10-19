#!/bin/bash
# Test script for DTR/RTS control on ESP32
# Usage: sudo ./test_dtr_rts.sh [/dev/ttyUSB0]

DEVICE=${1:-/dev/ttyUSB0}

if [ ! -e "$DEVICE" ]; then
    echo "Error: Device $DEVICE not found!"
    exit 1
fi

echo "========================================"
echo "ESP32 DTR/RTS Test Script"
echo "Device: $DEVICE"
echo "========================================"
echo ""
echo "This script will test the DTR/RTS control sequence."
echo "Monitor your ESP32 to see if it resets and boots correctly."
echo ""

# Check if we have python3 with pyserial
if ! python3 -c "import serial" 2>/dev/null; then
    echo "Installing pyserial..."
    sudo pip3 install pyserial
fi

# Create Python test script
cat > /tmp/test_esp32_dtr_rts.py << 'EOF'
import serial
import time
import sys

device = sys.argv[1] if len(sys.argv) > 1 else '/dev/ttyUSB0'

print(f"Opening {device}...")
ser = serial.Serial(device, 1000000, timeout=1)

print("\n=== Testing DTR/RTS control (DIRECT LOGIC) ===\n")

print("Step 1: RTS=HIGH (GPIO0 should be HIGH for normal mode)")
ser.rts = True
time.sleep(0.05)
print(f"  RTS state: {ser.rts} (True=HIGH)")

print("\nStep 2: DTR=LOW (EN=LOW, ESP32 in reset)")
ser.dtr = False
time.sleep(0.1)
print(f"  DTR state: {ser.dtr} (False=LOW)")

print("\nStep 3: DTR=HIGH (EN=HIGH, releasing reset)")
print("  >>> ESP32 should start booting now in NORMAL MODE <<<")
ser.dtr = True
time.sleep(0.1)
print(f"  DTR state: {ser.dtr} (True=HIGH)")

print("\nStep 4: Waiting 1 second for ESP32 to boot...")
time.sleep(1)

print("\n=== Final State ===")
print(f"DTR: {ser.dtr} (should be True/HIGH = EN=HIGH)")
print(f"RTS: {ser.rts} (should be True/HIGH = GPIO0=HIGH)")
print("\nThis board uses DIRECT LOGIC: RTS=HIGH -> GPIO0=HIGH")
print("\nIf ESP32 boots correctly, you should see boot messages below:")
print("=" * 50)

# Try to read boot messages
ser.timeout = 0.5
for i in range(10):
    data = ser.read(100)
    if data:
        try:
            print(data.decode('utf-8', errors='ignore'), end='')
        except:
            print(f"[Binary data: {len(data)} bytes]")
    else:
        break

print("\n" + "=" * 50)
print("\n⚠️  IMPORTANT: Keeping serial port open to maintain DTR/RTS state")
print("=" * 50)
print(f"\nFinal GPIO state: EN=HIGH, GPIO0=HIGH")
print(f"Serial port: {device} is still open")
print("\nOptions:")
print("  1. Press Ctrl+C to close port (DTR/RTS will reset to LOW)")
print("  2. Leave this running and start led-image-viewer in another terminal")
print("  3. Wait 10 seconds and port will close automatically")
print("\nWaiting... (Ctrl+C to exit now)")

try:
    time.sleep(10)
    print("\n10 seconds elapsed, closing port...")
except KeyboardInterrupt:
    print("\n\nUser interrupted, closing port...")

print(f"Final check before closing:")
print(f"  DTR: {ser.dtr} (EN)")
print(f"  RTS: {ser.rts} (GPIO0)")
print("\nClosing port now (DTR/RTS will return to default state)...")
ser.close()
print("Port closed.")
print("\n⚠️  After closing, DTR/RTS return to LOW (GPIO0 will be LOW)")
print("✅ To keep ESP32 in normal mode, run: sudo ./bin/led-image-viewer")
EOF

# Run the Python test
python3 /tmp/test_esp32_dtr_rts.py "$DEVICE"

echo ""
echo "========================================"
echo "Diagnostic Information:"
echo "========================================"
echo ""
echo "If ESP32 entered bootloader mode (waiting for download):"
echo "  → Problem: GPIO0 is LOW when it should be HIGH"
echo "  → Your board might have non-inverted logic!"
echo "  → Try changing RTS to HIGH in the code"
echo ""
echo "If ESP32 boots normally:"
echo "  → Success! The inverted logic is correct"
echo "  → RTS=LOW gives GPIO0=HIGH (normal mode)"
echo ""
echo "If nothing happens:"
echo "  → Check: Does your USB-Serial have DTR/RTS?"
echo "  → Check: Are DTR and RTS connected to EN and GPIO0?"
echo "  → Try: Increase delays in the sequence"
echo ""

# Cleanup
rm -f /tmp/test_esp32_dtr_rts.py

