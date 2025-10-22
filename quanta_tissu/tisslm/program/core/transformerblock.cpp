#include "transformerblock.h"

namespace TissNum {

TransformerBlock::TransformerBlock(size_t d_model, size_t num_heads, size_t d_ff, float dropout_p, int lora_rank, const std::string& name)
    : mha_(d_model, num_heads, lora_rank, name + ".mha"),
      ffn_(d_model, d_ff, name + ".ffn"),
      ln1_(d_model, name + ".ln1"),
      ln2_(d_model, name + ".ln2"),
      dropout1_(dropout_p),
      dropout2_(dropout_p) {}

Matrix TransformerBlock::forward(const Matrix& x, const Matrix& mask, std::optional<std::pair<Matrix, Matrix>> past_kv, std::optional<std::pair<Matrix, Matrix>>* new_kv_cache) {
    cached_x_for_ln1_ = x; // Cache input for residual connection

    // Self-attention part
    auto [x_norm1, ln1_cache] = ln1_.forward(x);
    Matrix attn_out = mha_.forward(x_norm1, x_norm1, x_norm1, mask, past_kv, new_kv_cache);
    attn_out = dropout1_.forward(attn_out);
    Matrix x_plus_attn = x + attn_out;

    cached_x_for_ffn_ = x_plus_attn; // Cache for residual connection

    // Feed-forward part
    auto [x_norm2, ln2_cache] = ln2_.forward(x_plus_attn);
    Matrix ffn_out = ffn_.forward(x_norm2);
    ffn_out = dropout2_.forward(ffn_out);
    Matrix x_plus_ffn = x_plus_attn + ffn_out;

    return x_plus_ffn;
}

Matrix TransformerBlock::backward(const Matrix& d_out, const Matrix& cache) {
    // d_out is the gradient from the next layer or the final output

    // Backprop through the second residual connection
    Matrix d_x_plus_attn = d_out; // Gradient flowing back to the input of the second LN
    Matrix d_ffn_out = d_out;     // Gradient flowing to the output of the FFN

    // Backprop through dropout2
    d_ffn_out = dropout2_.backward(d_ffn_out);

    // Backprop through ffn_
    Matrix dx_norm2 = ffn_.backward(d_ffn_out, cached_x_for_ffn_);

    // Backprop through ln2_
    Matrix d_x_plus_attn_from_ln = ln2_.backward(dx_norm2, cached_x_for_ffn_);
    d_x_plus_attn = d_x_plus_attn + d_x_plus_attn_from_ln;

    // Backprop through the first residual connection
    Matrix dx = d_x_plus_attn;      // Gradient flowing back to the original input x
    Matrix d_attn_out = d_x_plus_attn; // Gradient flowing to the output of MHA

    // Backprop through dropout1
    d_attn_out = dropout1_.backward(d_attn_out);

    // Backprop through ln1_
    // Note: The MHA takes the output of ln1. Its gradient needs to be computed.
    // This part is complex. For now, assume a simplified gradient path.
    // A full implementation would require caching the output of ln1
    // and backpropagating through MHA.
    // Let's assume dx_norm1 is the gradient w.r.t. the output of ln1.
    // This is a placeholder and likely incorrect.
    Matrix dx_norm1 = mha_.backward(d_attn_out, cache);

    // Backpropagate through ln1
    Matrix dx_from_ln1 = ln1_.backward(dx_norm1, cached_x_for_ln1_);
    dx = dx + dx_from_ln1;

    return dx;
}

std::vector<Parameter*> TransformerBlock::parameters() {
    std::vector<Parameter*> params;
    auto mha_params = mha_.parameters();
    params.insert(params.end(), mha_params.begin(), mha_params.end());
    auto ffn_params = ffn_.parameters();
    params.insert(params.end(), ffn_params.begin(), ffn_params.end());
    auto ln1_params = ln1_.parameters();
    params.insert(params.end(), ln1_params.begin(), ln1_params.end());
    auto ln2_params = ln2_.parameters();
    params.insert(params.end(), ln2_params.begin(), ln2_params.end());
    return params;
}

} // namespace TissNum
