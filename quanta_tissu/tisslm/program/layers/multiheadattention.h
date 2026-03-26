#ifndef MULTIHEADATTENTION_H
#define MULTIHEADATTENTION_H

#include "matrix.h"

class MultiHeadAttention {
public:
    MultiHeadAttention(int d_model, int num_heads);

    Matrix forward(const Matrix& x, const Matrix* mask = nullptr);
    Matrix backward(const Matrix& d_out, const Matrix& x);
    size_t get_parameter_count() const { return wq.size() + wk.size() + wv.size() + wo.size(); }

    std::vector<Matrix*> get_parameters() { return {&wq, &wk, &wv, &wo}; }
    std::vector<Matrix*> get_gradients() { return {&dwq, &dwk, &dwv, &dwo}; }

private:
    int d_model;
    int num_heads;
    int d_k;

    Matrix wq;
    Matrix wk;
    Matrix wv;
    Matrix wo;
    Matrix dwq, dwk, dwv, dwo;

    Matrix split_heads(const Matrix& x);
    Matrix combine_heads(const Matrix& x);
    Matrix scaled_dot_product_attention(const Matrix& q, const Matrix& k, const Matrix& v, const Matrix* mask);
};

#endif // MULTIHEADATTENTION_H