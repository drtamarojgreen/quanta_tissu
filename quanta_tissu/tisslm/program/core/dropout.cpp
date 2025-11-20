#include "dropout.h"
#include <random>
#include <algorithm>

namespace TissNum {

Dropout::Dropout(float p) : p_(std::max(0.0f, std::min(1.0f, p))) {}

Matrix Dropout::forward(const Matrix& x, bool training) {
    if (!training || p_ == 0.0f) {
        return x;
    }

    mask_ = Matrix(x.shape());
    std::random_device rd;
    std::mt19937 gen(rd());
    std::bernoulli_distribution dist(1.0 - p_);

    // This is inefficient. A proper implementation would iterate recursively.
    // Flattening for simplicity to allow compilation.
    std::vector<int> indices(x.shape().size());
    std::function<void(int)> recurse =
        [&](int k) {
        if (k == (int)x.shape().size()) {
            mask_.at(indices) = dist(gen) ? 1.0f : 0.0f;
            return;
        }
        for (int i = 0; i < x.shape()[k]; ++i) {
            indices[k] = i;
            recurse(k + 1);
        }
    };
    recurse(0);

    Matrix out = x * mask_;
    return out / (1.0f - p_);
}

Matrix Dropout::backward(const Matrix& d_out) {
    if (p_ == 0.0f) {
        return d_out;
    }
    Matrix dx = d_out * mask_;
    return dx / (1.0f - p_);
}

} // namespace TissNum