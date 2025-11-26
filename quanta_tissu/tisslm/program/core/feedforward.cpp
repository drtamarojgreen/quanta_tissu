#include "feedforward.h"
#include <algorithm>

namespace TissNum {

// Placeholder for ReLU activation
Matrix relu(const Matrix& x) {
    Matrix result(x.get_shape());
    if (x.get_shape().size() == 2) {
        for (size_t i = 0; i < x.rows(); ++i) {
            for (size_t j = 0; j < x.cols(); ++j) {
                result({i, j}) = std::max(0.0f, x({i, j}));
            }
        }
    } else if (x.get_shape().size() == 3) {
        for (size_t i = 0; i < x.get_shape()[0]; ++i) {
            for (size_t j = 0; j < x.get_shape()[1]; ++j) {
                for (size_t k = 0; k < x.get_shape()[2]; ++k) {
                    result({i, j, k}) = std::max(0.0f, x({i, j, k}));
                }
            }
        }
    } else {
        throw std::invalid_argument("ReLU supports only 2D and 3D matrices.");
    }
    return result;
}

// Placeholder for ReLU backward
Matrix relu_backward(const Matrix& d_out, const Matrix& x) {
    Matrix result(x.get_shape());
    if (x.get_shape().size() == 2) {
        for (size_t i = 0; i < x.rows(); ++i) {
            for (size_t j = 0; j < x.cols(); ++j) {
                result({i, j}) = (x({i, j}) > 0) ? d_out({i, j}) : 0.0f;
            }
        }
    } else if (x.get_shape().size() == 3) {
        for (size_t i = 0; i < x.get_shape()[0]; ++i) {
            for (size_t j = 0; j < x.get_shape()[1]; ++j) {
                for (size_t k = 0; k < x.get_shape()[2]; ++k) {
                    result({i, j, k}) = (x({i, j, k}) > 0) ? d_out({i, j, k}) : 0.0f;
                }
            }
        }
    } else {
        throw std::invalid_argument("ReLU backward supports only 2D and 3D matrices.");
    }
    return result;
}

FeedForward::FeedForward(size_t d_model, size_t d_ff, const std::string& name)
    : w1_(Parameter(Matrix::random({d_model, d_ff}), name + ".w1")),
      b1_(Parameter(Matrix::zeros({1, d_ff}), name + ".b1")),
      w2_(Parameter(Matrix::random({d_ff, d_model}), name + ".w2")),
      b2_(Parameter(Matrix::zeros({1, d_model}), name + ".b2")) {}

Matrix FeedForward::forward(const Matrix& x) {
    cached_x_ = x;

    Matrix hidden = Matrix::matmul(x, w1_.value());
    hidden = hidden + b1_.value();
    cached_hidden_ = hidden;
    hidden = relu(hidden);

    Matrix output = Matrix::matmul(hidden, w2_.value());
    output = output + b2_.value();
    return output;
}

Matrix FeedForward::backward(const Matrix& d_out) {
    // Backward pass for output layer
    Matrix d_output = d_out;
    
    size_t batch_size = d_output.get_shape()[0];
    
    if (d_output.get_shape().size() == 3) {
        size_t seq_len = d_output.get_shape()[1];
        
        // Reshape to 2D for matmul
        Matrix cached_hidden_reshaped = cached_hidden_.reshape({batch_size * seq_len, cached_hidden_.get_shape()[2]});
        Matrix d_output_reshaped = d_output.reshape({batch_size * seq_len, d_output.get_shape()[2]});
        
        w2_.grad() = Matrix::matmul(cached_hidden_reshaped.transpose(), d_output_reshaped);
        
        // Bias gradient: sum over batch and sequence
        // d_output is (B, L, D). sum(0) -> (L, D). sum(0) of that -> (D).
        // TissNum::Matrix::sum(axis) returns a Matrix.
        // Let's assume we can chain or just sum the reshaped (N, D) over axis 0.
        b2_.grad() = d_output_reshaped.sum(0); 
        
        Matrix d_hidden_reshaped = Matrix::matmul(d_output_reshaped, w2_.value().transpose());
        Matrix d_hidden = d_hidden_reshaped.reshape({batch_size, seq_len, cached_hidden_.get_shape()[2]});
        
        d_hidden = relu_backward(d_hidden, cached_hidden_);
        
        // Hidden layer
        Matrix cached_x_reshaped = cached_x_.reshape({batch_size * seq_len, cached_x_.get_shape()[2]});
        d_hidden_reshaped = d_hidden.reshape({batch_size * seq_len, d_hidden.get_shape()[2]});
        
        w1_.grad() = Matrix::matmul(cached_x_reshaped.transpose(), d_hidden_reshaped);
        b1_.grad() = d_hidden_reshaped.sum(0);
        
        Matrix dx_reshaped = Matrix::matmul(d_hidden_reshaped, w1_.value().transpose());
        return dx_reshaped.reshape({batch_size, seq_len, cached_x_.get_shape()[2]});
        
    } else {
        // 2D case
        w2_.grad() = Matrix::matmul(cached_hidden_.transpose(), d_output);
        b2_.grad() = d_output.sum(0);

        Matrix d_hidden = Matrix::matmul(d_output, w2_.value().transpose());
        d_hidden = relu_backward(d_hidden, cached_hidden_);

        w1_.grad() = Matrix::matmul(cached_x_.transpose(), d_hidden);
        b1_.grad() = d_hidden.sum(0);

        return Matrix::matmul(d_hidden, w1_.value().transpose());
    }
}


std::vector<Parameter*> FeedForward::parameters() {
    return {&w1_, &b1_, &w2_, &b2_};
}

} // namespace TissNum
