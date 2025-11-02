#pragma once

#include "matrix.h"
#include "parameter.h"

namespace TissNum {

class LayerNorm {
public:
    LayerNorm(size_t dim, const std::string& name = "", bool bias = true, float eps = 1e-5);

    Matrix forward(const Matrix& x);
    Matrix backward(const Matrix& d_out);

    std::vector<Parameter*> parameters();

private:
    size_t dim_;
    float eps_;
    Parameter gamma_;
    Parameter beta_;
    bool has_bias_;
    Matrix cached_x_;
};

} // namespace TissNum
