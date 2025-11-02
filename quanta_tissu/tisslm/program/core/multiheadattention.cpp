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

Matrix MultiHeadAttention::forward(const Matrix& q_in, const Matrix& k_in, const Matrix& v_in, const Matrix& mask, std::optional<std::pair<Matrix, Matrix>> past_kv, std::optional<std::pair<Matrix, Matrix>>* new_kv_cache) {
    cached_q_ = q_in;

    Matrix current_k = k_in;
    Matrix current_v = v_in;

    if (past_kv.has_value()) {
        // Concatenate past keys and values with current ones
        current_k = Matrix::concatenate(past_kv->first, current_k, 0); // Concatenate along sequence length dimension
        current_v = Matrix::concatenate(past_kv->second, current_v, 0); // Concatenate along sequence length dimension
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

    // Reshape for multi-head attention (simplified - actual reshape is more complex)
    // For now, assume q, k, v are already in the correct shape for scaled_dot_product_attention

    Matrix scaled_attention_output = scaled_dot_product_attention(q, k, v, mask);
    cached_scaled_attention_ = scaled_attention_output;

    // Concatenate and final linear layer
    // (simplified - actual concatenation is more complex)
    Matrix output = Matrix::matmul(scaled_attention_output, w_o_.value());
    cached_output_projection_input_ = scaled_attention_output;

    if (new_kv_cache != nullptr) {
        *new_kv_cache = std::make_pair(current_k, current_v);
    }

    return output;
}

Matrix MultiHeadAttention::backward(const Matrix& d_out) {
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
