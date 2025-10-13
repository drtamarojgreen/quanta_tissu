#include "layernorm.h"
#include <cmath>

namespace TissNum {

LayerNorm::LayerNorm(size_t dim, const std::string& name, bool bias, float eps)
    : dim_(dim),
      eps_(eps),
      gamma_(Parameter(Matrix::ones(1, dim), name + ".gamma")),
      beta_(Parameter(Matrix::zeros(1, dim), name + ".beta")),
      has_bias_(bias) {}

Matrix LayerNorm::forward(const Matrix& x) {
    // x shape: (batch_size, dim)
    Matrix mean = x.mean(1); // Mean across the feature dimension
    Matrix var = x.variance(1, mean); // Variance across the feature dimension

    Matrix x_norm = (x - mean) / Matrix::sqrt(var + eps_);

    Matrix out = gamma_.value() * x_norm;
    if (has_bias_) {
        out = out + beta_.value();
    }

    // For backward pass, we need to cache some values
    // In a real implementation, we would have a more sophisticated cache mechanism
    // For now, we'll just return the normalized matrix and recompute others in backward
    return out;
}

Matrix LayerNorm::backward(const Matrix& d_out, const Matrix& cache) {
    size_t N = cache.rows();
    size_t D = cache.cols();

    Matrix mean = cache.mean(1);
    Matrix var = cache.variance(1, mean);
    Matrix x_norm = (cache - mean) / Matrix::sqrt(var + eps_);

    // Gradients for gamma and beta
    gamma_.grad() = (d_out * x_norm).sum(0);
    if (has_bias_) {
        beta_.grad() = d_out.sum(0);
    }

    // Gradient for the input x
    Matrix dx_norm = d_out * gamma_.value();
    Matrix dvar = (dx_norm * (cache - mean) * -0.5 * Matrix::pow(var + eps_, -1.5)).sum(1);
    Matrix dmean = (dx_norm * (-1.0 / Matrix::sqrt(var + eps_))).sum(1) + (dvar * (-2.0 * (cache - mean)).mean(1) / D);

    Matrix dx = (dx_norm / Matrix::sqrt(var + eps_)) + (dvar * 2.0 * (cache - mean) / D) + (dmean / D);

    return dx;
}

std::vector<Parameter*> LayerNorm::parameters() {
    if (has_bias_) {
        return {&gamma_, &beta_};
    } else {
        return {&gamma_};
    }
}

} // namespace TissNum
