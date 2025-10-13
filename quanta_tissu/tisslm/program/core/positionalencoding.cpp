#include "positionalencoding.h"
#include <cmath>

namespace TissNum {

PositionalEncoding::PositionalEncoding(size_t d_model, size_t max_len)
    : d_model_(d_model), max_len_(max_len), pe_(max_len, d_model) {
    
    // Initialize positional encoding matrix
    // PE(pos, 2i) = sin(pos / 10000^(2i/d_model))
    // PE(pos, 2i+1) = cos(pos / 10000^(2i/d_model))
    
    for (size_t pos = 0; pos < max_len; ++pos) {
        for (size_t i = 0; i < d_model; ++i) {
            float angle = static_cast<float>(pos) / 
                         std::pow(10000.0f, (2.0f * (i / 2)) / static_cast<float>(d_model));
            
            if (i % 2 == 0) {
                // Even indices: sine
                pe_(pos, i) = std::sin(angle);
            } else {
                // Odd indices: cosine
                pe_(pos, i) = std::cos(angle);
            }
        }
    }
}

Matrix PositionalEncoding::forward(const Matrix& x, size_t start_pos) {
    // x shape: (seq_len, d_model) or (batch_size, seq_len, d_model)
    // For simplicity, assuming 2D input (seq_len, d_model)
    // In a full implementation, this would handle batched inputs
    
    size_t seq_len = x.rows();
    
    if (x.cols() != d_model_) {
        throw std::runtime_error("Input dimension mismatch in PositionalEncoding");
    }
    
    if (start_pos + seq_len > max_len_) {
        throw std::runtime_error("Sequence length exceeds maximum positional encoding length");
    }
    
    // Create output matrix
    Matrix output(seq_len, d_model_);
    
    // Add positional encoding to input
    for (size_t i = 0; i < seq_len; ++i) {
        for (size_t j = 0; j < d_model_; ++j) {
            output(i, j) = x(i, j) + pe_(start_pos + i, j);
        }
    }
    
    return output;
}

} // namespace TissNum
