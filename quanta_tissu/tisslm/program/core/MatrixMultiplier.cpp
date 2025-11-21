#include "MatrixMultiplier.h"

namespace TissNum {

Matrix MatrixMultiplier::matmul(const Matrix& a, const Matrix& b) {
    if (a.get_shape().size() == 2 && b.get_shape().size() == 2) {
        if (a.get_shape()[1] != b.get_shape()[0]) {
            throw std::invalid_argument("Matrix dimensions are not compatible for 2D multiplication.");
        }
        Matrix result({a.get_shape()[0], b.get_shape()[1]});
        for (size_t i = 0; i < a.get_shape()[0]; ++i) {
            for (size_t j = 0; j < b.get_shape()[1]; ++j) {
                float sum = 0;
                for (size_t k = 0; k < a.get_shape()[1]; ++k) {
                    sum += a({i, k}) * b({k, j});
                }
                result({i, j}) = sum;
            }
        }
        return result;
    } else if (a.get_shape().size() == 3 && b.get_shape().size() == 2) {
        if (a.get_shape()[2] != b.get_shape()[0]) {
            throw std::invalid_argument("Matrix dimensions are not compatible for 3D x 2D multiplication.");
        }
        Matrix result({a.get_shape()[0], a.get_shape()[1], b.get_shape()[1]});
        for (size_t i = 0; i < a.get_shape()[0]; ++i) {
            for (size_t j = 0; j < a.get_shape()[1]; ++j) {
                for (size_t k = 0; k < b.get_shape()[1]; ++k) {
                    float sum = 0;
                    for (size_t l = 0; l < a.get_shape()[2]; ++l) {
                        sum += a({i, j, l}) * b({l, k});
                    }
                    result({i, j, k}) = sum;
                }
            }
        }
        return result;
    } else {
        throw std::invalid_argument("Unsupported matrix multiplication dimensions.");
    }
}

Matrix MatrixMultiplier::batch_matmul(const Matrix& a, const Matrix& b) {
    if (a.get_shape().size() != 3 || b.get_shape().size() != 3) {
        throw std::invalid_argument("Batch matrix multiplication is only supported for 3D matrices.");
    }
    if (a.get_shape()[0] != b.get_shape()[0]) {
        throw std::invalid_argument("Batch sizes must be equal for batch matrix multiplication.");
    }
    if (a.get_shape()[2] != b.get_shape()[1]) {
        throw std::invalid_argument("Matrix dimensions are not compatible for batch multiplication.");
    }

    Matrix result({a.get_shape()[0], a.get_shape()[1], b.get_shape()[2]});
    for (size_t i = 0; i < a.get_shape()[0]; ++i) {
        for (size_t j = 0; j < a.get_shape()[1]; ++j) {
            for (size_t k = 0; k < b.get_shape()[2]; ++k) {
                float sum = 0;
                for (size_t l = 0; l < a.get_shape()[2]; ++l) {
                    sum += a({i, j, l}) * b({i, l, k});
                }
                result({i, j, k}) = sum;
            }
        }
    }
    return result;
}

} // namespace TissNum
