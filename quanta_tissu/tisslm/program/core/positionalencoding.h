#pragma once

#include "matrix.h"

namespace TissNum {

class PositionalEncoding {
public:
    PositionalEncoding(size_t d_model, size_t max_len = 5000);

    Matrix forward(const Matrix& x, size_t start_pos = 0);

private:
    size_t d_model_;
    size_t max_len_;
    Matrix pe_;
};

} // namespace TissNum
