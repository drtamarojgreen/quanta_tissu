#include "layernorm.h"
#include <cmath>

LayerNorm::LayerNorm(int d_model, float eps)
    : gamma(1, d_model), beta(1, d_model), d_gamma(1, d_model), d_beta(1, d_model), eps(eps) {
    for (int i = 0; i < d_model; ++i) {
        gamma({0, (size_t)i}) = 1.0f;
        beta({0, (size_t)i}) = 0.0f;
    }
}

Matrix LayerNorm::forward(const Matrix& x) {
    size_t rows = x.rows(), cols = x.cols();
    Matrix res(rows, cols);
    for (size_t i = 0; i < rows; ++i) {
        float m = 0.0f; for (size_t j = 0; j < cols; ++j) m += x({i, j});
        m /= cols;
        float v = 0.0f; for (size_t j = 0; j < cols; ++j) v += std::pow(x({i, j}) - m, 2);
        v /= cols;
        float inv_std = 1.0f / std::sqrt(v + eps);
        for (size_t j = 0; j < cols; ++j) res({i, j}) = (x({i, j}) - m) * inv_std * gamma({0, j}) + beta({0, j});
    }
    return res;
}

Matrix LayerNorm::backward(const Matrix& d_out, const Matrix& x) {
    size_t rows = x.rows(), cols = x.cols();
    Matrix dx(rows, cols);
    for (size_t i = 0; i < rows; ++i) {
        float m = 0.0f; for (size_t j = 0; j < cols; ++j) m += x({i, j});
        m /= cols;
        float v = 0.0f; for (size_t j = 0; j < cols; ++j) v += std::pow(x({i, j}) - m, 2);
        v /= cols;
        float inv_std = 1.0f / std::sqrt(v + eps);
        float d_m = 0.0f, d_v = 0.0f;
        for (size_t j = 0; j < cols; ++j) {
            float x_hat = (x({i, j}) - m) * inv_std;
            d_gamma({0, j}) += d_out({i, j}) * x_hat;
            d_beta({0, j}) += d_out({i, j});
            float d_x_hat = d_out({i, j}) * gamma({0, j});
            d_v += d_x_hat * (x({i, j}) - m) * (-0.5f) * std::pow(v + eps, -1.5f);
            d_m += d_x_hat * (-inv_std);
        }
        for (size_t j = 0; j < cols; ++j) {
            dx({i, j}) = d_out({i, j}) * gamma({0, j}) * inv_std + d_v * 2.0f * (x({i, j}) - m) / cols + d_m / cols;
        }
    }
    return dx;
}
