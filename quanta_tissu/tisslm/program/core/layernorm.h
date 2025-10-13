#pragma once

#include "matrix.h"
#include "parameter.h"

namespace TissNum {

class LayerNorm {
public:
    LayerNorm(size_t dim, const std::string& name = "", bool bias = true, float eps = 1e-5);

    Matrix forward(const Matrix& x);
    Matrix backward(const Matrix& d_out, const Matrix& cache);

    std::vector<Parameter*> parameters();

private:
    size_t dim_;
    float eps_;
    Parameter gamma_;
    Parameter beta_;
    bool has_bias_;
};

} // namespace TissNum
