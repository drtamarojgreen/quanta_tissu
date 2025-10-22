#include "multiheadattention.h"
#include <cmath>
#include <numeric>
#include <optional>

namespace TissNum {

// Placeholder for Softmax activation
Matrix softmax(const Matrix& x) {
    Matrix result(x.rows(), x.cols());
    for (size_t i = 0; i < x.rows(); ++i) {
        float max_val = -std::numeric_limits<float>::infinity();
        for (size_t j = 0; j < x.cols(); ++j) {
            if (x(i, j) > max_val) {
                max_val = x(i, j);
            }
        }

        float sum_exp = 0.0f;
        for (size_t j = 0; j < x.cols(); ++j) {
            result(i, j) = std::exp(x(i, j) - max_val);
            sum_exp += result(i, j);
        }

        for (size_t j = 0; j < x.cols(); ++j) {
            result(i, j) /= sum_exp;
        }
    }
    return result;
}

MultiHeadAttention::MultiHeadAttention(size_t d_model, size_t num_heads, int lora_rank, const std::string& name)
    : d_model_(d_model),
      num_heads_(num_heads),
      head_dim_(d_model / num_heads),
      lora_rank_(lora_rank),
      use_lora_(lora_rank > 0),
      w_q_(Parameter(Matrix::random(d_model, d_model), name + ".wq")),
      w_k_(Parameter(Matrix::random(d_model, d_model), name + ".wk")),
      w_v_(Parameter(Matrix::random(d_model, d_model), name + ".wv")),
      w_o_(Parameter(Matrix::random(d_model, d_model), name + ".wo")) {
    if (d_model % num_heads != 0) {
        throw std::invalid_argument("d_model must be divisible by num_heads");
    }
    if (use_lora_) {
        w_q_lora_a_ = std::make_optional(Parameter(Matrix::random(d_model, lora_rank), name + ".wq.lora_a"));
        w_q_lora_b_ = std::make_optional(Parameter(Matrix::zeros(lora_rank, d_model), name + ".wq.lora_b"));
        w_v_lora_a_ = std::make_optional(Parameter(Matrix::random(d_model, lora_rank), name + ".wv.lora_a"));
        w_v_lora_b_ = std::make_optional(Parameter(Matrix::zeros(lora_rank, d_model), name + ".wv.lora_b"));
    }
}

Matrix MultiHeadAttention::scaled_dot_product_attention(const Matrix& q, const Matrix& k, const Matrix& v, const Matrix& mask) {
    // q, k, v are (batch_size * num_heads, seq_len, head_dim)
    // For simplicity, assume q, k, v are already reshaped for a single head
    // and batch_size * num_heads is handled implicitly by matrix operations

    // (seq_len, head_dim) @ (head_dim, seq_len) -> (seq_len, seq_len)
    Matrix scores = Matrix::matmul(q, k.transpose());
    scores = scores / std::sqrt(static_cast<float>(head_dim_));

    if (mask.rows() > 0) {
        // Apply mask (assuming mask is broadcastable)
        // This is a simplified application of mask
        scores = scores + mask;
    }

    Matrix attention_weights = softmax(scores);
    cached_attn_weights_ = attention_weights;

    Matrix output = Matrix::matmul(attention_weights, v);
    return output;
}

// Helper function to split a matrix for multi-head attention
// Input: (seq_len, d_model), Output: std::vector of (seq_len, head_dim) matrices
std::vector<Matrix> split_heads(const Matrix& m, size_t num_heads, size_t head_dim) {
    std::vector<Matrix> heads;
    size_t seq_len = m.rows();
    heads.reserve(num_heads);
    for (size_t i = 0; i < num_heads; ++i) {
        Matrix head(seq_len, head_dim);
        for (size_t r = 0; r < seq_len; ++r) {
            for (size_t c = 0; c < head_dim; ++c) {
                head(r, c) = m(r, i * head_dim + c);
            }
        }
        heads.push_back(head);
    }
    return heads;
}

// Helper function to combine heads back
// Input: std::vector of (seq_len, head_dim) matrices, Output: (seq_len, d_model)
Matrix combine_heads(const std::vector<Matrix>& heads) {
    if (heads.empty()) {
        return Matrix();
    }
    size_t seq_len = heads[0].rows();
    size_t head_dim = heads[0].cols();
    size_t num_heads = heads.size();
    size_t d_model = num_heads * head_dim;

    Matrix result(seq_len, d_model);
    for (size_t i = 0; i < num_heads; ++i) {
        for (size_t r = 0; r < seq_len; ++r) {
            for (size_t c = 0; c < head_dim; ++c) {
                result(r, i * head_dim + c) = heads[i](r, c);
            }
        }
    }
    return result;
}


Matrix MultiHeadAttention::forward(const Matrix& q_in, const Matrix& k_in, const Matrix& v_in, const Matrix& mask, std::optional<std::pair<Matrix, Matrix>> past_kv, std::optional<std::pair<Matrix, Matrix>>* new_kv_cache) {
    cached_q_ = q_in;

    Matrix current_k = k_in;
    Matrix current_v = v_in;

    if (past_kv.has_value()) {
        current_k = Matrix::concatenate(past_kv->first, current_k, 0);
        current_v = Matrix::concatenate(past_kv->second, current_v, 0);
    }

    cached_k_ = current_k;
    cached_v_ = current_v;

    // Linear projections
    Matrix q = Matrix::matmul(q_in, w_q_.value());
    Matrix k = Matrix::matmul(current_k, w_k_.value());
    Matrix v = Matrix::matmul(current_v, w_v_.value());

    if (use_lora_) {
        q = q + Matrix::matmul(Matrix::matmul(q_in, w_q_lora_a_.value().value()), w_q_lora_b_.value().value());
        v = v + Matrix::matmul(Matrix::matmul(current_v, w_v_lora_a_.value().value()), w_v_lora_b_.value().value());
    }

    // Split heads
    auto q_heads = split_heads(q, num_heads_, head_dim_);
    auto k_heads = split_heads(k, num_heads_, head_dim_);
    auto v_heads = split_heads(v, num_heads_, head_dim_);

    std::vector<Matrix> attention_outputs;
    attention_outputs.reserve(num_heads_);
    for (size_t i = 0; i < num_heads_; ++i) {
        Matrix attention_output = scaled_dot_product_attention(q_heads[i], k_heads[i], v_heads[i], mask);
        attention_outputs.push_back(attention_output);
    }

    // Combine heads
    Matrix scaled_attention_output = combine_heads(attention_outputs);
    cached_scaled_attention_ = scaled_attention_output;

    // Final linear layer
    Matrix output = Matrix::matmul(scaled_attention_output, w_o_.value());
    cached_output_projection_input_ = scaled_attention_output;

    if (new_kv_cache != nullptr) {
        *new_kv_cache = std::make_pair(current_k, current_v);
    }

    return output;
}

Matrix MultiHeadAttention::backward(const Matrix& d_out, const Matrix& cache) {
    // This is a highly simplified backward pass for MultiHeadAttention.
    // A full implementation would would involve backpropagating through all the reshapes, splits, and concatenations.
    // For now, this is a placeholder.

    // Backpropagate through output projection
    w_o_.grad() = Matrix::matmul(cached_output_projection_input_.transpose(), d_out);
    Matrix d_scaled_attention_output = Matrix::matmul(d_out, w_o_.value().transpose());

    // Backpropagate through scaled_dot_product_attention (simplified)
    // This part is very complex and requires gradients for q, k, v, and attention weights
    // For now, we'll just return d_scaled_attention_output as a placeholder for dx

    // Placeholder for gradients of W_q, W_k, W_v
    w_q_.grad() = Matrix::zeros(d_model_, d_model_);
    w_k_.grad() = Matrix::zeros(d_model_, d_model_);
    w_v_.grad() = Matrix::zeros(d_model_, d_model_);

    if (use_lora_) {
        // Placeholders for LoRA gradients
        if (w_q_lora_a_.has_value()) w_q_lora_a_->grad() = Matrix::zeros(d_model_, lora_rank_);
        if (w_q_lora_b_.has_value()) w_q_lora_b_->grad() = Matrix::zeros(lora_rank_, d_model_);
        if (w_v_lora_a_.has_value()) w_v_lora_a_->grad() = Matrix::zeros(d_model_, lora_rank_);
        if (w_v_lora_b_.has_value()) w_v_lora_b_->grad() = Matrix::zeros(lora_rank_, d_model_);
    }

    return d_scaled_attention_output; // Placeholder for dx
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
