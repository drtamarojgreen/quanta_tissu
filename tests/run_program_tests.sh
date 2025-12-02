#!/bin/bash

# --- Safety Preamble ---
# Get the absolute path of the directory where this script is located.
SCRIPT_DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &> /dev/null && pwd)

# Navigate to the repository root from the script's location.
cd "$SCRIPT_DIR/.."

# --- Main Script Logic ---
echo "Guaranteed to be running from repository root: $(pwd)"

TEST_DIR="tests/model/program"
BUILD_DIR="$TEST_DIR/build"

# Clean the build directory if it exists to ensure a fresh build
if [ -d "$BUILD_DIR" ]; then
    echo "Cleaning existing build directory: $BUILD_DIR"
    rm -rf "$BUILD_DIR"
fi

# Create a new build directory and navigate into it
echo "Creating new build directory: $BUILD_DIR"
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Configure the project with CMake.
echo "Configuring with CMake..."
cmake -DTOKENIZER_PATH_FROM_CMAKE=../../../test_tokenizer/test_tokenizer ..

# Build the test executables
echo "Building test executables..."
cmake --build .
if [ $? -ne 0 ]; then
    echo "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
    echo "!!! Build failed. Aborting tests."
    echo "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
    exit 1
fi

# --- Test Execution and Summary Generation ---
echo "Running test executables..."

# Arrays to store summary data
declare -a test_names
declare -a test_statuses
declare -a test_details
declare -a test_descriptions
declare -a test_durations

OVERALL_EXIT_CODE=0
TEST_COUNT=0
FAILED_TESTS=0

# Descriptions for each test executable
get_description() {
    case "$1" in
        ./test_db_and_retrieval) echo "Database CRUD, search, and retrieval strategies." ;;
        ./test_new_components) echo "Core model layers (Embedding, Positional Encoding)." ;;
        ./matrix_test) echo "Low-level matrix operations and broadcasting." ;;
        ./tokenizer_test) echo "BPE Tokenizer encoding, decoding, and training." ;;
        ./tokenizer_unicode_test) echo "Unicode handling in tokenizer." ;;
        ./training_test) echo "Optimizer and Loss Function implementation." ;;
        ./model_generation_test) echo "Full Transformer Model forward pass and text generation." ;;
        ./full_fledged_workout_cpp) echo "End-to-end model workout with various parameters." ;;
        ./kv_cache_test) echo "Performance and correctness of the KV Cache." ;;
        ./rag_self_update_test) echo "RAG pipeline and knowledge base self-updating." ;;
        ./experimental_sampling_test) echo "Experimental text generation sampling methods." ;;
        ./standard_generation_test) echo "Standard text generation sampling methods." ;;
        ./rule_enforcement_test) echo "Grammar and style rule enforcement on generated text." ;;
        ./advanced_analysis_test) echo "Advanced text analysis and understanding." ;;
        ./tissdb_lite_integration_test) echo "Integration with the TissDB-Lite database." ;;
        ./advanced_sampling_test) echo "Advanced, unimplemented sampling methods." ;;
        ./bayesian_sampling_test) echo "Bayesian sampling methods." ;;
        ./encoding_test) echo "Tokenizer encoding verification." ;;
        ./checkpoint_test) echo "Model checkpointing and restoration." ;;
        ./dimension_integrity_test) echo "Matrix dimension consistency checks." ;;
        ./lora_test) echo "LoRA (Low-Rank Adaptation) implementation." ;;
        ./forward_pass_test) echo "Model forward pass verification." ;;
        ./backward_pass_test) echo "Model backward pass verification." ;;
        ./training_checkpoint_test) echo "Training state checkpointing." ;;
        ./transformer_block_test) echo "Transformer block component tests." ;;
        ./concatenate_test) echo "Matrix concatenation operations." ;;
        ./mqa_test) echo "Multi-Query Attention implementation." ;;
        ./mha_backward_test) echo "Multi-Head Attention backward pass." ;;
        ./mha_backward_test) echo "Multi-Head Attention backward pass." ;;
        ./bug_fix_verification_test) echo "Regression tests for specific bug fixes." ;;
        ./checkpoint_comparison_test) echo "Comparison of generated text across training checkpoints." ;;
        *) echo "Description not available." ;;
    esac
}

analyze_failure() {
    local log_file="$1"
    local reason=""

    if grep -q "Connection failed" "$log_file"; then
        reason="DB Connection Failed"
    elif grep -q "Shape mismatch" "$log_file"; then
        reason=$(grep -o "Shape mismatch:.*" "$log_file" | head -n 1)
    elif grep -q "Matrix dimensions are not compatible" "$log_file"; then
        reason="Broadcasting Error"
    elif grep -q "Segmentation fault" "$log_file"; then
        reason="Segmentation Fault"
    elif grep -q "Assertion failed" "$log_file"; then
        reason="Assertion Failed"
    elif grep -q "std::exception" "$log_file"; then
        reason="Unhandled Exception"
    else
        reason="Unknown Failure"
    fi
    echo "$reason"
}

TEMP_OUTPUT_FILE="test_output.log"

# Run tests
# If an argument is provided, filter tests by name
FILTER="$1"

for test_exe in $(find . -maxdepth 1 -type f -executable -not -name "*.*" | sort); do
    # Skip if filter is set and doesn't match
    if [ -n "$FILTER" ] && [[ "$test_exe" != *"$FILTER"* ]]; then
        continue
    fi
    TEST_COUNT=$((TEST_COUNT + 1))
    test_names+=("$test_exe")
    test_descriptions+=("$(get_description "$test_exe")")

    echo "========================================="
    echo "Executing: $test_exe"
    echo "========================================="

    start_ts=$(date +%s%N)

    # Execute the test, capture output and exit code, while showing it in real-time
    set -o pipefail
    "./$test_exe" 2>&1 | tee "$TEMP_OUTPUT_FILE"
    exit_code=$?
    set +o pipefail

    end_ts=$(date +%s%N)
    diff=$((end_ts - start_ts))
    sec=$((diff / 1000000000))
    ms=$(( (diff % 1000000000) / 1000000 ))
    printf -v duration_str "%d.%03ds" $sec $ms
    test_durations+=("$duration_str")

    if [ $exit_code -ne 0 ]; then
        echo "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
        echo "!!! Test Failed: $test_exe"
        echo "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
        OVERALL_EXIT_CODE=1
        FAILED_TESTS=$((FAILED_TESTS + 1))
        test_statuses+=("FAIL")
        
        failure_reason=$(analyze_failure "$TEMP_OUTPUT_FILE")
        test_details+=("$failure_reason")
    else
        test_statuses+=("PASS")
        
        # Try to extract passed/failed counts if available
        passed_count=$(grep -o 'Passed: [0-9]*' "$TEMP_OUTPUT_FILE" | awk '{print $2}' | tail -n 1)
        failed_count=$(grep -o 'Failed: [0-9]*' "$TEMP_OUTPUT_FILE" | awk '{print $2}' | tail -n 1)
        
        if [[ -n "$passed_count" && -n "$failed_count" ]]; then
            test_details+=("Passed: $passed_count, Failed: $failed_count")
        else
            test_details+=("All checks passed")
        fi
    fi
    echo "========================================="
done

rm -f "$TEMP_OUTPUT_FILE"

# --- Executive Summary ---
echo
echo "============================================================================================================================================"
echo "                                           TissLM C++ Test Suite: Executive Summary"
echo "============================================================================================================================================"
printf "%-30s | %-55s | %-6s | %-8s | %s\n" "Test Executable" "Description" "Status" "Duration" "Details"
echo "--------------------------------------------------------------------------------------------------------------------------------------------"

for i in "${!test_names[@]}"; do
    printf "%-30s | %-55s | %-6s | %-8s | %s\n" "${test_names[i]}" "${test_descriptions[i]}" "${test_statuses[i]}" "${test_durations[i]}" "${test_details[i]}"
done

echo "============================================================================================================================================"
echo "Overall Result: $((TEST_COUNT - FAILED_TESTS)) / $TEST_COUNT tests passed."
echo "============================================================================================================================================"

# Actionable Recommendations
if [ $OVERALL_EXIT_CODE -ne 0 ]; then
    echo
    echo "Actionable Recommendations:"
    echo "---------------------------"
    
    # Check for specific failure patterns in the details array
    db_failures=0
    shape_failures=0
    
    for detail in "${test_details[@]}"; do
        if [[ "$detail" == *"Connection Failed"* ]]; then
            db_failures=$((db_failures + 1))
        fi
        if [[ "$detail" == *"Shape mismatch"* || "$detail" == *"Broadcasting Error"* ]]; then
            shape_failures=$((shape_failures + 1))
        fi
    done

    if [ $db_failures -gt 0 ]; then
        echo "1. [Database] $db_failures tests failed due to connection issues. Ensure 'tissdb' server is running on port 9876."
    fi
    
    if [ $shape_failures -gt 0 ]; then
        echo "2. [Matrix Ops] $shape_failures tests failed due to shape/broadcasting errors. Check tensor dimensions in 'Dropout', 'LayerNorm', or 'Attention' layers."
    fi
    
    echo "3. Review the logs above for specific assertion failures or stack traces."
    exit 1
else
    echo
    echo "All tests completed successfully. System is stable."
    exit 0
fi
