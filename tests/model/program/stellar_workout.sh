#!/bin/bash

# --- Robustness and Reliability Foundation ---
set -euo pipefail
IFS=$'\n\t'
export LC_ALL=C

# Script constants
VERSION="1.1.0"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
STELLAR_DIR="$SCRIPT_DIR/stellar_inference"
BUILD_DIR="$SCRIPT_DIR/stellar_build"
REPORT_FILE="stellar_report.txt"

# Color codes
GREEN='\033[0;32m'
CYAN='\033[0;36m'
MAGENTA='\033[0;35m'
NC='\033[0m'
BOLD='\033[1m'

log()     { echo -e "${CYAN}${BOLD}[STELLAR]${NC} $1"; }
success() { echo -e "${GREEN}${BOLD}[SUCCESS]${NC} $1"; }

display_header() {
    echo -e "${MAGENTA}${BOLD}"
    echo "================================================================"
    echo "    QuantaTissu Frontier - Stellar Pipeline Workout v$VERSION"
    echo "================================================================"
    echo -e "${NC}"
}

display_header

# 1. Setup Build Directory
log "Creating build directory: $BUILD_DIR..."
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# 2. Configure with CMake
log "Configuring Stellar package with CMake..."
cmake "$STELLAR_DIR"

# 3. Compile
log "Compiling Stellar Integrated Showcase..."
make -j$(nproc)

# 4. Execute
log "Executing Stellar Integrated Showcase..."
./stellar_inference_test

# 5. Result Verification
if [[ -f "$REPORT_FILE" ]]; then
    success "Stellar Pipeline executed successfully."
    log "Final Report available at: $(pwd)/$REPORT_FILE"
    echo "----------------------------------------------------------------"
    cat "$REPORT_FILE"
    echo "----------------------------------------------------------------"
    # Copy report to root for easier access
    cp "$REPORT_FILE" "$SCRIPT_DIR/../../../stellar_report.txt"
else
    echo "Error: $REPORT_FILE was not generated."
    exit 1
fi

success "Workout Complete."
