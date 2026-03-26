#ifndef FEEDFORWARD_H
#define FEEDFORWARD_H

#include "matrix.h"

class FeedForward {
public:
    FeedForward(int d_model, int d_ff);

    Matrix forward(const Matrix& x);
    Matrix backward(const Matrix& d_out, const Matrix& x);
    size_t get_parameter_count() const { return w1.size() + b1.size() + w2.size() + b2.size(); }

    std::vector<Matrix*> get_parameters() { return {&w1, &b1, &w2, &b2}; }
    std::vector<Matrix*> get_gradients() { return {&dw1, &db1, &dw2, &db2}; }

private:
    Matrix w1;
    Matrix b1;
    Matrix w2;
    Matrix b2;
    Matrix dw1, db1, dw2, db2;
};

#endif // FEEDFORWARD_H