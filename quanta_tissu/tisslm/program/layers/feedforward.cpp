#include "feedforward.h"
#include <algorithm>
#include <cmath>

FeedForward::FeedForward(int d_model, int d_ff)
    : w1(Matrix::random(d_model, d_ff)),
      b1(Matrix::zeros(1, d_ff)),
      w2(Matrix::random(d_ff, d_model)),
      b2(Matrix::zeros(1, d_model)),
      dw1(d_model, d_ff), db1(1, d_ff),
      dw2(d_ff, d_model), db2(1, d_model) {}

Matrix FeedForward::forward(const Matrix& x) {
    // First linear layer
    Matrix z = Matrix::matmul(x, w1);
    for (size_t i = 0; i < z.rows(); ++i) {
        for (size_t j = 0; j < z.cols(); ++j) {
            z({i, j}) += b1({0, j});
        }
    }

    // ReLU activation
    Matrix h(static_cast<int>(z.rows()), static_cast<int>(z.cols()));
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

Matrix FeedForward::backward(const Matrix& d_out, const Matrix& x) {
    // Re-run first layer for activations
    Matrix z = Matrix::matmul(x, w1);
    for (size_t i = 0; i < z.rows(); ++i) for (size_t j = 0; j < z.cols(); ++j) z({i, j}) += b1({0, j});
    Matrix h(z.rows(), z.cols());
    for (size_t i = 0; i < z.rows(); ++i) for (size_t j = 0; j < z.cols(); ++j) h({i, j}) = std::max(0.0f, z({i, j}));

    // Backprop through second linear layer
    dw2 = dw2 + Matrix::matmul(h.transpose(), d_out);
    for (size_t i = 0; i < d_out.rows(); ++i) for (size_t j = 0; j < d_out.cols(); ++j) db2({0, j}) += d_out({i, j});
    Matrix dh = Matrix::matmul(d_out, w2.transpose());

    // Backprop through ReLU
    Matrix dz(z.rows(), z.cols());
    for (size_t i = 0; i < z.rows(); ++i) for (size_t j = 0; j < z.cols(); ++j) dz({i, j}) = (z({i, j}) > 0) ? dh({i, j}) : 0.0f;

    // Backprop through first linear layer
    dw1 = dw1 + Matrix::matmul(x.transpose(), dz);
    for (size_t i = 0; i < dz.rows(); ++i) for (size_t j = 0; j < dz.cols(); ++j) db1({0, j}) += dz({i, j});
    return Matrix::matmul(dz, w1.transpose());
}