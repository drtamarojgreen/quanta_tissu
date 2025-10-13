#include "multiheadattention.h"
#include <cmath>
#include <stdexcept>
#include <numeric>
#include <algorithm>

// Helper function for softmax
Matrix softmax(Matrix scores, int axis = -1) {
    if (axis != -1 && axis != scores.get_cols() -1) {
        // This implementation only supports softmax over the last dimension
        throw std::invalid_argument("Softmax only supported for the last axis.");
    }

    for (int i = 0; i < scores.get_rows(); ++i) {
        float max_val = scores.at(i, 0);
        for (int j = 1; j < scores.get_cols(); ++j) {
            if (scores.at(i, j) > max_val) {
                max_val = scores.at(i, j);
            }
        }

        float sum = 0.0f;
        for (int j = 0; j < scores.get_cols(); ++j) {
            scores.at(i, j) = std::exp(scores.at(i, j) - max_val);
            sum += scores.at(i, j);
        }

        for (int j = 0; j < scores.get_cols(); ++j) {
            scores.at(i, j) /= sum;
        }
    }
    return scores;
}


MultiHeadAttention::MultiHeadAttention(int d_model, int num_heads)
    : d_model(d_model), num_heads(num_heads), d_k(d_model / num_heads),
      wq(Matrix::random(d_model, d_model)),
      wk(Matrix::random(d_model, d_model)),
      wv(Matrix::random(d_model, d_model)),
      wo(Matrix::random(d_model, d_model)) {
    if (d_model % num_heads != 0) {
        throw std::invalid_argument("d_model must be divisible by num_heads");
    }
}

Matrix MultiHeadAttention::scaled_dot_product_attention(const Matrix& q, const Matrix& k, const Matrix& v, const Matrix* mask) {
    Matrix scores = q.dot(k.transpose()) / std::sqrt(static_cast<float>(d_k));
    if (mask) {
        scores = scores + *mask;
    }
    Matrix weights = softmax(scores);
    return weights.dot(v);
}

Matrix MultiHeadAttention::split_heads(const Matrix& x) {
    int batch_size = x.get_rows();
    int seq_len = x.get_cols() / d_model;
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
    Matrix q = x.dot(wq);
    Matrix k = x.dot(wk);
    Matrix v = x.dot(wv);

    // The split_heads and combine_heads operations require proper 3D tensor support.
    // For this implementation, we will perform attention on the full matrices
    // as a simplification. This is not multi-head attention, but a single-head version.
    Matrix attention_output = scaled_dot_product_attention(q, k, v, mask);

    return attention_output.dot(wo);
}