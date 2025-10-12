cmake_minimum_required(VERSION 3.10)
project(led_image_viewer)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(ImageMagick COMPONENTS Magick++ REQUIRED)

# Set the path to rpi-rgb-led-matrix library
set(RGB_MATRIX_DIR "${CMAKE_SOURCE_DIR}/../rpi-rgb-led-matrix")

# Include directories
include_directories(
    ${ImageMagick_INCLUDE_DIRS}
    ${RGB_MATRIX_DIR}/include
)

# Add source files
add_executable(led-image-viewer
    main.cpp
    led-image-viewer.cc
)

# Link libraries
target_link_libraries(led-image-viewer
    ${RGB_MATRIX_DIR}/lib/librgbmatrix.a
    ${ImageMagick_LIBRARIES}
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
)