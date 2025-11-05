#include "feedforward.h"
#include <algorithm>
#include <cmath>

FeedForward::FeedForward(int d_model, int d_ff)
    : w1(Matrix::random({(size_t)d_model, (size_t)d_ff})),
      b1(Matrix::zeros({1, (size_t)d_ff})),
      w2(Matrix::random({(size_t)d_ff, (size_t)d_model})),
      b2(Matrix::zeros({1, (size_t)d_model})) {}

Matrix FeedForward::forward(const Matrix& x) {
    // First linear layer
    Matrix z = Matrix::matmul(x, w1);
    for (size_t i = 0; i < z.rows(); ++i) {
        for (size_t j = 0; j < z.cols(); ++j) {
            z({i, j}) += b1({0, j});
        }
    }

    // ReLU activation
    Matrix h({z.rows(), z.cols()});
    for (size_t i = 0; i < z.rows(); ++i) {
        for (size_t j = 0; j < z.cols(); ++j) {
            h({i, j}) = std::max(0.0f, z({i, j}));
        }
    }

    // Second linear layer
    Matrix y = Matrix::matmul(h, w2);
    for (size_t i = 0; i < y.rows(); ++i) {
        for (size_t j = 0; j < y.cols(); ++j) {
            y({i, j}) += b2({0, j});
        }
    }

    return y;
}