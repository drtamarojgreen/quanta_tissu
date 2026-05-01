#!/usr/bin/env bash
set -euo pipefail
IFS=$'\n\t'
LC_ALL=C

SCRIPT_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")" &> /dev/null && pwd)
PROJECT_ROOT=$(cd "$SCRIPT_DIR/../../.." &> /dev/null && pwd)

############################################
# Internal configuration
############################################
RETRY_COUNT=3

############################################
# retry: runs a command up to RETRY_COUNT times
############################################
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



############################################
echo "=== Compile Resume Training Model ==="
############################################
GPP_RESUME_SOURCES="$PROJECT_ROOT/tests/model/program/resume_training.cpp \
    $PROJECT_ROOT/quanta_tissu/tisslm/program/core/matrix.cpp $PROJECT_ROOT/quanta_tissu/tisslm/program/core/parameter.cpp \
    $PROJECT_ROOT/quanta_tissu/tisslm/program/core/layernorm.cpp $PROJECT_ROOT/quanta_tissu/tisslm/program/core/dropout.cpp \
    $PROJECT_ROOT/quanta_tissu/tisslm/program/core/feedforward.cpp $PROJECT_ROOT/quanta_tissu/tisslm/program/core/multiheadattention.cpp \
    $PROJECT_ROOT/quanta_tissu/tisslm/program/core/transformerblock.cpp $PROJECT_ROOT/quanta_tissu/tisslm/program/core/positionalencoding.cpp \
    $PROJECT_ROOT/quanta_tissu/tisslm/program/core/embedding.cpp $PROJECT_ROOT/quanta_tissu/tisslm/program/core/transformer_model.cpp \
    $PROJECT_ROOT/quanta_tissu/tisslm/program/generation/generator.cpp $PROJECT_ROOT/quanta_tissu/tisslm/program/tokenizer/tokenizer.cpp \
    $PROJECT_ROOT/quanta_tissu/tisslm/program/tokenizer/pre_tokenizer.cpp $PROJECT_ROOT/quanta_tissu/tisslm/program/training/optimizer.cpp \
    $PROJECT_ROOT/quanta_tissu/tisslm/program/training/loss_function.cpp $PROJECT_ROOT/quanta_tissu/tisslm/program/training/trainer.cpp \
    $PROJECT_ROOT/quanta_tissu/tisslm/program/training/dataset.cpp $PROJECT_ROOT/quanta_tissu/tisslm/program/retrieval/retrieval_strategy.cpp \
    $PROJECT_ROOT/tissdb/json/json.cpp"

GPP_RESUME_INCLUDES="-I$PROJECT_ROOT/tests/model/program -I$PROJECT_ROOT/quanta_tissu/tisslm/program -I$PROJECT_ROOT/quanta_tissu/tisslm/program/core \
    -I$PROJECT_ROOT/quanta_tissu/tisslm/program/generation -I$PROJECT_ROOT/quanta_tissu/tisslm/program/tokenizer \
    -I$PROJECT_ROOT/quanta_tissu/tisslm/program/training -I$PROJECT_ROOT -lpthread"

COMPILE_RESUME_COMMAND="g++ -std=c++17 -o resume_train_model_exe $GPP_RESUME_SOURCES $GPP_RESUME_INCLUDES"

echo "$COMPILE_RESUME_COMMAND"
retry bash -c "$COMPILE_RESUME_COMMAND"

############################################
echo "=== Run Resume Training ==="
############################################
$PROJECT_ROOT/resume_train_model_exe



echo "=== DONE ==="
