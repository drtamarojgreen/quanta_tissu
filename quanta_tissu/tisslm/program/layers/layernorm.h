#ifndef LAYERNORM_H
#define LAYERNORM_H

#include "matrix.h"

class LayerNorm {
public:
    LayerNorm(int d_model, float eps = 1e-6);

    Matrix forward(const Matrix& x);
    Matrix backward(const Matrix& d_out, const Matrix& cache_x);
    size_t get_parameter_count() const { return gamma.size() + beta.size(); }

    Matrix gamma;
    Matrix beta;
    Matrix d_gamma;
    Matrix d_beta;
    float eps;
};

#endif // LAYERNORM_H
