#!/bin/bash
# Simple script to hold ESP32 in normal mode by keeping DTR=HIGH, RTS=HIGH
# Usage: sudo ./hold_esp32_normal_mode.sh [/dev/ttyUSB0]

DEVICE=${1:-/dev/ttyUSB0}

if [ ! -e "$DEVICE" ]; then
    echo "Error: Device $DEVICE not found!"
    exit 1
fi

echo "========================================"
echo "ESP32 Normal Mode Holder"
echo "Device: $DEVICE"
echo "========================================"
echo ""

# Check if we have python3 with pyserial
if ! python3 -c "import serial" 2>/dev/null; then
    echo "Installing pyserial..."
    sudo pip3 install pyserial
fi

# Create Python script
python3 << EOF
import serial
import time
import signal
import sys

device = "$DEVICE"

def signal_handler(sig, frame):
    print("\n\n⚠️  Interrupted! Closing port...")
    print("Note: DTR/RTS will return to LOW after closing")
    ser.close()
    sys.exit(0)

signal.signal(signal.SIGINT, signal_handler)

print(f"Opening {device}...")
ser = serial.Serial(device, 1000000, timeout=1)

print("\nSetting DTR=HIGH, RTS=HIGH...")
ser.dtr = True   # EN=HIGH (ESP32 running)
ser.rts = True   # GPIO0=HIGH (normal mode)
time.sleep(0.1)

print(f"\n✅ ESP32 should be in NORMAL MODE")
print(f"   DTR={ser.dtr} (EN=HIGH)")
print(f"   RTS={ser.rts} (GPIO0=HIGH)")
print("\n" + "=" * 50)
print("Port is being held open to maintain DTR/RTS state")
print("=" * 50)
print("\nInstructions:")
print("  • ESP32 will stay in normal mode as long as this runs")
print("  • Press Ctrl+C to exit (DTR/RTS will reset)")
print("  • To run led-image-viewer, exit this first")
print("\nHolding port open... (press Ctrl+C to exit)")
print("")

# Hold forever
try:
    while True:
        time.sleep(1)
        # Optionally check and display any data
        if ser.in_waiting > 0:
            data = ser.read(ser.in_waiting)
            try:
                print(data.decode('utf-8', errors='ignore'), end='')
            except:
                pass
except Exception as e:
    print(f"\nError: {e}")
finally:
    print("\nClosing port...")
    ser.close()
    print("Done.")
EOF

echo ""
echo "========================================"
echo "Port closed. DTR/RTS returned to LOW."
echo "To restart ESP32 properly, run:"
echo "  sudo ./bin/led-image-viewer"
echo "========================================"

