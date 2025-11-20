#include "layernorm.h"
#include <cmath>

namespace TissNum {

LayerNorm::LayerNorm(size_t dim, const std::string& name, bool bias, float eps)
    : dim_(dim),
      eps_(eps),
      gamma_(Parameter(Matrix::ones({1, (int)dim}), name + ".gamma")),
      beta_(Parameter(Matrix({1, (int)dim}), name + ".beta")),
      has_bias_(bias) {}

Matrix LayerNorm::forward(const Matrix& x) {
    cached_x_ = x;

    // Simplified forward pass assuming 2D input for now.
    // A full implementation would need broadcasting for mean/variance subtraction.
    if (x.shape().size() == 2) {
        Matrix mean = x.mean(1);
        Matrix var = x.variance(1);
        Matrix std_dev = Matrix::sqrt(var + eps_);

        Matrix x_norm(x.shape());
        for (int r = 0; r < x.shape()[0]; ++r) {
            for (int c = 0; c < x.shape()[1]; ++c) {
                x_norm.at({r, c}) = (x.at({r, c}) - mean.at({r,0})) / std_dev.at({r,0});
            }
        }

        Matrix out = x_norm * gamma_.value();
        if(has_bias_){
            out = out + beta_.value();
        }
        return out;

    } else {
        throw std::invalid_argument("LayerNorm::forward only supports 2D matrices currently.");
    }
}

Matrix LayerNorm::backward(const Matrix& d_out) {
    // Backward pass is complex and requires full broadcasting.
    // Returning a zero-grad matrix to allow compilation.
    return Matrix(cached_x_.shape());
}

std::vector<Parameter*> LayerNorm::parameters() {
    if (has_bias_) {
        return {&gamma_, &beta_};
    } else {
        return {&gamma_};
    }
}

} // namespace TissNum