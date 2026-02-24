#!/bin/bash

# --- Robustness and Reliability Foundation ---
set -euo pipefail
IFS=$'\n\t'
export LC_ALL=C

# Script constants
VERSION="1.0.0"
STATE_FILE=".db_test_state"
LOCK_FILE=".db_test.lock"

# Default configuration
TIMEOUT_SECONDS=3600

# Color codes
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
MAGENTA='\033[0;35m'
CYAN='\033[0;36m'
NC='\033[0m'
BOLD='\033[1m'
CHECK="✓"
CROSS="✗"
SPINNER="⠋⠙⠹⠸⠼⠴⠦⠧⠇⠏"

# --- Logging Functions ---
log()     { echo -e "${CYAN}${BOLD}[INFO]${NC}   $(date '+%Y-%m-%d %H:%M:%S') - $1"; }
success() { echo -e "${GREEN}${BOLD}[SUCCESS]${NC} $(date '+%Y-%m-%d %H:%M:%S') - $1"; }
warn()    { echo -e "${YELLOW}${BOLD}[WARN]${NC}   $(date '+%Y-%m-%d %H:%M:%S') - $1" >&2; }
error()   { echo -e "${RED}${BOLD}[ERROR]${NC}  $(date '+%Y-%m-%d %H:%M:%S') - $1" >&2; }

# --- Process Tracking ---
TISSDB_PID=""
TISSDB_LITE_PID=""

cleanup() {
    log "Performing cleanup..."
    if [ -n "${TISSDB_PID:-}" ]; then
        log "Stopping TissDB Server (PID: $TISSDB_PID)..."
        kill "$TISSDB_PID" 2>/dev/null || true
    fi
    if [ -n "${TISSDB_LITE_PID:-}" ]; then
        log "Stopping TissDBLite Server (PID: $TISSDB_LITE_PID)..."
        kill "$TISSDB_LITE_PID" 2>/dev/null || true
    fi
    rm -f "$LOCK_FILE"
}
trap cleanup EXIT INT TERM

# --- Utilities ---
retry() {
    local n=1
    local max=3
    local delay=2
    while true; do
        "$@" && break || {
            if [[ $n -lt $max ]]; then
                ((n++))
                warn "Command failed. Attempt $n/$max in ${delay}s..."
                sleep $delay;
            else
                error "The command has failed after $n attempts."
                return 1
            fi
        }
    done
}

run_with_spinner() {
    local -r msg="$1"
    local -r cmd="$2"
    local -r log_file=$(mktemp)

    echo -e "${CYAN}${BOLD}$msg${NC}"
    (
        set -o pipefail
        bash -c "$cmd" &> "$log_file"
    ) &
    local pid=$!
    local i=0
    while kill -0 $pid 2>/dev/null; do
        i=$(( (i+1) % ${#SPINNER} ))
        printf "\r ${SPINNER:$i:1} Working..."
        sleep 0.1
    done
    wait $pid
    local exit_code=$?
    if [ $exit_code -eq 0 ]; then
        printf "\r ${GREEN}${CHECK}${NC} Done.      \n"
        rm -f "$log_file"
    else
        printf "\r ${RED}${CROSS}${NC} Failed.    \n"
        error "Command failed. View log: $log_file"
        cat "$log_file" >&2
        return $exit_code
    fi
}

# --- Workflow Stages ---
STAGES=("COMPILE_TISSDB" "START_TISSDB" "START_TISSDB_LITE" "COMPILE_BDD_RUNNER" "RUN_BDD_SUITE")

# --- Commands ---
COMPILE_TISSDB_CMD="g++ -std=c++17 -Wall -Wextra -g -march=native \
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
    -latomic -lpthread -o tissdb_exe"

COMPILE_BDD_RUNNER_CMD="g++ -std=c++17 -o db_bdd_runner \
    tests/model/program/db/bdd_framework.cpp \
    tests/model/program/db/db_test_actions.cpp \
    tests/model/program/db/steps/database_steps.cpp \
    tests/model/program/db/steps/query_steps.cpp \
    tests/model/program/db/steps/transaction_steps.cpp \
    tests/model/program/db/steps/lite_steps.cpp \
    tests/model/program/db/db_bdd_test_main.cpp \
    quanta_tissu/tisslm/program/db/tissdb_client.cpp \
    quanta_tissu/tisslm/program/db/tissdb_lite_client.cpp \
    quanta_tissu/tisslm/program/db/http_client.cpp \
    tissdb/json/json.cpp tissdb/common/document.cpp \
    -Itests/model/program/db -Iquanta_tissu/tisslm/program/db -I. -lpthread"

main() {
    log "=== TissLM DB Integration Test Master Runner (C++ BDD Runner) ==="

    # 0. Cleanup stale data
    log "[0/5] Cleaning up stale data..."
    rm -rf tissdb_data

    # 1. Compile TissDB
    run_with_spinner "[1/5] Compiling TissDB..." "$COMPILE_TISSDB_CMD"

    # 2. Start TissDB
    log "[2/5] Starting TissDB Server..."
    ./tissdb_exe --port 9876 &
    TISSDB_PID=$!
    log "Waiting for TissDB (PID: $TISSDB_PID) to be ready..."
    local elapsed=0
    while ! curl -s -H "Authorization: Bearer static_test_token" http://localhost:9876/_health > /dev/null; do
        sleep 1
        elapsed=$((elapsed+1))
        if [ $elapsed -gt 30 ]; then error "TissDB timeout"; exit 1; fi
    done
    success "TissDB is ready."

    # 3. Start TissDBLite
    log "[3/5] Starting TissDBLite Server..."
    node lite/tissdblite_server.js &
    TISSDB_LITE_PID=$!
    log "Waiting for TissDBLite (PID: $TISSDB_LITE_PID) to be ready..."
    sleep 2
    success "TissDBLite started."

    # 4. Compile BDD Runner
    run_with_spinner "[4/5] Compiling C++ BDD Runner..." "$COMPILE_BDD_RUNNER_CMD"

    # 5. Run BDD Suite
    log "[5/5] Running Comprehensive BDD Test Suite..."
    if ! ./db_bdd_runner tests/model/program/db/features/; then
        error "BDD Suite failed"
        exit 1
    fi

    success "All C++ BDD Integration tests passed!"
}

if [[ "${BASH_SOURCE[0]}" == "${0}" ]]; then
    main "$@"
fi
