#include "embedding.h"
#include <cmath>
#include <stdexcept>

namespace TissNum {

Embedding::Embedding(size_t vocab_size, size_t d_model, const std::string& name)
    : vocab_size_(vocab_size), d_model_(d_model), name_(name) {
    
    Matrix weight_matrix = Matrix::random({(int)vocab_size, (int)d_model});
    
    float scale = std::sqrt(1.0f / static_cast<float>(d_model));
    for (size_t i = 0; i < vocab_size; ++i) {
        for (size_t j = 0; j < d_model; ++j) {
            weight_matrix.at({(int)i, (int)j}) *= scale;
        }
    }
    
    weight_ = std::make_unique<Parameter>(weight_matrix, name + ".weight");
}

Matrix Embedding::forward(const std::vector<size_t>& input) {
    size_t seq_len = input.size();
    Matrix output({(int)seq_len, (int)d_model_});
    
    for (size_t i = 0; i < seq_len; ++i) {
        size_t token_id = input[i];
        
        if (token_id >= vocab_size_) {
            throw std::out_of_range("Token ID exceeds vocabulary size");
        }
        
        for (size_t j = 0; j < d_model_; ++j) {
            output.at({(int)i, (int)j}) = weight_->value().at({(int)token_id, (int)j});
        }
    }
    
    return output;
}

void Embedding::backward(const Matrix& d_out, const std::vector<size_t>& input) {
    if ((size_t)d_out.shape()[0] != input.size() || (size_t)d_out.shape()[1] != d_model_) {
        throw std::runtime_error("Gradient dimensions mismatch in Embedding backward");
    }
    
    for (size_t i = 0; i < input.size(); ++i) {
        size_t token_id = input[i];
        
        if (token_id >= vocab_size_) {
            throw std::out_of_range("Token ID exceeds vocabulary size");
        }
        
        for (size_t j = 0; j < d_model_; ++j) {
            weight_->grad().at({(int)token_id, (int)j}) += d_out.at({(int)i, (int)j});
        }
    }
}

std::vector<Parameter*> Embedding::parameters() {
    return {weight_.get()};
}

const Matrix& Embedding::get_weight() const {
    return weight_->value();
}

} // namespace TissNum