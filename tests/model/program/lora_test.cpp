#include "gtest/gtest.h"
#include "core/multiheadattention.h"
#include "core/matrix.h"

TEST(LoraTest, LoraForward) {
    TissNum::MultiHeadAttention mha(16, 4, 4);
    TissNum::Matrix q_in({1, 16});
    TissNum::Matrix k_in({1, 16});
    TissNum::Matrix v_in({1, 16});

    TissNum::Matrix output = mha.forward(q_in, k_in, v_in);

    EXPECT_EQ(output.get_shape(), std::vector<size_t>({1, 16}));
}

// Function to compute numerical gradient for LoRA parameters
float compute_numerical_gradient_lora(TissNum::MultiHeadAttention& mha, TissNum::Matrix& q_in, TissNum::Matrix& k_in, TissNum::Matrix& v_in, TissNum::Parameter& param, int i, int j) {
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

TEST(LoraTest, LoraBackward) {
    TissNum::MultiHeadAttention mha(16, 4, 4);
    TissNum::Matrix q_in = TissNum::Matrix::random({1, 10, 16});
    TissNum::Matrix k_in = TissNum::Matrix::random({1, 10, 16});
    TissNum::Matrix v_in = TissNum::Matrix::random({1, 10, 16});
    TissNum::Matrix d_out = TissNum::Matrix::random({1, 10, 16});

    mha.forward(q_in, k_in, v_in);
    mha.backward(d_out);

    auto params = mha.parameters();
    for (auto p : params) {
        if (p->name().find("lora") != std::string::npos) {
            for (size_t i = 0; i < p->grad().rows(); ++i) {
                for (size_t j = 0; j < p->grad().cols(); ++j) {
                    float num_grad = compute_numerical_gradient_lora(mha, q_in, k_in, v_in, *p, i, j);
                    float backprop_grad = p->grad()({i, j});
                    EXPECT_NEAR(num_grad, backprop_grad, 1e-3);
                }
            }
        }
    }
}

TEST(LoraTest, NoLora) {
    TissNum::MultiHeadAttention mha(16, 4, 0);
    auto params = mha.parameters();
    for (auto p : params) {
        EXPECT_EQ(p->name().find("lora"), std::string::npos);
    }
}

TEST(LoraTest, LoraBackward) {
    TissNum::MultiHeadAttention mha(16, 4, 4);
    TissNum::Matrix q_in = TissNum::Matrix::random({1, 16});
    TissNum::Matrix k_in = TissNum::Matrix::random({1, 16});
    TissNum::Matrix v_in = TissNum::Matrix::random({1, 16});
    TissNum::Matrix d_out = TissNum::Matrix::random({1, 16});

    mha.forward(q_in, k_in, v_in);
    mha.backward(d_out);

    auto params = mha.parameters();
    for (auto p : params) {
        if (p->name().find("lora") != std::string::npos) {
            // Check that the gradients are not all zero
            bool all_zero = true;
            for (size_t i = 0; i < p->grad().get_shape()[0]; ++i) {
                for (size_t j = 0; j < p->grad().get_shape()[1]; ++j) {
                    if (p->grad()({i, j}) != 0.0f) {
                        all_zero = false;
                        break;
                    }
                }
                if (!all_zero) break;
            }
            EXPECT_FALSE(all_zero);
        }
    }
}