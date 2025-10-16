# LED Image Viewer - Directory Structure

## Project Organization

```
/home/erwinek/liv/
├── bin/                    - Compiled binaries
│   └── led-image-viewer    - Main executable
│
├── build/                  - CMake build files
│   ├── CMakeCache.txt
│   ├── CMakeFiles/
│   ├── cmake_install.cmake
│   └── Makefile
│
├── tests/                  - Test files
│   ├── test_serial.py      - Serial protocol test script
│   ├── test*.cpp           - C++ test files
│   └── test_*              - Test executables
│
├── logs/                   - Application logs
│   ├── led_debug.log
│   └── led_output.log
│
├── docs/                   - Documentation
│   ├── FILES_CREATED.txt
│   ├── PROJECT_SUMMARY.md
│   ├── QUICK_FIX.md
│   ├── START_HERE.txt
│   ├── TROUBLESHOOTING.md
│   ├── LEDMatrix_Standalone.ino
│   └── Green-animated-arrow-right.gif
│
├── anim/                   - GIF animations
│   └── *.gif
│
├── fonts/                  - BDF fonts
│   ├── ComicNeue-Regular-20.bdf
│   ├── ComicNeue-Bold-48.bdf
│   └── *.bdf
│
├── LEDMatrix/              - LED Matrix library
│   ├── LEDMatrix.h
│   ├── LEDMatrix.cpp
│   └── examples/
│
├── pgm/                    - PGM image files
│
└── [Source files]          - Main source code
    ├── main.cpp
    ├── DisplayManager.cpp/h
    ├── SerialProtocol.cpp/h
    ├── LedImgViewer.cpp/h
    ├── BdfFont.cpp/h
    ├── CMakeLists.txt
    ├── PROTOCOL.md
    └── README.md
```

## Building the Project

From the project root:

```bash
cmake .                      # Generate Makefile
make                         # Build (output to bin/)
```

Or use clean build:

```bash
make clean                   # Clean build files
cmake .                      # Regenerate
make                         # Build
```

## Running the Application

```bash
sudo bin/led-image-viewer [options]
```

## Testing

Serial protocol test:
```bash
python3 tests/test_serial.py
```

Note: The LED viewer uses `/dev/ttyUSB0`, test script uses `/dev/ttyUSB1` (cross-connected).

