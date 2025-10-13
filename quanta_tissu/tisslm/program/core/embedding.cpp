#include "embedding.h"
#include <cmath>
#include <stdexcept>

namespace TissNum {

Embedding::Embedding(size_t vocab_size, size_t d_model, const std::string& name)
    : vocab_size_(vocab_size), d_model_(d_model), name_(name) {

    // Initialize embedding weight matrix with random values
    // Using Xavier/Glorot initialization: scale by sqrt(1/d_model)
    Matrix weight_matrix = Matrix::random(vocab_size, d_model);

    // Scale the random values
    float scale = std::sqrt(1.0f / static_cast<float>(d_model));
    for (size_t i = 0; i < vocab_size; ++i) {
        for (size_t j = 0; j < d_model; ++j) {
            weight_matrix(i, j) *= scale;
        }
    }

    weight_ = std::make_unique<Parameter>(weight_matrix, name + ".weight");
}

Matrix Embedding::forward(const std::vector<size_t>& input) {
    // input: vector of token indices
    // output: (seq_len, d_model) matrix of embeddings

    size_t seq_len = input.size();
    Matrix output(seq_len, d_model_);

    // Look up embeddings for each token
    for (size_t i = 0; i < seq_len; ++i) {
        size_t token_id = input[i];

        if (token_id >= vocab_size_) {
            throw std::out_of_range("Token ID exceeds vocabulary size");
        }

        // Copy the embedding vector for this token
        for (size_t j = 0; j < d_model_; ++j) {
            output(i, j) = weight_->value()(token_id, j);
        }
    }

    return output;
}

void Embedding::backward(const Matrix& d_out, const std::vector<size_t>& input) {
    // d_out: gradient from upstream (seq_len, d_model)
    // input: original token indices used in forward pass

    if (d_out.rows() != input.size() || d_out.cols() != d_model_) {
        throw std::runtime_error("Gradient dimensions mismatch in Embedding backward");
    }

    // Accumulate gradients for each token's embedding
    // Multiple tokens may have the same ID, so we accumulate
    for (size_t i = 0; i < input.size(); ++i) {
        size_t token_id = input[i];

        if (token_id >= vocab_size_) {
            throw std::out_of_range("Token ID exceeds vocabulary size");
        }

        // Accumulate gradient for this token's embedding
        for (size_t j = 0; j < d_model_; ++j) {
            weight_->grad()(token_id, j) += d_out(i, j);
        }
    }
}

std::vector<Parameter*> Embedding::parameters() {
    return {weight_.get()};
}

} // namespace TissNum
