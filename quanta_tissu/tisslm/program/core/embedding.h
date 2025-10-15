#pragma once

#include "matrix.h"
#include "parameter.h"
#include <memory>

namespace TissNum {

class Embedding {
public:
    // Constructor
    // vocab_size: size of the vocabulary
    // d_model: dimension of the embedding vectors
    Embedding(size_t vocab_size, size_t d_model, const std::string& name = "");

    // Forward pass
    // input: token indices (seq_len,) - for simplicity, 1D array of token IDs
    // Returns: embedded vectors (seq_len, d_model)
    Matrix forward(const std::vector<size_t>& input);

    // Backward pass
    // d_out: gradient from upstream (seq_len, d_model)
    // input: original token indices used in forward pass
    void backward(const Matrix& d_out, const std::vector<size_t>& input);

    // Get parameters
    std::vector<Parameter*> parameters();

    const Matrix& get_weight() const;

private:
    size_t vocab_size_;
    size_t d_model_;
    std::string name_;
    std::unique_ptr<Parameter> weight_;  // Embedding weight matrix (vocab_size, d_model)
};

} // namespace TissNum
