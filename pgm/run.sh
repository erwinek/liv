arduino-cli compile --fqbn esp32:esp32:esp32 --library /home/erwinek/liv/LEDMatrix /home/erwinek/liv/pgm -v
arduino-cli upload -p /dev/ttyUSB0 --fqbn esp32:esp32:esp32 /home/erwinek/liv/pgm -v
