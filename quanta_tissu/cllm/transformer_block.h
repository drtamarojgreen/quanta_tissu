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

    // Performs the forward pass for the transformer block.
    Eigen::MatrixXf forward(const Eigen::MatrixXf& input);

private:
    // Placeholder for Layer Normalization
    Eigen::MatrixXf layer_norm(const Eigen::MatrixXf& input);

    ModelConfig config_;
    std::unique_ptr<MultiHeadAttention> attention_;
    std::unique_ptr<FeedForward> ffn_;

    // Parameters for layer normalization would be here (gamma, beta)
    Eigen::VectorXf ln1_gamma_;
    Eigen::VectorXf ln1_beta_;
    Eigen::VectorXf ln2_gamma_;
    Eigen::VectorXf ln2_beta_;
};

} // namespace cllm

#endif // CLLM_TRANSFORMER_BLOCK_H
