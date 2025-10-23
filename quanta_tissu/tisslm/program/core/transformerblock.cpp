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
    // Self-attention part
    Matrix attn_out = mha_.forward(x, x, x, mask, past_kv, new_kv_cache);
    attn_out = dropout1_.forward(attn_out);

    Matrix x_plus_attn = x + attn_out;
    Matrix x_norm1 = ln1_.forward(x_plus_attn);

    // Feed-forward part
    Matrix ffn_out = ffn_.forward(x_norm1);
    ffn_out = dropout2_.forward(ffn_out);

    Matrix x_plus_ffn = x_norm1 + ffn_out;
    Matrix x_norm2 = ln2_.forward(x_plus_ffn);

    // Cache for backward pass (simplified for now)
    cached_x_for_ln1_ = x_plus_attn;
    cached_x_for_ffn_ = x_plus_ffn;

    return x_norm2;
}

Matrix TransformerBlock::backward(const Matrix& d_out, const Matrix& cache) {
    // Backpropagate through ln2
    Matrix dx_norm2 = ln2_.backward(d_out, cached_x_for_ffn_);

    // Backpropagate through addition (x_norm1 + ffn_out)
    Matrix d_ffn_out = dx_norm2; // Gradient flowing to the ffn_out branch
    Matrix dx_norm1_from_residual = dx_norm2; // Gradient flowing to the x_norm1 branch (residual connection)

    // Backpropagate through dropout2
    d_ffn_out = dropout2_.backward(d_ffn_out);

    // Backpropagate through ffn to get gradient contribution for x_norm1
    Matrix dx_norm1_from_ffn = ffn_.backward(d_ffn_out, cached_x_for_ln1_);

    // Total gradient for x_norm1 is the sum from both branches
    Matrix total_dx_norm1 = dx_norm1_from_residual + dx_norm1_from_ffn;

    // Backpropagate through ln1
    Matrix dx_plus_attn = ln1_.backward(total_dx_norm1, cached_x_for_ln1_);

    // Backpropagate through addition (x + attn_out)
    Matrix d_attn_out = dx_plus_attn; // Gradient flowing to the attn_out branch
    Matrix dx_from_residual = dx_plus_attn; // Gradient flowing to the x branch (residual connection)

    // Backpropagate through dropout1
    d_attn_out = dropout1_.backward(d_attn_out);

    // Backpropagate through mha to get gradient contribution for x
    Matrix dx_from_mha = mha_.backward(d_attn_out, cache);

    // Total gradient for x is the sum from both branches
    Matrix dx_from_attn = dx_from_residual + dx_from_mha;

    return dx_from_attn;
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
