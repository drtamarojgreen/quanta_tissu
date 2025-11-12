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
    w2_.grad() = Matrix::matmul(cached_hidden_.transpose(), d_output);
    b2_.grad() = d_output.sum(0); // Sum along batch dimension

    Matrix d_hidden = Matrix::matmul(d_output, w2_.value().transpose());
    d_hidden = relu_backward(d_hidden, cached_hidden_);

    // Backward pass for hidden layer
    w1_.grad() = Matrix::matmul(cached_x_.transpose(), d_hidden);
    b1_.grad() = d_hidden.sum(0); // Sum along batch dimension

    Matrix dx = Matrix::matmul(d_hidden, w1_.value().transpose());
    return dx;
}

std::vector<Parameter*> FeedForward::parameters() {
    return {&w1_, &b1_, &w2_, &b2_};
}

} // namespace TissNum
