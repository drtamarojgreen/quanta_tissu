#pragma once

#include "multiheadattention.h"
#include "feedforward.h"
#include "layernorm.h"
#include "dropout.h"
#include <optional>

namespace TissNum {

class TransformerBlock {
public:
    TransformerBlock(size_t d_model, size_t num_heads, size_t d_ff, float dropout_p, int lora_rank = 0, const std::string& name = "", AttentionMode attention_mode = AttentionMode::STANDARD);

    Matrix forward(const Matrix& x, const Matrix& mask = Matrix(), std::optional<std::pair<Matrix, Matrix>> past_kv = std::nullopt, std::optional<std::pair<Matrix, Matrix>>* new_kv_cache = nullptr, bool training = false);
    Matrix backward(const Matrix& d_out);

    std::vector<Parameter*> parameters();

    // Getters for intermediate values (for testing/debugging)
    const Matrix& get_attn_out() const { return attn_out_; }
    const Matrix& get_x_plus_attn() const { return x_plus_attn_; }
    const Matrix& get_x_norm1() const { return x_norm1_; }
    const Matrix& get_ffn_out() const { return ffn_out_; }
    const Matrix& get_x_plus_ffn() const { return x_plus_ffn_; }
    const Matrix& get_x_norm2() const { return x_norm2_; }

private:
    MultiHeadAttention mha_;
    FeedForward ffn_;
    LayerNorm ln1_;
    LayerNorm ln2_;
    Dropout dropout1_;
    Dropout dropout2_;

    // Intermediate values (for testing/debugging)
    Matrix attn_out_;
    Matrix x_plus_attn_;
    Matrix x_norm1_;
    Matrix ffn_out_;
    Matrix x_plus_ffn_;
    Matrix x_norm2_;

    // Cached matrices for backward pass (only used when training)
    Matrix cached_x_;
    Matrix cached_x_plus_attn_;
    Matrix cached_x_norm1_;
    Matrix cached_x_plus_ffn_;
    Matrix cached_x_norm2_;
};

} // namespace TissNum