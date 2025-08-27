#include "transformer_block.h"

namespace cllm {

TransformerBlock::TransformerBlock(const ModelConfig& config) : config_(config) {
    attention_ = std::make_unique<MultiHeadAttention>(config);
    ffn_ = std::make_unique<FeedForward>(config);

    // Initialize layer norm parameters
    ln1_gamma_ = Eigen::VectorXf::Ones(config.d_model);
    ln1_beta_ = Eigen::VectorXf::Zero(config.d_model);
    ln2_gamma_ = Eigen::VectorXf::Ones(config.d_model);
    ln2_beta_ = Eigen::VectorXf::Zero(config.d_model);
}

// Placeholder for Layer Normalization
Eigen::MatrixXf TransformerBlock::layer_norm(const Eigen::MatrixXf& input) {
    // This is a simplified implementation. A real one would be more complex.
    Eigen::VectorXf mean = input.rowwise().mean();
    Eigen::MatrixXf centered = input.rowwise() - mean;
    Eigen::VectorXf std_dev = centered.array().square().rowwise().sum().sqrt() / std::sqrt(input.cols());

    Eigen::MatrixXf normalized = centered.array().rowwise() / std_dev.array();

    // Apply scale and shift
    return (normalized.array().rowwise() * ln1_gamma_.transpose().array()).rowwise() + ln1_beta_.transpose().array();
}


Eigen::MatrixXf TransformerBlock::forward(const Eigen::MatrixXf& input) {
    // 1. Multi-Head Attention with residual connection and layer norm
    Eigen::MatrixXf attn_output = attention_->forward(input);
    Eigen::MatrixXf sublayer1_output = layer_norm(input + attn_output);

    // 2. Feed-Forward Network with residual connection and layer norm
    Eigen::MatrixXf ffn_output = ffn_->forward(sublayer1_output);
    Eigen::MatrixXf sublayer2_output = layer_norm(sublayer1_output + ffn_output);

    return sublayer2_output;
}

} // namespace cllm
