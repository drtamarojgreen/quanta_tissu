#ifndef TISSNUM_MATRIX_MULTIPLIER_H
#define TISSNUM_MATRIX_MULTIPLIER_H

#include "matrix.h"

namespace TissNum {

class MatrixMultiplier {
public:
    static Matrix matmul(const Matrix& a, const Matrix& b);
    static Matrix batch_matmul(const Matrix& a, const Matrix& b);
};

} // namespace TissNum

#endif // TISSNUM_MATRIX_MULTIPLIER_H
