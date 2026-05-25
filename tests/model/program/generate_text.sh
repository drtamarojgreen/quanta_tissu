#!/usr/bin/env bash
set -euo pipefail
IFS=$'\n\t'
LC_ALL=C

SCRIPT_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")" &> /dev/null && pwd)
PROJECT_ROOT=$(cd "$SCRIPT_DIR/../../.." &> /dev/null && pwd)

if [ "$#" -lt 2 ]; then
    echo "Usage: $0 <checkpoint_path> \"<prompt>\" [generation_length]"
    exit 1
fi

CHECKPOINT_PATH="$1"
PROMPT="$2"
GEN_LEN="${3:-30}"

RETRY_COUNT=3
retry() {
    local attempt=1
    until "$@"; do
        if (( attempt >= RETRY_COUNT )); then
            echo "Command failed after $attempt attempts"
            return 1
        fi
        echo "Retry $attempt failed — retrying..."
        attempt=$((attempt+1))
        sleep 1
    done
}

echo "=== Compiling Generator Utility ==="
GPP_SOURCES="$PROJECT_ROOT/tests/model/program/generate_text.cpp \
    $PROJECT_ROOT/quanta_tissu/tisslm/program/core/matrix.cpp \
    $PROJECT_ROOT/quanta_tissu/tisslm/program/core/parameter.cpp \
    $PROJECT_ROOT/quanta_tissu/tisslm/program/core/layernorm.cpp \
    $PROJECT_ROOT/quanta_tissu/tisslm/program/core/dropout.cpp \
    $PROJECT_ROOT/quanta_tissu/tisslm/program/core/feedforward.cpp \
    $PROJECT_ROOT/quanta_tissu/tisslm/program/core/multiheadattention.cpp \
    $PROJECT_ROOT/quanta_tissu/tisslm/program/core/transformerblock.cpp \
    $PROJECT_ROOT/quanta_tissu/tisslm/program/core/positionalencoding.cpp \
    $PROJECT_ROOT/quanta_tissu/tisslm/program/core/embedding.cpp \
    $PROJECT_ROOT/quanta_tissu/tisslm/program/core/transformer_model.cpp \
    $PROJECT_ROOT/quanta_tissu/tisslm/program/generation/generator.cpp \
    $PROJECT_ROOT/quanta_tissu/tisslm/program/tokenizer/tokenizer.cpp \
    $PROJECT_ROOT/quanta_tissu/tisslm/program/tokenizer/pre_tokenizer.cpp \
    $PROJECT_ROOT/quanta_tissu/tisslm/program/retrieval/retrieval_strategy.cpp \
    $PROJECT_ROOT/tissdb/json/json.cpp"

GPP_INCLUDES="-I$PROJECT_ROOT/tests/model/program \
    -I$PROJECT_ROOT/quanta_tissu/tisslm/program \
    -I$PROJECT_ROOT/quanta_tissu/tisslm/program/core \
    -I$PROJECT_ROOT/quanta_tissu/tisslm/program/generation \
    -I$PROJECT_ROOT/quanta_tissu/tisslm/program/tokenizer \
    -I$PROJECT_ROOT/quanta_tissu/tisslm/program/training \
    -I$PROJECT_ROOT -lpthread"

COMPILE_COMMAND="g++ -std=c++17 -O3 -o generate_text_exe $GPP_SOURCES $GPP_INCLUDES"

echo "$COMPILE_COMMAND"
retry bash -c "$COMPILE_COMMAND"

echo "=== Running Text Generation ==="
$PROJECT_ROOT/generate_text_exe "$CHECKPOINT_PATH" "$PROMPT" "$GEN_LEN"

echo "=== DONE ==="
