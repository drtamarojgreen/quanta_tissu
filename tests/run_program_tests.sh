#!/bin/bash

# Exit immediately if a command exits with a non-zero status.
set -e

# --- Safety Preamble ---
# Get the absolute path of the directory where this script is located.
SCRIPT_DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &> /dev/null && pwd)

# Navigate to the repository root from the script's location.
# This script is expected to be in 'tests/', so the root is one level up.
# This makes the script safe to run from any directory.
cd "$SCRIPT_DIR/.."

# --- Main Script Logic ---
# Now that we are in the repo root, all paths are predictable.
echo "Guaranteed to be running from repository root: $(pwd)"

TEST_DIR="tests/model/program"
BUILD_DIR="$TEST_DIR/build"

# Clean the build directory if it exists to ensure a fresh build
if [ -d "$BUILD_DIR" ]; then
    echo "Cleaning existing build directory: $BUILD_DIR"
    rm -rf "$BUILD_DIR"
fi

# Create a new build directory and navigate into it
echo "Creating new build directory: $BUILD_DIR"
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Configure the project with CMake. '..' points to the parent dir (tests/model/program)
echo "Configuring with CMake..."
cmake ..

# Build the test executables
echo "Building test executables..."
cmake --build .

# Run all the test executables individually
echo "Running test executables..."

for test_exe in $(find . -maxdepth 1 -type f -executable -not -name "*.*"); do
    echo "========================================"
    echo "Executing: $test_exe"
    echo "========================================"
    "./$test_exe"
    echo "========================================"
done

echo "All tests completed successfully."
