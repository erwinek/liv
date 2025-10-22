#!/bin/bash
# Uruchomienie ekranu #2 (64x512, ID=2) - Pionowy
# Run screen #2 (64x512, ID=2) - Vertical

cd "$(dirname "$0")"
sudo ./bin/led-image-viewer --config screen_config_vertical.ini

