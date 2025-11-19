#ifndef CONFIGURABLEATTENTION_H
#define CONFIGURABLEATTENTION_H

#include "matrix.h"

// Defines the different attention mechanisms the block can use.
enum class AttentionMode {
    STANDARD_MULTI_HEAD,
    MULTI_QUERY,
    MULTI_HEAD_LATENT
};

class ConfigurableAttention {
public:
    ConfigurableAttention(int d_model, int num_heads, AttentionMode mode);

    Matrix forward(const Matrix& x, const Matrix* mask = nullptr);

private:
    int d_model;
    int num_heads;
    int d_k;
    AttentionMode mode_;

    Matrix wq;
    Matrix wk;
    Matrix wv;
    Matrix wo;

    // Latent compressor (placeholder for RNN/Conv)
    Matrix latent_k;
    Matrix latent_v;

    Matrix split_heads(const Matrix& x);
    Matrix combine_heads(const Matrix& x);
    Matrix scaled_dot_product_attention(const Matrix& q, const Matrix& k, const Matrix& v, const Matrix* mask);
};

#endif // CONFIGURABLEATTENTION_H