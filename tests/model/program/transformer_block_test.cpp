#include "../../../quanta_tissu/tisslm/program/core/transformerblock.h"
#include "../../../quanta_tissu/tisslm/program/core/matrix.h"
#include "config/TestConfig.h"
#include <iostream>
#include <vector>
#include <memory>
#include <string>
#include <numeric>
#include <random>
#include <limits>
#include <cmath>

using namespace TissNum;

// Helper to create a causal attention mask
Matrix create_causal_mask(size_t seq_len) {
    Matrix mask = Matrix::zeros({1, 1, seq_len, seq_len});
    for (size_t i = 0; i < seq_len; ++i) {
        for (size_t j = i + 1; j < seq_len; ++j) {
            mask({0, 0, i, j}) = -std::numeric_limits<float>::infinity();
        }
    }
    return mask;
}

// Helper to check if two floats are approximately equal
bool almost_equal(float a, float b, float epsilon = 1e-6) {
    return std::abs(a - b) < epsilon;
}

// Helper to check if two matrices are approximately equal
bool matrices_almost_equal(const Matrix& m1, const Matrix& m2, float epsilon = 1e-6) {
    if (m1.get_shape() != m2.get_shape()) {
        std::cerr << "Shape mismatch: m1=";
        for(size_t s : m1.get_shape()) std::cerr << s << " ";
        std::cerr << ", m2=";
        for(size_t s : m2.get_shape()) std::cerr << s << " ";
        std::cerr << std::endl;
        return false;
    }
    for (size_t i = 0; i < m1.data_size(); ++i) {
        if (!almost_equal(m1.get_data()[i], m2.get_data()[i], epsilon)) {
            std::cerr << "Value mismatch at index " << i << ": m1=" << m1.get_data()[i] << ", m2=" << m2.get_data()[i] << std::endl;
            return false;
        }
    }
    return true;
}

void run_transformer_block_test() {
    std::cout << "=== Running TransformerBlock Test ===" << std::endl;

    size_t d_model = TestConfig::EmbedDim;
    size_t num_heads = TestConfig::NumHeads;
    size_t d_ff = TestConfig::FFNDim;
    float dropout_p = 0.0f; // No dropout for this test
    int lora_rank = 0; // Not using LoRA for this test
    size_t num_layers = TestConfig::NumLayers; // Use multiple layers

    // Instantiate multiple TransformerBlocks
    std::vector<TransformerBlock> blocks;
    for (size_t i = 0; i < num_layers; ++i) {
        blocks.emplace_back(d_model, num_heads, d_ff, dropout_p, lora_rank, "test_block_" + std::to_string(i));
    }

    // Test case 1: Full sequence processing (mimicking non-cached run)
    std::cout << "\n--- Test Case 1: Full Sequence Processing ---" << std::endl;
    size_t seq_len_full = 10;
    Matrix x_full({1, seq_len_full, d_model}); // Batch size 1, seq_len, d_model
    // Fill with some dummy data
    for (size_t i = 0; i < x_full.data_size(); ++i) {
        x_full.get_data()[i] = static_cast<float>(i) * 0.01f;
    }

    Matrix mask = create_causal_mask(seq_len_full);
    Matrix current_x_full = x_full;
    for (size_t i = 0; i < num_layers; ++i) {
        current_x_full = blocks[i].forward(current_x_full, mask);
    }
    Matrix output_full = current_x_full;

    std::cout << "Output full shape: ";
    for (size_t s : output_full.get_shape()) std::cout << s << " ";
    std::cout << std::endl;
    std::cout << "Output full sum: " << output_full.sum(-1)({0}) << std::endl;

    // Test case 2: Single token processing with KV cache (mimicking cached run)
    std::cout << "\n--- Test Case 2: Single Token Processing with KV Cache ---" << std::endl;
    // KV cache for each layer
    std::vector<std::optional<std::pair<Matrix, Matrix>>> kv_caches(num_layers, std::nullopt);
    Matrix output_cached;

    // Process tokens one by one and build cache
    for (size_t i = 0; i < seq_len_full; ++i) {
        Matrix x_single({1, 1, d_model}); // Batch size 1, seq_len 1, d_model
        // Extract the i-th token's data from x_full
        for (size_t j = 0; j < d_model; ++j) {
            x_single({0, 0, j}) = x_full({0, i, j});
        }

        Matrix current_x_cached = x_single;
        std::vector<std::optional<std::pair<Matrix, Matrix>>> new_kv_caches_for_step(num_layers);

        for (size_t layer_idx = 0; layer_idx < num_layers; ++layer_idx) {
            std::optional<std::pair<Matrix, Matrix>> current_past_kv = kv_caches[layer_idx];
            std::optional<std::pair<Matrix, Matrix>> current_new_kv;
            current_x_cached = blocks[layer_idx].forward(current_x_cached, Matrix(), current_past_kv, &current_new_kv);
            new_kv_caches_for_step[layer_idx] = current_new_kv;
        }
        kv_caches = new_kv_caches_for_step;
        output_cached = current_x_cached;
    }

    std::cout << "Output cached shape: ";
    for (size_t s : output_cached.get_shape()) std::cout << s << " ";
    std::cout << std::endl;
    std::cout << "Output cached sum: " << output_cached.sum(-1)({0}) << std::endl;

    // Compare the output for the last token
    std::cout << "\n--- Comparing Last Token Outputs ---" << std::endl;
    // Extract the last token's output from the full sequence run
    Matrix last_token_output_full({1, 1, d_model});
    for (size_t j = 0; j < d_model; ++j) {
        last_token_output_full({0, 0, j}) = output_full({0, seq_len_full - 1, j});
    }

    if (matrices_almost_equal(last_token_output_full, output_cached)) {
        std::cout << "[  PASSED  ] Last token output matches for full and cached runs." << std::endl;
    } else {
        std::cout << "[  FAILED  ] Last token output mismatch for full and cached runs." << std::endl;
    }

    std::cout << "=== TransformerBlock Test Completed ===" << std::endl;
}


// ==================================================================================
// Comprehensive Test Suite
// ==================================================================================

// Helper to get a slice of a matrix for a specific token
Matrix get_token_slice(const Matrix& m, size_t token_idx, size_t d_model) {
    Matrix slice({1, 1, d_model});
    for (size_t j = 0; j < d_model; ++j) {
        slice({0, 0, j}) = m({0, token_idx, j});
    }
    return slice;
}

// Overloaded helper for comprehensive tests with more verbose output
bool comprehensive_matrices_almost_equal(const Matrix& m1, const Matrix& m2, const std::string& name, int step, int layer, float epsilon = 1e-5) {
    if (m1.get_shape() != m2.get_shape()) {
        std::cerr << "[  FAILED  ] " << name << " shape mismatch at step " << step << ", layer " << layer << std::endl;
        return false;
    }
    for (size_t i = 0; i < m1.data_size(); ++i) {
        if (!almost_equal(m1.get_data()[i], m2.get_data()[i], epsilon)) {
            std::cerr << "[  FAILED  ] " << name << " value mismatch at step " << step << ", layer " << layer << ", index " << i << ": full=" << m1.get_data()[i] << ", cached=" << m2.get_data()[i] << std::endl;
            return false;
        }
    }
    std::cout << "[  PASSED  ] " << name << " matches at step " << step << ", layer " << layer << std::endl;
    return true;
}


struct ComprehensiveTestResult {
    bool passed = true;
    std::string message;
};

ComprehensiveTestResult run_single_comprehensive_test_case(const std::string& test_name, size_t seq_len, const Matrix& x_full) {
    std::cout << "\n--- Running Comprehensive Test Case: " << test_name << " (seq_len=" << seq_len << ") ---" << std::endl;

    size_t d_model = TestConfig::EmbedDim;
    size_t num_heads = TestConfig::NumHeads;
    size_t d_ff = TestConfig::FFNDim;
    float dropout_p = 0.0f;
    int lora_rank = 0;
    size_t num_layers = TestConfig::NumLayers;

    std::vector<TransformerBlock> blocks;
    for (size_t i = 0; i < num_layers; ++i) {
        blocks.emplace_back(d_model, num_heads, d_ff, dropout_p, lora_rank, "comp_test_block_" + std::to_string(i));
    }

    // --- Full Sequence Processing ---
    Matrix mask = create_causal_mask(seq_len);
    std::vector<Matrix> full_attn_out(num_layers), full_x_plus_attn(num_layers), full_x_norm1(num_layers);
    std::vector<Matrix> full_ffn_out(num_layers), full_x_plus_ffn(num_layers), full_x_norm2(num_layers);
    Matrix current_x_full = x_full;
    for (size_t i = 0; i < num_layers; ++i) {
        current_x_full = blocks[i].forward(current_x_full, mask);
        full_attn_out[i] = blocks[i].get_attn_out();
        full_x_plus_attn[i] = blocks[i].get_x_plus_attn();
        full_x_norm1[i] = blocks[i].get_x_norm1();
        full_ffn_out[i] = blocks[i].get_ffn_out();
        full_x_plus_ffn[i] = blocks[i].get_x_plus_ffn();
        full_x_norm2[i] = blocks[i].get_x_norm2();
    }

    // --- Step-by-step Cached Processing and Comparison ---
    std::vector<std::optional<std::pair<Matrix, Matrix>>> kv_caches(num_layers, std::nullopt);

    for (size_t step = 0; step < seq_len; ++step) {
        std::cout << "\n--- Step " << step << " ---" << std::endl;
        Matrix x_single = get_token_slice(x_full, step, d_model);
        Matrix current_x_cached = x_single;
        std::vector<std::optional<std::pair<Matrix, Matrix>>> new_kv_caches_for_step(num_layers);

        for (size_t layer_idx = 0; layer_idx < num_layers; ++layer_idx) {
            std::optional<std::pair<Matrix, Matrix>> current_past_kv = kv_caches[layer_idx];
            std::optional<std::pair<Matrix, Matrix>> current_new_kv;
            current_x_cached = blocks[layer_idx].forward(current_x_cached, Matrix(), current_past_kv, &current_new_kv);
            new_kv_caches_for_step[layer_idx] = current_new_kv;

            // Compare all intermediate values
            if (!comprehensive_matrices_almost_equal(get_token_slice(full_attn_out[layer_idx], step, d_model), blocks[layer_idx].get_attn_out(), "attn_out", step, layer_idx)) return {false, "Mismatch in attn_out"};
            if (!comprehensive_matrices_almost_equal(get_token_slice(full_x_plus_attn[layer_idx], step, d_model), blocks[layer_idx].get_x_plus_attn(), "x_plus_attn", step, layer_idx)) return {false, "Mismatch in x_plus_attn"};
            if (!comprehensive_matrices_almost_equal(get_token_slice(full_x_norm1[layer_idx], step, d_model), blocks[layer_idx].get_x_norm1(), "x_norm1", step, layer_idx)) return {false, "Mismatch in x_norm1"};
            if (!comprehensive_matrices_almost_equal(get_token_slice(full_ffn_out[layer_idx], step, d_model), blocks[layer_idx].get_ffn_out(), "ffn_out", step, layer_idx)) return {false, "Mismatch in ffn_out"};
            if (!comprehensive_matrices_almost_equal(get_token_slice(full_x_plus_ffn[layer_idx], step, d_model), blocks[layer_idx].get_x_plus_ffn(), "x_plus_ffn", step, layer_idx)) return {false, "Mismatch in x_plus_ffn"};
            if (!comprehensive_matrices_almost_equal(get_token_slice(full_x_norm2[layer_idx], step, d_model), blocks[layer_idx].get_x_norm2(), "x_norm2", step, layer_idx)) return {false, "Mismatch in x_norm2"};
        }
        kv_caches = new_kv_caches_for_step;
    }

    return {true, "All steps and layers passed."};
}

void run_comprehensive_transformer_block_test_suite() {
    std::cout << "\n\n=== Running Comprehensive TransformerBlock Test Suite ===" << std::endl;

    size_t d_model = TestConfig::EmbedDim;

    struct TestCase {
        std::string name;
        size_t seq_len;
        Matrix data;
    };

    std::vector<TestCase> test_cases;

    // Test Case 1: Simple Ascending
    size_t seq_len1 = 10;
    Matrix data1({1, seq_len1, d_model});
    for (size_t i = 0; i < data1.data_size(); ++i) data1.get_data()[i] = static_cast<float>(i) * 0.01f;
    test_cases.push_back({"Simple Ascending", seq_len1, data1});

    // Test Case 2: Random
    size_t seq_len2 = 15;
    test_cases.push_back({"Random", seq_len2, Matrix::random({1, seq_len2, d_model})});

    // Test Case 3: All Zeros
    size_t seq_len3 = 5;
    test_cases.push_back({"All Zeros", seq_len3, Matrix::zeros({1, seq_len3, d_model})});

    // Test Case 4: All Ones
    size_t seq_len4 = 8;
    test_cases.push_back({"All Ones", seq_len4, Matrix::ones({1, seq_len4, d_model})});

    // Test Case 5: Short Sequence
    size_t seq_len5 = 2;
    test_cases.push_back({"Short Sequence", seq_len5, Matrix::random({1, seq_len5, d_model})});

    // Test Case 6: Long Sequence
    size_t seq_len6 = 30;
    test_cases.push_back({"Long Sequence", seq_len6, Matrix::random({1, seq_len6, d_model})});

    // Test Case 7: Negative Values
    size_t seq_len7 = 12;
    Matrix data7 = Matrix::random({1, seq_len7, d_model});
    for (size_t i = 0; i < data7.data_size(); ++i) data7.get_data()[i] *= -1.0f;
    test_cases.push_back({"Negative Values", seq_len7, data7});

    // Test Case 8: Repeating Pattern
    size_t seq_len8 = 20;
    Matrix data8({1, seq_len8, d_model});
    for (size_t i = 0; i < seq_len8; ++i) {
        for (size_t j = 0; j < d_model; ++j) {
            data8({0, i, j}) = static_cast<float>((i % 4) * 0.1f);
        }
    }
    test_cases.push_back({"Repeating Pattern", seq_len8, data8});

    // Test Case 9: Sudden Change
    size_t seq_len9 = 18;
    Matrix data9({1, seq_len9, d_model});
    for (size_t i = 0; i < seq_len9; ++i) {
        for (size_t j = 0; j < d_model; ++j) {
            data9({0, i, j}) = (i < seq_len9 / 2) ? 0.1f : 0.9f;
        }
    }
    test_cases.push_back({"Sudden Change", seq_len9, data9});

    // Test Case 10: Very Long Sequence
    size_t seq_len10 = 50;
    test_cases.push_back({"Very Long Sequence", seq_len10, Matrix::random({1, seq_len10, d_model})});

    // Test Case 11: Sequence of Length 1
    size_t seq_len11 = 1;
    test_cases.push_back({"Sequence of Length 1", seq_len11, Matrix::random({1, seq_len11, d_model})});

    // Test Case 12: Alternating Positive and Negative
    size_t seq_len12 = 16;
    Matrix data12({1, seq_len12, d_model});
    for (size_t i = 0; i < data12.data_size(); ++i) {
        data12.get_data()[i] = (i % 2 == 0) ? 0.5f : -0.5f;
    }
    test_cases.push_back({"Alternating Positive and Negative", seq_len12, data12});

    // Test Case 13: Sinusoidal Pattern
    size_t seq_len13 = 25;
    Matrix data13({1, seq_len13, d_model});
    for (size_t i = 0; i < seq_len13; ++i) {
        for (size_t j = 0; j < d_model; ++j) {
            data13({0, i, j}) = std::sin(static_cast<float>(i + j));
        }
    }
    test_cases.push_back({"Sinusoidal Pattern", seq_len13, data13});


    bool all_tests_passed = true;
    for (const auto& tc : test_cases) {
        ComprehensiveTestResult result = run_single_comprehensive_test_case(tc.name, tc.seq_len, tc.data);
        if (!result.passed) {
            all_tests_passed = false;
            std::cout << "[  FATAL  ] Comprehensive Test Case '" << tc.name << "' FAILED: " << result.message << std::endl;
            break; 
        } else {
            std::cout << "[ SUCCESS ] Comprehensive Test Case '" << tc.name << "' PASSED." << std::endl;
        }
    }

    std::cout << "\n=== Comprehensive TransformerBlock Test Suite Completed ===" << std::endl;
    if (all_tests_passed) {
        std::cout << "All comprehensive test cases passed!" << std::endl;
    } else {
        std::cout << "One or more comprehensive test cases failed." << std::endl;
    }
}


int main() {
    run_transformer_block_test();
    run_comprehensive_transformer_block_test_suite();
    return 0;
}