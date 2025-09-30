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
    // This is a simplified backward pass and assumes cache contains x_norm, var, etc.
    // A full implementation would be more complex.
    // For now, this is a placeholder.
    return d_out; // Placeholder
}

std::vector<Parameter*> LayerNorm::parameters() {
    if (has_bias_) {
        return {&gamma_, &beta_};
    } else {
        return {&gamma_};
    }
}

} // namespace TissNum
