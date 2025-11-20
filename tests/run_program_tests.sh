#!/bin/bash

# --- Safety Preamble ---
SCRIPT_DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &> /dev/null && pwd)
cd "$SCRIPT_DIR/.."

# --- Main Script Logic ---
echo "Guaranteed to be running from repository root: $(pwd)"

BUILD_DIR="build"

# Clean and create the build directory
if [ -d "$BUILD_DIR" ]; then
    echo "Cleaning existing build directory: $BUILD_DIR"
    rm -rf "$BUILD_DIR"
fi
echo "Creating new build directory: $BUILD_DIR"
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Configure the project with CMake, pointing to the root CMakeLists.txt
echo "Configuring with CMake..."
cmake ..

# Build the test executables
echo "Building test executables..."
cmake --build . -j

# --- Test Execution ---
echo "Running test executables..."
for test_exe in $(find . -maxdepth 1 -type f -executable -not -name "*.*"); do
    echo "========================================="
    echo "Executing: $test_exe"
    echo "========================================="
    "./$test_exe"
    if [ $? -ne 0 ]; then
        echo "!!! Test Failed: $test_exe"
        exit 1
    fi
done

echo "All tests passed!"
