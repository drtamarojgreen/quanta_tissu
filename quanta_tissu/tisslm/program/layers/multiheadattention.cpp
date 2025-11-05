#include "multiheadattention.h"
#include <cmath>
#include <stdexcept>
#include <numeric>
#include <algorithm>

// Helper function for softmax
Matrix softmax(Matrix scores, int axis = -1) {
    if (axis != -1 && axis != scores.cols() -1) {
        // This implementation only supports softmax over the last dimension
        throw std::invalid_argument("Softmax only supported for the last axis.");
    }

    for (size_t i = 0; i < scores.rows(); ++i) {
        float max_val = scores({i, 0});
        for (size_t j = 1; j < scores.cols(); ++j) {
            if (scores({i, j}) > max_val) {
                max_val = scores({i, j});
            }
        }

        float sum = 0.0f;
        for (size_t j = 0; j < scores.cols(); ++j) {
            scores({i, j}) = std::exp(scores({i, j}) - max_val);
            sum += scores({i, j});
        }

        for (size_t j = 0; j < scores.cols(); ++j) {
            scores({i, j}) /= sum;
        }
    }
    return scores;
}


MultiHeadAttention::MultiHeadAttention(int d_model, int num_heads)
    : d_model(d_model), num_heads(num_heads), d_k(d_model / num_heads),
      wq(Matrix::random({(size_t)d_model, (size_t)d_model})),
      wk(Matrix::random({(size_t)d_model, (size_t)d_model})),
      wv(Matrix::random({(size_t)d_model, (size_t)d_model})),
      wo(Matrix::random({(size_t)d_model, (size_t)d_model})) {
    if (d_model % num_heads != 0) {
        throw std::invalid_argument("d_model must be divisible by num_heads");
    }
}

Matrix MultiHeadAttention::scaled_dot_product_attention(const Matrix& q, const Matrix& k, const Matrix& v, const Matrix* mask) {
    Matrix scores = Matrix::matmul(q, k.transpose()) / std::sqrt(static_cast<float>(d_k));
    if (mask) {
        scores = scores + *mask;
    }
    Matrix weights = softmax(scores);
    return Matrix::matmul(weights, v);
}

Matrix MultiHeadAttention::split_heads(const Matrix& x) {
    size_t batch_size = x.rows();
    size_t seq_len = x.cols() / d_model;
    // This is a simplified split. A real implementation would need to handle 3D tensors.
    // For now, we'll assume the input is 2D and reshape accordingly.
    // This part of the code is a placeholder and will need to be adapted for a full 3D tensor library.
    return x; // Placeholder
}

Matrix MultiHeadAttention::combine_heads(const Matrix& x) {
    // This is the inverse of split_heads. Placeholder for now.
    return x; // Placeholder
}

Matrix MultiHeadAttention::forward(const Matrix& x, const Matrix* mask) {
    Matrix q = Matrix::matmul(x, wq);
    Matrix k = Matrix::matmul(x, wk);
    Matrix v = Matrix::matmul(x, wv);

    // The split_heads and combine_heads operations require proper 3D tensor support.
    // For this implementation, we will perform attention on the full matrices
    // as a simplification. This is not multi-head attention, but a single-head version.
    Matrix attention_output = scaled_dot_product_attention(q, k, v, mask);

    return Matrix::matmul(attention_output, wo);
}