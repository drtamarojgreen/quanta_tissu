#include "configurableattention.h"
#include <cmath>
#include <stdexcept>

ConfigurableAttention::ConfigurableAttention(int d_model, int num_heads, AttentionMode mode)
    : d_model(d_model), num_heads(num_heads), d_k(d_model / num_heads), mode_(mode) {

    if (d_model % num_heads != 0) {
        throw std::invalid_argument("d_model must be divisible by num_heads");
    }

    wq = Matrix::random({d_model, d_model});
    wo = Matrix::random({d_model, d_model});

    if (mode_ == AttentionMode::MULTI_QUERY) {
        wk = Matrix::random({d_model, d_k});
        wv = Matrix::random({d_model, d_k});
    } else {
        wk = Matrix::random({d_model, d_model});
        wv = Matrix::random({d_model, d_model});
    }

    if (mode_ == AttentionMode::MULTI_HEAD_LATENT) {
        int latent_dim = 128;
        latent_k = Matrix::random({latent_dim, d_k});
        latent_v = Matrix::random({latent_dim, d_k});
    }
}

Matrix ConfigurableAttention::scaled_dot_product_attention(const Matrix& q, const Matrix& k, const Matrix& v, const Matrix* mask) {
    Matrix scores = Matrix::matmul(q, k.transpose(1, 2));
    scores = scores / std::sqrt(static_cast<float>(d_k));
    if (mask) {
        scores = scores + *mask;
    }
    Matrix weights = Matrix::softmax(scores);
    return Matrix::matmul(weights, v);
}

Matrix ConfigurableAttention::split_heads(const Matrix& x) {
    std::vector<int> shape = x.shape();
    int batch_size = shape[0];
    int seq_len = shape[1];

    Matrix reshaped = x;
    reshaped.reshape({batch_size, seq_len, num_heads, d_k});
    return reshaped.transpose(1, 2);
}

Matrix ConfigurableAttention::combine_heads(const Matrix& x) {
    std::vector<int> shape = x.shape();

    Matrix transposed = x.transpose(1, 2);
    std::vector<int> original_shape = transposed.shape();
    transposed.reshape({original_shape[0], original_shape[1], d_model});
    return transposed;
}


Matrix ConfigurableAttention::forward(const Matrix& x, const Matrix* mask) {
    Matrix q_proj = Matrix::matmul(x, wq);
    Matrix k_proj = Matrix::matmul(x, wk);
    Matrix v_proj = Matrix::matmul(x, wv);

    Matrix q_split = split_heads(q_proj);
    Matrix k_final, v_final;

    switch (mode_) {
        case AttentionMode::STANDARD_MULTI_HEAD:
            k_final = split_heads(k_proj);
            v_final = split_heads(v_proj);
            break;
        case AttentionMode::MULTI_QUERY:
            k_final = k_proj; // No head splitting
            v_final = v_proj;
            break;
        case AttentionMode::MULTI_HEAD_LATENT:
            k_final = latent_k;
            v_final = latent_v;
            break;
    }

    Matrix attention_output = scaled_dot_product_attention(q_split, k_final, v_final, mask);
    Matrix combined = combine_heads(attention_output);

    return Matrix::matmul(combined, wo);
}