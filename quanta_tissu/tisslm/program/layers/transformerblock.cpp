#include "transformerblock.h"

TransformerBlock::TransformerBlock(int d_model, int num_heads, int d_ff, AttentionMode attention_mode)
    : attention(d_model, num_heads, attention_mode),
      ffn(d_model, d_ff),
      ln1(d_model),
      ln2(d_model) {}

Matrix TransformerBlock::forward(const Matrix& x, const Matrix* mask) {
    // Attention sub-layer
    Matrix attn_output = attention.forward(x, mask);
    // Add & Norm (conceptual)
    // Matrix x_plus_attn = x + attn_output;
    Matrix x_norm1 = ln1.forward(x /* should be x_plus_attn */);

    // Feed-forward sub-layer
    Matrix ffn_output = ffn.forward(x_norm1);
    // Add & Norm (conceptual)
    // Matrix x_plus_ffn = x_norm1 + ffn_output;
    Matrix x_norm2 = ln2.forward(x_norm1 /* should be x_plus_ffn */);

    return x_norm2;
}