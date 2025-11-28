#include "dropout.h"
#include <random>
#include <algorithm>

namespace TissNum {

Dropout::Dropout(float p) : p_(std::max(0.0f, std::min(1.0f, p))) {}

Matrix Dropout::forward(const Matrix& x, bool training) {
    if (!training) {
        return x;
    }

    mask_ = Matrix(x.get_shape());
    std::random_device rd;
    std::mt19937 gen(rd());
    std::bernoulli_distribution dist(1.0 - p_);

    size_t total_elements = x.data_size();
    float* mask_data = mask_.get_data();

    for (size_t i = 0; i < total_elements; ++i) {
        mask_data[i] = dist(gen) ? 1.0f : 0.0f;
    }

    Matrix out = x * mask_;
    out = out / (1.0f - p_);

    return out;
}

Matrix Dropout::backward(const Matrix& d_out) {
    Matrix dx = d_out * mask_;
    dx = dx / (1.0f - p_);
    return dx;
}

} // namespace TissNum
