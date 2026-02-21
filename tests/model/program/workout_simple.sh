#!/usr/bin/env bash
set -euo pipefail
IFS=$'\n\t'
LC_ALL=C

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
# process_training_output:
# currently passthrough — preserves pipeline behavior
############################################
process_training_output() {
    cat
}

############################################
echo "=== Compile TissDB ==="
############################################
retry g++ -std=c++17 -Wall -Wextra -g -march=native \
    -Itissdb -Iquanta_tissu/tisslm/program -I. \
    tissdb/main.cpp tissdb/api/http_server.cpp tissdb/audit/audit_logger.cpp \
    tissdb/auth/rbac.cpp tissdb/auth/token_manager.cpp tissdb/common/binary_stream_buffer.cpp \
    tissdb/common/checksum.cpp tissdb/common/document.cpp tissdb/common/schema_validator.cpp \
    tissdb/common/serialization.cpp tissdb/crypto/kms.cpp tissdb/json/json.cpp \
    tissdb/query/executor.cpp tissdb/query/executor_common.cpp tissdb/query/executor_delete.cpp \
    tissdb/query/executor_insert.cpp tissdb/query/executor_select.cpp tissdb/query/executor_update.cpp \
    tissdb/query/join_algorithms.cpp tissdb/query/parser.cpp tissdb/storage/collection.cpp \
    tissdb/storage/database_manager.cpp tissdb/storage/indexer.cpp tissdb/storage/lsm_tree.cpp \
    tissdb/storage/memtable.cpp tissdb/storage/native_b_tree.cpp tissdb/storage/sstable.cpp \
    tissdb/storage/transaction_manager.cpp tissdb/storage/wal.cpp \
    quanta_tissu/tisslm/program/ddl_parser.cpp quanta_tissu/tisslm/program/schema_manager.cpp \
    quanta_tissu/tisslm/program/tissu_sinew.cpp tests/db/http_client.cpp \
    -latomic -lpthread -o tissdb_exe

############################################
echo "=== Start TissDB ==="
############################################
./tissdb_exe &
TISSDB_PID=$!
sleep 2

############################################
echo "=== Compile Training Model ==="
############################################
retry g++ -std=c++17 -o train_model_exe \
    tests/model/program/train_model.cpp \
    quanta_tissu/tisslm/program/core/matrix.cpp quanta_tissu/tisslm/program/core/parameter.cpp \
    quanta_tissu/tisslm/program/core/layernorm.cpp quanta_tissu/tisslm/program/core/dropout.cpp \
    quanta_tissu/tisslm/program/core/feedforward.cpp quanta_tissu/tisslm/program/core/multiheadattention.cpp \
    quanta_tissu/tisslm/program/core/transformerblock.cpp quanta_tissu/tisslm/program/core/positionalencoding.cpp \
    quanta_tissu/tisslm/program/core/embedding.cpp quanta_tissu/tisslm/program/core/transformer_model.cpp \
    quanta_tissu/tisslm/program/generation/generator.cpp quanta_tissu/tisslm/program/tokenizer/tokenizer.cpp \
    quanta_tissu/tisslm/program/tokenizer/pre_tokenizer.cpp quanta_tissu/tisslm/program/training/optimizer.cpp \
    quanta_tissu/tisslm/program/training/loss_function.cpp quanta_tissu/tisslm/program/training/trainer.cpp \
    quanta_tissu/tisslm/program/training/dataset.cpp quanta_tissu/tisslm/program/retrieval/retrieval_strategy.cpp \
    tissdb/json/json.cpp \
    -Itests/model/program -Iquanta_tissu/tisslm/program -Iquanta_tissu/tisslm/program/core \
    -Iquanta_tissu/tisslm/program/generation -Iquanta_tissu/tisslm/program/tokenizer \
    -Iquanta_tissu/tisslm/program/training -I. -lpthread

############################################
echo "=== Run Training ==="
############################################
./train_model_exe | process_training_output

############################################
echo "=== Shutdown TissDB ==="
############################################
kill $TISSDB_PID
wait $TISSDB_PID 2>/dev/null || true

echo "=== DONE ==="
