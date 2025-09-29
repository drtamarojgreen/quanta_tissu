#include "layernorm.h"
#include <cmath>

LayerNorm::LayerNorm(int d_model, float eps)
    : gamma(1, d_model), beta(1, d_model), eps(eps) {
    // Initialize gamma to ones and beta to zeros
    for (int i = 0; i < d_model; ++i) {
        gamma.at(0, i) = 1.0f;
        beta.at(0, i) = 0.0f;
    }
}

Matrix LayerNorm::forward(const Matrix& x) {
    int rows = x.get_rows();
    int cols = x.get_cols();
    Matrix result(rows, cols);

    for (int i = 0; i < rows; ++i) {
        float mean = 0.0f;
        for (int j = 0; j < cols; ++j) {
            mean += x.at(i, j);
        }
        mean /= cols;

        float variance = 0.0f;
        for (int j = 0; j < cols; ++j) {
            variance += std::pow(x.at(i, j) - mean, 2);
        }
        variance /= cols;

        float inv_std = 1.0f / std::sqrt(variance + eps);

        for (int j = 0; j < cols; ++j) {
            result.at(i, j) = (x.at(i, j) - mean) * inv_std * gamma.at(0, j) + beta.at(0, j);
        }
    }

    return result;
}