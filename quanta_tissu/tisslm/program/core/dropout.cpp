#include "dropout.h"
#include <random>
#include <algorithm>

namespace TissNum {

Dropout::Dropout(float p) : p_(std::max(0.0f, std::min(1.0f, p))) {}

Matrix Dropout::forward(const Matrix& x, bool training) {
    if (!training) {
        return x;
    }

    mask_ = Matrix(x.rows(), x.cols());
    std::random_device rd;
    std::mt19937 gen(rd());
    std::bernoulli_distribution dist(1.0 - p_);

    for (size_t i = 0; i < x.rows(); ++i) {
        for (size_t j = 0; j < x.cols(); ++j) {
            mask_(i, j) = dist(gen) ? 1.0f : 0.0f;
        }
    }

    Matrix out = x * mask_;
    for (size_t i = 0; i < out.rows(); ++i) {
        for (size_t j = 0; j < out.cols(); ++j) {
            out(i, j) /= (1.0f - p_);
        }
    }

    return out;
}

Matrix Dropout::backward(const Matrix& d_out) {
    Matrix dx = d_out * mask_;
    for (size_t i = 0; i < dx.rows(); ++i) {
        for (size_t j = 0; j < dx.cols(); ++j) {
            dx(i, j) /= (1.0f - p_);
        }
    }
    return dx;
}

} // namespace TissNum
