#!/bin/bash
# Reset ESP32 properly and hold in normal mode
# Usage: sudo ./reset_esp32_and_hold.sh [/dev/ttyUSB0]

DEVICE=${1:-/dev/ttyUSB0}

if [ ! -e "$DEVICE" ]; then
    echo "Error: Device $DEVICE not found!"
    exit 1
fi

echo "========================================"
echo "ESP32 Reset and Hold Script"
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
    print("To keep ESP32 in normal mode, run: sudo ./bin/led-image-viewer")
    ser.close()
    sys.exit(0)

signal.signal(signal.SIGINT, signal_handler)

print(f"Opening {device}...")
ser = serial.Serial(device, 1000000, timeout=1)

print("\n" + "=" * 50)
print("Executing ESP32 Reset Sequence (DIRECT LOGIC)")
print("=" * 50)

# Step 1: Set GPIO0=HIGH before reset
print("\n[1/4] Setting RTS=HIGH (GPIO0=HIGH for normal mode)")
ser.rts = True
time.sleep(0.05)
print(f"      RTS: {ser.rts} → GPIO0=HIGH ✓")

# Step 2: Assert reset
print("\n[2/4] Asserting reset (DTR=LOW, EN=LOW)")
ser.dtr = False
time.sleep(0.1)
print(f"      DTR: {ser.dtr} → EN=LOW (reset active) ✓")

# Step 3: Release reset
print("\n[3/4] Releasing reset (DTR=HIGH, EN=HIGH)")
print("      >>> ESP32 will sample GPIO0 and boot into NORMAL MODE <<<")
ser.dtr = True
time.sleep(0.1)
print(f"      DTR: {ser.dtr} → EN=HIGH (running) ✓")

# Step 4: Wait for boot
print("\n[4/4] Waiting for ESP32 to boot (1 second)...")
time.sleep(1)
print("      Boot complete ✓")

print("\n" + "=" * 50)
print("✅ ESP32 Reset Complete - Normal Mode Active")
print("=" * 50)
print(f"\nCurrent state:")
print(f"  DTR: {ser.dtr} (EN=HIGH - ESP32 running)")
print(f"  RTS: {ser.rts} (GPIO0=HIGH - normal mode)")
print(f"\n✅ ESP32 is now running in NORMAL MODE")

# Read any boot messages
print("\n" + "=" * 50)
print("Boot messages (if any):")
print("=" * 50)
ser.timeout = 0.5
for i in range(5):
    if ser.in_waiting > 0:
        data = ser.read(ser.in_waiting)
        try:
            print(data.decode('utf-8', errors='ignore'), end='')
        except:
            pass
    time.sleep(0.1)

print("\n" + "=" * 50)
print("Holding port open to maintain state")
print("=" * 50)
print("\nOptions:")
print("  • Press Ctrl+C to exit and close port")
print("  • Leave running and start led-image-viewer in another terminal")
print("  • Auto-close after 30 seconds")
print("\nWaiting... (Ctrl+C to exit now)")
print("")

# Hold for 30 seconds or until interrupted
try:
    for remaining in range(30, 0, -1):
        time.sleep(1)
        if ser.in_waiting > 0:
            data = ser.read(ser.in_waiting)
            try:
                print(data.decode('utf-8', errors='ignore'), end='')
            except:
                pass
        # Print countdown every 5 seconds
        if remaining % 5 == 0:
            print(f"[Auto-close in {remaining} seconds... Ctrl+C to exit now]")
    
    print("\n30 seconds elapsed, closing port...")
    
except KeyboardInterrupt:
    print("\n\nUser interrupted, closing port...")
except Exception as e:
    print(f"\nError: {e}")
finally:
    print(f"\nFinal state before closing:")
    print(f"  DTR: {ser.dtr} (EN)")
    print(f"  RTS: {ser.rts} (GPIO0)")
    print("\nClosing port...")
    ser.close()
    print("Port closed.")
    print("\n⚠️  DTR/RTS have returned to LOW (GPIO0 is now LOW)")
    print("✅ To restart ESP32 properly, run: sudo ./bin/led-image-viewer")
EOF

echo ""
echo "========================================"
echo "Script complete."
echo "========================================"

