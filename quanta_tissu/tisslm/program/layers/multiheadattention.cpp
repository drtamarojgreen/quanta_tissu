#include "multiheadattention.h"
#include <cmath>
#include <stdexcept>
#include <numeric>
#include <algorithm>

// Helper function for softmax
Matrix softmax(Matrix scores, int axis = -1) {
    if (axis != -1 && (size_t)axis != scores.get_shape().size() - 1 && axis != -1) {
         // Simplified softmax only supports last axis
    }

    size_t rows = scores.rows();
    size_t cols = scores.cols();

    for (size_t i = 0; i < rows; ++i) {
        float max_val = scores({i, 0});
        for (size_t j = 1; j < cols; ++j) {
            if (scores({i, j}) > max_val) {
                max_val = scores({i, j});
            }
        }

        float sum = 0.0f;
        for (size_t j = 0; j < cols; ++j) {
            scores({i, j}) = std::exp(scores({i, j}) - max_val);
            sum += scores({i, j});
        }

        for (size_t j = 0; j < cols; ++j) {
            scores({i, j}) /= sum;
        }
    }
    return scores;
}


MultiHeadAttention::MultiHeadAttention(int d_model, int num_heads)
    : d_model(d_model), num_heads(num_heads), d_k(d_model / num_heads),
      wq(Matrix::random(std::vector<size_t>{(size_t)d_model, (size_t)d_model})),
      wk(Matrix::random(std::vector<size_t>{(size_t)d_model, (size_t)d_model})),
      wv(Matrix::random(std::vector<size_t>{(size_t)d_model, (size_t)d_model})),
      wo(Matrix::random(std::vector<size_t>{(size_t)d_model, (size_t)d_model})) {
    if (d_model % num_heads != 0) {
        throw std::invalid_argument("d_model must be divisible by num_heads");
    }
}

Matrix MultiHeadAttention::scaled_dot_product_attention(const Matrix& q, const Matrix& k, const Matrix& v, const Matrix* mask) {
    Matrix k_t = k.transpose();
    Matrix scores = Matrix::matmul(q, k_t) / std::sqrt(static_cast<float>(d_k));
    if (mask) {
        scores = scores + *mask;
    }
    Matrix weights = softmax(scores);
    return Matrix::matmul(weights, v);
}

Matrix MultiHeadAttention::split_heads(const Matrix& x) {
    size_t seq_len = x.rows();
    Matrix reshaped = x.reshape({seq_len, (size_t)num_heads, (size_t)d_k});
    return reshaped.transpose(0, 1);
}

Matrix MultiHeadAttention::combine_heads(const Matrix& x) {
    Matrix transposed = x.transpose(0, 1);
    return transposed.reshape({transposed.rows(), (size_t)d_model});
}

Matrix MultiHeadAttention::forward(const Matrix& x, const Matrix* mask) {
    size_t seq_len = x.rows();
    Matrix q = Matrix::matmul(x, wq);
    Matrix k = Matrix::matmul(x, wk);
    Matrix v = Matrix::matmul(x, wv);

    Matrix q_heads = split_heads(q);
    Matrix k_heads = split_heads(k);
    Matrix v_heads = split_heads(v);

    Matrix output_heads(std::vector<size_t>{(size_t)num_heads, seq_len, (size_t)d_k});

    for (int h = 0; h < num_heads; ++h) {
        Matrix q_h(seq_len, (size_t)d_k);
        Matrix k_h(seq_len, (size_t)d_k);
        Matrix v_h(seq_len, (size_t)d_k);

        for (size_t i = 0; i < seq_len; ++i) {
            for (size_t j = 0; j < (size_t)d_k; ++j) {
                q_h({i, j}) = q_heads({(size_t)h, i, j});
                k_h({i, j}) = k_heads({(size_t)h, i, j});
                v_h({i, j}) = v_heads({(size_t)h, i, j});
            }
        }

        Matrix head_attn = scaled_dot_product_attention(q_h, k_h, v_h, mask);

        for (size_t i = 0; i < seq_len; ++i) {
            for (size_t j = 0; j < (size_t)d_k; ++j) {
                output_heads({(size_t)h, i, j}) = head_attn({i, j});
            }
        }
    }

    Matrix combined = combine_heads(output_heads);
    return Matrix::matmul(combined, wo);
}
