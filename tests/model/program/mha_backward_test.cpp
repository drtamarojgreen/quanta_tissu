#include <iostream>
#include <cassert>
#include <vector>
#include <stdexcept>
#include "../../../quanta_tissu/tisslm/program/core/matrix.h"
#include "../../../quanta_tissu/tisslm/program/core/multiheadattention.h"

using namespace TissNum;

// A minimal assertion helper to provide more context on failure
void assert_true(bool condition, const std::string& message) {
    if (!condition) {
        throw std::runtime_error("Assertion failed: " + message);
    }
}

void test_mha_backward_mqa() {
    std::cout << "--- Running MHA Backward Pass MQA Test ---" << std::endl;

    // 1. Setup MQA parameters
    size_t d_model = 32;
    size_t num_heads = 8;
    size_t batch_size = 1;
    size_t seq_len = 10;
    int lora_rank = 0;

    // 2. Instantiate MHA in MQA mode
    MultiHeadAttention mha(d_model, num_heads, lora_rank, "test_mha", AttentionMode::MULTI_QUERY);

    // 3. Create input matrices
    Matrix q_in = Matrix::random({batch_size, seq_len, d_model});
    Matrix k_in = Matrix::random({batch_size, seq_len, d_model});
    Matrix v_in = Matrix::random({batch_size, seq_len, d_model});
    Matrix mask = Matrix::zeros({1, 1, seq_len, seq_len});

    // 4. Perform a forward pass to populate cached values
    Matrix forward_output = mha.forward(q_in, k_in, v_in, mask, std::nullopt, nullptr);

    // 5. Create a dummy d_out gradient matrix
    Matrix d_out = Matrix::random(forward_output.get_shape());

    // 6. Perform a backward pass
    Matrix dx = mha.backward(d_out);

    // 7. Assertions
    assert_true(dx.get_shape() == q_in.get_shape(), "Gradient dx should have the same shape as the input q_in.");
    std::cout << "[  PASSED  ] Gradient dx has the correct shape." << std::endl;
}

int main() {
    try {
        test_mha_backward_mqa();
        std::cout << "\nMHA Backward MQA test PASSED." << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "\nMHA Backward MQA test FAILED: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
