#ifndef FEEDFORWARD_H
#define FEEDFORWARD_H

#include "matrix.h"

class FeedForward {
public:
    FeedForward(int d_model, int d_ff);

    Matrix forward(const Matrix& x);

private:
    Matrix w1;
    Matrix b1;
    Matrix w2;
    Matrix b2;
};

#endif // FEEDFORWARD_H