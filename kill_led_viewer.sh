#!/bin/bash

# Kill all led-image-viewer processes (including sudo wrappers)
echo "Killing all led-image-viewer processes..."

# Function to kill processes with retry
kill_processes() {
    local signal=$1
    local signal_name=$2
    
    # Find all processes (including sudo wrappers)
    PIDS=$(pgrep -f "led-image-viewer|bin/led-image-viewer" | sort -u)
    
    if [ -z "$PIDS" ]; then
        return 1  # No processes found
    fi
    
    echo "Sending $signal_name to processes: $PIDS"
    for pid in $PIDS; do
        # Use sudo to kill in case process runs as root
        if sudo kill $signal $pid 2>/dev/null; then
            echo "  Sent $signal_name to PID $pid"
        fi
    done
    
    return 0
}

# Try graceful shutdown first (SIGTERM)
if kill_processes "-TERM" "SIGTERM"; then
    echo "Waiting for graceful shutdown..."
    sleep 2
    
    # Check if processes are still running
    if pgrep -f "led-image-viewer|bin/led-image-viewer" > /dev/null; then
        echo "Some processes still running, sending SIGKILL..."
        kill_processes "-KILL" "SIGKILL"
        sleep 1
        
        # Final check
        FINAL=$(pgrep -f "led-image-viewer|bin/led-image-viewer")
        if [ ! -z "$FINAL" ]; then
            echo "Warning: Some processes may still be running:"
            ps aux | grep -E "led-image-viewer|bin/led-image-viewer" | grep -v grep
        else
            echo "All processes killed successfully"
        fi
    else
        echo "All processes terminated gracefully"
    fi
else
    echo "No led-image-viewer processes found"
fi
