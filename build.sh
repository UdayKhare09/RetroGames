#!/bin/bash

# Exit on error
set -e

# Create and navigate to the build directory
mkdir -p build
cd build

# Run CMake to configure the project
cmake ..

# Build the project
cmake --build .

# Navigate back to the root directory
cd ..
