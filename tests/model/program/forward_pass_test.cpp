#include "../../../quanta_tissu/tisslm/program/core/transformerblock.h"
#include "../../../quanta_tissu/tisslm/program/core/configurableattention.h"
#include "../../../quanta_tissu/tisslm/program/core/matrix.h"
#include <iostream>
#include <vector>
#include <stdexcept>

using namespace TissNum;

void check(bool condition, const std::string& test_name) {
    if (condition) {
        std::cout << "[  PASSED  ] " << test_name << std::endl;
    } else {
        std::cout << "[  FAILED  ] " << test_name << std::endl;
        throw std::runtime_error("Test failed: " + test_name);
    }
}

void test_transformer_block_forward() {
    std::cout << "--- Testing TransformerBlock Forward ---" << std::endl;
    TransformerBlock block(16, 4, 64, 0.1, AttentionMode::STANDARD_MULTI_HEAD, 4);
    Matrix x({1, 10, 16});

    Matrix output = block.forward(x);

    check(output.shape() == std::vector<int>({1, 10, 16}), "TransformerBlock forward output shape");
}

void test_configurable_attention_forward() {
    std::cout << "--- Testing ConfigurableAttention Forward ---" << std::endl;
    ConfigurableAttention mha(16, 4, AttentionMode::STANDARD_MULTI_HEAD, 4);
    Matrix x = Matrix::random({1, 10, 16});

    Matrix output = mha.forward(x);

    check(output.shape() == std::vector<int>({1, 10, 16}), "ConfigurableAttention forward output shape");
}

void test_configurable_attention_forward_with_mask() {
    std::cout << "--- Testing ConfigurableAttention Forward with Mask ---" << std::endl;
    ConfigurableAttention mha(16, 4, AttentionMode::STANDARD_MULTI_HEAD, 4);
    Matrix x = Matrix::random({1, 10, 16});
    Matrix mask = Matrix::ones({1, 10, 10});

    Matrix output = mha.forward(x, mask);

    check(output.shape() == std::vector<int>({1, 10, 16}), "ConfigurableAttention forward with mask output shape");
}


int main() {
    try {
        test_transformer_block_forward();
        test_configurable_attention_forward();
        test_configurable_attention_forward_with_mask();
        std::cout << "\nAll Forward pass tests passed!" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "\nForward pass tests failed with exception: " << e.what() << std::endl;
        return 1;
    }
}