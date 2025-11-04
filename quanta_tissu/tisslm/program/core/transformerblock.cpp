#include "transformerblock.h"

namespace TissNum {

TransformerBlock::TransformerBlock(size_t d_model, size_t num_heads, size_t d_ff, float dropout_p, int lora_rank, const std::string& name)
    : mha_(d_model, num_heads, lora_rank, name + ".mha"),
      ffn_(d_model, d_ff, name + ".ffn"),
      ln1_(d_model, name + ".ln1"),
      ln2_(d_model, name + ".ln2"),
      dropout1_(dropout_p),
      dropout2_(dropout_p) {}

Matrix TransformerBlock::forward(const Matrix& x, const Matrix& mask, std::optional<std::pair<Matrix, Matrix>> past_kv, std::optional<std::pair<Matrix, Matrix>>* new_kv_cache, bool training) {
    Matrix x_norm1;

    // Self-attention part
    Matrix attn_out = mha_.forward(x, x, x, mask, past_kv, new_kv_cache);
    attn_out = dropout1_.forward(attn_out, training);

    Matrix x_plus_attn = x + attn_out;
    x_norm1 = ln1_.forward(x_plus_attn);

    // Feed-forward part
    Matrix ffn_out = ffn_.forward(x_norm1);
    ffn_out = dropout2_.forward(ffn_out, training);

    Matrix x_plus_ffn = x_norm1 + ffn_out;
    Matrix x_norm2 = ln2_.forward(x_plus_ffn);

    if (training) {
        cached_x_ = x;
        cached_x_plus_attn_ = x_plus_attn;
        cached_x_norm1_ = x_norm1;
        cached_x_plus_ffn_ = x_plus_ffn;
        cached_x_norm2_ = x_norm2;
    }

    return x_norm2;
}

Matrix TransformerBlock::backward(const Matrix& d_out) {
    // Backpropagate through the second layer norm
    Matrix dx_plus_ffn = ln2_.backward(d_out);

    // Backpropagate through the residual connection (x_norm1 + ffn_out)
    Matrix d_x_norm1_residual = dx_plus_ffn;
    Matrix d_ffn_out = dx_plus_ffn;

    // Backpropagate through the second dropout layer
    d_ffn_out = dropout2_.backward(d_ffn_out);

    // Backpropagate through the feed-forward network
    Matrix d_x_norm1_ffn = ffn_.backward(d_ffn_out);

    // Combine gradients for x_norm1
    Matrix dx_norm1 = d_x_norm1_residual + d_x_norm1_ffn;

    // Backpropagate through the first layer norm
    Matrix dx_plus_attn = ln1_.backward(dx_norm1);

    // Backpropagate through the residual connection (x + attn_out)
    Matrix d_x_residual = dx_plus_attn;
    Matrix d_attn_out = dx_plus_attn;

    // Backpropagate through the first dropout layer
    d_attn_out = dropout1_.backward(d_attn_out);

    // Backpropagate through the multi-head attention
    Matrix dx_mha = mha_.backward(d_attn_out);

    // Combine gradients for the final output
    return d_x_residual + dx_mha;
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
