#pragma once

#include "matrix.h"

namespace TissNum {

class Dropout {
public:
    Dropout(float p = 0.5);

    Matrix forward(const Matrix& x, bool training = true);
    Matrix backward(const Matrix& d_out);

private:
    float p_;
    Matrix mask_;
};

} // namespace TissNum
