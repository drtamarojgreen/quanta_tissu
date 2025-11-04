#include "feedforward.h"
#include <algorithm>

namespace TissNum {

// Placeholder for ReLU activation
Matrix relu(const Matrix& x) {
    Matrix result({x.rows(), x.cols()});
    for (size_t i = 0; i < x.rows(); ++i) {
        for (size_t j = 0; j < x.cols(); ++j) {
            result({i, j}) = std::max(0.0f, x({i, j}));
        }
    }
    return result;
}

// Placeholder for ReLU backward
Matrix relu_backward(const Matrix& d_out, const Matrix& x) {
    Matrix result({x.rows(), x.cols()});
    for (size_t i = 0; i < x.rows(); ++i) {
        for (size_t j = 0; j < x.cols(); ++j) {
            result({i, j}) = (x({i, j}) > 0) ? d_out({i, j}) : 0.0f;
        }
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

    Matrix hidden_no_bias = Matrix::matmul(x, w1_.value());
    Matrix hidden({hidden_no_bias.rows(), hidden_no_bias.cols()});
    for (size_t r = 0; r < hidden.rows(); ++r) {
        for (size_t c = 0; c < hidden.cols(); ++c) {
            hidden({r, c}) = hidden_no_bias({r, c}) + b1_.value()({0, c});
        }
    }
    cached_hidden_ = hidden;
    hidden = relu(hidden);

    Matrix output_no_bias = Matrix::matmul(hidden, w2_.value());
    Matrix output({output_no_bias.rows(), output_no_bias.cols()});
    for (size_t r = 0; r < output.rows(); ++r) {
        for (size_t c = 0; c < output.cols(); ++c) {
            output({r, c}) = output_no_bias({r, c}) + b2_.value()({0, c});
        }
    }
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
