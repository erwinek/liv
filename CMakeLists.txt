cmake_minimum_required(VERSION 3.10)
project(led_image_viewer)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

# Find required packages - using pkg-config for GraphicsMagick
include(FindPkgConfig)
pkg_check_modules(GRAPHICSMAGICK REQUIRED GraphicsMagick++>=1.3.35)

# Set the path to rpi-rgb-led-matrix library
set(RGB_MATRIX_DIR "${CMAKE_SOURCE_DIR}/../rpi-rgb-led-matrix")

# Include directories
include_directories(
    ${GRAPHICSMAGICK_INCLUDE_DIRS}
    ${RGB_MATRIX_DIR}/include
    ${CMAKE_SOURCE_DIR}
)

# Add source files
add_executable(led-image-viewer
    main.cpp
    LedImgViewer.cpp
    SerialProtocol.cpp
    DisplayManager.cpp
    BdfFont.cpp
)

# Link libraries
target_link_libraries(led-image-viewer
    ${RGB_MATRIX_DIR}/lib/librgbmatrix.a
    ${GRAPHICSMAGICK_LIBRARIES}
    pthread
    rt
    m
)

# Add compiler definitions
target_compile_definitions(led-image-viewer PRIVATE
    _FILE_OFFSET_BITS=64
)

# Set compiler flags
target_compile_options(led-image-viewer PRIVATE
    -O3
    -Wall
    -Wextra
    -Wno-unused-parameter
    -Wno-deprecated-declarations
)