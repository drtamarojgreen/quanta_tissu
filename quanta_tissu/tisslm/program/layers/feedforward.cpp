#include "feedforward.h"
#include <algorithm>
#include <cmath>

FeedForward::FeedForward(int d_model, int d_ff)
    : w1(Matrix::random(d_model, d_ff)),
      b1(Matrix::zeros(1, d_ff)),
      w2(Matrix::random(d_ff, d_model)),
      b2(Matrix::zeros(1, d_model)) {}

Matrix FeedForward::forward(const Matrix& x) {
    // First linear layer
    Matrix z = x.dot(w1);
    for (int i = 0; i < z.get_rows(); ++i) {
        for (int j = 0; j < z.get_cols(); ++j) {
            z.at(i, j) += b1.at(0, j);
        }
    }

    // ReLU activation
    Matrix h(z.get_rows(), z.get_cols());
    for (int i = 0; i < z.get_rows(); ++i) {
        for (int j = 0; j < z.get_cols(); ++j) {
            h.at(i, j) = std::max(0.0f, z.at(i, j));
        }
    }

    // Second linear layer
    Matrix y = h.dot(w2);
    for (int i = 0; i < y.get_rows(); ++i) {
        for (int j = 0; j < y.get_cols(); ++j) {
            y.at(i, j) += b2.at(0, j);
        }
    }

    return y;
}