#pragma once

#include "multiheadattention.h"
#include "feedforward.h"
#include "layernorm.h"
#include "dropout.h"
#include <optional>

namespace TissNum {

class TransformerBlock {
public:
    TransformerBlock(size_t d_model, size_t num_heads, size_t d_ff, float dropout_p, int lora_rank = 0, const std::string& name = "");

    Matrix forward(const Matrix& x, const Matrix& mask = Matrix(), std::optional<std::pair<Matrix, Matrix>> past_kv = std::nullopt, std::optional<std::pair<Matrix, Matrix>>* new_kv_cache = nullptr, bool training = false);
    Matrix backward(const Matrix& d_out);

    std::vector<Parameter*> parameters();

private:
    MultiHeadAttention mha_;
    FeedForward ffn_;
    LayerNorm ln1_;
    LayerNorm ln2_;
    Dropout dropout1_;
    Dropout dropout2_;

    // Cached matrices for backward pass (only used when training)
    Matrix cached_x_;
    Matrix cached_x_norm1_;
};

} // namespace TissNum
