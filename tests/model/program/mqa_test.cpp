#include <iostream>
#include <cassert>
#include <vector>
#include <stdexcept>
#include "../../../quanta_tissu/tisslm/program/core/matrix.h"

using namespace TissNum;

// A minimal assertion helper to provide more context on failure
void assert_true(bool condition, const std::string& message) {
    if (!condition) {
        throw std::runtime_error("Assertion failed: " + message);
    }
}

void test_mqa_concatenate_bug() {
    std::cout << "--- Running Hyper-Focused MQA Concatenate Test ---" << std::endl;

    // 1. Setup: Replicate the exact shapes from the MQA KV cache failure
    // `past_kv` shape: {batch=1, heads=8, seq_len=5, head_dim=4}
    Matrix past_k({1, 8, 5, 4});
    // `k_new` shape: {batch=1, heads=1, seq_len=1, head_dim=4}
    Matrix k_new({1, 1, 1, 4});

    // Fill with identifiable data
    for (size_t h = 0; h < 8; ++h) {
        for (size_t s = 0; s < 5; ++s) {
            past_k({0, h, s, 0}) = 1.0f; // Fill one dimension to check later
        }
    }
    k_new({0, 0, 0, 0}) = 999.0f; // The value we expect to be broadcast

    // 2. The Operation That Fails: Concatenate along axis 2 (sequence length)
    Matrix result = Matrix::concatenate(past_k, k_new, 2);

    // 3. Assertions: Check shape and data integrity
    std::vector<size_t> expected_shape = {1, 8, 6, 4};
    assert_true(result.get_shape() == expected_shape, "Shape of concatenated matrix is incorrect.");
    std::cout << "[  PASSED  ] Concatenated shape is correct." << std::endl;

    bool broadcast_successful = true;
    for (size_t h = 0; h < 8; ++h) {
        float value_at_new_token = result({0, h, 5, 0});
        if (value_at_new_token != 999.0f) {
            broadcast_successful = false;
            std::cerr << "[  FAILED  ] Data was not broadcast correctly to head " << h 
                      << ". Expected 999.0, but got " << value_at_new_token << std::endl;
            break; 
        }
    }

    assert_true(broadcast_successful, "Data from the single-headed tensor was not broadcast to all heads.");
    std::cout << "[  PASSED  ] Data was correctly broadcast to all heads." << std::endl;
}

int main() {
    try {
        test_mqa_concatenate_bug();
        std::cout << "\nHyper-Focused MQA test PASSED." << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "\nHyper-Focused MQA test FAILED: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
