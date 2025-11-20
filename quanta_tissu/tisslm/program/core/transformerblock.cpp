#include "transformerblock.h"

namespace TissNum {

TransformerBlock::TransformerBlock(size_t d_model, size_t num_heads, size_t d_ff, float dropout_p, AttentionMode attention_mode, int lora_rank, const std::string& name)
    : attention_(d_model, num_heads, attention_mode, lora_rank, name + ".attention"),
      ffn_(d_model, d_ff, name + ".ffn"),
      ln1_(d_model, name + ".ln1"),
      ln2_(d_model, name + ".ln2"),
      dropout1_(dropout_p),
      dropout2_(dropout_p) {}

Matrix TransformerBlock::forward(const Matrix& x, const Matrix& mask, bool training) {
    cached_x_ = x;

    // First residual connection
    Matrix attn_input = ln1_.forward(x);
    Matrix attn_output = attention_.forward(attn_input, mask);
    Matrix x_plus_attn = x + dropout1_.forward(attn_output, training);

    // Second residual connection
    cached_x_norm1_ = ln2_.forward(x_plus_attn);
    Matrix ffn_output = ffn_.forward(cached_x_norm1_);
    Matrix x_plus_ffn = x_plus_attn + dropout2_.forward(ffn_output, training);

    return x_plus_ffn;
}

Matrix TransformerBlock::backward(const Matrix& d_out) {
    // Backprop through second residual
    Matrix d_x_plus_attn = d_out;
    Matrix d_ffn_out = dropout2_.backward(d_out);

    // Backprop through FFN
    Matrix d_x_norm1 = ffn_.backward(d_ffn_out);
    d_x_plus_attn += ln2_.backward(d_x_norm1);

    // Backprop through first residual
    Matrix d_x = d_x_plus_attn;
    Matrix d_attn_out = dropout1_.backward(d_x_plus_attn);

    // Backprop through MHA
    Matrix d_attn_input = attention_.backward(d_attn_out);
    d_x += ln1_.backward(d_attn_input);

    return d_x;
}

std::vector<Parameter*> TransformerBlock::parameters() {
    std::vector<Parameter*> params;
    auto attn_params = attention_.parameters();
    params.insert(params.end(), attn_params.begin(), attn_params.end());
    auto ffn_params = ffn_.parameters();
    params.insert(params.end(), ffn_params.begin(), ffn_params.end());
    auto ln1_params = ln1_.parameters();
    params.insert(params.end(), ln1_params.begin(), ln1_params.end());
    auto ln2_params = ln2_.parameters();
    params.insert(params.end(), ln2_params.begin(), ln2_params.end());
    return params;
}

} // namespace TissNum