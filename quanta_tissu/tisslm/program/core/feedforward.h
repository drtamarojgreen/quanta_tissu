#pragma once

#include "matrix.h"
#include "parameter.h"

namespace TissNum {

class FeedForward {
public:
    FeedForward(size_t d_model, size_t d_ff, const std::string& name = "");

    Matrix forward(const Matrix& x);
    Matrix backward(const Matrix& d_out);

    std::vector<Parameter*> parameters();

private:
    Parameter w1_;
    Parameter b1_;
    Parameter w2_;
    Parameter b2_;

    // Cache for backward pass
    Matrix cached_x_;
    Matrix cached_hidden_;
};

} // namespace TissNum
