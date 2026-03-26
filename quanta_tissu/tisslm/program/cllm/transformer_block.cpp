#include "transformer_block.h"

namespace cllm {

TransformerBlock::TransformerBlock(const ModelConfig& config) : config_(config) {
    attention_ = std::make_unique<MultiHeadAttention>(config);
    ffn_ = std::make_unique<FeedForward>(config);

    ln1_gamma_ = Eigen::VectorXf::Ones(config.d_model);
    ln1_beta_ = Eigen::VectorXf::Zero(config.d_model);
    ln2_gamma_ = Eigen::VectorXf::Ones(config.d_model);
    ln2_beta_ = Eigen::VectorXf::Zero(config.d_model);
}

Eigen::MatrixXf TransformerBlock::layer_norm(const Eigen::MatrixXf& input, const Eigen::VectorXf& gamma, const Eigen::VectorXf& beta) {
    Eigen::VectorXf mean = input.rowwise().mean();
    Eigen::MatrixXf centered = input.rowwise() - mean;
    Eigen::VectorXf std_dev = (centered.array().square().rowwise().sum() / input.cols() + 1e-5).sqrt();

    Eigen::MatrixXf normalized = centered.array().rowwise() / std_dev.array();

    return (normalized.array().rowwise() * gamma.transpose().array()).rowwise() + beta.transpose().array();
}


Eigen::MatrixXf TransformerBlock::forward(const Eigen::MatrixXf& input) {
    Eigen::MatrixXf attn_output = attention_->forward(input);
    Eigen::MatrixXf sublayer1_output = layer_norm(input + attn_output, ln1_gamma_, ln1_beta_);

    Eigen::MatrixXf ffn_output = ffn_->forward(sublayer1_output);
    Eigen::MatrixXf sublayer2_output = layer_norm(sublayer1_output + ffn_output, ln2_gamma_, ln2_beta_);

    return sublayer2_output;
}

Eigen::MatrixXf TransformerBlock::layer_norm_backward(const Eigen::MatrixXf& d_out, const Eigen::MatrixXf& input, const Eigen::VectorXf& gamma, const Eigen::VectorXf& beta, Eigen::VectorXf& d_gamma, Eigen::VectorXf& d_beta) {
    int N = input.rows(), D = input.cols();
    Eigen::VectorXf mean = input.rowwise().mean();
    Eigen::MatrixXf centered = input.rowwise() - mean;
    Eigen::VectorXf var = (centered.array().square().rowwise().sum() / D);
    Eigen::VectorXf std_inv = (var.array() + 1e-5).rsqrt();
    Eigen::MatrixXf x_hat = centered.array().rowwise() * std_inv.transpose().array();

    d_gamma = (d_out.array() * x_hat.array()).colwise().sum();
    d_beta = d_out.colwise().sum();

    Eigen::MatrixXf dx_hat = d_out.array().rowwise() * gamma.transpose().array();
    Eigen::VectorXf d_var = (dx_hat.array() * centered.array()).rowwise().sum() * -0.5f * (var.array() + 1e-5).pow(-1.5f);
    Eigen::VectorXf d_mean = (dx_hat.array().rowwise() * -std_inv.array()).rowwise().sum() + d_var.array() * (-2.0f * centered.array().rowwise().sum() / D);

    return dx_hat.array().rowwise() * std_inv.transpose().array() + centered.array().rowwise() * (2.0f * d_var / D).transpose().array() + d_mean.replicate(1, D).array() / D;
}

Eigen::MatrixXf TransformerBlock::backward(const Eigen::MatrixXf& d_out, const Eigen::MatrixXf& input, float lr) {
    Eigen::MatrixXf attn_output = attention_->forward(input);
    Eigen::MatrixXf sub1 = input + attn_output;
    Eigen::MatrixXf ffn_input = layer_norm(sub1, ln1_gamma_, ln1_beta_);
    Eigen::MatrixXf ffn_output = ffn_->forward(ffn_input);
    Eigen::MatrixXf sub2 = ffn_input + ffn_output;

    Eigen::VectorXf d_g2, d_b2;
    Eigen::MatrixXf d_sub2 = layer_norm_backward(d_out, sub2, ln2_gamma_, ln2_beta_, d_g2, d_b2);
    ln2_gamma_ -= lr * d_g2; ln2_beta_ -= lr * d_b2;

    Eigen::MatrixXf d_ffn = ffn_->backward(d_sub2, ffn_input, lr);
    Eigen::MatrixXf d_sub1_norm = d_sub2 + d_ffn;

    Eigen::VectorXf d_g1, d_b1;
    Eigen::MatrixXf d_sub1 = layer_norm_backward(d_sub1_norm, sub1, ln1_gamma_, ln1_beta_, d_g1, d_b1);
    ln1_gamma_ -= lr * d_g1; ln1_beta_ -= lr * d_b1;

    Eigen::MatrixXf d_attn = attention_->backward(d_sub1, input, lr);
    return d_sub1 + d_attn;
}

}
