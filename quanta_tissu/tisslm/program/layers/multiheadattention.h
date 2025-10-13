#ifndef MULTIHEADATTENTION_H
#define MULTIHEADATTENTION_H

#include "matrix.h"

class MultiHeadAttention {
public:
    MultiHeadAttention(int d_model, int num_heads);

    Matrix forward(const Matrix& x, const Matrix* mask = nullptr);

private:
    int d_model;
    int num_heads;
    int d_k;

    Matrix wq;
    Matrix wk;
    Matrix wv;
    Matrix wo;

    Matrix split_heads(const Matrix& x);
    Matrix combine_heads(const Matrix& x);
    Matrix scaled_dot_product_attention(const Matrix& q, const Matrix& k, const Matrix& v, const Matrix* mask);
};

#endif // MULTIHEADATTENTION_H