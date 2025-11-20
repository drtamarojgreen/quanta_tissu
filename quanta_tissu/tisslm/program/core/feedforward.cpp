#include "feedforward.h"
#include <algorithm>
#include <functional>

namespace TissNum {

Matrix relu(const Matrix& x) {
    Matrix result(x.shape());
    std::vector<int> indices(x.shape().size());
    std::function<void(int)> recurse =
        [&](int k) {
        if (k == (int)x.shape().size()) {
            result.at(indices) = std::max(0.0f, x.at(indices));
            return;
        }
        for (int i = 0; i < x.shape()[k]; ++i) {
            indices[k] = i;
            recurse(k + 1);
        }
    };
    recurse(0);
    return result;
}

Matrix relu_backward(const Matrix& d_out, const Matrix& x) {
    Matrix result(x.shape());
     std::vector<int> indices(x.shape().size());
    std::function<void(int)> recurse =
        [&](int k) {
        if (k == (int)x.shape().size()) {
            result.at(indices) = (x.at(indices) > 0) ? d_out.at(indices) : 0.0f;
            return;
        }
        for (int i = 0; i < x.shape()[k]; ++i) {
            indices[k] = i;
            recurse(k + 1);
        }
    };
    recurse(0);
    return result;
}

FeedForward::FeedForward(size_t d_model, size_t d_ff, const std::string& name)
    : w1_(Parameter(Matrix::random({(int)d_model, (int)d_ff}), name + ".w1")),
      b1_(Parameter(Matrix({1, (int)d_ff}), name + ".b1")),
      w2_(Parameter(Matrix::random({(int)d_ff, (int)d_model}), name + ".w2")),
      b2_(Parameter(Matrix({1, (int)d_model}), name + ".b2")) {}

Matrix FeedForward::forward(const Matrix& x) {
    cached_x_ = x;
    Matrix hidden = Matrix::matmul(x, w1_.value());
    hidden = hidden + b1_.value(); // Broadcasting needed here
    cached_hidden_ = hidden;
    hidden = relu(hidden);
    Matrix output = Matrix::matmul(hidden, w2_.value());
    output = output + b2_.value(); // Broadcasting needed here
    return output;
}

Matrix FeedForward::backward(const Matrix& d_out) {
    Matrix d_output = d_out;
    w2_.grad() = Matrix::matmul(cached_hidden_.transpose(0, 1), d_output);
    b2_.grad() = d_output.sum(0);

    Matrix d_hidden = Matrix::matmul(d_output, w2_.value().transpose(0, 1));
    d_hidden = relu_backward(d_hidden, cached_hidden_);

    w1_.grad() = Matrix::matmul(cached_x_.transpose(0, 1), d_hidden);
    b1_.grad() = d_hidden.sum(0);

    Matrix dx = Matrix::matmul(d_hidden, w1_.value().transpose(0, 1));
    return dx;
}

std::vector<Parameter*> FeedForward::parameters() {
    return {&w1_, &b1_, &w2_, &b2_};
}

} // namespace TissNum