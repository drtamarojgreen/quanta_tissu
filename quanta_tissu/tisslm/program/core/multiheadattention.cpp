#include "multiheadattention.h"
#include <cmath>
#include <numeric>
#include <optional>

namespace TissNum {

// Placeholder for Softmax activation
Matrix softmax(const Matrix& x, int axis = -1) {
    Matrix max_val = x.max(axis);
    Matrix exp_x = Matrix::exp(x - max_val);
    Matrix sum_exp_x = exp_x.sum(axis);
    return exp_x / sum_exp_x;
}

Matrix softmax_backward(const Matrix& d_out, const Matrix& softmax_output) {
    Matrix d_input({d_out.rows(), d_out.cols()});
    for (size_t i = 0; i < d_out.rows(); ++i) {
        for (size_t j = 0; j < d_out.cols(); ++j) {
            float s_ij = softmax_output({i, j});
            float d_s_ij = 0;
            for (size_t k = 0; k < d_out.cols(); ++k) {
                float s_ik = softmax_output({i, k});
                if (j == k) {
                    d_s_ij += d_out({i, k}) * s_ik * (1 - s_ik);
                } else {
                    d_s_ij -= d_out({i, k}) * s_ik * s_ij;
                }
            }
            d_input({i, j}) = d_s_ij;
        }
    }
    return d_input;
}

Matrix MultiHeadAttention::split_heads(const Matrix& x) {
    size_t batch_size = x.rows();
    size_t seq_len = x.cols() / d_model_;
    return x.reshape({batch_size, seq_len, num_heads_, head_dim_}).transpose(1, 2);
}

Matrix MultiHeadAttention::merge_heads(const Matrix& x) {
    size_t batch_size = x.get_shape()[0];
    size_t seq_len = x.get_shape()[2];
    return x.transpose(1, 2).reshape({batch_size, seq_len, d_model_});
}

Matrix MultiHeadAttention::forward(const Matrix& q_in, const Matrix& k_in, const Matrix& v_in, const Matrix& mask, std::optional<std::pair<Matrix, Matrix>> past_kv, std::optional<std::pair<Matrix, Matrix>>* new_kv_cache) {
    cached_q_ = q_in;

    // Project the new query, key, and value vectors from the input.
    Matrix q_proj = Matrix::matmul(q_in, w_q_.value());
    Matrix k_proj = Matrix::matmul(k_in, w_k_.value());
    Matrix v_proj = Matrix::matmul(v_in, w_v_.value());

    // Apply LoRA adjustments if enabled.
    if (use_lora_) {
        q_proj = q_proj + Matrix::matmul(Matrix::matmul(q_in, w_q_lora_a_.value().value()), w_q_lora_b_.value().value());
        v_proj = v_proj + Matrix::matmul(Matrix::matmul(v_in, w_v_lora_a_.value().value()), w_v_lora_b_.value().value());
    }

    // Split heads
    Matrix q = split_heads(q_proj);
    Matrix k_new = split_heads(k_proj);
    Matrix v_new = split_heads(v_proj);

    Matrix k, v;
    if (past_kv.has_value()) {
        // If a cache is provided, concatenate the new K/V vectors with the cached ones.
        k = Matrix::concatenate(past_kv->first, k_new, 2); // Concatenate along seq_len dimension
        v = Matrix::concatenate(past_kv->second, v_new, 2);
    } else {
        // Otherwise, use the new K/V vectors as is.
        k = k_new;
        v = v_new;
    }

    // Update the cache with the new, full K/V sequences for the next step.
    if (new_kv_cache != nullptr) {
        *new_kv_cache = std::make_pair(k, v);
    }

    // Cache the K and V matrices for the backward pass.
    cached_k_ = k;
    cached_v_ = v;

    // Perform scaled dot-product attention.
    Matrix scaled_attention_output = scaled_dot_product_attention(q, k, v, mask);
    cached_scaled_attention_ = scaled_attention_output;

    // Merge heads
    Matrix merged_output = merge_heads(scaled_attention_output);

    // Apply the final output projection.
    Matrix output = Matrix::matmul(merged_output, w_o_.value());
    cached_output_projection_input_ = merged_output;

    return output;
}

Matrix MultiHeadAttention::backward(const Matrix& d_out) {
    // Backpropagate through the output projection.
    w_o_.grad() = Matrix::matmul(cached_output_projection_input_.transpose(), d_out);
    Matrix d_merged_output = Matrix::matmul(d_out, w_o_.value().transpose());

    // Backpropagate through merge_heads.
    Matrix d_scaled_attention_output = merge_heads(d_merged_output); // merge_heads is its own inverse in terms of shape

    // Backpropagate through the scaled dot-product attention.
    Matrix d_attention_weights = Matrix::matmul(d_scaled_attention_output, cached_v_.transpose());
    Matrix d_v_attention = Matrix::matmul(cached_attn_weights_.transpose(), d_scaled_attention_output);

    Matrix d_scores = softmax_backward(d_attention_weights, cached_attn_weights_);

    float scale = std::sqrt(static_cast<float>(head_dim_));
    Matrix d_scores_scaled = d_scores / scale;

    Matrix d_q_attention = Matrix::matmul(d_scores_scaled, cached_k_);
    Matrix d_k_attention = Matrix::matmul(d_scores_scaled.transpose(), cached_q_);

    // Backpropagate through split_heads.
    Matrix d_q_proj = split_heads(d_q_attention); // split_heads is its own inverse in terms of shape
    Matrix d_k_proj = split_heads(d_k_attention);
    Matrix d_v_proj = split_heads(d_v_attention);

    // Backpropagate through the input projections.
    w_q_.grad() = Matrix::matmul(cached_q_.transpose(), d_q_proj);
    w_k_.grad() = Matrix::matmul(cached_k_.transpose(), d_k_proj);
    w_v_.grad() = Matrix::matmul(cached_v_.transpose(), d_v_proj);

    Matrix dx = Matrix::matmul(d_q_proj, w_q_.value().transpose());

    if (use_lora_) {
        // Backpropagate through LoRA layers
        Matrix d_lora_q_b = Matrix::matmul(cached_q_.transpose(), d_q_proj);
        Matrix d_lora_q_a = Matrix::matmul(w_q_lora_b_.value().value().transpose(), d_lora_q_b);
        w_q_lora_a_->grad() = d_lora_q_a;
        w_q_lora_b_->grad() = d_lora_q_b;

        Matrix d_lora_v_b = Matrix::matmul(cached_v_.transpose(), d_v_proj);
        Matrix d_lora_v_a = Matrix::matmul(w_v_lora_b_.value().value().transpose(), d_lora_v_b);
        w_v_lora_a_->grad() = d_lora_v_a;
        w_v_lora_b_->grad() = d_lora_v_b;
    }

    return dx;
}

std::vector<Parameter*> MultiHeadAttention::parameters() {
    std::vector<Parameter*> params = {&w_q_, &w_k_, &w_v_, &w_o_};
    if (use_lora_) {
        if (w_q_lora_a_.has_value()) params.push_back(&w_q_lora_a_.value());
        if (w_q_lora_b_.has_value()) params.push_back(&w_q_lora_b_.value());
        if (w_v_lora_a_.has_value()) params.push_back(&w_v_lora_a_.value());
        if (w_v_lora_b_.has_value()) params.push_back(&w_v_lora_b_.value());
    }
    return params;
}

} // namespace TissNum
