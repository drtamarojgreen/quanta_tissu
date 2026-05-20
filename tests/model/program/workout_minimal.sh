#!/usr/bin/env bash
set -euo pipefail

# 1. Configuration
export TISSLM_N_EMBD=8
export TISSLM_N_LAYER=1
export TISSLM_N_HEAD=1
export TISSLM_D_FF=16
export TISSLM_NUM_EPOCHS=1
export TISSLM_BATCH_SIZE=1
export TISSLM_VOCAB_SIZE=256
export TISSLM_MAX_BATCHES=5

TISSDB_PORT=9999
DATA_DIR="tissdb_data/minimal_run"
mkdir -p "$DATA_DIR"

# 2. Start TissDB
echo "=== Start TissDB (Minimal) ==="
fuser -k $TISSDB_PORT/tcp 2>/dev/null || true
./tissdb_exe --port $TISSDB_PORT --data-dir "$DATA_DIR" &
TISSDB_PID=$!
sleep 1

# 3. Start Analyzer
echo "=== Start RMA Analyzer (Session 1) ==="
# Ensure analyzer is built
make -C tests/model/analyzer all
# The analyzer uses -s for session id
./tests/model/analyzer/analyzer -s 1 > analyzer_minimal.log 2>&1 &
ANALYZER_PID=$!
sleep 1

# 4. Run Training
echo "=== Run Training (Minimal) ==="
# Run with session_id 1 to connect to the analyzer
# Positional argument for train_model_exe
./train_model_exe 1

# 5. Cleanup
echo "=== Shutdown RMA Analyzer ==="
kill $ANALYZER_PID

echo "=== Shutdown TissDB ==="
kill $TISSDB_PID
wait $TISSDB_PID 2>/dev/null || true

echo "=== Analyzer Results (Top 10 lines) ==="
head -n 15 analyzer_minimal.log || echo "No analyzer log found."

echo "=== DONE ==="
