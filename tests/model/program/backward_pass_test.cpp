#include "gtest/gtest.h"
#include "core/transformerblock.h"
#include "core/matrix.h"

// Function to compute numerical gradient
float compute_numerical_gradient(TissNum::TransformerBlock& block, TissNum::Matrix& x, TissNum::Parameter& param, int i, int j) {
    float epsilon = 1e-4;
    float old_val = param.value()({i, j});

    param.value()({i, j}) = old_val + epsilon;
    TissNum::Matrix out1 = block.forward(x, TissNum::Matrix(), std::nullopt, nullptr, true);
    float loss1 = out1.sum(-1)({0, 0});

    param.value()({i, j}) = old_val - epsilon;
    TissNum::Matrix out2 = block.forward(x, TissNum::Matrix(), std::nullopt, nullptr, true);
    float loss2 = out2.sum(-1)({0, 0});

    param.value()({i, j}) = old_val;

    return (loss1 - loss2) / (2 * epsilon);
}

TEST(BackwardPassTest, TransformerBlockBackward) {
    TissNum::TransformerBlock block(4, 2, 16, 0.0, 0);
    TissNum::Matrix x = TissNum::Matrix::random({1, 4});

    // Forward and backward pass
    TissNum::Matrix out = block.forward(x, TissNum::Matrix(), std::nullopt, nullptr, true);
    TissNum::Matrix d_out = TissNum::Matrix::ones(out.get_shape());
    block.backward(d_out);

    // Check gradients
    auto params = block.parameters();
    for (auto p : params) {
        for (size_t i = 0; i < p->grad().rows(); ++i) {
            for (size_t j = 0; j < p->grad().cols(); ++j) {
                float num_grad = compute_numerical_gradient(block, x, *p, i, j);
                float backprop_grad = p->grad()({i, j});
                EXPECT_NEAR(num_grad, backprop_grad, 1e-3);
            }
        }
    }
}

// Function to compute numerical gradient for MultiHeadAttention
float compute_numerical_gradient_mha(TissNum::MultiHeadAttention& mha, TissNum::Matrix& q_in, TissNum::Matrix& k_in, TissNum::Matrix& v_in, TissNum::Parameter& param, int i, int j) {
    float epsilon = 1e-4;
    float old_val = param.value()({i, j});

    param.value()({i, j}) = old_val + epsilon;
    TissNum::Matrix out1 = mha.forward(q_in, k_in, v_in);
    float loss1 = out1.sum(-1)({0, 0});

    param.value()({i, j}) = old_val - epsilon;
    TissNum::Matrix out2 = mha.forward(q_in, k_in, v_in);
    float loss2 = out2.sum(-1)({0, 0});

    param.value()({i, j}) = old_val;

    return (loss1 - loss2) / (2 * epsilon);
}

TEST(BackwardPassTest, MultiHeadAttentionBackward) {
    TissNum::MultiHeadAttention mha(16, 4, 0);
    TissNum::Matrix q_in = TissNum::Matrix::random({1, 10, 16});
    TissNum::Matrix k_in = TissNum::Matrix::random({1, 10, 16});
    TissNum::Matrix v_in = TissNum::Matrix::random({1, 10, 16});
    TissNum::Matrix d_out = TissNum::Matrix::random({1, 10, 16});

    mha.forward(q_in, k_in, v_in);
    mha.backward(d_out);

    auto params = mha.parameters();
    for (auto p : params) {
        for (size_t i = 0; i < p->grad().rows(); ++i) {
            for (size_t j = 0; j < p->grad().cols(); ++j) {
                float num_grad = compute_numerical_gradient_mha(mha, q_in, k_in, v_in, *p, i, j);
                float backprop_grad = p->grad()({i, j});
                EXPECT_NEAR(num_grad, backprop_grad, 1e-3);
            }
        }
    }
}

TEST(BackwardPassTest, ZeroGradients) {
    TissNum::TransformerBlock block(16, 4, 64, 0.1, 4);
    TissNum::Matrix x = TissNum::Matrix::random({1, 10, 16});
    TissNum::Matrix d_out = TissNum::Matrix::zeros({1, 10, 16});

    block.forward(x, TissNum::Matrix(), std::nullopt, nullptr, true);
    block.backward(d_out);

    auto params = block.parameters();
    for (auto p : params) {
        for (size_t i = 0; i < p->grad().rows(); ++i) {
            for (size_t j = 0; j < p->grad().cols(); ++j) {
                EXPECT_EQ(p->grad()({i, j}), 0.0f);
            }
        }
    }
}