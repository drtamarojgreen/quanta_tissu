#ifndef CLLM_TRANSFORMER_BLOCK_H
#define CLLM_TRANSFORMER_BLOCK_H

#include "config.h"
#include "attention.h"
#include "feed_forward.h"
#include <memory>
#include <Eigen/Dense>

namespace cllm {

class TransformerBlock {
public:
    explicit TransformerBlock(const ModelConfig& config);
    Eigen::MatrixXf forward(const Eigen::MatrixXf& input);

    MultiHeadAttention& attention() { return *attention_; }
    FeedForward& ffn() { return *ffn_; }
    Eigen::VectorXf& ln1_gamma() { return ln1_gamma_; }
    Eigen::VectorXf& ln1_beta() { return ln1_beta_; }
    Eigen::VectorXf& ln2_gamma() { return ln2_gamma_; }
    Eigen::VectorXf& ln2_beta() { return ln2_beta_; }

private:
    Eigen::MatrixXf layer_norm(const Eigen::MatrixXf& input, const Eigen::VectorXf& gamma, const Eigen::VectorXf& beta);
    ModelConfig config_;
    std::unique_ptr<MultiHeadAttention> attention_;
    std::unique_ptr<FeedForward> ffn_;
    Eigen::VectorXf ln1_gamma_, ln1_beta_, ln2_gamma_, ln2_beta_;
};

}

#endif // CLLM_TRANSFORMER_BLOCK_H
