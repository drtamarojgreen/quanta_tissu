#pragma once

#include "matrix.h"
#include "parameter.h"
#include <memory>

namespace TissNum {

/**
 * @brief Embedding layer.
 * Maps token indices to dense vectors.
 */
class Embedding {
public:
    Embedding(size_t vocab_size, size_t d_model, const std::string& name = "");

    /**
     * @brief Forward pass.
     * @param input 1D array of token IDs.
     * @return Matrix of embedded vectors (seq_len, d_model).
     */
    Matrix forward(const std::vector<size_t>& input);

    void backward(const Matrix& d_out, const std::vector<size_t>& input);

    std::vector<Parameter*> parameters();

    const Matrix& get_weight() const;

private:
    size_t vocab_size_;
    size_t d_model_;
    std::string name_;
    std::unique_ptr<Parameter> weight_;
};

}
