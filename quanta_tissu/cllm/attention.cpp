#include "attention.h"
#include <cmath>
#include <stdexcept>

namespace cllm {

MultiHeadAttention::MultiHeadAttention(const ModelConfig& config) : config_(config) {
    // Ensure d_model is divisible by n_heads
    if (config.d_model % config.n_heads != 0) {
        throw std::runtime_error("d_model must be divisible by n_heads");
    }

    // Initialize weight matrices.
    // In a real implementation, these would be loaded from a checkpoint.
    Wq_ = Eigen::MatrixXf::Random(config.d_model, config.d_model);
    Wk_ = Eigen::MatrixXf::Random(config.d_model, config.d_model);
    Wv_ = Eigen::MatrixXf::Random(config.d_model, config.d_model);
    Wo_ = Eigen::MatrixXf::Random(config.d_model, config.d_model);
}

Eigen::MatrixXf MultiHeadAttention::scaled_dot_product_attention(const Eigen::MatrixXf& q, const Eigen::MatrixXf& k, const Eigen::MatrixXf& v) {
    int d_k = k.cols();
    Eigen::MatrixXf scores = q * k.transpose();
    scores /= std::sqrt(d_k);

    // Softmax (row-wise)
    Eigen::MatrixXf attn_weights = scores.array().exp().matrix();
    attn_weights = attn_weights.array().rowwise() / attn_weights.rowwise().sum().array();

    return attn_weights * v;
}

Eigen::MatrixXf MultiHeadAttention::forward(const Eigen::MatrixXf& input) {
    int seq_len = input.rows();

    // 1. Project Q, K, V
    Eigen::MatrixXf q_proj = input * Wq_;
    Eigen::MatrixXf k_proj = input * Wk_;
    Eigen::MatrixXf v_proj = input * Wv_;

    // 2. Split into multiple heads
    int d_k = config_.d_model / config_.n_heads;

    Eigen::MatrixXf concatenated_output(seq_len, config_.d_model);

    for (int i = 0; i < config_.n_heads; ++i) {
        Eigen::MatrixXf q_head = q_proj.block(0, i * d_k, seq_len, d_k);
        Eigen::MatrixXf k_head = k_proj.block(0, i * d_k, seq_len, d_k);
        Eigen::MatrixXf v_head = v_proj.block(0, i * d_k, seq_len, d_k);

        Eigen::MatrixXf head_output = scaled_dot_product_attention(q_head, k_head, v_head);
        concatenated_output.block(0, i * d_k, seq_len, d_k) = head_output;
    }

    // 3. Concatenate heads and apply final projection
    Eigen::MatrixXf output = concatenated_output * Wo_;

    return output;
}

} // namespace cllm
