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

# Color codes & special characters
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
MAGENTA='\033[0;35m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color
BOLD='\033[1m'
DIM='\033[2m'
CHECK="✓"
CROSS="✗"
ARROW="→"
SPINNER="⠋⠙⠹⠸⠼⠴⠦⠧⠇⠏"

# --- Logging Functions ---
log()     { [[ "${QUIET:-false}" == "true" ]] || echo -e "${CYAN}${BOLD}[INFO]${NC}   $(date '+%Y-%m-%d %H:%M:%S') - $1"; }
success() { echo -e "${GREEN}${BOLD}[SUCCESS]${NC} $(date '+%Y-%m-%d %H:%M:%S') - $1"; }
warn()    { echo -e "${YELLOW}${BOLD}[WARN]${NC}   $(date '+%Y-%m-%d %H:%M:%S') - $1" >&2; }
error()   { echo -e "${RED}${BOLD}[ERROR]${NC}  $(date '+%Y-%m-%d %H:%M:%S') - $1" >&2; }

# --- UI Functions ---
start_heartbeat() {
    if [[ ! -t 1 ]] || [[ ! -w /dev/tty ]]; then
        return
    fi

    (
        while true; do
            for (( i=0; i<${#SPINNER}; i++ )); do
                printf "\r${CYAN}${BOLD}[RUNNING]${NC} ${SPINNER:$i:1} " > /dev/tty
                sleep 0.1
            done
        done
    ) &
    HEARTBEAT_PID=$!
}

stop_heartbeat() {
    if [[ -n "${HEARTBEAT_PID:-}" ]]; then
        kill "$HEARTBEAT_PID" 2>/dev/null || true
        printf "\r\033[K" > /dev/tty
        HEARTBEAT_PID=""
    fi
}

display_header() {
    echo -e "${MAGENTA}${BOLD}"
    echo "================================================================"
    echo "    QuantaTissu Frontier - Integrated Workout Workflow v$VERSION"
    echo "================================================================"
    echo -e "${NC}"
}

draw_progress_bar() {
    local -r _w=50 _p=$1 _t=$2
    # Ensure we don't divide by zero
    if [[ $_t -eq 0 ]]; then return; fi
    local _c=$((_p*100/_t))
    local -r _s=8 _l=$((_w-_s-5)) _n=$((_c*_l/100)) _m=$((_l-_n))
    local _bar
    _bar=$(printf "%${_n}s" | tr ' ' '█')
    local _space
    _space=$(printf "%${_m}s")
    printf "\r [%s%s] %3d%%" "$_bar" "$_space" "$_c" > /dev/tty
}

display_checklist() {
    clear
    display_header
    echo -e "${BOLD}Workflow Stages:${NC}"
    for i in "${!STAGES[@]}"; do
        local stage="${STAGES[$i]}"
        local status_char=" "
        local color=$DIM
        local status_text="PENDING"
        local status=$(get_stage_status "$stage")

        if [[ "$status" == "COMPLETED" ]]; then
            status_char="$CHECK"
            color=$GREEN
            status_text="COMPLETED"
        elif [[ "$status" == "FAILED" ]]; then
            status_char="$CROSS"
            color=$RED
            status_text="FAILED"
        elif [[ "${CURRENT_STAGE:-}" == "$stage" ]]; then
            status_char="$ARROW"
            color=$CYAN
            status_text="IN PROGRESS"
        fi

        echo -e "${color}${BOLD}[$status_char] Stage $((i+1))/${NUM_STAGES}: ${stage} - ${status_text}${NC}"
    done
    echo "----------------------------------------------------------------"
}

run_with_spinner() {
    local -r msg="$1"
    local -r cmd="$2"
    local -r log_file="$3"
    
    echo -e "${CYAN}${BOLD}$msg${NC}"
    
    (
        set -o pipefail
        bash -lc "$cmd" &> "$log_file"
    ) &
    local pid=$!

    local i=0
    local start_time=$(date +%s)
    while kill -0 $pid 2>/dev/null; do
        i=$(( (i+1) % ${#SPINNER} ))
        local elapsed=$(($(date +%s) - start_time))
        printf "\r ${SPINNER:$i:1} Running for ${elapsed}s..."
        sleep 0.1
    done
    
    wait $pid
    local exit_code=$?

    if [ $exit_code -eq 0 ]; then
        printf "\r ${GREEN}${CHECK}${NC} Done in $(($(date +%s) - start_time))s.      \n"
    else
        printf "\r ${RED}${CROSS}${NC} Failed after $(($(date +%s) - start_time))s.    \n"
        error "Command failed. View log for details: $log_file"
        cat "$log_file" >&2
        exit $exit_code
    fi
}


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
create_log_file() {
    mktemp "${TMPDIR:-/tmp}/integrated_workout.XXXXXX.log"
}

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
    if [[ -t 1 ]] && command -v tput > /dev/null 2>&1; then
        tput civis # Hide cursor
    fi
    while IFS= read -r line; do
        # Log every line from the training process
        echo "$(date '+%Y-%m-%d %H:%M:%S.%3N') $line" >> "$EVENTS_LOG"

        if [[ "$line" =~ Merge\ ([0-9]+)/([0-9]+) ]]; then
            local current=${BASH_REMATCH[1]}
            local total=${BASH_REMATCH[2]}
            
            # Decide whether to show verbose merge info or a progress bar
            if [[ "${VERBOSE_MERGES:-false}" == "true" ]]; then
                echo -e "\r\033[K${MAGENTA}[MERGE]${NC} $line" > /dev/tty
            else
                # Overwrite the spinner from run_with_spinner with a progress bar
                printf "\r\033[K" > /dev/tty
                draw_progress_bar "$current" "$total"
            fi
        elif [[ "${VERBOSE_TOKENS:-false}" == "true" ]]; then
             # Show individual token details if verbose
             echo -e "\r\033[K${CYAN}[TOKEN]${NC} $line" > /dev/tty
        fi
        # Non-matching lines are logged but not displayed to keep the UI clean
    done
    if [[ -t 1 ]] && command -v tput > /dev/null 2>&1; then
        tput cnorm # Restore cursor
    fi
    if [[ -t 1 ]] && [[ -w /dev/tty ]]; then
        echo "" > /dev/tty # Ensure a new line after the progress bar finishes
    fi
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

# --- Command Definitions ---
COMPILE_TISSDB_CMD="retry timeout $TIMEOUT_SECONDS g++ -std=c++17 -Wall -Wextra -g -march=native \
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

START_TISSDB_CMD="./tissdb_exe"

COMPILE_TRAINING_CMD="retry timeout $TIMEOUT_SECONDS g++ -std=c++17 -o train_model_exe \
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
    -Iquanta_tissu/tisslm/program/training -I. -lpthread"

RUN_TRAINING_CMD="timeout $TIMEOUT_SECONDS ./train_model_exe | process_training_output"

# --- Report Generator ---
generate_report() {
    log "Generating final report..."
    local end_time=$(date +%s)
    local total_duration=$((end_time - START_TIME))
    
    echo -e "${MAGENTA}${BOLD}================================================================${NC}"
    echo -e "${MAGENTA}${BOLD}   QuantaTissu Workout Final Report${NC}"
    echo -e "${MAGENTA}${BOLD}================================================================${NC}"
    echo -e "${BOLD}Total Duration:${NC} ${total_duration}s"
    echo -e "${BOLD}Ecosystem Version:${NC} $VERSION"
    if [[ -d .git ]]; then
        echo -e "${BOLD}Git Hash:${NC} $(git rev-parse --short HEAD)"
    fi
    echo -e "${DIM}----------------------------------------------------------------${NC}"
    printf "${BOLD}%-20s | %-15s | %-10s\n${NC}" "Stage" "Status" "Duration"
    echo -e "${DIM}----------------------------------------------------------------${NC}"
    
    for stage in "${STAGES[@]}"; do
        local status=$(jq -r ".stages[\"$stage\"].status // \"UNKNOWN\"" "$STATE_FILE")
        local dur=$(jq -r ".stages[\"$stage\"].metadata.duration // \"-\"" "$STATE_FILE")
        local color=$NC
        
        case "$status" in
            "COMPLETED") color=$GREEN ;;
            "FAILED") color=$RED ;;
            "UNKNOWN") color=$DIM ;;
        esac
        
        printf "${color}%-20s | %-15s | %-10s s\n${NC}" "$stage" "$status" "$dur"
    done
    echo -e "${MAGENTA}${BOLD}================================================================${NC}"
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

run_stage() {
    local idx=$1
    local stage=${STAGES[$idx]}
    local cmd_string=$2
    local stage_num=$((idx + 1))
    
    CURRENT_STAGE="$stage"

    if is_skip_stage "$stage"; then
        display_checklist
        log "[$stage_num/$NUM_STAGES] Skipping $stage (already completed)."
        sleep 1
        return
    fi
    
    display_checklist
    
    local start_ts=$(date +%s)
    local log_file
    log_file=$(create_log_file)
    
    if [[ "${DRY_RUN:-false}" == "true" ]]; then
        log "[DRY-RUN] Would run: $cmd_string"
        sleep 1
    else
        run_with_spinner "[$stage_num/$NUM_STAGES] Running $stage..." "$cmd_string" "$log_file"
    fi

    local end_ts=$(date +%s)
    local duration=$((end_ts - start_ts))
    local mem_usage=$(free -m | awk 'NR==2{print $3}')
    
    set_checkpoint "$stage" "COMPLETED" "{\"duration\": $duration, \"mem_mb\": $mem_usage}"
    
    display_checklist
    success "[$stage_num/$NUM_STAGES] $stage completed in ${duration}s."
    rm -f "$log_file"
    sleep 1 # Pause to let user see the checkmark
}

main_workflow() {
    display_checklist
    run_stage 0 "$COMPILE_TISSDB_CMD"
    
    # --- Stage 2: START_TISSDB ---
    CURRENT_STAGE="START_TISSDB"
    if ! is_skip_stage "START_TISSDB"; then
        display_checklist
        log "[2/4] Starting START_TISSDB..."
        if [[ "${DRY_RUN:-false}" == "false" ]]; then
            $START_TISSDB_CMD &
            TISSDB_PID=$!
            log "TissDB started with PID: $TISSDB_PID"
            
            local i=0
            local start_time=$(date +%s)
            local wait_msg="Waiting for TissDB to be ready..."
            printf "${CYAN}${BOLD}$wait_msg${NC}\n"
            while ! curl -s http://localhost:8080/ping > /dev/null; do
                i=$(( (i+1) % ${#SPINNER} ))
                local elapsed=$(($(date +%s) - start_time))
                printf "\r ${SPINNER:$i:1} Running for ${elapsed}s..."
                sleep 0.5
                if (( elapsed > 60 )); then
                    error "TissDB failed to start in 60s."
                    exit 1
                fi
            done
            printf "\r ${GREEN}${CHECK}${NC} Done in $(($(date +%s) - start_time))s.      \n"
        else
            log "[DRY-RUN] Would start TissDB"
        fi
        set_checkpoint "START_TISSDB" "COMPLETED"
        display_checklist
        success "[2/4] START_TISSDB completed."
        sleep 1
    else
      log "[2/4] Skipping START_TISSDB (already completed)."
      sleep 1
    fi

    run_stage 2 "$COMPILE_TRAINING_CMD"
    run_stage 3 "$RUN_TRAINING_CMD"
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
        start_heartbeat
        main_workflow
        stop_heartbeat
        success "Workflow complete."
    fi
}

if [[ "${BASH_SOURCE[0]}" == "${0}" ]]; then
    main "$@"
fi
