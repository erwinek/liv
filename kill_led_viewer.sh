#!/bin/bash

# Kill all led-image-viewer processes
echo "Killing all led-image-viewer processes..."

# Find all processes
PIDS=$(pgrep -f led-image-viewer)

if [ -z "$PIDS" ]; then
    echo "No led-image-viewer processes found"
    exit 0
fi

echo "Found processes: $PIDS"

# Try graceful shutdown first (SIGTERM)
echo "Sending SIGTERM..."
for pid in $PIDS; do
    if kill -TERM $pid 2>/dev/null; then
        echo "Sent SIGTERM to PID $pid"
    fi
done

# Wait a bit for graceful shutdown
sleep 2

# Check if processes are still running
REMAINING=$(pgrep -f led-image-viewer)
if [ ! -z "$REMAINING" ]; then
    echo "Some processes still running, sending SIGKILL..."
    for pid in $REMAINING; do
        if kill -KILL $pid 2>/dev/null; then
            echo "Sent SIGKILL to PID $pid"
        fi
    done
    
    # Wait a bit more
    sleep 1
    
    # Final check
    FINAL=$(pgrep -f led-image-viewer)
    if [ ! -z "$FINAL" ]; then
        echo "Warning: Some processes may still be running: $FINAL"
    else
        echo "All processes killed successfully"
    fi
else
    echo "All processes terminated gracefully"
fi
