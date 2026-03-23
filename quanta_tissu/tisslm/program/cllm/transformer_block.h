#ifndef CLLM_TRANSFORMER_BLOCK_H
#define CLLM_TRANSFORMER_BLOCK_H

#include "config.h"
#include "attention.h"
#include "feed_forward.h"
#include <memory>
#include <Eigen/Dense>

namespace cllm {

/**
 * @brief Transformer Block.
 * Integrates multi-head attention and feed-forward networks with layer normalization.
 */
class TransformerBlock {
public:
    explicit TransformerBlock(const ModelConfig& config);

    /**
     * @brief Performs the forward pass for the transformer block.
     * @param input Input matrix of shape (seq_len, d_model)
     * @return Output matrix after processing
     */
    Eigen::MatrixXf forward(const Eigen::MatrixXf& input);

private:
    /**
     * @brief Layer Normalization implementation.
     */
    Eigen::MatrixXf layer_norm(const Eigen::MatrixXf& input, const Eigen::VectorXf& gamma, const Eigen::VectorXf& beta);

    ModelConfig config_;
    std::unique_ptr<MultiHeadAttention> attention_;
    std::unique_ptr<FeedForward> ffn_;

    Eigen::VectorXf ln1_gamma_;
    Eigen::VectorXf ln1_beta_;
    Eigen::VectorXf ln2_gamma_;
    Eigen::VectorXf ln2_beta_;
};

} // namespace cllm

#endif // CLLM_TRANSFORMER_BLOCK_H
