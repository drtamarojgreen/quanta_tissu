#include "quanta_tissu/tisslm/program/layers/multiheadattention.h"
#include <cmath>
#include <stdexcept>
#include <numeric>
#include <algorithm>

static Matrix softmax(Matrix scores) {
    size_t rows = scores.rows(), cols = scores.cols();
    for (size_t i = 0; i < rows; ++i) {
        float max_v = scores({i, 0});
        for (size_t j = 1; j < cols; ++j) if (scores({i, j}) > max_v) max_v = scores({i, j});
        float sum = 0.0f;
        for (size_t j = 0; j < cols; ++j) {
            scores({i, j}) = std::exp(scores({i, j}) - max_v);
            sum += scores({i, j});
        }
        for (size_t j = 0; j < cols; ++j) scores({i, j}) /= sum;
    }
    return scores;
}

MultiHeadAttention::MultiHeadAttention(int d_model, int num_heads)
    : d_model(d_model), num_heads(num_heads), d_k(d_model / num_heads),
      wq(Matrix::random(d_model, d_model)), wk(Matrix::random(d_model, d_model)),
      wv(Matrix::random(d_model, d_model)), wo(Matrix::random(d_model, d_model)),
      dwq(d_model, d_model), dwk(d_model, d_model), dwv(d_model, d_model), dwo(d_model, d_model) {
    if (d_model % num_heads != 0) throw std::invalid_argument("d_model must be divisible by num_heads");
}

Matrix MultiHeadAttention::scaled_dot_product_attention(const Matrix& q, const Matrix& k, const Matrix& v, const Matrix* mask) {
    Matrix scores = Matrix::matmul(q, k.transpose()) / std::sqrt(static_cast<float>(d_k));
    if (mask) scores = scores + *mask;
    return Matrix::matmul(softmax(scores), v);
}

Matrix MultiHeadAttention::split_heads(const Matrix& x) {
    return x.reshape({x.rows(), (size_t)num_heads, (size_t)d_k}).transpose(0, 1);
}

Matrix MultiHeadAttention::combine_heads(const Matrix& x) {
    Matrix t = x.transpose(0, 1);
    return t.reshape({t.rows(), (size_t)d_model});
}

Matrix MultiHeadAttention::forward(const Matrix& x, const Matrix* mask) {
    Matrix q = split_heads(Matrix::matmul(x, wq));
    Matrix k = split_heads(Matrix::matmul(x, wk));
    Matrix v = split_heads(Matrix::matmul(x, wv));
    size_t seq_len = x.rows();
    Matrix out_heads({(size_t)num_heads, seq_len, (size_t)d_k});
    for (int h = 0; h < num_heads; ++h) {
        Matrix qh(seq_len, d_k), kh(seq_len, d_k), vh(seq_len, d_k);
        for (size_t i = 0; i < seq_len; ++i) for (size_t j = 0; j < (size_t)d_k; ++j) {
            qh({i, j}) = q({(size_t)h, i, j}); kh({i, j}) = k({(size_t)h, i, j}); vh({i, j}) = v({(size_t)h, i, j});
        }
        Matrix attn = scaled_dot_product_attention(qh, kh, vh, mask);
        for (size_t i = 0; i < seq_len; ++i) for (size_t j = 0; j < (size_t)d_k; ++j) out_heads({(size_t)h, i, j}) = attn({i, j});
    }
    return Matrix::matmul(combine_heads(out_heads), wo);
}

Matrix MultiHeadAttention::backward(const Matrix& d_out, const Matrix& x) {
    size_t seq_len = x.rows();
    Matrix q_raw = Matrix::matmul(x, wq), k_raw = Matrix::matmul(x, wk), v_raw = Matrix::matmul(x, wv);
    Matrix q_heads = split_heads(q_raw), k_heads = split_heads(k_raw), v_heads = split_heads(v_raw);

    Matrix d_merged = Matrix::matmul(d_out, wo.transpose());

    // Reconstruct attn_out for Wo grad
    Matrix attn_out_cat(seq_len, d_model);
    for (int h = 0; h < num_heads; ++h) {
        Matrix qh(seq_len, d_k), kh(seq_len, d_k), vh(seq_len, d_k);
        for (size_t i = 0; i < seq_len; ++i) for (size_t j = 0; j < (size_t)d_k; ++j) {
            qh({i, j}) = q_heads({(size_t)h, i, j}); kh({i, j}) = k_heads({(size_t)h, i, j}); vh({i, j}) = v_heads({(size_t)h, i, j});
        }
        Matrix attn = scaled_dot_product_attention(qh, kh, vh, nullptr);
        for (size_t i = 0; i < seq_len; ++i) for (size_t j = 0; j < (size_t)d_k; ++j) attn_out_cat({i, (size_t)h * d_k + j}) = attn({i, j});
    }
    dwo = dwo + Matrix::matmul(attn_out_cat.transpose(), d_out);

    Matrix dx(seq_len, d_model), dq_total(seq_len, d_model), dk_total(seq_len, d_model), dv_total(seq_len, d_model);
    float scale = 1.0f / std::sqrt(static_cast<float>(d_k));

    for (int h = 0; h < num_heads; ++h) {
        Matrix qh(seq_len, d_k), kh(seq_len, d_k), vh(seq_len, d_k), dh(seq_len, d_k);
        for (size_t i = 0; i < seq_len; ++i) for (size_t j = 0; j < (size_t)d_k; ++j) {
            qh({i, j}) = q_heads({(size_t)h, i, j}); kh({i, j}) = k_heads({(size_t)h, i, j});
            vh({i, j}) = v_heads({(size_t)h, i, j}); dh({i, j}) = d_merged({i, (size_t)h * d_k + j});
        }

        Matrix scores = Matrix::matmul(qh, kh.transpose()) * scale;
        Matrix probs = softmax(scores);

        Matrix d_vh = Matrix::matmul(probs.transpose(), dh);
        Matrix d_probs = Matrix::matmul(dh, vh.transpose());

        Matrix d_scores(seq_len, seq_len);
        for (size_t i = 0; i < seq_len; ++i) {
            for (size_t j = 0; j < seq_len; ++j) {
                float sum = 0; for (size_t l = 0; l < seq_len; ++l) sum += d_probs({i, l}) * probs({i, l});
                d_scores({i, j}) = probs({i, j}) * (d_probs({i, j}) - sum);
            }
        }
        d_scores = d_scores * scale;

        Matrix d_qh = Matrix::matmul(d_scores, kh);
        Matrix d_kh = Matrix::matmul(d_scores.transpose(), qh);

        for (size_t i = 0; i < seq_len; ++i) for (size_t j = 0; j < (size_t)d_k; ++j) {
            dq_total({i, (size_t)h * d_k + j}) += d_qh({i, j});
            dk_total({i, (size_t)h * d_k + j}) += d_kh({i, j});
            dv_total({i, (size_t)h * d_k + j}) += d_vh({i, j});
        }
    }

    dwq = dwq + Matrix::matmul(x.transpose(), dq_total);
    dwk = dwk + Matrix::matmul(x.transpose(), dk_total);
    dwv = dwv + Matrix::matmul(x.transpose(), dv_total);

    return Matrix::matmul(dq_total, wq.transpose()) + Matrix::matmul(dk_total, wk.transpose()) + Matrix::matmul(dv_total, wv.transpose());
}
