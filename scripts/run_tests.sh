#!/bin/bash

set -e

# Create build directory if it doesn't exist
mkdir -p build
cd build

# Configure and build
cmake ..
cmake --build .

# Run tests
ctest --output-on-failure
