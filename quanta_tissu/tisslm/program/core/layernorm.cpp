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

    Matrix x_norm(x.rows(), x.cols());
    Matrix std_dev = Matrix::sqrt(var + eps_);

    for (size_t r = 0; r < x.rows(); ++r) {
        for (size_t c = 0; c < x.cols(); ++c) {
            x_norm(r, c) = (x(r, c) - mean(r, 0)) / std_dev(r, 0);
        }
    }

    Matrix out(x.rows(), x.cols());
    for (size_t r = 0; r < x.rows(); ++r) {
        for (size_t c = 0; c < x.cols(); ++c) {
            out(r, c) = x_norm(r, c) * gamma_.value()(0, c);
            if (has_bias_) {
                out(r, c) += beta_.value()(0, c);
            }
        }
    }
    return out;
}

Matrix LayerNorm::backward(const Matrix& d_out, const Matrix& cache) {
    size_t N = cache.rows();
    size_t D = cache.cols();

    Matrix mean = cache.mean(1);
    Matrix var = cache.variance(1, mean);
    Matrix std_dev = Matrix::sqrt(var + eps_);

    Matrix x_norm(N, D);
    Matrix cache_centered(N, D);

    for (size_t r = 0; r < N; ++r) {
        for (size_t c = 0; c < D; ++c) {
            cache_centered(r, c) = cache(r, c) - mean(r, 0);
            x_norm(r, c) = cache_centered(r, c) / std_dev(r, 0);
        }
    }

    // Gradients for gamma and beta
    gamma_.grad() = (d_out * x_norm).sum(0);
    if (has_bias_) {
        beta_.grad() = d_out.sum(0);
    }

    // Gradient for the input x
    Matrix dx_norm(N, D);
    for (size_t r = 0; r < N; ++r) {
        for (size_t c = 0; c < D; ++c) {
            dx_norm(r, c) = d_out(r, c) * gamma_.value()(0, c);
        }
    }

    Matrix inv_std_dev = 1.0f / std_dev;
    Matrix pow_var_term = Matrix::pow(var + eps_, -1.5f);

    Matrix dvar_sum_term = dx_norm * cache_centered;
    Matrix dvar(N, 1);
    for(size_t r=0; r < N; ++r) {
        float row_sum = 0.0f;
        for(size_t c=0; c < D; ++c) {
            row_sum += dvar_sum_term(r, c);
        }
        dvar(r, 0) = row_sum * -0.5f * pow_var_term(r, 0);
    }

    Matrix dmean_term1 = dx_norm.sum(1);
    for(size_t r=0; r < N; ++r) {
        dmean_term1(r, 0) *= -1.0f * inv_std_dev(r, 0);
    }

    Matrix dmean_term2 = (cache_centered.sum(1) * -2.0f / D);
    for(size_t r=0; r < N; ++r) {
        dmean_term2(r, 0) *= dvar(r, 0);
    }
    Matrix dmean = dmean_term1 + dmean_term2;

    Matrix dx(N, D);
    for(size_t r = 0; r < N; ++r) {
        for (size_t c = 0; c < D; ++c) {
            float term1 = dx_norm(r, c) * inv_std_dev(r, 0);
            float term2 = dvar(r, 0) * (2.0f * cache_centered(r, c)) / D;
            float term3 = dmean(r, 0) / D;
            dx(r, c) = term1 + term2 + term3;
        }
    }

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
