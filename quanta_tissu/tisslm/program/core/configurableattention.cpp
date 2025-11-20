#include "multiheadattention.h"
#include <cmath>
#include <numeric>
#include <optional>

namespace TissNum {

ConfigurableAttention::ConfigurableAttention(size_t d_model, size_t num_heads, AttentionMode mode, int lora_rank, const std::string& name)
    : d_model_(d_model),
      num_heads_(num_heads),
      head_dim_(d_model / num_heads),
      lora_rank_(lora_rank),
      use_lora_(lora_rank > 0),
      mode_(mode),
      w_q_(Parameter(Matrix::random({d_model, d_model}), name + ".w_q")),
      w_k_(Parameter(Matrix::random({d_model, d_model}), name + ".w_k")),
      w_v_(Parameter(Matrix::random({d_model, d_model}), name + ".w_v")),
      w_o_(Parameter(Matrix::random({d_model, d_model}), name + ".w_o")) {
    if (d_model % num_heads != 0) {
        throw std::invalid_argument("d_model must be divisible by num_heads");
    }
    if (use_lora_) {
        w_q_lora_a_.emplace(Parameter(Matrix::random({d_model, (size_t)lora_rank}), name + ".w_q_lora_a"));
        w_q_lora_b_.emplace(Parameter(Matrix::zeros({(size_t)lora_rank, d_model}), name + ".w_q_lora_b"));
        w_v_lora_a_.emplace(Parameter(Matrix::random({d_model, (size_t)lora_rank}), name + ".w_v_lora_a"));
        w_v_lora_b_.emplace(Parameter(Matrix::zeros({(size_t)lora_rank, d_model}), name + ".w_v_lora_b"));
    }
}

// Placeholder for Softmax activation
Matrix softmax(const Matrix& x, int axis = -1) {
    Matrix max_val = x.max(axis);
    Matrix exp_x = Matrix::exp(x - max_val);
    Matrix sum_exp_x = exp_x.sum(axis);
    return exp_x / sum_exp_x;
}

Matrix softmax_backward(const Matrix& d_out, const Matrix& softmax_output) {
    Matrix d_input(d_out.get_shape());

    if (d_out.get_shape().size() == 2) {
        size_t rows = d_out.rows();
        size_t cols = d_out.cols();
        for (size_t i = 0; i < rows; ++i) {
            for (size_t j = 0; j < cols; ++j) {
                float s_ij = softmax_output({i, j});
                float d_s_ij = 0;
                for (size_t k = 0; k < cols; ++k) {
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
    } else if (d_out.get_shape().size() == 4) {
        size_t batch_size = d_out.get_shape()[0];
        size_t num_heads = d_out.get_shape()[1];
        size_t seq_len_q = d_out.get_shape()[2];
        size_t seq_len_k = d_out.get_shape()[3];

        for (size_t b = 0; b < batch_size; ++b) {
            for (size_t h = 0; h < num_heads; ++h) {
                for (size_t i = 0; i < seq_len_q; ++i) {
                    for (size_t j = 0; j < seq_len_k; ++j) {
                        float s_ij = softmax_output({b, h, i, j});
                        float d_s_ij = 0;
                        for (size_t k = 0; k < seq_len_k; ++k) {
                            float s_ik = softmax_output({b, h, i, k});
                            if (j == k) {
                                d_s_ij += d_out({b, h, i, k}) * s_ik * (1 - s_ik);
                            } else {
                                d_s_ij -= d_out({b, h, i, k}) * s_ik * s_ij;
                            }
                        }
                        d_input({b, h, i, j}) = d_s_ij;
                    }
                }
            }
        }
    } else {
        throw std::invalid_argument("softmax_backward not implemented for this shape.");
    }
    return d_input;
}

Matrix MultiHeadAttention::split_heads(const Matrix& x) {
    size_t batch_size = x.get_shape()[0];
    size_t seq_len = x.get_shape()[1];
    Matrix reshaped = x.reshape({batch_size, seq_len, num_heads_, head_dim_});
    Matrix transposed = reshaped.transpose(1, 2);
    return transposed;
}

Matrix MultiHeadAttention::merge_heads(const Matrix& x) {
    size_t batch_size = x.get_shape()[0];
    size_t seq_len = x.get_shape()[2];

    Matrix transposed = x.transpose(1, 2);
    Matrix reshaped = transposed.reshape({batch_size, seq_len, d_model_});
    return reshaped;
}

Matrix ConfigurableAttention::forward(const Matrix& q_in, const Matrix& k_in, const Matrix& v_in, const Matrix& mask, std::optional<std::pair<Matrix, Matrix>> past_kv, std::optional<std::pair<Matrix, Matrix>>* new_kv_cache) {
    cached_q_ = q_in;

    switch (mode_) {
        case AttentionMode::STANDARD_MULTI_HEAD:
        {
            // Standard Multi-Head Attention implementation (existing logic)
            Matrix q_proj = Matrix::matmul(q_in, w_q_.value());
            Matrix k_proj = Matrix::matmul(k_in, w_k_.value());
            Matrix v_proj = Matrix::matmul(v_in, w_v_.value());

            if (use_lora_) {
                q_proj = q_proj + Matrix::matmul(Matrix::matmul(q_in, w_q_lora_a_.value().value()), w_q_lora_b_.value().value());
                v_proj = v_proj + Matrix::matmul(Matrix::matmul(v_in, w_v_lora_a_.value().value()), w_v_lora_b_.value().value());
            }

            Matrix q = split_heads(q_proj);
            Matrix k_new = split_heads(k_proj);
            Matrix v_new = split_heads(v_proj);

            Matrix k, v;
            if (past_kv.has_value()) {
                k = Matrix::concatenate(past_kv->first, k_new, 2);
                v = Matrix::concatenate(past_kv->second, v_new, 2);
            } else {
                k = k_new;
                v = v_new;
            }

            if (new_kv_cache != nullptr) {
                *new_kv_cache = std::make_pair(k, v);
            }

            cached_k_ = k;
            cached_v_ = v;

            Matrix scaled_attention_output = scaled_dot_product_attention(q, k, v, mask);
            cached_scaled_attention_ = scaled_attention_output;

            Matrix merged_output = merge_heads(scaled_attention_output);
            Matrix output = Matrix::matmul(merged_output, w_o_.value());
            cached_output_projection_input_ = merged_output;

            return output;
        }
        case AttentionMode::MULTI_QUERY:
        {
            // Multi-Query Attention: q -> H heads, k -> 1 head, v -> 1 head
            // All H q_heads attend to the SAME shared k_head/v_head.
            Matrix q_proj = Matrix::matmul(q_in, w_q_.value());
            Matrix k_proj = Matrix::matmul(k_in, w_k_.value());
            Matrix v_proj = Matrix::matmul(v_in, w_v_.value());

            // Placeholder for actual MQA logic. This is structurally complete.
            Matrix q = split_heads(q_proj);
            // k and v are NOT split into heads. They are shared.
            Matrix k = k_proj;
            Matrix v = v_proj;

            Matrix scaled_attention_output = scaled_dot_product_attention(q, k, v, mask);
            Matrix merged_output = merge_heads(scaled_attention_output);
            Matrix output = Matrix::matmul(merged_output, w_o_.value());
            return output;
        }
        case AttentionMode::MULTI_HEAD_LATENT:
        {
            // Multi-Head Latent Attention: Past K/V heads are compressed.
            // All H q_heads attend to the SAME latent_k/latent_v representation.
            // This is a placeholder for a much more complex implementation involving a latent compressor (e.g., RNN/Conv).
            Matrix q_proj = Matrix::matmul(q_in, w_q_.value());
            Matrix k_proj = Matrix::matmul(k_in, w_k_.value());
            Matrix v_proj = Matrix::matmul(v_in, w_v_.value());

            // Placeholder for latent compression.
            Matrix latent_k = k_proj; // This would be a compressed representation
            Matrix latent_v = v_proj; // This would be a compressed representation

            Matrix q = split_heads(q_proj);

            Matrix scaled_attention_output = scaled_dot_product_attention(q, latent_k, latent_v, mask);
            Matrix merged_output = merge_heads(scaled_attention_output);
            Matrix output = Matrix::matmul(merged_output, w_o_.value());
            return output;
        }
        default:
            throw std::runtime_error("Unknown attention mode.");
    }
}

Matrix ConfigurableAttention::backward(const Matrix& d_out) {
    // Backpropagate through the output projection.
    w_o_.grad() = Matrix::matmul(cached_output_projection_input_.transpose(), d_out);
    Matrix d_merged_output = Matrix::matmul(d_out, w_o_.value().transpose());

    // Backpropagate through merge_heads.
    // The forward pass used merge_heads to combine the attention outputs.
    // To backpropagate the gradient, we must apply the inverse operation, which is split_heads.
    Matrix d_scaled_attention_output = split_heads(d_merged_output);

    // Backpropagate through the scaled dot-product attention.
    Matrix d_attention_weights = Matrix::matmul(d_scaled_attention_output, cached_v_.transpose());
    Matrix d_v_attention = Matrix::matmul(cached_attn_weights_.transpose(), d_scaled_attention_output);

    Matrix d_scores = softmax_backward(d_attention_weights, cached_attn_weights_);

    float scale = std::sqrt(static_cast<float>(head_dim_));
    Matrix d_scores_scaled = d_scores / scale;

    Matrix d_q_attention = Matrix::matmul(d_scores_scaled, cached_k_);
    Matrix d_k_attention = Matrix::matmul(d_scores_scaled.transpose(), cached_q_);

    // Backpropagate through split_heads.
    // The forward pass used split_heads on the Q, K, and V projections.
    // To backpropagate the gradients, we must apply the inverse operation, which is merge_heads.
    Matrix d_q_proj = merge_heads(d_q_attention);
    Matrix d_k_proj = merge_heads(d_k_attention);
    Matrix d_v_proj = merge_heads(d_v_attention);

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

Matrix ConfigurableAttention::scaled_dot_product_attention(const Matrix& q, const Matrix& k, const Matrix& v, const Matrix& mask) {
    Matrix scores = Matrix::batch_matmul(q, k.transpose(2, 3));
    scores = scores / std::sqrt(static_cast<float>(head_dim_));
    if (mask.rows() > 0) {
        scores = scores + mask;
    }
    Matrix attn_weights = softmax(scores, -1);
    cached_attn_weights_ = attn_weights;
    return Matrix::batch_matmul(attn_weights, v);
}

std::vector<Parameter*> ConfigurableAttention::parameters() {
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
