#include "gtest/gtest.h"
#include "core/transformerblock.h"
#include "core/matrix.h"

TEST(ForwardPassTest, TransformerBlockForward) {
    TissNum::TransformerBlock block(16, 4, 64, 0.1, 4);
    TissNum::Matrix x({1, 10, 16}); // Batch, SeqLen, Dim

    TissNum::Matrix output = block.forward(x);

    EXPECT_EQ(output.get_shape(), std::vector<size_t>({1, 10, 16}));
}

TEST(ForwardPassTest, TransformerBlockForwardDifferentShapes) {
    TissNum::TransformerBlock block(32, 8, 128, 0.1, 8);
    TissNum::Matrix x = TissNum::Matrix::random({2, 5, 32});

    TissNum::Matrix output = block.forward(x);

    EXPECT_EQ(output.get_shape(), std::vector<size_t>({2, 5, 32}));
}

TEST(ForwardPassTest, MultiHeadAttentionForward) {
    TissNum::MultiHeadAttention mha(16, 4, 4);
    TissNum::Matrix q_in = TissNum::Matrix::random({1, 10, 16});
    TissNum::Matrix k_in = TissNum::Matrix::random({1, 10, 16});
    TissNum::Matrix v_in = TissNum::Matrix::random({1, 10, 16});

    TissNum::Matrix output = mha.forward(q_in, k_in, v_in);

    EXPECT_EQ(output.get_shape(), std::vector<size_t>({1, 10, 16}));
}

TEST(ForwardPassTest, MultiHeadAttentionForwardWithMask) {
    TissNum::MultiHeadAttention mha(16, 4, 4);
    TissNum::Matrix q_in = TissNum::Matrix::random({1, 10, 16});
    TissNum::Matrix k_in = TissNum::Matrix::random({1, 10, 16});
    TissNum::Matrix v_in = TissNum::Matrix::random({1, 10, 16});
    TissNum::Matrix mask = TissNum::Matrix::ones({1, 10, 10});

    TissNum::Matrix output = mha.forward(q_in, k_in, v_in, mask);

    EXPECT_EQ(output.get_shape(), std::vector<size_t>({1, 10, 16}));
}

TEST(ForwardPassTest, EmptyInput) {
    TissNum::TransformerBlock block(16, 4, 64, 0.1, 4);
    TissNum::Matrix x;
    EXPECT_THROW(block.forward(x), std::invalid_argument);
}

TEST(ForwardPassTest, MultiHeadAttentionForward) {
    TissNum::MultiHeadAttention mha(16, 4, 4);
    TissNum::Matrix q_in = TissNum::Matrix::random({1, 10, 16});
    TissNum::Matrix k_in = TissNum::Matrix::random({1, 10, 16});
    TissNum::Matrix v_in = TissNum::Matrix::random({1, 10, 16});

    TissNum::Matrix output = mha.forward(q_in, k_in, v_in);

    EXPECT_EQ(output.get_shape(), std::vector<size_t>({1, 10, 16}));
}