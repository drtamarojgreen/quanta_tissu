#include "positionalencoding.h"
#include <cmath>

namespace TissNum {

PositionalEncoding::PositionalEncoding(size_t d_model, size_t max_len)
    : d_model_(d_model), max_len_(max_len), pe_({(int)max_len, (int)d_model}) {
    
    for (size_t pos = 0; pos < max_len; ++pos) {
        for (size_t i = 0; i < d_model; ++i) {
            float angle = static_cast<float>(pos) / 
                         std::pow(10000.0f, (2.0f * (i / 2)) / static_cast<float>(d_model));
            
            if (i % 2 == 0) {
                pe_.at({(int)pos, (int)i}) = std::sin(angle);
            } else {
                pe_.at({(int)pos, (int)i}) = std::cos(angle);
            }
        }
    }
}

Matrix PositionalEncoding::forward(const Matrix& x, size_t start_pos) {
    int seq_len = x.shape()[0];
    
    if (x.shape()[1] != (int)d_model_) {
        throw std::runtime_error("Input dimension mismatch in PositionalEncoding");
    }
    
    if (start_pos + seq_len > max_len_) {
        throw std::runtime_error("Sequence length exceeds maximum positional encoding length");
    }
    
    Matrix output(x.shape());
    
    for (int i = 0; i < seq_len; ++i) {
        for (size_t j = 0; j < d_model_; ++j) {
            output.at({i, (int)j}) = x.at({i, (int)j}) + pe_.at({(int)start_pos + i, (int)j});
        }
    }
    
    return output;
}

} // namespace TissNum