#include "transformerblock.h"

TransformerBlock::TransformerBlock(int d_model, int num_heads, int d_ff)
    : mha(d_model, num_heads), ffn(d_model, d_ff), ln1(d_model), ln2(d_model) {}

Matrix TransformerBlock::forward(const Matrix& x, const Matrix* mask) {
    Matrix attn_out = mha.forward(x, mask);
    Matrix x_attn = x + attn_out;
    Matrix x_norm1 = ln1.forward(x_attn);
    Matrix ffn_out = ffn.forward(x_norm1);
    Matrix x_ffn = x_norm1 + ffn_out;
    return ln2.forward(x_ffn);
}

Matrix TransformerBlock::backward(const Matrix& d_out, const Matrix& cache_x) {
    // Note: Accurate backward requires caching intermediate activations.
    // For this implementation, we re-run segments or approximate where state is missing.
    Matrix attn_out = mha.forward(cache_x);
    Matrix x_attn = cache_x + attn_out;
    Matrix x_norm1 = ln1.forward(x_attn);
    Matrix ffn_out = ffn.forward(x_norm1);
    Matrix x_ffn = x_norm1 + ffn_out;

    Matrix d_ln2 = ln2.backward(d_out, x_ffn);
    Matrix d_ffn = ffn.backward(d_ln2, x_norm1);
    Matrix d_res1 = d_ln2 + d_ffn;
    Matrix d_ln1 = ln1.backward(d_res1, x_attn);
    Matrix d_mha = mha.backward(d_ln1, cache_x);
    return d_ln1 + d_mha;
}
