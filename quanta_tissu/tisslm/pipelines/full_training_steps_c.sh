#!/bin/bash

# Get the directory of this script
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

# Navigate to the tisslm directory (parent of pipelines)
cd "$DIR/.."

echo "Current directory: $(pwd)"
if [ -d "program" ]; then
    echo "Found program directory."
else
    echo "Error: program directory not found in $(pwd)"
    ls -F
    exit 1
fi

# Compile the C++ training pipeline
echo "Compiling C++ training pipeline..."

# Find all cpp files
SOURCES=$(find program/core program/tokenizer program/training -name "*.cpp")

g++ -std=c++17 -O3 \
    -I./program \
    $SOURCES \
    -o tisslm_train

if [ $? -eq 0 ]; then
    echo "Compilation successful."
    echo "Running training pipeline..."
    # Pass the absolute path to the corpus directory
    # Using relative path from tisslm directory (where the script cds to)
    # corpus is in project root, tisslm is in project_root/quanta_tissu/tisslm
    # So we need ../../corpus
    ./tisslm_train "../../corpus"
else
    echo "Compilation failed."
    exit 1
fi
