#!/bin/bash

# --- Safety Preamble ---
# Get the absolute path of the directory where this script is located.
SCRIPT_DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &> /dev/null && pwd)

# Navigate to the repository root from the script's location.
# This script is expected to be in 'tests/', so the root is one level up.
# This makes the script safe to run from any directory.
cd "$SCRIPT_DIR/.."

# --- Main Script Logic ---
# Now that we are in the repo root, all paths are predictable.
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

# Configure the project with CMake. '..' points to the parent dir (tests/model/program)
echo "Configuring with CMake..."
cmake ..

# Build the test executables
echo "Building test executables..."
cmake --build .

# --- Test Execution and Summary Generation ---
echo "Running test executables..."

# Arrays to store summary data
declare -a test_names
declare -a test_statuses
declare -a test_details
declare -a test_descriptions

OVERALL_EXIT_CODE=0
TEST_COUNT=0
FAILED_TESTS=0

# Descriptions for each test executable (as per plan item 6)
get_description() {
    case "$1" in
        ./test_db_and_retrieval) echo "Database CRUD, search, and retrieval strategies." ;;
        ./test_new_components) echo "Core model layers (Embedding, Positional Encoding)." ;;
        ./matrix_test) echo "Low-level matrix operations." ;;
        ./tokenizer_test) echo "BPE Tokenizer encoding, decoding, and training." ;;
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
        ./advanced_sampling_test) echo "Advanced, unimplemented sampling methods (placeholders)." ;;
        *) echo "N/A" ;;
    esac
}

TEMP_OUTPUT_FILE="test_output.log"

for test_exe in $(find . -maxdepth 1 -type f -executable -not -name "*.*"); do
    TEST_COUNT=$((TEST_COUNT + 1))
    test_names+=("$test_exe")
    test_descriptions+=("$(get_description "$test_exe")")

    echo "========================================"
    echo "Executing: $test_exe"
    echo "========================================"

    # Execute the test, capture output and exit code
    "./$test_exe" > "$TEMP_OUTPUT_FILE" 2>&1
    exit_code=$?

    # Print the captured output for real-time feedback
    cat "$TEMP_OUTPUT_FILE"

    if [ $exit_code -ne 0 ]; then
        echo "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
        echo "!!! Test Failed: $test_exe"
        echo "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
        OVERALL_EXIT_CODE=1
        FAILED_TESTS=$((FAILED_TESTS + 1))
        test_statuses+=("FAIL")
    else
        test_statuses+=("PASS")
    fi
    echo "========================================"

    # Parse output for detailed summary
    passed_count=$(grep -o 'Passed: [0-9]*' "$TEMP_OUTPUT_FILE" | awk '{print $2}')
    failed_count=$(grep -o 'Failed: [0-9]*' "$TEMP_OUTPUT_FILE" | awk '{print $2}')

    if [[ -n "$passed_count" && -n "$failed_count" ]]; then
        test_details+=("Passed: $passed_count, Failed: $failed_count")
    else
        # Check for "All tests passed!" as a fallback for simpler tests
        if grep -q "All tests passed!" "$TEMP_OUTPUT_FILE"; then
            test_details+=("All sub-tests passed.")
        else
            test_details+=("No detailed summary.")
        fi
    fi
done

rm -f "$TEMP_OUTPUT_FILE"

# --- Executive Summary ---
echo
echo "========================================================================================================================"
echo "                                           TissLM C++ Test Suite: Executive Summary"
echo "========================================================================================================================"
printf "%-35s | %-60s | %-8s | %s\n" "Test Executable" "Description" "Status" "Details"
echo "------------------------------------------------------------------------------------------------------------------------"

for i in "${!test_names[@]}"; do
    printf "%-35s | %-60s | %-8s | %s\n" "${test_names[i]}" "${test_descriptions[i]}" "${test_statuses[i]}" "${test_details[i]}"
done

echo "========================================================================================================================"
echo "Overall Result: $((TEST_COUNT - FAILED_TESTS)) / $TEST_COUNT tests passed."
echo "========================================================================================================================"

# Actionable Recommendations (as per plan item 6)
if [ $OVERALL_EXIT_CODE -ne 0 ]; then
    echo
    echo "Actionable Recommendations:"
    echo "---------------------------"
    echo "1. Critical failures detected in one or more test suites. Prioritize fixing tests marked as 'FAIL'."
    if grep -q 'Connection failed' <<< "$(for i in "${!test_names[@]}"; do if [[ "${test_statuses[i]}" == "FAIL" ]]; then echo "${test_names[i]}"; fi; done)"; then
        echo "2. Multiple database tests are failing with 'Connection failed'. Ensure the TissDB server is compiled and running before executing this script."
    fi
    if grep -q 'tokenizer_test' <<< "$(for i in "${!test_names[@]}"; do if [[ "${test_statuses[i]}" == "FAIL" ]]; then echo "${test_names[i]}"; fi; done)"; then
        echo "3. The 'tokenizer_test' is failing. This is a critical dependency for most other tests. Resolve this issue first."
    fi
    exit 1
else
    echo
    echo "All tests completed successfully. Great work!"
    exit 0
fi
