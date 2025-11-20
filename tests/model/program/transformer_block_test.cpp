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

void check(bool condition, const std::string& test_name) {
    if (condition) {
        std::cout << "[  PASSED  ] " << test_name << std::endl;
    } else {
        std::cout << "[  FAILED  ] " << test_name << std::endl;
        throw std::runtime_error("Test failed: " + test_name);
    }
}

bool matrices_almost_equal(const Matrix& m1, const Matrix& m2, float epsilon = 1e-5) {
    if (m1.shape() != m2.shape()) {
        return false;
    }
    std::vector<int> indices(m1.shape().size());
    bool equal = true;
    std::function<void(int)> recurse =
        [&](int k) {
        if (!equal) return;
        if (k == (int)m1.shape().size()) {
            if (std::abs(m1.at(indices) - m2.at(indices)) > epsilon) {
                equal = false;
            }
            return;
        }
        for (int i = 0; i < m1.shape()[k]; ++i) {
            indices[k] = i;
            recurse(k + 1);
        }
    };
    recurse(0);
    return equal;
}


void run_transformer_block_test() {
    std::cout << "--- Running TransformerBlock Test (Simplified) ---" << std::endl;

    size_t d_model = TestConfig::EmbedDim;
    size_t num_heads = TestConfig::NumHeads;
    size_t d_ff = TestConfig::FFNDim;
    float dropout_p = 0.0f;
    int lora_rank = 0;

    TransformerBlock block(d_model, num_heads, d_ff, dropout_p, AttentionMode::STANDARD_MULTI_HEAD, lora_rank, "test_block");

    size_t seq_len = 5;
    Matrix x_full = Matrix::random({1, (int)seq_len, (int)d_model});

    // Non-cached forward pass (simplified)
    Matrix output_full = block.forward(x_full, Matrix(), false);

    check(output_full.shape() == std::vector<int>({1, (int)seq_len, (int)d_model}), "Full forward pass shape");
}


int main() {
    try {
        run_transformer_block_test();
        std::cout << "\nAll TransformerBlock tests passed!" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "\nTransformerBlock tests failed with exception: " << e.what() << std::endl;
        return 1;
    }
}