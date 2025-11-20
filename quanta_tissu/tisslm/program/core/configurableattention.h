#pragma once

#include "matrix.h"
#include "parameter.h"
#include <optional>
#include <string>

namespace TissNum {

// Defines the different attention mechanisms the block can use.
enum class AttentionMode {
    STANDARD_MULTI_HEAD,
    MULTI_QUERY,
    MULTI_HEAD_LATENT
};

class ConfigurableAttention {
public:
    ConfigurableAttention(size_t d_model, size_t num_heads, AttentionMode mode, int lora_rank = 0, const std::string& name = "");

    Matrix forward(const Matrix& x, const Matrix& mask = Matrix()); // Simplified forward
    Matrix backward(const Matrix& d_out); // Kept for compatibility

    std::vector<Parameter*> parameters();

private:
    size_t d_model_;
    size_t num_heads_;
    size_t d_k_;
    AttentionMode mode_;
    int lora_rank_;
    bool use_lora_;

    Parameter w_q_;
    Parameter w_k_;
    Parameter w_v_;
    Parameter w_o_;

    // Placeholder for latent representation
    Matrix latent_k_;
    Matrix latent_v_;

    // LoRA parameters
    std::optional<Parameter> w_q_lora_a_;
    std::optional<Parameter> w_q_lora_b_;
    std::optional<Parameter> w_v_lora_a_;
    std::optional<Parameter> w_v_lora_b_;

    // Cache for backward pass
    Matrix cached_input_;
    Matrix cached_q_proj_;
    Matrix cached_k_proj_;
    Matrix cached_v_proj_;
    Matrix cached_attn_weights_;

    // Helpers
    Matrix scaled_dot_product_attention(const Matrix& q, const Matrix& k, const Matrix& v, const Matrix& mask);
    Matrix split_heads(const Matrix& x);
    Matrix combine_heads(const Matrix& x);
};

} // namespace TissNum