#include "layernorm.h"
#include <cmath>

LayerNorm::LayerNorm(int d_model, float eps)
    : gamma({1, (size_t)d_model}), beta({1, (size_t)d_model}), eps(eps) {
    // Initialize gamma to ones and beta to zeros
    for (int i = 0; i < d_model; ++i) {
        gamma({0, (size_t)i}) = 1.0f;
        beta({0, (size_t)i}) = 0.0f;
    }
}

Matrix LayerNorm::forward(const Matrix& x) {
    size_t rows = x.rows();
    size_t cols = x.cols();
    Matrix result({rows, cols});

    for (size_t i = 0; i < rows; ++i) {
        float mean = 0.0f;
        for (size_t j = 0; j < cols; ++j) {
            mean += x({i, j});
        }
        mean /= cols;

        float variance = 0.0f;
        for (size_t j = 0; j < cols; ++j) {
            variance += std::pow(x({i, j}) - mean, 2);
        }
        variance /= cols;

        float inv_std = 1.0f / std::sqrt(variance + eps);

        for (size_t j = 0; j < cols; ++j) {
            result({i, j}) = (x({i, j}) - mean) * inv_std * gamma({0, j}) + beta({0, j});
        }
    }

    return result;
}