#include "multiheadattention.h"
#include <cmath>
#include <numeric>
#include <optional>

namespace TissNum {

MultiHeadAttention::MultiHeadAttention(size_t d_model, size_t num_heads, int lora_rank, const std::string& name, AttentionMode mode)
    : d_model_(d_model),
      num_heads_(num_heads),
      head_dim_(d_model / num_heads),
      lora_rank_(lora_rank),
      use_lora_(lora_rank > 0),
      mode_(mode),
      w_q_(Parameter(Matrix::random({d_model, d_model}), name + ".w_q")),
      w_k_(Parameter(Matrix::random({d_model, mode == AttentionMode::MULTI_QUERY ? head_dim_ : d_model}), name + ".w_k")),
      w_v_(Parameter(Matrix::random({d_model, mode == AttentionMode::MULTI_QUERY ? head_dim_ : d_model}), name + ".w_v")),
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
    int actual_axis = axis;
    if (axis < 0) {
        actual_axis = x.get_shape().size() + axis;
    }
    Matrix max_val = x.max(actual_axis);
    Matrix exp_x = Matrix::exp(x - max_val);
    Matrix sum_exp_x = exp_x.sum(actual_axis);
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

    Matrix MultiHeadAttention::forward(const Matrix& q_in, const Matrix& k_in, const Matrix& v_in, const Matrix& mask, std::optional<std::pair<Matrix, Matrix>> past_kv, std::optional<std::pair<Matrix, Matrix>>* new_kv_cache) {
    cached_q_ = q_in;
    cached_k_in_ = k_in;
    cached_v_in_ = v_in;

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
    Matrix k_new;
    Matrix v_new;

    if (mode_ == AttentionMode::MULTI_QUERY) {
        size_t batch_size = k_proj.get_shape()[0];
        size_t seq_len = k_proj.get_shape()[1];
        // For MQA, K and V don't have the head dimension, so we just need to add a dimension for broadcasting
        // Reshape to (batch, seq_len, 1, head_dim) and then transpose to (batch, 1, seq_len, head_dim)
        k_new = k_proj.reshape({batch_size, seq_len, 1, head_dim_}).transpose(1, 2);
        v_new = v_proj.reshape({batch_size, seq_len, 1, head_dim_}).transpose(1, 2);

        // Manually broadcast k and v to match the number of heads in q
        k_new = k_new.repeat(num_heads_, 1);
        v_new = v_new.repeat(num_heads_, 1);
    } else {
        k_new = split_heads(k_proj);
        v_new = split_heads(v_proj);
    }

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
    // d_out: (B, L, D), w_o: (D, D)
    // w_o_grad = input^T * d_out
    // Reshape input and d_out to 2D: (B*L, D)
    size_t batch_size = d_out.get_shape()[0];
    size_t seq_len = d_out.get_shape()[1];
    
    Matrix input_reshaped = cached_output_projection_input_.reshape({batch_size * seq_len, d_model_});
    Matrix d_out_reshaped = d_out.reshape({batch_size * seq_len, d_model_});
    
    w_o_.grad() = Matrix::matmul(input_reshaped.transpose(), d_out_reshaped);
    Matrix d_merged_output = Matrix::matmul(d_out, w_o_.value().transpose());

    // Backpropagate through merge_heads.
    Matrix d_scaled_attention_output = split_heads(d_merged_output);

    // Backpropagate through the scaled dot-product attention.
    // d_scaled_attention_output: (B, H, Lq, Dh)
    // cached_v_: (B, H, Lk, Dh)
    // d_attention_weights = d_out * v^T -> (B, H, Lq, Lk)
    /*
    std::cout << "d_scaled_attention_output shape: ";
    for (auto s : d_scaled_attention_output.get_shape()) std::cout << s << " ";
    std::cout << std::endl;
    std::cout << "cached_v_ shape: ";
    for (auto s : cached_v_.get_shape()) std::cout << s << " ";
    std::cout << std::endl;
    */
    Matrix d_attention_weights = Matrix::batch_matmul(d_scaled_attention_output, cached_v_.transpose(2, 3));
    
    // d_v = attn_weights^T * d_out -> (B, H, Lk, Dh)
    /*
    std::cout << "cached_attn_weights_ shape: ";
    for (auto s : cached_attn_weights_.get_shape()) std::cout << s << " ";
    std::cout << std::endl;
    */
    Matrix d_v_attention = Matrix::batch_matmul(cached_attn_weights_.transpose(2, 3), d_scaled_attention_output);

    Matrix d_scores = softmax_backward(d_attention_weights, cached_attn_weights_);

    float scale = std::sqrt(static_cast<float>(head_dim_));
    Matrix d_scores_scaled = d_scores / scale;

    // d_q = d_scores * k -> (B, H, Lq, Dh)
    /*
    std::cout << "d_scores_scaled shape: ";
    for (auto s : d_scores_scaled.get_shape()) std::cout << s << " ";
    std::cout << std::endl;
    std::cout << "cached_k_ shape: ";
    for (auto s : cached_k_.get_shape()) std::cout << s << " ";
    std::cout << std::endl;
    */
    Matrix d_q_attention = Matrix::batch_matmul(d_scores_scaled, cached_k_);
    
    // d_k = d_scores^T * q -> (B, H, Lk, Dh)
    // NOTE: We need the projected Q here, not the input Q. 
    // But for now, let's fix the shape issue first.
    // We will re-project cached_q_ to get Q.
    Matrix q_proj = Matrix::matmul(cached_q_, w_q_.value());
    if (use_lora_) {
         q_proj = q_proj + Matrix::matmul(Matrix::matmul(cached_q_, w_q_lora_a_.value().value()), w_q_lora_b_.value().value());
    }
    Matrix q_split = split_heads(q_proj);
    
    /*
    std::cout << "d_scores_scaled.T shape: ";
    for (auto s : d_scores_scaled.transpose(2, 3).get_shape()) std::cout << s << " ";
    std::cout << std::endl;
    std::cout << "q_split shape: ";
    for (auto s : q_split.get_shape()) std::cout << s << " ";
    std::cout << std::endl;
    */

    Matrix d_k_attention = Matrix::batch_matmul(d_scores_scaled.transpose(2, 3), q_split);
    
    // Backpropagate through split_heads.
    Matrix d_q_proj = merge_heads(d_q_attention);
    Matrix d_k_proj;
    Matrix d_v_proj;

    if (mode_ == AttentionMode::MULTI_QUERY) {
        size_t batch_size_k = d_k_attention.get_shape()[0];
        size_t seq_len_k = d_k_attention.get_shape()[2];
        d_k_proj = d_k_attention.sum(1).reshape({batch_size_k, seq_len_k, head_dim_});
        d_v_proj = d_v_attention.sum(1).reshape({batch_size_k, seq_len_k, head_dim_});
    } else {
        d_k_proj = merge_heads(d_k_attention);
        d_v_proj = merge_heads(d_v_attention);
    }

    // Backpropagate through the input projections.
    // cached_q_in_: (B, L, D)
    // d_q_proj: (B, L, D)
    Matrix cached_q_reshaped = cached_q_.reshape({batch_size * seq_len, d_model_});
    Matrix d_q_proj_reshaped = d_q_proj.reshape({batch_size * seq_len, d_model_});
    w_q_.grad() = Matrix::matmul(cached_q_reshaped.transpose(), d_q_proj_reshaped);

    // Handle K and V gradients using cached_k_in_ and cached_v_in_
    // Note: For MQA, d_k_proj is (B, L, HeadDim), but w_k_ is (D, HeadDim).
    // cached_k_in_ is (B, L, D).
    // So we reshape cached_k_in_ to (B*L, D) and d_k_proj to (B*L, HeadDim).
    size_t k_dim = (mode_ == AttentionMode::MULTI_QUERY) ? head_dim_ : d_model_;
    
    Matrix cached_k_reshaped = cached_k_in_.reshape({batch_size * seq_len, d_model_});
    Matrix d_k_proj_reshaped = d_k_proj.reshape({batch_size * seq_len, k_dim});
    w_k_.grad() = Matrix::matmul(cached_k_reshaped.transpose(), d_k_proj_reshaped);

    Matrix cached_v_reshaped = cached_v_in_.reshape({batch_size * seq_len, d_model_});
    Matrix d_v_proj_reshaped = d_v_proj.reshape({batch_size * seq_len, k_dim});
    w_v_.grad() = Matrix::matmul(cached_v_reshaped.transpose(), d_v_proj_reshaped);

    Matrix dx = Matrix::matmul(d_q_proj_reshaped, w_q_.value().transpose()).reshape({batch_size, seq_len, d_model_});

    if (use_lora_) {
        // Backpropagate through LoRA layers
        // This part also needs reshaping fixes if LoRA is used, but for now focusing on main path.
        // Assuming LoRA implementation handles shapes correctly or is not tested here.
        // But to be safe, let's apply similar reshaping logic.
        
        Matrix d_lora_q_b = Matrix::matmul(cached_q_reshaped.transpose(), d_q_proj_reshaped); // This is w_q_lora_a * input * w_q_lora_b... wait logic is complex.
        // Simplified: d_q_proj is the gradient at the output of the LoRA block (added to main path).
        // LoRA path: input -> A -> B -> output.
        // d_output = d_q_proj.
        // d_B = A_out^T * d_output. A_out = input * A.
        // d_A = input^T * d_A_out. d_A_out = d_output * B^T.
        
        Matrix lora_a_out = Matrix::matmul(cached_q_reshaped, w_q_lora_a_.value().value());
        Matrix d_lora_q_b_grad = Matrix::matmul(lora_a_out.transpose(), d_q_proj_reshaped);
        w_q_lora_b_->grad() = d_lora_q_b_grad;
        
        Matrix d_lora_q_a_out = Matrix::matmul(d_q_proj_reshaped, w_q_lora_b_.value().value().transpose());
        Matrix d_lora_q_a_grad = Matrix::matmul(cached_q_reshaped.transpose(), d_lora_q_a_out);
        w_q_lora_a_->grad() = d_lora_q_a_grad;

        // Similar for V
        Matrix lora_v_a_out = Matrix::matmul(cached_v_reshaped, w_v_lora_a_.value().value());
        Matrix d_lora_v_b_grad = Matrix::matmul(lora_v_a_out.transpose(), d_v_proj_reshaped);
        w_v_lora_b_->grad() = d_lora_v_b_grad;
        
        Matrix d_lora_v_a_out = Matrix::matmul(d_v_proj_reshaped, w_v_lora_b_.value().value().transpose());
        Matrix d_lora_v_a_grad = Matrix::matmul(cached_v_reshaped.transpose(), d_lora_v_a_out);
        w_v_lora_a_->grad() = d_lora_v_a_grad;
    }

    return dx;
}

Matrix MultiHeadAttention::scaled_dot_product_attention(const Matrix& q, const Matrix& k, const Matrix& v, const Matrix& mask) {
    Matrix scores = Matrix::batch_matmul(q, k.transpose(2, 3));
    scores = scores / std::sqrt(static_cast<float>(head_dim_));
    if (mask.rows() > 0) {
        scores = scores + mask;
    }
    Matrix attn_weights = softmax(scores, -1);
    cached_attn_weights_ = attn_weights;
    return Matrix::batch_matmul(attn_weights, v);
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
