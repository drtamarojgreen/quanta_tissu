#!/bin/bash

# This script accepts the command line argument --target which can have four options:
#   clean:  Removes the build directory.
#   build:  Configures and compiles the Stellar showcase.
#   run:    Executes the Stellar showcase and displays the report.
#   all:    Performs clean, build, and run operations sequentially.

# --- Robustness and Reliability Foundation ---
set -euo pipefail
IFS=$'\n\t'
export LC_ALL=C

# Script constants
VERSION="1.4.0"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# Project root is 3 levels up from tests/model/program
REPO_ROOT="$(cd "$SCRIPT_DIR/../../.." && pwd)"
STELLAR_DIR="$SCRIPT_DIR/stellar_inference"
BUILD_DIR="$SCRIPT_DIR/stellar_build"
REPORT_FILE="stellar_report.txt"

# Color codes
GREEN='\033[0;32m'
CYAN='\033[0;36m'
MAGENTA='\033[0;35m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m'
BOLD='\033[1m'

log()     { echo -e "${CYAN}${BOLD}[STELLAR]${NC} $1"; }
success() { echo -e "${GREEN}${BOLD}[SUCCESS]${NC} $1"; }
warn()    { echo -e "${YELLOW}${BOLD}[WARN]${NC} $1"; }
error()   { echo -e "${RED}${BOLD}[ERROR]${NC} $1"; }

display_header() {
    echo -e "${MAGENTA}${BOLD}"
    echo "================================================================"
    echo "    QuantaTissu Frontier - Stellar Pipeline Workout v$VERSION"
    echo "================================================================"
    echo -e "${NC}"
}

# --- Target Implementations ---

do_clean() {
    log "Cleaning: Removing build directory $BUILD_DIR..."
    rm -rf "$BUILD_DIR"
    success "Clean complete."
}

do_build() {
    log "Building: Configuring and compiling Stellar package..."
    mkdir -p "$BUILD_DIR"
    cd "$BUILD_DIR"
    # Use absolute paths for CMake to avoid directory-not-found errors
    cmake "$STELLAR_DIR"
    make -j$(nproc)
    success "Build complete."
}

do_run() {
    log "Running: Executing Stellar Integrated Showcase..."
    if [[ ! -d "$BUILD_DIR" ]]; then
        error "Build directory not found. Please run with --target build or --target all first."
        exit 1
    fi
    cd "$BUILD_DIR"
    if [[ ! -f "./stellar_inference_test" ]]; then
        error "Binary stellar_inference_test not found. Please build first."
        exit 1
    fi

    # Export the project root so the test can reliably find source files
    export STELLAR_PROJECT_ROOT="$REPO_ROOT"
    log "Project Root: $REPO_ROOT"

    ./stellar_inference_test

    if [[ -f "$REPORT_FILE" ]]; then
        success "Stellar Pipeline executed successfully."
        log "Final Report available at: $(pwd)/$REPORT_FILE"
        echo "----------------------------------------------------------------"
        cat "$REPORT_FILE"
        echo "----------------------------------------------------------------"
        # Copy report back to the script directory and root for visibility
        cp "$REPORT_FILE" "$SCRIPT_DIR/stellar_report.txt"
        cp "$REPORT_FILE" "$REPO_ROOT/stellar_report.txt"
    else
        error "Report file $REPORT_FILE was not generated."
        exit 1
    fi
}

# --- Main Logic ---

TARGET="all" # Default target

# Simple argument parser
for arg in "$@"; do
    case $arg in
        --target)
            if [[ -n "${2:-}" ]]; then
                TARGET="$2"
                shift 2
            else
                error "--target requires an argument."
                exit 1
            fi
            ;;
        --clean)
            TARGET="clean"
            ;;
    esac
done

display_header

case $TARGET in
    clean)
        do_clean
        ;;
    build)
        do_build
        ;;
    run)
        do_run
        ;;
    all)
        do_clean
        do_build
        do_run
        ;;
    *)
        error "Invalid target: $TARGET. Supported: clean, build, run, all."
        exit 1
        ;;
esac

success "Workout Sequence Complete."
