#!/bin/bash
set -e

# Navigate to the directory containing this script to ensure relative paths work
cd "$(dirname "$0")"

# --- TissDB Build & Run ---
TISSDB_PID=""
cleanup() {
    if [ ! -z "$TISSDB_PID" ]; then
        echo "Stopping TissDB Server (PID: $TISSDB_PID)..."
        kill $TISSDB_PID || true
    fi
    rm -f train_tokenizer_temp train_tokenizer_temp.cpp
}
trap cleanup EXIT

if [ -d "../../../tissdb/server" ]; then
    echo "Building TissDB Server..."
    g++ -std=c++17 -o tissdb_server \
        ../../../tissdb/server/*.cpp \
        ../../../tissdb/storage/*.cpp \
        ../../../tissdb/query/*.cpp \
        ../../../tissdb/api/*.cpp \
        ../../../tissdb/common/*.cpp \
        ../../../tissdb/json/*.cpp \
        -I../../../tissdb \
        -lpthread

    echo "Starting TissDB Server..."
    ./tissdb_server &
    TISSDB_PID=$!
    echo "TissDB Server running with PID: $TISSDB_PID"
    sleep 2
fi

echo "Building tokenizer trainer..."

# Create a temporary C++ file to train the tokenizer
cat <<EOF > train_tokenizer_temp.cpp
#include "tokenizer.h"
#include <iostream>
#include <fstream>
#include <sstream>

int main() {
    // Create dummy files to satisfy constructor if it tries to load immediately
    std::ofstream("tokenizer_vocab.json").close();
    std::ofstream("tokenizer_merges.txt").close();

    try {
        TissLM::Tokenizer::Tokenizer tokenizer("tokenizer");
        std::ifstream f("../../../quanta_tissu/tisslm/corpus/corpus.txt");
        if (!f.is_open()) {
            std::cerr << "Failed to open corpus file" << std::endl;
            return 1;
        }
        std::stringstream buffer;
        buffer << f.rdbuf();
        tokenizer.train(buffer.str(), 256);
        tokenizer.save("tokenizer");
        std::cout << "Tokenizer trained successfully." << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error training tokenizer: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
EOF

# Compile the tokenizer trainer
g++ -std=c++17 -o train_tokenizer_temp \
    train_tokenizer_temp.cpp \
    ../../../quanta_tissu/tisslm/program/tokenizer/tokenizer.cpp \
    ../../../quanta_tissu/tisslm/program/tokenizer/pre_tokenizer.cpp \
    -I. \
    -I../../../quanta_tissu/tisslm/program \
    -I../../../quanta_tissu/tisslm/program/tokenizer

# Run the tokenizer trainer
./train_tokenizer_temp

echo "Compiling full_fledged_workout_cpp..."

g++ -std=c++17 -o full_fledged_workout_cpp \
    full_fledged_workout_cpp.cpp \
    ../../../quanta_tissu/tisslm/program/core/matrix.cpp \
    ../../../quanta_tissu/tisslm/program/core/parameter.cpp \
    ../../../quanta_tissu/tisslm/program/core/layernorm.cpp \
    ../../../quanta_tissu/tisslm/program/core/dropout.cpp \
    ../../../quanta_tissu/tisslm/program/core/feedforward.cpp \
    ../../../quanta_tissu/tisslm/program/core/multiheadattention.cpp \
    ../../../quanta_tissu/tisslm/program/core/transformerblock.cpp \
    ../../../quanta_tissu/tisslm/program/core/positionalencoding.cpp \
    ../../../quanta_tissu/tisslm/program/core/embedding.cpp \
    ../../../quanta_tissu/tisslm/program/core/transformer_model.cpp \
    ../../../quanta_tissu/tisslm/program/core/mock_embedder.cpp \
    ../../../quanta_tissu/tisslm/program/generation/generator.cpp \
    ../../../quanta_tissu/tisslm/program/tokenizer/tokenizer.cpp \
    ../../../quanta_tissu/tisslm/program/tokenizer/pre_tokenizer.cpp \
    ../../../quanta_tissu/tisslm/program/training/optimizer.cpp \
    ../../../quanta_tissu/tisslm/program/training/loss_function.cpp \
    ../../../quanta_tissu/tisslm/program/training/trainer.cpp \
    ../../../quanta_tissu/tisslm/program/training/dataset.cpp \
    ../../../quanta_tissu/tisslm/program/db/tissdb_client.cpp \
    ../../../quanta_tissu/tisslm/program/db/tissdb_lite_client.cpp \
    ../../../quanta_tissu/tisslm/program/db/http_client.cpp \
    ../../../quanta_tissu/tisslm/program/retrieval/retrieval_strategy.cpp \
    ../../../tissdb/json/json.cpp \
    ../../../tissdb/common/document.cpp \
    -I. \
    -I../../../quanta_tissu/tisslm/program \
    -I../../../quanta_tissu/tisslm/program/core \
    -I../../../quanta_tissu/tisslm/program/generation \
    -I../../../quanta_tissu/tisslm/program/tokenizer \
    -I../../../quanta_tissu/tisslm/program/training \
    -I../../../quanta_tissu/tisslm/program/db \
    -I../../../quanta_tissu/tisslm/program/retrieval \
    -I../../../tissdb/json \
    -I../../../tissdb/common \
    -DTOKENIZER_PATH_FROM_CMAKE=\"tokenizer\" \
    -lpthread

echo "Compilation successful. Executable created at $(pwd)/full_fledged_workout_cpp"
