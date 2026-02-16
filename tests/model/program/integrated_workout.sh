#!/bin/bash

# --- Robustness and Reliability Foundation ---
set -euo pipefail
IFS=$'\n\t'
export LC_ALL=C

# Script constants
VERSION="1.0.5"
MIN_DISK_SPACE_MB=500
STATE_FILE=".workout_state"
LOCK_FILE=".workout.lock"
EVENTS_LOG="events.log"

# Default configuration
DRY_RUN=false
FORCE=false
VERBOSE=false
RESUME=false
RESUME_FROM=""
PROGRESS_INTERVAL=5
QUIET=false
VERBOSE_MERGES=false
VERBOSE_TOKENS=false
JSON_EVENTS=false
MENU_MODE=false
TIMEOUT_SECONDS=3600 # 1 hour default

# Color codes
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
MAGENTA='\033[0;35m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

# --- Logging Functions ---
log()     { [[ "${QUIET:-false}" == "true" ]] || echo -e "${BLUE}[INFO]${NC} $(date '+%Y-%m-%d %H:%M:%S') - $1"; }
success() { echo -e "${GREEN}[SUCCESS]${NC} $(date '+%Y-%m-%d %H:%M:%S') - $1"; }
warn()    { echo -e "${YELLOW}[WARN]${NC} $(date '+%Y-%m-%d %H:%M:%S') - $1" >&2; }
error()   { echo -e "${RED}[ERROR]${NC} $(date '+%Y-%m-%d %H:%M:%S') - $1" >&2; }

# --- Error Handling ---
on_error() {
    local line_no=$1
    local command=$2
    error "Command '$command' failed at line $line_no"
    if [[ -n "${CURRENT_STAGE:-}" ]]; then
        set_checkpoint "$CURRENT_STAGE" "FAILED"
    fi
    stop_heartbeat
}
trap 'on_error $LINENO "$BASH_COMMAND"' ERR

TISSDB_PID=""
CURRENT_STAGE=""
START_TIME=$(date +%s)
HEARTBEAT_PID=""

# Setup cleanup trap
cleanup() {
    stop_heartbeat
    log "Performing cleanup..."
    if [ ! -z "${TISSDB_PID:-}" ]; then
        log "Stopping TissDB Server (PID: $TISSDB_PID)..."
        kill "$TISSDB_PID" 2>/dev/null || true
        TISSDB_PID=""
    fi
    if [[ "${FORCE:-false}" == "true" ]]; then
        log "Force cleanup: removing binaries and state."
        rm -f tissdb_exe train_model_exe
        rm -f tokenizer_vocab.json tokenizer_merges.txt
        rm -rf tokenizer
        rm -f "$STATE_FILE" "$EVENTS_LOG"
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

# --- Heartbeat & Progress ---
start_heartbeat() {
    local stage=$1
    (
        while true; do
            sleep "$PROGRESS_INTERVAL"
            local elapsed=$(($(date +%s) - START_TIME))
            log "Heartbeat [$stage]: Running for ${elapsed}s..."
        done
    ) &
    HEARTBEAT_PID=$!
}

stop_heartbeat() {
    if [[ -n "${HEARTBEAT_PID:-}" ]]; then
        kill "$HEARTBEAT_PID" 2>/dev/null || true
        HEARTBEAT_PID=""
    fi
}

# --- State Management ---
set_checkpoint() {
    local stage=$1
    local status=$2
    local extra=${3:-"{}"}
    local timestamp
    timestamp=$(date +%s)
    if [[ ! -f "$STATE_FILE" ]]; then
        echo "{\"stages\": {}}" > "$STATE_FILE"
    fi
    local tmp_state
    tmp_state=$(jq --arg stage "$stage" --arg status "$status" --arg ts "$timestamp" --argjson extra "$extra" \
        '.stages[$stage] = {status: $status, timestamp: $ts, metadata: $extra}' "$STATE_FILE")
    echo "$tmp_state" > "$STATE_FILE"
}

get_stage_status() {
    local stage=$1
    if [[ ! -f "$STATE_FILE" ]]; then
        echo "PENDING"
        return
    fi
    jq -r ".stages[\"$stage\"].status // \"PENDING\"" "$STATE_FILE"
}

# --- Argument Parsing ---
parse_args() {
    while [[ $# -gt 0 ]]; do
        case $1 in
            --dry-run) DRY_RUN=true; shift ;;
            --force) FORCE=true; shift ;;
            --verbose) VERBOSE=true; shift ;;
            --resume) RESUME=true; shift ;;
            --resume-from) RESUME_FROM="$2"; shift 2 ;;
            --progress-interval)
                if [[ "$2" =~ ^[0-9]+$ ]]; then
                    PROGRESS_INTERVAL="$2"; shift 2
                else
                    error "Invalid interval: $2"; exit 1
                fi
                ;;
            --quiet) QUIET=true; shift ;;
            --verbose-merges) VERBOSE_MERGES=true; shift ;;
            --verbose-tokens) VERBOSE_TOKENS=true; shift ;;
            --json-events) JSON_EVENTS=true; shift ;;
            --menu) MENU_MODE=true; shift ;;
            --version) echo "Version $VERSION"; exit 0 ;;
            *) error "Unknown argument: $1"; exit 1 ;;
        esac
    done
}

# --- Visibility Filters ---
process_training_output() {
    local line
    while IFS= read -r line; do
        echo "$(date '+%Y-%m-%d %H:%M:%S.%3N') $line" >> "$EVENTS_LOG"
        if [[ "$line" =~ Merge\ ([0-9]+)/([0-9]+):\ \(([0-9]+),\ ([0-9]+)\)\ -\>\ ([0-9]+) ]]; then
            local current=${BASH_REMATCH[1]}
            local total=${BASH_REMATCH[2]}
            local p1=${BASH_REMATCH[3]}
            local p2=${BASH_REMATCH[4]}
            local nt=${BASH_REMATCH[5]}
            if [[ "${JSON_EVENTS:-false}" == "true" ]]; then
                echo "{\"event\": \"merge\", \"step\": $current, \"total\": $total, \"pair\": [$p1, $p2], \"new_token\": $nt}"
            fi
            if [[ "${VERBOSE_MERGES:-false}" == "true" ]]; then
                echo -e "${MAGENTA}[MERGE]${NC} Step $current/$total: ($p1, $p2) → Token $nt"
            elif [[ $((current % 100)) -eq 0 ]]; then
                 echo -en "\r${CYAN}[PROGRESS]${NC} Merging: $current/$total tokens created..."
            fi
        elif [[ "${VERBOSE_TOKENS:-false}" == "true" ]]; then
             echo -e "${CYAN}[TOKEN]${NC} $line"
        else
            echo "$line"
        fi
    done
}

# --- Preflight Checks ---
check_dependencies() {
    log "Checking dependencies..."
    local deps=("g++" "curl" "awk" "sed" "jq" "grep" "df" "git" "free" "timeout")
    for dep in "${deps[@]}"; do
        if ! command -v "$dep" &> /dev/null; then
            error "Required command '$dep' not found."
            exit 1
        fi
    done
}

check_root() {
    if [[ $EUID -eq 0 ]]; then
       error "This script should not be run as root."
       exit 1
    fi
}

check_disk_space() {
    local available_space
    available_space=$(df -m . | awk 'NR==2 {print $4}')
    if [[ $available_space -lt $MIN_DISK_SPACE_MB ]]; then
        error "Low disk space: ${available_space}MB available."
        exit 1
    fi
}

check_directories() {
    local dirs=("tissdb" "quanta_tissu" "tests/model/program")
    for dir in "${dirs[@]}"; do
        if [[ ! -d "$dir" ]]; then
            error "Required directory '$dir' not found."
            exit 1
        fi
    done
}

acquire_lock() {
    if [[ -f "$LOCK_FILE" ]]; then
        local pid
        pid=$(cat "$LOCK_FILE")
        if kill -0 "$pid" 2>/dev/null; then
            error "Another instance is already running (PID: $pid)."
            exit 1
        else
            rm -f "$LOCK_FILE"
        fi
    fi
    echo $$ > "$LOCK_FILE"
}

# --- Workflow Stages ---
STAGES=("COMPILE_TISSDB" "START_TISSDB" "COMPILE_TRAINING" "RUN_TRAINING")
NUM_STAGES=${#STAGES[@]}

is_skip_stage() {
    local stage=$1
    if [[ -n "${RESUME_FROM:-}" ]]; then
        if [[ "$stage" == "$RESUME_FROM" ]]; then
            RESUME_FROM=""
            return 1
        else
            return 0
        fi
    fi
    if [[ "${RESUME:-false}" == "true" ]]; then
        local status
        status=$(get_stage_status "$stage")
        if [[ "$status" == "COMPLETED" ]]; then
            return 0
        fi
    fi
    return 1
}

run_stage() {
    local idx=$1
    local stage=${STAGES[$idx]}
    local func=$2
    local stage_num=$((idx + 1))
    CURRENT_STAGE="$stage"
    if is_skip_stage "$stage"; then
        log "[$stage_num/$NUM_STAGES] Skipping $stage."
        return
    fi
    log "[$stage_num/$NUM_STAGES] Starting $stage..."
    local start_ts=$(date +%s)
    start_heartbeat "$stage"

    # Run the function in the CURRENT shell to preserve variables and background PIDs
    $func

    stop_heartbeat
    local end_ts=$(date +%s)
    local duration=$((end_ts - start_ts))
    local mem_usage=$(free -m | awk 'NR==2{print $3}')
    set_checkpoint "$stage" "COMPLETED" "{\"duration\": $duration, \"mem_mb\": $mem_usage}"
    success "[$stage_num/$NUM_STAGES] $stage completed in ${duration}s."
}

do_compile_tissdb() {
    if [[ "${DRY_RUN:-false}" == "false" ]]; then
        retry timeout "$TIMEOUT_SECONDS" g++ -std=c++17 -Wall -Wextra -g -march=native \
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
    else
        log "[DRY-RUN] Would compile TissDB"
    fi
}

do_start_tissdb() {
    if [[ "${DRY_RUN:-false}" == "false" ]]; then
        ./tissdb_exe &
        TISSDB_PID=$!
        log "TissDB started with PID: $TISSDB_PID"
        sleep 2
    else
        log "[DRY-RUN] Would start TissDB"
    fi
}

do_compile_training() {
    if [[ "${DRY_RUN:-false}" == "false" ]]; then
        retry timeout "$TIMEOUT_SECONDS" g++ -std=c++17 -o train_model_exe \
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
    else
        log "[DRY-RUN] Would compile training"
    fi
}

do_run_training() {
    if [[ "${DRY_RUN:-false}" == "false" ]]; then
        timeout "$TIMEOUT_SECONDS" ./train_model_exe | process_training_output
    else
        log "[DRY-RUN] Would run training"
    fi
}

# --- Report Generator ---
generate_report() {
    log "Generating final report..."
    local end_time=$(date +%s)
    local total_duration=$((end_time - START_TIME))
    echo "===================================================="
    echo "   QuantaTissu Workout Final Report"
    echo "===================================================="
    echo "Total Duration: ${total_duration}s"
    echo "Ecosystem Version: $VERSION"
    if [[ -d .git ]]; then
        echo "Git Hash: $(git rev-parse HEAD)"
    fi
    echo "----------------------------------------------------"
    printf "%-20s | %-10s | %-10s\n" "Stage" "Status" "Duration"
    echo "----------------------------------------------------"
    for stage in "${STAGES[@]}"; do
        local status=$(jq -r ".stages[\"$stage\"].status // \"UNKNOWN\"" "$STATE_FILE")
        local dur=$(jq -r ".stages[\"$stage\"].metadata.duration // \"-\"" "$STATE_FILE")
        printf "%-20s | %-10s | %-10ss\n" "$stage" "$status" "$dur"
    done
    echo "===================================================="
}

# --- Menu ---
show_menu() {
    PS3='Please enter your choice: '
    options=("Run Full Workout" "Resume Workout" "Cleanup" "View Status" "Preflight Checks" "Quit")
    select opt in "${options[@]}"
    do
        case $opt in
            "Run Full Workout")
                FORCE=true main_workflow
                break
                ;;
            "Resume Workout")
                RESUME=true main_workflow
                break
                ;;
            "Cleanup")
                FORCE=true cleanup
                success "Cleanup complete."
                ;;
            "View Status")
                if [[ -f "$STATE_FILE" ]]; then
                    jq . "$STATE_FILE"
                else
                    log "No state file found."
                fi
                ;;
            "Preflight Checks")
                check_dependencies
                check_directories
                check_disk_space
                success "Preflight checks passed."
                ;;
            "Quit")
                break
                ;;
            *) echo "invalid option $REPLY";;
        esac
    done
}

main_workflow() {
    run_stage 0 do_compile_tissdb
    run_stage 1 do_start_tissdb
    run_stage 2 do_compile_training
    run_stage 3 do_run_training
    generate_report
}

# --- Main Execution ---
main() {
    parse_args "$@"
    check_root
    acquire_lock
    if [[ "${MENU_MODE:-false}" == "true" ]]; then
        show_menu
    else
        check_dependencies
        check_directories
        check_disk_space
        if [[ "${FORCE:-false}" == "true" ]]; then
            log "Force mode: clearing state."
            rm -f "$STATE_FILE" "$EVENTS_LOG"
        fi
        log "=== QuantaTissu Frontier Integrated Workout Workflow v$VERSION ==="
        main_workflow
        success "Workflow complete."
    fi
}

if [[ "${BASH_SOURCE[0]}" == "${0}" ]]; then
    main "$@"
fi
