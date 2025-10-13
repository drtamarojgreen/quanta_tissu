#include "transformerblock.h"

TransformerBlock::TransformerBlock(int d_model, int num_heads, int d_ff)
    : mha(d_model, num_heads),
      ffn(d_model, d_ff),
      ln1(d_model),
      ln2(d_model) {}

Matrix TransformerBlock::forward(const Matrix& x, const Matrix* mask) {
    // Multi-head attention sub-layer
    Matrix attn_output = mha.forward(x, mask);
    // Add & Norm
    Matrix x_plus_attn = x + attn_output;
    Matrix x_norm1 = ln1.forward(x_plus_attn);

    // Feed-forward sub-layer
    Matrix ffn_output = ffn.forward(x_norm1);
    // Add & Norm
    Matrix x_plus_ffn = x_norm1 + ffn_output;
    Matrix x_norm2 = ln2.forward(x_plus_ffn);

    return x_norm2;
}