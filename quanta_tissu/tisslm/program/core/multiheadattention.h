#pragma once

#include "matrix.h"
#include "parameter.h"
#include <optional>

namespace TissNum {

class MultiHeadAttention {
public:
    MultiHeadAttention(size_t d_model, size_t num_heads, int lora_rank = 0, const std::string& name = "");

    Matrix forward(const Matrix& q, const Matrix& k, const Matrix& v, const Matrix& mask = Matrix(), std::optional<std::pair<Matrix, Matrix>> past_kv = std::nullopt, std::optional<std::pair<Matrix, Matrix>>* new_kv_cache = nullptr);
    Matrix backward(const Matrix& d_out);

    std::vector<Parameter*> parameters();

private:
    Matrix scaled_dot_product_attention(const Matrix& q, const Matrix& k, const Matrix& v, const Matrix& mask);
    Matrix split_heads(const Matrix& x);
    Matrix merge_heads(const Matrix& x);

    size_t d_model_;
    size_t num_heads_;
    size_t head_dim_;
    int lora_rank_;
    bool use_lora_;

    Parameter w_q_;
    Parameter w_k_;
    Parameter w_v_;
    Parameter w_o_;

    std::optional<Parameter> w_q_lora_a_;
    std::optional<Parameter> w_q_lora_b_;
    std::optional<Parameter> w_v_lora_a_;
    std::optional<Parameter> w_v_lora_b_;

    // Cached matrices for backward pass
    Matrix cached_q_;
    Matrix cached_k_;
    Matrix cached_v_;
    Matrix cached_attn_weights_;
    Matrix cached_scaled_attention_;
    Matrix cached_output_projection_input_;
};

} // namespace TissNum
