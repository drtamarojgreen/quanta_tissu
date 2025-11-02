#include "../../../quanta_tissu/tisslm/program/core/transformer_model.h"
#include "../../../quanta_tissu/tisslm/program/training/optimizer.h"
#include "../../../quanta_tissu/tisslm/program/training/loss_function.h"
#include "../../../quanta_tissu/tisslm/program/core/embedding.h"
#include "../../../quanta_tissu/tisslm/program/core/layernorm.h"
#include "../../../quanta_tissu/tisslm/program/core/feedforward.h"
#include "../../../quanta_tissu/tisslm/program/core/multiheadattention.h"
#include "config/TestConfig.h"
#include <iostream>
#include <vector>
#include <memory>
#include <cassert>

using namespace TissLM::Core;
using namespace TissLM::Training;
using namespace TissNum;

void test_embedding_dimensions() {
    std::cout << "  --- Testing Embedding Layer Dimensions ---" << std::endl;
    Embedding layer(100, TestConfig::EmbedDim);
    std::vector<size_t> input_tokens = {1, 2, 3, 4, 5};
    Matrix d_out = Matrix::random(5, TestConfig::EmbedDim);

    layer.forward(input_tokens);
    layer.backward(d_out, input_tokens);

    auto* param = layer.parameters()[0];
    assert(param->value().rows() == param->grad().rows() && "Embedding grad rows mismatch");
    assert(param->value().cols() == param->grad().cols() && "Embedding grad cols mismatch");
    std::cout << "    [PASSED] Embedding layer dimensions are consistent." << std::endl;
}

void test_layernorm_dimensions() {
    std::cout << "  --- Testing LayerNorm Layer Dimensions ---" << std::endl;
    LayerNorm layer(TestConfig::EmbedDim);
    Matrix input = Matrix::random(5, TestConfig::EmbedDim);
    Matrix d_out = Matrix::random(5, TestConfig::EmbedDim);

    layer.forward(input);
    layer.backward(d_out);

    for (auto* param : layer.parameters()) {
        assert(param->value().rows() == param->grad().rows() && "LayerNorm grad rows mismatch");
        assert(param->value().cols() == param->grad().cols() && "LayerNorm grad cols mismatch");
    }
    std::cout << "    [PASSED] LayerNorm layer dimensions are consistent." << std::endl;
}

void test_feedforward_dimensions() {
    std::cout << "  --- Testing FeedForward Layer Dimensions ---" << std::endl;
    FeedForward layer(TestConfig::EmbedDim, TestConfig::FFNDim);
    Matrix input = Matrix::random(5, TestConfig::EmbedDim);
    Matrix d_out = Matrix::random(5, TestConfig::EmbedDim);

    layer.forward(input);
    layer.backward(d_out);

    for (auto* param : layer.parameters()) {
        assert(param->value().rows() == param->grad().rows() && "FeedForward grad rows mismatch");
        assert(param->value().cols() == param->grad().cols() && "FeedForward grad cols mismatch");
    }
    std::cout << "    [PASSED] FeedForward layer dimensions are consistent." << std::endl;
}

void test_attention_dimensions() {
    std::cout << "  --- Testing MultiHeadAttention Layer Dimensions ---" << std::endl;
    MultiHeadAttention layer(TestConfig::EmbedDim, TestConfig::NumHeads, 0);
    Matrix input = Matrix::random(5, TestConfig::EmbedDim);
    Matrix d_out = Matrix::random(5, TestConfig::EmbedDim);

    layer.forward(input, input, input);
    layer.backward(d_out);

    for (auto* param : layer.parameters()) {
        assert(param->value().rows() == param->grad().rows() && "Attention grad rows mismatch");
        assert(param->value().cols() == param->grad().cols() && "Attention grad cols mismatch");
    }
    std::cout << "    [PASSED] MultiHeadAttention layer dimensions are consistent." << std::endl;
}


void run_dimension_integrity_suite() {
    std::cout << "=== Running Dimension Integrity Test Suite ===" << std::endl;
    test_embedding_dimensions();
    test_layernorm_dimensions();
    test_feedforward_dimensions();
    test_attention_dimensions();
}

int main() {
    try {
        run_dimension_integrity_suite();
        std::cout << "\n[SUCCESS] All component dimension integrity tests passed." << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "\n[FAILED] A dimension integrity test failed with exception: " << e.what() << std::endl;
        return 1;
    }
}
